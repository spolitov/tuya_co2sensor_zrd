#include "app_cfg.h"
#include "zb_common.h"
#include "zcl_include.h"
#include "gp.h"

#include "app_endpoint_cfg.h"
#include "app_utility.h"
#include "app_zcl.h"

#define PIN_LED GPIO_PB4
#define PIN_BUTTON GPIO_PA0
#define PIN_DHT GPIO_PC2

extern int join_in_progress();
extern void app_init_zb();
extern void app_init_bdb();

static unsigned long led_switch_time = 0;
static int led_state = 0;
static ev_timer_event_t* factory_reset_timer = NULL;

static u8 uart_data[0x200];
static u32 last_calibration_request_time = 0;

static reportCfgInfo_t* co2_report_cfg = NULL;
static reportCfgInfo_t* temp_report_cfg = NULL;
static reportCfgInfo_t* hum_report_cfg = NULL;
static publish_info_t co2_last_calibration_publish_info;
static unsigned long co2_measurement_time = 0;
static unsigned long dht_measurement_time = 0;

static int factory_reset_trigger(void* arg) {
  zb_resetDevice2FN();
  factory_reset_timer = NULL;
  return -1;
}

static void button_press() {
  if (drv_gpio_read(PIN_BUTTON)) {
    drv_gpio_irq_set(PIN_BUTTON, GPIO_FALLING_EDGE);

    TL_ZB_TIMER_CANCEL(&factory_reset_timer);
  } else {
    drv_gpio_irq_set(PIN_BUTTON, GPIO_RISING_EDGE);

    if (!factory_reset_timer) {
      factory_reset_timer = TL_ZB_TIMER_SCHEDULE(factory_reset_trigger, NULL, SEC_TO_MS(5));
    }
  }
}

static bool reset_timer_started() {
  return factory_reset_timer != NULL;
}

static void publish_co2_last_calibration(void* arg) {
  publish_attribute(co2_last_calibration_publish_info);
}

static void uart_recv_cb() {
  // The only full packets are handled.
  // It is not a problem since we don't have any important information that could be lost because of
  // ignored incomplete packed.
  u32 len;
  memcpy(&len, uart_data, sizeof(len));
  const u8* data = uart_data + sizeof(len);
  if (len == 4) {
    if (load_le16(data) == 0x0116 && load_le16(data + 2) == 0xe603) {
      if (last_calibration_request_time) {
        zcl_co2.last_calibration = last_calibration_request_time;
        tl_zbTaskPost(publish_co2_last_calibration, NULL);
      }
    }
    return;
  }
  if (len == 14) {
    u16 received_crc = load_le16(data + len - MODBUS_CRC_LEN);
    u16 expected_crc = modbus_crc(data, len - MODBUS_CRC_LEN);
    if (received_crc != expected_crc) {
      return;
    }

    u16 command = load_le16(data);
    if (command == 0x6964) {
      zcl_co2.measured_value = load_le16(data + 4);
    }
  }
}

static void init_zcl() {
  zcl_init(NULL);

  endpoint_info_t* endpoints = get_endpoints();
  for (endpoint_info_t* i = endpoints; i->id != 0; ++i) {
    af_endpointRegister(i->id, (af_simple_descriptor_t*)i->descriptor, zcl_rx_handler, NULL);
  }

  zcl_reportingTabInit();

  for (endpoint_info_t* i = endpoints; i->id != 0; ++i) {
    zcl_register(i->id, i->cluster_size, (zcl_specClusterInfo_t*)i->cluster);
  }

  co2_report_cfg = zcl_reportCfgInfoEntryFind(
      APP_ENDPOINT1, ZCL_CLUSTER_MS_CO2_MEASUREMENT, ZCL_ATTRID_CO2_MEASUREMENT_MEASUREDVALUE);

  co2_last_calibration_publish_info = obtain_publish_info(
      APP_ENDPOINT1, ZCL_CLUSTER_MS_CO2_MEASUREMENT, ZCL_ATTRID_CO2_MEASUREMENT_LAST_CALIBRATION);

  temp_report_cfg = zcl_reportCfgInfoEntryFind(
      APP_ENDPOINT1, ZCL_CLUSTER_MS_TEMPERATURE_MEASUREMENT, ZCL_ATTRID_TEMPERATURE_MEASUREMENT_MEASUREDVALUE);

  hum_report_cfg = zcl_reportCfgInfoEntryFind(
      APP_ENDPOINT1, ZCL_CLUSTER_MS_RELATIVE_HUMIDITY, ZCL_ATTRID_RELATIVE_HUMIDITY_MEASUREDVALUE);
}

static void init_drv() {
  drv_gpio_func_set(PIN_BUTTON);
  drv_gpio_output_en(PIN_BUTTON, 0);
  drv_gpio_input_en(PIN_BUTTON, 1);
  drv_gpio_up_down_resistor(PIN_BUTTON, PM_PIN_PULLUP_10K);

  drv_gpio_irq_config(GPIO_IRQ_MODE, PIN_BUTTON, GPIO_FALLING_EDGE, button_press);
  drv_gpio_irq_en(PIN_BUTTON);

  drv_gpio_func_set(PIN_DHT);
  drv_gpio_output_en(PIN_DHT, 0);
  drv_gpio_input_en(PIN_DHT, 0);

  drv_enable_irq();

  drv_uart_pin_set(GPIO_UART_TX, GPIO_UART_RX);
  memset(uart_data, 0, sizeof(uart_data));
  drv_uart_init(9600, uart_data, sizeof(uart_data), uart_recv_cb);
}

static int zigbee_bound() {
  return zb_isDeviceJoinedNwk();
}

