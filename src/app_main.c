#include "app_main.h"

#define PIN_LED GPIO_PB4
#define PIN_BUTTON GPIO_PA0

app_ctx_t g_appCtx = {
  .bdbFBTimerEvt = NULL,
  .short_poll    = POLL_RATE * 3,
  .long_poll     = POLL_RATE * LONG_POLL,
};

#ifdef ZCL_OTA
extern ota_callBack_t app_otaCb;

//running code firmware information
ota_preamble_t app_otaInfo = {
  .fileVer          = FILE_VERSION,
  .imageType        = IMAGE_TYPE,
  .manufacturerCode = MANUFACTURER_CODE_TELINK,
};
#endif

zdo_appIndCb_t app_ind_cb = {
  .zdpStartDevCnfCb = bdb_zdoStartDevCnf,
  .zdpResetCnfCb = NULL,
  .zdpDevAnnounceIndCb = NULL,
  .zdpLeaveIndCb = app_leaveIndHandler,
  .zdpLeaveCnfCb = app_leaveCnfHandler,
  .zdpNwkUpdateIndCb = app_nwkUpdateIndicateHandler,
  .zdpPermitJoinIndCb = NULL,
  .zdoNlmeSyncCnfCb = NULL,
  .zdoTcJoinIndCb = NULL,
  .ssTcFrameCntReachedCb = NULL,
  .nwkStatusIndCb = NULL,
};

bdb_commissionSetting_t g_bdbCommissionSetting = {
  .linkKey.tcLinkKey.keyType = SS_GLOBAL_LINK_KEY,
  .linkKey.tcLinkKey.key = (uint8_t *)tcLinkKeyCentralDefault,             //can use unique link key stored in NV

  .linkKey.distributeLinkKey.keyType = MASTER_KEY,
  .linkKey.distributeLinkKey.key = (uint8_t *)linkKeyDistributedMaster,    //use linkKeyDistributedCertification before testing

  .linkKey.touchLinkKey.keyType = MASTER_KEY,
  .linkKey.touchLinkKey.key = (uint8_t *)touchLinkKeyMaster,               //use touchLinkKeyCertification before testing

#if TOUCHLINK_SUPPORT
  .touchlinkEnable = 1,                                               /* enable touch-link */
#else
  .touchlinkEnable = 0,                                               /* disable touch-link */
#endif
  .touchlinkChannel = DEFAULT_CHANNEL,                                /* touch-link default operation channel for target */
  .touchlinkLqiThreshold = 0xA0,                                      /* threshold for touch-link scan req/resp command */
};

void stack_init(void) {
  zb_init();
  zb_zdoCbRegister(&app_ind_cb);
}

#define PIN_CO2_PWM GPIO_PB4
#define PIN_CALIBRATION GPIO_PA0

uint32_t button_press_time = 0;
unsigned long led_switch_time = 0;
int led_state = 0;
ev_timer_event_t* factory_reset_timer = NULL;

void button_release();

int factory_reset_trigger(void* arg) {
  zb_resetDevice2FN();
  factory_reset_timer = NULL;
  return -1;
}

void button_press() {
  button_press_time = clock_time();

  drv_gpio_irq_set(PIN_BUTTON, GPIO_RISING_EDGE);

  if (!factory_reset_timer) {
    factory_reset_timer = TL_ZB_TIMER_SCHEDULE(factory_reset_trigger, NULL, TIMEOUT_5SEC);
  }
}

void button_release() {
  button_press_time = 0;

  drv_gpio_irq_set(PIN_BUTTON, GPIO_FALLING_EDGE);

  if (factory_reset_timer) {
    TL_ZB_TIMER_CANCEL(&factory_reset_timer);
  }
}

bool reset_timer_started() {
  return factory_reset_timer != NULL;
}

void user_app_init() {
  drv_gpio_func_set(PIN_LED);
  drv_gpio_output_en(PIN_LED, 1);
  drv_gpio_input_en(PIN_LED, 0);
  drv_gpio_write(PIN_LED, 1);

#if ZCL_POLL_CTRL_SUPPORT
  af_powerDescPowerModeUpdate(POWER_MODE_RECEIVER_COMES_PERIODICALLY);
#else
  af_powerDescPowerModeUpdate(POWER_MODE_RECEIVER_COMES_WHEN_STIMULATED);
#endif

  /* Initialize ZCL layer */
  /* Register Incoming ZCL Foundation command/response messages */
  zcl_init(app_zclProcessIncomingMsg);

  /* register endPoint */
  EndpointInfo* endpoints = get_endpoints();
  for (EndpointInfo* i = endpoints; i->id != 0; ++i) {
    af_endpointRegister(i->id, i->descriptor, zcl_rx_handler, NULL);
  }

  zcl_reportingTabInit();

  /* Register ZCL specific cluster information */
  for (EndpointInfo* i = endpoints; i->id != 0; ++i) {
    zcl_register(i->id, i->cluster_size, i->cluster);
  }

#if ZCL_GP_SUPPORT
    /* Initialize GP */
  gp_init(APP_ENDPOINT1);
#endif

#if ZCL_OTA_SUPPORT
  ota_init(OTA_TYPE_CLIENT, (af_simple_descriptor_t *)&app_ep1Desc, &app_otaInfo, &app_otaCb);
#endif

    /*app_uart_init();
*/

  drv_gpio_func_set(PIN_BUTTON);
  drv_gpio_output_en(PIN_BUTTON, 0);
  drv_gpio_input_en(PIN_BUTTON, 1);
  drv_gpio_up_down_resistor(PIN_BUTTON, PM_PIN_PULLUP_10K);

  drv_gpio_irq_config(GPIO_IRQ_MODE, PIN_BUTTON, GPIO_FALLING_EDGE, button_press);
  drv_gpio_irq_en(PIN_BUTTON);

  drv_enable_irq();

  drv_gpio_write(PIN_LED, 0);
}

