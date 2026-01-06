#include "app_zcl.h"

#include "zcl.h"

#include "app_endpoint_cfg.h"

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