static int handle_led_blink(u32 interval) {
  if (clock_time_exceed(led_switch_time, interval)) {
    return !led_state;
  }
  return led_state;
}

static int desired_led_state() {
  if (reset_timer_started()) {
    return 1;
  }
  if (zigbee_bound()) {
    if (zcl_identify.time > 0) {
      return handle_led_blink(MS_TO_US(500));
    }

    return 0;
  }
  return handle_led_blink((join_in_progress() || led_state) ? MS_TO_US(250) : MS_TO_US(2500));
}

static void update_led() {
  int desired_state = desired_led_state();
  if (desired_state != led_state) {
    led_state = desired_state;
    led_switch_time = clock_time();
    drv_gpio_write(PIN_LED, led_state);
  }
}

static void update_co2() {
  unsigned int period_sec = co2_report_cfg ? co2_report_cfg->minInterval : 10;
  if (period_sec < 2) {
    period_sec = 2;
  }
  if (!clock_time_exceed(co2_measurement_time, SEC_TO_US(period_sec ? period_sec : 1))) {
    return;
  }
  co2_measurement_time = clock_time();

  u8 packet[] = {0x64, 0x69, 0x03, 0x5E, 0x4E};
  if (drv_uart_tx_start(packet, ARRAY_SIZE(packet))) {
  }
}

static void dht22_read() {
  sleep_us(30);

  if (gpio_read(PIN_DHT)) {
    return;
  }

  u8 byte = 0;
  u8 bytes[5];
  u32 read_start = clock_time();
  u32 deadline_us = MS_TO_US(10);
  for (int i = 0; i != 41; ++i) {
    byte <<= 1;
    while (!gpio_read(PIN_DHT) && !clock_time_exceed(read_start, deadline_us));
    u32 this_start = clock_time();
    while (gpio_read(PIN_DHT) && !clock_time_exceed(read_start, deadline_us));
    if (i == 0) {
      continue;
    }
    if (clock_time_exceed(this_start, 50)) {
      byte |= 1;
    }
    if ((i & 7) == 0) {
      bytes[(i - 1) >> 3] = byte;
      byte = 0;
    }
  }

  if ((u8)(bytes[0] + bytes[1] + bytes[2] + bytes[3]) != bytes[4]) {
    return;
  }

  zcl_humidity.measured_value = load_be16(bytes) * 10;
  u16 temp_raw = load_be16(bytes + 2);
  if (temp_raw & 0x8000) {
    zcl_temperature.measured_value = -(s16)(temp_raw ^ 0x8000) * 10;
  } else {
    zcl_temperature.measured_value = temp_raw * 10;
  }
}

static int dht22_read_start(void* arg) {
  drv_gpio_output_en(PIN_DHT, 0);
  drv_gpio_input_en(PIN_DHT, 1);

  dht22_read();

  drv_gpio_input_en(PIN_DHT, 0);
  return -1;
}

void update_dht() {
  unsigned int period_sec = 0;
  if (temp_report_cfg && temp_report_cfg->minInterval) {
    period_sec = temp_report_cfg->minInterval;
  }
  if (hum_report_cfg && hum_report_cfg->minInterval) {
    if (!period_sec || hum_report_cfg->minInterval < period_sec) {
      period_sec = hum_report_cfg->minInterval;
    }
  }
  if (period_sec == 0) {
    period_sec = 10;
  } else if (period_sec < 2) {
    period_sec = 2;
  }
  if (!clock_time_exceed(dht_measurement_time, SEC_TO_US(period_sec))) {
    return;
  }
  dht_measurement_time = clock_time();

  drv_gpio_input_en(PIN_DHT, 0);
  drv_gpio_output_en(PIN_DHT, 1);
  drv_gpio_write(PIN_DHT, 0);
  TL_ZB_TIMER_SCHEDULE(dht22_read_start, NULL, 1);
}

void start_calibration(u32 time) {
  u16 value = zcl_co2.calibration_value;
  u8 packet[] = {0x11, 0x03, 0x03, 0x00, 0x00, 0x00};
  u32 len = ARRAY_SIZE(packet);
  store_be16(packet + 3, value);
  u8 sum = 0;
  for (u8* i = packet; i != packet + len - 1; ++i) {
    sum += *i;
  }
  packet[len - 1] = 0x100 - sum;
  last_calibration_request_time = time;
  if (!drv_uart_tx_start(packet, len)) {
    last_calibration_request_time = 0;
  }
}

void app_task() {
  if (bdb_isIdle()) {
    report_handler();
  }

  update_led();
  if (zigbee_bound()) {
    update_co2();
    // update_dht();
  }
}

extern volatile u16 T_evtExcept[4];

static void app_sys_exception(void) {
  SYSTEM_RESET();
}

void user_init(bool is_retention) {
  drv_gpio_func_set(PIN_LED);
  drv_gpio_output_en(PIN_LED, 1);
  drv_gpio_input_en(PIN_LED, 0);
  drv_gpio_write(PIN_LED, 1);

#ifdef CHECK_BOOTLOADER
  bootloader_check();
#endif

  app_init_zb();

  af_powerDescPowerModeUpdate(POWER_MODE_RECEIVER_COMES_WHEN_STIMULATED);

  init_zcl();

  gp_init(APP_ENDPOINT1);

  init_drv();

  /* Register except handler for test */
  sys_exceptHandlerRegister(app_sys_exception);

#if ZBHCI_EN
  zbhciInit();
  ev_on_poll(EV_POLL_HCI, zbhciTask);
#endif
  ev_on_poll(EV_POLL_IDLE, app_task);

  app_init_bdb();

  drv_gpio_write(PIN_LED, 0);
}
