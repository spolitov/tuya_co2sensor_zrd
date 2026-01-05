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
uart_data_t uart_data;

reportCfgInfo_t* co2_report_cfg = NULL;
unsigned long co2_measurement_time = 0;

void button_release();

int factory_reset_trigger(void* arg) {
  zb_resetDevice2FN();
  factory_reset_timer = NULL;
  return -1;
}

void button_press() {
  if (drv_gpio_read(PIN_BUTTON)) {
    button_press_time = 0;

    drv_gpio_irq_set(PIN_BUTTON, GPIO_FALLING_EDGE);

    if (factory_reset_timer) {
      TL_ZB_TIMER_CANCEL(&factory_reset_timer);
    }
  } else {
    button_press_time = clock_time();

    drv_gpio_irq_set(PIN_BUTTON, GPIO_RISING_EDGE);

    if (!factory_reset_timer) {
      factory_reset_timer = TL_ZB_TIMER_SCHEDULE(factory_reset_trigger, NULL, TIMEOUT_5SEC);
    }
  }
}

void button_release() {
}

bool reset_timer_started() {
  return factory_reset_timer != NULL;
}

const uint8_t auchCRCHi[] = {
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};
const uint8_t auchCRCLo[] = {
  0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

uint16_t modbus_crc(const uint8_t *dataarray, uint16_t datalen) {
  uint8_t uchCRCHi = 0xFF ; /* CRC High byte initialization*/
  uint8_t uchCRCLo = 0xFF ; /* CRC Low byte initialization*/
  while (datalen--) {
    uint16_t uIndex = uchCRCLo ^ *dataarray++;/* count CRC */
    uchCRCLo = uchCRCHi ^ auchCRCHi[uIndex];
    uchCRCHi = auchCRCLo[uIndex];
  }
  return (uint16_t)uchCRCHi * 256 + (uint16_t)uchCRCLo;
}

#define MODBUS_CRC_LEN 2

uint16_t load_le16(const u8* start) {
  return *start + ((u16)start[1] << 8);
}

static void uart_recv_cb() {
  // The only full packets are handled.
  // It is not a problem since we don't have any important information that could be lost because of
  // ignored incomplete packed.
  const u8* data = uart_data.data;
  u32 len = uart_data.dma_len;
  if (len < MODBUS_CRC_LEN) {
    return;
  }
  uint16_t received_crc = load_le16(data + len - MODBUS_CRC_LEN);
  uint16_t expected_crc = modbus_crc(data, len - MODBUS_CRC_LEN);
  if (received_crc != expected_crc) {
    return;
  }

  uint16_t command = load_le16(data);
  if (command == 0x6964) {
    attributes.co2 = load_le16(data + 4) / 1000000.0f;
  }
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
    af_endpointRegister(i->id, (af_simple_descriptor_t*)i->descriptor, zcl_rx_handler, NULL);
  }

  zcl_reportingTabInit();

  /* Register ZCL specific cluster information */
  for (EndpointInfo* i = endpoints; i->id != 0; ++i) {
    zcl_register(i->id, i->cluster_size, (zcl_specClusterInfo_t*)i->cluster);
  }

#if ZCL_GP_SUPPORT
    /* Initialize GP */
  gp_init(APP_ENDPOINT1);
#endif

#if ZCL_OTA_SUPPORT
  ota_init(OTA_TYPE_CLIENT, (af_simple_descriptor_t *)&app_ep1Desc, &app_otaInfo, &app_otaCb);
#endif

  drv_gpio_func_set(PIN_BUTTON);
  drv_gpio_output_en(PIN_BUTTON, 0);
  drv_gpio_input_en(PIN_BUTTON, 1);
  drv_gpio_up_down_resistor(PIN_BUTTON, PM_PIN_PULLUP_10K);

  drv_gpio_irq_config(GPIO_IRQ_MODE, PIN_BUTTON, GPIO_FALLING_EDGE, button_press);
  drv_gpio_irq_en(PIN_BUTTON);

  drv_enable_irq();

  drv_uart_pin_set(GPIO_UART_TX, GPIO_UART_RX);
  memset(&uart_data, 0, sizeof(uart_data));
  drv_uart_init(UART_BAUDRATE_9600, (u8*)&uart_data, sizeof(uart_data), uart_recv_cb);
  drv_gpio_up_down_resistor(GPIO_UART_RX, PM_PIN_PULLUP_1M);

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

void update_co2() {
  unsigned int period_sec = co2_report_cfg ? co2_report_cfg->minInterval : 10;
  if (!clock_time_exceed(co2_measurement_time, TIMEOUT_TICK_1SEC * period_sec)) {
    return;
  }
  u8 packet[] = {0x64, 0x69, 0x03, 0x5E, 0x4E};
  if (drv_uart_tx_start(packet, ARRAY_SIZE(packet))) {
    co2_measurement_time = clock_time();
  }
}

void app_task() {
  if (bdb_isIdle()) {
    report_handler();
  }

  update_led();
  update_co2();
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
  if(bdb_preInstallCodeLoad(&g_appCtx.tcLinkKey.keyType, g_appCtx.tcLinkKey.key) == RET_OK) {
    g_bdbCommissionSetting.linkKey.tcLinkKey.keyType = g_appCtx.tcLinkKey.keyType;
    g_bdbCommissionSetting.linkKey.tcLinkKey.key = g_appCtx.tcLinkKey.key;
  }

  /* Set default reporting configuration */
  uint8_t reportableChange = 0x00;

  bdb_defaultReportingCfg(APP_ENDPOINT1, HA_PROFILE_ID, ZCL_CLUSTER_MS_CO2_MEASUREMENT,
                          ZCL_CO2_MEASUREMENT_ATTRID_MEASUREDVALUE, 10, 300, (uint8_t *)&reportableChange);
  bdb_defaultReportingCfg(APP_ENDPOINT1, HA_PROFILE_ID, ZCL_CLUSTER_MS_TEMPERATURE_MEASUREMENT,
                          ZCL_TEMPERATURE_MEASUREMENT_ATTRID_MEASUREDVALUE, 10, 300, (uint8_t *)&reportableChange);
  bdb_defaultReportingCfg(APP_ENDPOINT1, HA_PROFILE_ID, ZCL_CLUSTER_MS_RELATIVE_HUMIDITY,
                          ZCL_ATTRID_HUMIDITY_MEASUREDVALUE, 10, 300, (uint8_t *)&reportableChange);
  bdb_defaultReportingCfg(APP_ENDPOINT1, HA_PROFILE_ID, ZCL_CLUSTER_MS_FHYD_MEASUREMENT,
                          ZCL_FHYD_MEASUREMENT_ATTRID_MEASUREDVALUE, 10, 300, (uint8_t *)&reportableChange);
  bdb_defaultReportingCfg(APP_ENDPOINT1, HA_PROFILE_ID, ZCL_CLUSTER_GEN_ANALOG_INPUT_BASIC,
                          ZCL_ANALOG_INPUT_ATTRID_PRESENT_VALUE, 10, 300, (uint8_t *)&reportableChange);
  /* Initialize BDB */
  bdb_init((af_simple_descriptor_t *)&app_ep1Desc, &g_bdbCommissionSetting, &g_zbBdbCb, 1);

  co2_report_cfg = zcl_reportCfgInfoEntryFind(APP_ENDPOINT1, ZCL_CLUSTER_MS_CO2_MEASUREMENT, ZCL_CO2_MEASUREMENT_ATTRID_MEASUREDVALUE);
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
