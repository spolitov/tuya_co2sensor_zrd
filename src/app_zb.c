#include "tl_common.h"
#include "zcl_include.h"
#include "ota.h"

#include "app_main.h"

static ota_preamble_t app_otaInfo = {
  .fileVer          = FILE_VERSION,
  .imageType        = IMAGE_TYPE,
  .manufacturerCode = MANUFACTURER_CODE_TELINK,
};

static ev_timer_event_t *switchRejoinBackoffTimerEvt = NULL;
static int scheduled_join = 0;
static u8 reportable_change_buffer[0x100];
static u8* reportable_change = reportable_change_buffer;

static s32 app_bdbNetworkSteerStart(void *arg) {
  bdb_networkSteerStart();
  --scheduled_join;

  return -1;
}

static void schedule_join() {
  u16 jitter = (1 + (zb_random() & 0xf)) * 100;
  ++scheduled_join;
  TL_ZB_TIMER_SCHEDULE(app_bdbNetworkSteerStart, NULL, jitter);
}

static s32 rejoin_backoff(void *arg) {
  if (zb_isDeviceFactoryNew()) {
    switchRejoinBackoffTimerEvt = NULL;
    return -1;
  }

  zb_rejoinReq(zb_apsChannelMaskGet(), g_bdbAttrs.scanDuration);
  return 0;
}

static void schedule_rejoin() {
  if (!switchRejoinBackoffTimerEvt) {
    switchRejoinBackoffTimerEvt = TL_ZB_TIMER_SCHEDULE(rejoin_backoff, NULL, MIN_TO_MS(1));
  }
}

static void zb_bdbInitCb(u8 status, u8 joinedNetwork) {
  if (status == BDB_INIT_STATUS_SUCCESS) {
    if (joinedNetwork) {
        zb_setPollRate(POLL_RATE * 3);
        ota_queryStart(OTA_PERIODIC_QUERY_INTERVAL);
    } else {
      schedule_join();
    }
  } else {
    if (joinedNetwork) {
      schedule_rejoin();
    }
  }
}

static void zb_bdbCommissioningCb(u8 status, void *arg) {
  switch (status) {
    case BDB_COMMISSION_STA_SUCCESS:
      zb_setPollRate(POLL_RATE * 3);
      TL_ZB_TIMER_CANCEL(&switchRejoinBackoffTimerEvt);
			ota_queryStart(OTA_PERIODIC_QUERY_INTERVAL);
			break;
    case BDB_COMMISSION_STA_IN_PROGRESS:
    case BDB_COMMISSION_STA_NOT_AA_CAPABLE:
    case BDB_COMMISSION_STA_FORMATION_FAILURE:
    case BDB_COMMISSION_STA_NO_IDENTIFY_QUERY_RESPONSE:
    case BDB_COMMISSION_STA_BINDING_TABLE_FULL:
    case BDB_COMMISSION_STA_NO_SCAN_RESPONSE:
    case BDB_COMMISSION_STA_NOT_PERMITTED:
      break;
    case BDB_COMMISSION_STA_NO_NETWORK:
    case BDB_COMMISSION_STA_TCLK_EX_FAILURE:
    case BDB_COMMISSION_STA_TARGET_FAILURE:
      schedule_join();
      break;
    case BDB_COMMISSION_STA_PARENT_LOST:
      zb_rejoinReq(zb_apsChannelMaskGet(), g_bdbAttrs.scanDuration);
      break;
    case BDB_COMMISSION_STA_REJOIN_FAILURE:
      schedule_rejoin();
      break;
    default:
      break;
  }
}

static void app_otaProcessMsgHandler(u8 evt, u8 status) {
  if (evt == OTA_EVT_START) {
    if (status == ZCL_STA_SUCCESS) {
      zb_setPollRate(QUEUE_POLL_RATE);
    }
  } else if(evt == OTA_EVT_COMPLETE) {
    zb_setPollRate(POLL_RATE * 3);

    if (status == ZCL_STA_SUCCESS) {
      ota_mcuReboot();
    } else {
      ota_queryStart(OTA_PERIODIC_QUERY_INTERVAL);
    }
  } else if(evt == OTA_EVT_IMAGE_DONE) {
    zb_setPollRate(POLL_RATE * 3);
  }
}

static void app_leaveCnfHandler(nlme_leave_cnf_t *pLeaveCnf) {
  if (pLeaveCnf->status == SUCCESS) {
    TL_ZB_TIMER_CANCEL(&switchRejoinBackoffTimerEvt);
  }
}

static bool app_nwkUpdateIndicateHandler(nwkCmd_nwkUpdate_t *pNwkUpdate){
  return FAILURE;
}

