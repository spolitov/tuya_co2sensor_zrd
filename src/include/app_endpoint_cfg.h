#ifndef SRC_INCLUDE_APP_ENDPOINT_CFG_H_
#define SRC_INCLUDE_APP_ENDPOINT_CFG_H_

#include "types.h"

#define APP_ENDPOINT1 0x01

typedef struct {
  u16 time;
} zcl_identifyAttr_t;

extern zcl_identifyAttr_t g_zcl_identifyAttrs;

typedef struct {
  u16 measured_value;
  u16 calibration_value;
  u32 last_calibration;
} zcl_co2Attr_t;

extern zcl_co2Attr_t g_zcl_co2Attrs;

typedef struct {
  s16 measured_value;
} zcl_temperatureAttr_t;

extern zcl_temperatureAttr_t g_zcl_temperatureAttrs;

typedef struct {
  u16 measured_value;
} zcl_humidityAttr_t;

extern zcl_humidityAttr_t g_zcl_humidityAttrs;

typedef struct {
  int id;
  const af_simple_descriptor_t* descriptor;
  int cluster_size;
  const zcl_specClusterInfo_t* cluster;
} endpoint_info_t;

endpoint_info_t* get_endpoints();

#endif /* SRC_INCLUDE_APP_ENDPOINT_CFG_H_ */
