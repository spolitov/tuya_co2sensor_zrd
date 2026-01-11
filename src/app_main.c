#include "app_cfg.h"
#include "zb_common.h"
#include "zcl_include.h"
#include "gp.h"

#include "app_endpoint_cfg.h"
#include "app_utility.h"
#include "app_zcl.h"

#define PIN_LED GPIO_PB4
#define PIN_BUTTON GPIO_PA0

#define LOG_INIT(...) LOG_ON("INIT", __VA_ARGS__)

extern int join_in_progress();
extern void app_init_zb();
extern void app_init_bdb();

extern void co2_init();
extern void co2_update();

extern void dht22_init();
extern void dht22_update();

static unsigned long led_switch_time = 0;
static int led_state = 0;
static ev_timer_event_t* factory_reset_timer = NULL;

static int factory_reset_trigger(void* arg) {
  zb_resetDevice2FN();
  factory_reset_timer = NULL;
  return -1;
}

void button_update() {
  if (drv_gpio_read(PIN_BUTTON)) {
    TL_ZB_TIMER_CANCEL(&factory_reset_timer);
  } else {
    if (!factory_reset_timer) {
      factory_reset_timer = TL_ZB_TIMER_SCHEDULE(factory_reset_trigger, NULL, SEC_TO_MS(5));
    }
  }
}

static bool reset_timer_started() {
  return factory_reset_timer != NULL;
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
}

static int zigbee_bound() {
  return zb_isDeviceJoinedNwk();
}

static int led_handle_blink(u32 interval) {
  if (clock_time_exceed(led_switch_time, interval)) {
    return !led_state;
  }
  return led_state;
}

static int led_desired_state() {
  if (reset_timer_started()) {
    return 1;
  }
  if (zigbee_bound()) {
    if (zcl_identify.time > 0) {
      return led_handle_blink(MS_TO_US(500));
    }

    return 0;
  }
  return led_handle_blink((join_in_progress() || led_state) ? MS_TO_US(250) : MS_TO_US(2500));
}

static void led_update() {
  int desired_state = led_desired_state();
  if (desired_state != led_state) {
    led_state = desired_state;
    led_switch_time = clock_time();
    drv_gpio_write(PIN_LED, led_state);
  }
}

void app_task() {
  if (bdb_isIdle()) {
    report_handler();
  }

  button_update();
  led_update();
  if (zigbee_bound()) {
    co2_update();
    dht22_update();
  }
}

extern volatile u16 T_evtExcept[4];

static void app_sys_exception(void) {
  SYSTEM_RESET();
}

static void init_drv() {
  drv_gpio_func_set(PIN_LED);
  drv_gpio_output_en(PIN_LED, 1);
  drv_gpio_input_en(PIN_LED, 0);
  drv_gpio_write(PIN_LED, 1);

  drv_gpio_func_set(PIN_BUTTON);
  drv_gpio_output_en(PIN_BUTTON, 0);
  drv_gpio_input_en(PIN_BUTTON, 1);
  drv_gpio_up_down_resistor(PIN_BUTTON, PM_PIN_PULLUP_10K);
}

void user_init(bool is_retention) {
  LOG_INIT("tick per us: %d", sys_tick_per_us);

  init_drv();

#ifdef CHECK_BOOTLOADER
  bootloader_check();
#endif

  app_init_zb();

  af_powerDescPowerModeUpdate(POWER_MODE_RECEIVER_COMES_WHEN_STIMULATED);

  init_zcl();

  co2_init();
  dht22_init();

  gp_init(APP_ENDPOINT1);

  /* Register except handler for test */
  sys_exceptHandlerRegister(app_sys_exception);

  ev_on_poll(EV_POLL_IDLE, app_task);

  app_init_bdb();

  drv_gpio_write(PIN_LED, 0);
}