int zigbee_bound() {
  return zb_getLocalShortAddr() < 0xFFF8;
}

int desired_led_state() {
  if (zigbee_bound()) {
    return reset_timer_started();
  }
  if (clock_time_exceed(led_switch_time, TIMEOUT_TICK_100MS * 5)) {
    return !led_state;
  }
  return led_state;
}

void update_led() {
  int desired_state = desired_led_state();
  if (desired_state != led_state) {
    led_state = desired_state;
    led_switch_time = clock_time();
    drv_gpio_write(PIN_LED, led_state);
  }
}

void app_task() {

    // uart_cmd_handler();

  if (bdb_isIdle()) {
    report_handler();
  }

  update_led();
}

extern volatile uint16_t T_evtExcept[4];

static void appSysException(void) {

#if UART_PRINTF_MODE
    printf("app_sysException, line: %d, event: %d, reset\r\n", T_evtExcept[0], T_evtExcept[1]);
#endif

#if 1
    SYSTEM_RESET();
#else
    led_on(LED_STATUS);
    while(1);
#endif
}

/*********************************************************************
 * @fn      user_init
 *
 * @brief   User level initialization code.
 *
 * @param   isRetention - if it is waking up with ram retention.
 *
 * @return  None
 */
void user_init(bool isRetention) {

    (void)isRetention;

    start_message();

#ifdef CHECK_BOOTLOADER
    bootloader_check();
#endif


    /* Initialize Stack */
    stack_init();

    /* Initialize user application */
    user_app_init();

    /* Register except handler for test */
    sys_exceptHandlerRegister(appSysException);


    /* User's Task */
#if ZBHCI_EN
    zbhciInit();
    ev_on_poll(EV_POLL_HCI, zbhciTask);
#endif
    ev_on_poll(EV_POLL_IDLE, app_task);

    /* Read the pre-install code from NV */
    if(bdb_preInstallCodeLoad(&g_appCtx.tcLinkKey.keyType, g_appCtx.tcLinkKey.key) == RET_OK){
        g_bdbCommissionSetting.linkKey.tcLinkKey.keyType = g_appCtx.tcLinkKey.keyType;
        g_bdbCommissionSetting.linkKey.tcLinkKey.key = g_appCtx.tcLinkKey.key;
    }

    /* Set default reporting configuration */
    uint8_t reportableChange = 0x00;

    bdb_defaultReportingCfg(APP_ENDPOINT1, HA_PROFILE_ID, ZCL_CLUSTER_MS_CO2_MEASUREMENT,
                            ZCL_CO2_MEASUREMENT_ATTRID_MEASUREDVALUE, 300, 3600, (uint8_t *)&reportableChange);
    bdb_defaultReportingCfg(APP_ENDPOINT1, HA_PROFILE_ID, ZCL_CLUSTER_MS_TEMPERATURE_MEASUREMENT,
                            ZCL_TEMPERATURE_MEASUREMENT_ATTRID_MEASUREDVALUE, 300, 3600, (uint8_t *)&reportableChange);
    bdb_defaultReportingCfg(APP_ENDPOINT1, HA_PROFILE_ID, ZCL_CLUSTER_MS_RELATIVE_HUMIDITY,
                            ZCL_ATTRID_HUMIDITY_MEASUREDVALUE, 300, 3600, (uint8_t *)&reportableChange);
    bdb_defaultReportingCfg(APP_ENDPOINT1, HA_PROFILE_ID, ZCL_CLUSTER_MS_FHYD_MEASUREMENT,
                            ZCL_FHYD_MEASUREMENT_ATTRID_MEASUREDVALUE, 300, 3600, (uint8_t *)&reportableChange);
    bdb_defaultReportingCfg(APP_ENDPOINT1, HA_PROFILE_ID, ZCL_CLUSTER_GEN_ANALOG_INPUT_BASIC,
                            ZCL_ANALOG_INPUT_ATTRID_PRESENT_VALUE, 300, 3600, (uint8_t *)&reportableChange);
    /* Initialize BDB */
    bdb_init((af_simple_descriptor_t *)&app_ep1Desc, &g_bdbCommissionSetting, &g_zbBdbCb, 1);
}

ev_timer_event_t *calibrationTimerEvt = NULL;

int32_t calibrationFinish(void *arg) {
    drv_gpio_write(PIN_CALIBRATION, 0);
    drv_gpio_output_en(PIN_CALIBRATION, 0);
    drv_gpio_input_en(PIN_CALIBRATION, 1);

    calibrationTimerEvt = NULL;
    return -1;
}

status_t app_onOffCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload) {
    if (cmdId == 1) {
      if (calibrationTimerEvt) {
        return ZCL_STA_DUPLICATE_EXISTS;
      }
      drv_gpio_func_set(PIN_CALIBRATION);
      drv_gpio_output_en(PIN_CALIBRATION, 1);
      drv_gpio_input_en(PIN_CALIBRATION, 0);
      drv_gpio_write(PIN_CALIBRATION, 1);

      calibrationTimerEvt = TL_ZB_TIMER_SCHEDULE(calibrationFinish, NULL, 8 * 1000);
    }
    return ZCL_STA_SUCCESS;
}