bdb_appCb_t bdb_appCb = {
  .bdbInitCb = zb_bdbInitCb,
  .bdbcommissioningCb = zb_bdbCommissioningCb,
  .bdbIdentifyCb = NULL,
  .bdbFindBindSuccessCb = NULL,
};

static zdo_appIndCb_t app_ind_cb = {
  .zdpStartDevCnfCb = bdb_zdoStartDevCnf,
  .zdpResetCnfCb = NULL,
  .zdpDevAnnounceIndCb = NULL,
  .zdpLeaveIndCb = NULL,
  .zdpLeaveCnfCb = app_leaveCnfHandler,
  .zdpNwkUpdateIndCb = app_nwkUpdateIndicateHandler,
  .zdpPermitJoinIndCb = NULL,
  .zdoNlmeSyncCnfCb = NULL,
  .zdoTcJoinIndCb = NULL,
  .ssTcFrameCntReachedCb = NULL,
  .nwkStatusIndCb = NULL,
};

static ota_callBack_t app_otaCb = {
  app_otaProcessMsgHandler,
};

bdb_commissionSetting_t bdb_commissionSetting = {
  .linkKey.tcLinkKey.keyType = SS_GLOBAL_LINK_KEY,
  .linkKey.tcLinkKey.key = (u8 *)tcLinkKeyCentralDefault,             //can use unique link key stored in NV

  .linkKey.distributeLinkKey.keyType = MASTER_KEY,
  .linkKey.distributeLinkKey.key = (u8 *)linkKeyDistributedMaster,    //use linkKeyDistributedCertification before testing

  .linkKey.touchLinkKey.keyType = MASTER_KEY,
  .linkKey.touchLinkKey.key = (u8 *)touchLinkKeyMaster,               //use touchLinkKeyCertification before testing

  .touchlinkEnable = 1,                                               /* enable touch-link */
  .touchlinkChannel = DEFAULT_CHANNEL,                                /* touch-link default operation channel for target */
  .touchlinkLqiThreshold = 0xA0,                                      /* threshold for touch-link scan req/resp command */
};

static void default_reporting_cfg(u16 clusterID, u16 attrID, u16 minReportInt, u16 maxReportInt) {
  memset(reportable_change, 0, REPORTABLE_CHANGE_MAX_ANALOG_SIZE);
  if (bdb_defaultReportingCfg(APP_ENDPOINT1, HA_PROFILE_ID, clusterID, attrID, minReportInt, maxReportInt, reportable_change) != ZCL_STA_SUCCESS) {
    sleep_ms(5000);
  }
  reportable_change += REPORTABLE_CHANGE_MAX_ANALOG_SIZE;
}

void app_init_zb() {
  zb_init();
  zb_zdoCbRegister(&app_ind_cb);

  ota_init(OTA_TYPE_CLIENT, (af_simple_descriptor_t *)&app_ep1Desc, &app_otaInfo, &app_otaCb);
}

void app_init_bdb() {
  if(bdb_preInstallCodeLoad(&g_appCtx.tcLinkKey.keyType, g_appCtx.tcLinkKey.key) == RET_OK) {
    bdb_commissionSetting.linkKey.tcLinkKey.keyType = g_appCtx.tcLinkKey.keyType;
    bdb_commissionSetting.linkKey.tcLinkKey.key = g_appCtx.tcLinkKey.key;
  }

  default_reporting_cfg(ZCL_CLUSTER_MS_CO2_MEASUREMENT, ZCL_ATTRID_CO2_MEASUREMENT_MEASUREDVALUE, 10, 300);
  default_reporting_cfg(ZCL_CLUSTER_MS_CO2_MEASUREMENT, ZCL_ATTRID_CO2_MEASUREMENT_LAST_CALIBRATION, 1, 3000);
  default_reporting_cfg(ZCL_CLUSTER_MS_TEMPERATURE_MEASUREMENT, ZCL_TEMPERATURE_MEASUREMENT_ATTRID_MEASUREDVALUE, 10, 300);
  default_reporting_cfg(ZCL_CLUSTER_MS_RELATIVE_HUMIDITY, ZCL_ATTRID_RELATIVE_HUMIDITY_MEASUREDVALUE, 10, 300);

  bdb_init((af_simple_descriptor_t *)&app_ep1Desc, &bdb_commissionSetting, &bdb_appCb, 1);
}

int join_in_progress() {
  return switchRejoinBackoffTimerEvt != NULL ||
         scheduled_join != 0 ||
         BDB_STATE_GET() == BDB_STATE_COMMISSIONING_NETWORK_STEER;
}
