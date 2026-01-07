#ifndef SRC_ZCL_H_
#define SRC_ZCL_H_

#include "zcl_include.h"

#define ZCL_ATTRID_RELATIVE_HUMIDITY_MEASUREDVALUE       0x0000

status_t zcl_humidity_measurement_register(u8 endpoint, u16 manuCode, u8 attrNum, const zclAttrInfo_t attrTbl[], cluster_forAppCb_t cb);

#define ZCL_CLUSTER_MS_CO2_MEASUREMENT                  0x040D

#define ZCL_ATTRID_CO2_MEASUREMENT_MEASUREDVALUE        0x0000
#define ZCL_ATTRID_CO2_MEASUREMENT_CALIBRATION_VALUE    0x4900
#define ZCL_ATTRID_CO2_MEASUREMENT_LAST_CALIBRATION     0x4901

status_t zcl_co2_measurement_register(u8 endpoint, u16 manuCode, u8 attrNum, const zclAttrInfo_t attrTbl[], cluster_forAppCb_t cb);

#endif /* SRC_ZCL_H_ */
