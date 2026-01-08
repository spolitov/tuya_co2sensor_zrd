#include "app_cfg.h"
#include "zb_common.h"
#include "zcl_include.h"
#include "gp.h"

#include "app_endpoint_cfg.h"
#include "app_utility.h"
#include "app_zcl.h"

#define PIN_LED GPIO_PB4
#define PIN_BUTTON GPIO_PA0

extern int join_in_progress();
extern void app_init_zb();
extern void app_init_bdb();

static unsigned long led_switch_time = 0;
static int led_state = 0;
static ev_timer_event_t* factory_reset_timer = NULL;

static u8 uart_data[0x200];
static u32 last_calibration_request_time = 0;

static reportCfgInfo_t* co2_report_cfg = NULL;
static publish_info_t co2_last_calibration_publish_info;
static unsigned long co2_measurement_time = 0;

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
        g_zcl_co2Attrs.last_calibration = last_calibration_request_time;
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
      g_zcl_co2Attrs.measured_value = load_le16(data + 4);
    }
  }
}

void init_zcl() {
  zcl_init(NULL);

  endpoint_info_t* endpoints = get_endpoints();
  for (endpoint_info_t* i = endpoints; i->id != 0; ++i) {
    af_endpointRegister(i->id, (af_simple_descriptor_t*)i->descriptor, zcl_rx_handler, NULL);
  }

  zcl_reportingTabInit();

  for (endpoint_info_t* i = endpoints; i->id != 0; ++i) {
    zcl_register(i->id, i->cluster_size, (zcl_specClusterInfo_t*)i->cluster);
  }
}

void init_drv() {
  drv_gpio_func_set(PIN_BUTTON);
  drv_gpio_output_en(PIN_BUTTON, 0);
  drv_gpio_input_en(PIN_BUTTON, 1);
  drv_gpio_up_down_resistor(PIN_BUTTON, PM_PIN_PULLUP_10K);

  drv_gpio_irq_config(GPIO_IRQ_MODE, PIN_BUTTON, GPIO_FALLING_EDGE, button_press);
  drv_gpio_irq_en(PIN_BUTTON);

  drv_enable_irq();

  drv_uart_pin_set(GPIO_UART_TX, GPIO_UART_RX);
  memset(uart_data, 0, sizeof(uart_data));
  drv_uart_init(9600, uart_data, sizeof(uart_data), uart_recv_cb);
}

int zigbee_bound() {
  return zb_isDeviceJoinedNwk();
}

int handle_led_blink(u32 interval) {
  if (clock_time_exceed(led_switch_time, interval)) {
    return !led_state;
  }
  return led_state;
}

int desired_led_state() {
  if (reset_timer_started()) {
    return 1;
  }
  if (zigbee_bound()) {
    if (g_zcl_identifyAttrs.time > 0) {
      return handle_led_blink(MS_TO_US(500));
    }

    return 0;
  }
  return handle_led_blink((join_in_progress() || led_state) ? MS_TO_US(250) : MS_TO_US(2500));
}

void update_led() {
  int desired_state = desired_led_state();
  if (desired_state != led_state) {
    led_state = desired_state;
    led_switch_time = clock_time();
    drv_gpio_write(PIN_LED, led_state);
  }
}

void update_co2() {
  unsigned int period_sec = co2_report_cfg ? co2_report_cfg->minInterval : 10;
  if (!clock_time_exceed(co2_measurement_time, SEC_TO_US(period_sec ? period_sec : 1))) {
    return;
  }
  u8 packet[] = {0x64, 0x69, 0x03, 0x5E, 0x4E};
  if (drv_uart_tx_start(packet, ARRAY_SIZE(packet))) {
    co2_measurement_time = clock_time();
  }
}

void start_calibration(u32 time) {
  u16 value = g_zcl_co2Attrs.calibration_value;
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
  update_co2();
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

  co2_report_cfg = zcl_reportCfgInfoEntryFind(
      APP_ENDPOINT1, ZCL_CLUSTER_MS_CO2_MEASUREMENT, ZCL_ATTRID_CO2_MEASUREMENT_MEASUREDVALUE);

  co2_last_calibration_publish_info = obtain_publish_info(
      APP_ENDPOINT1, ZCL_CLUSTER_MS_CO2_MEASUREMENT, ZCL_ATTRID_CO2_MEASUREMENT_LAST_CALIBRATION);

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
