#include "app_zcl.h"

#include "zcl_include.h"

#include "app_endpoint_cfg.h"
#include "app_utility.h"

extern void start_calibration(u32 time);

_CODE_ZCL_ status_t zcl_humidity_measurement_register(u8 endpoint, u16 manuCode, u8 attrNum, const zclAttrInfo_t attrTbl[], cluster_forAppCb_t cb) {
  return zcl_registerCluster(endpoint, ZCL_CLUSTER_MS_RELATIVE_HUMIDITY, manuCode, attrNum, attrTbl, NULL, cb);
}

#define ZCL_CMD_CALIBRATE 0x80

static status_t zcl_co2_client_command_handler(zclIncoming_t* msg) {
  if (msg->hdr.cmd != ZCL_CMD_CALIBRATE) {
    return ZCL_STA_UNSUP_CLUSTER_COMMAND;
  }
  u32 calibration_time = 0;
  if (msg->dataLen >= 4) {
    memcpy(&calibration_time, msg->pData, 4);
  }

  start_calibration(calibration_time);
  return ZCL_STA_UNSUP_CLUSTER_COMMAND;
}

static status_t zcl_co2_command_handler(zclIncoming_t* msg) {
  if (msg->hdr.frmCtrl.bf.dir == ZCL_FRAME_CLIENT_SERVER_DIR) {
    return zcl_co2_client_command_handler(msg);
  }
  return ZCL_STA_UNSUP_CLUSTER_COMMAND;
}

_CODE_ZCL_ status_t zcl_co2_measurement_register(u8 endpoint, u16 manuCode, u8 attrNum, const zclAttrInfo_t attrTbl[], cluster_forAppCb_t cb) {
  return zcl_registerCluster(endpoint, ZCL_CLUSTER_MS_CO2_MEASUREMENT, manuCode, attrNum, attrTbl, zcl_co2_command_handler, cb);
}

static ev_timer_event_t *identifyTimerEvt = NULL;

static s32 app_zclIdentifyTimerCb(void *arg) {
	if(g_zcl_identifyAttrs.time <= 0){
		identifyTimerEvt = NULL;
		return -1;
	}
	g_zcl_identifyAttrs.time--;
	return 0;
}

static void app_zclIdentifyTimerStop(void) {
  TL_ZB_TIMER_CANCEL(&identifyTimerEvt);
}

static void app_zclIdentifyCmdHandler(u8 endpoint, u16 srcAddr, u16 identifyTime) {
	g_zcl_identifyAttrs.time = identifyTime;

	if(identifyTime == 0){
		app_zclIdentifyTimerStop();
	}else{
		if(!identifyTimerEvt){
			identifyTimerEvt = TL_ZB_TIMER_SCHEDULE(app_zclIdentifyTimerCb, NULL, SEC_TO_MS(1));
		}
	}
}

status_t app_identifyCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload) {
	if(pAddrInfo->dstEp == APP_ENDPOINT1){
		if(pAddrInfo->dirCluster == ZCL_FRAME_CLIENT_SERVER_DIR){
			switch(cmdId){
				case ZCL_CMD_IDENTIFY:
					app_zclIdentifyCmdHandler(pAddrInfo->dstEp, pAddrInfo->srcAddr, ((zcl_identifyCmd_t *)cmdPayload)->identifyTime);
					break;
				default:
					break;
			}
		}
	}

	return ZCL_STA_SUCCESS;
}
