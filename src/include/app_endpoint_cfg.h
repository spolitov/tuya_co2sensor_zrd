#ifndef SRC_INCLUDE_APP_ENDPOINT_CFG_H_
#define SRC_INCLUDE_APP_ENDPOINT_CFG_H_

#include "types.h"
#include "zb_common.h"
#include "zcl_include.h"
#include "cluster_defs.h"

#define APP_ENDPOINT1 0x01
#define ZCL_ATTRID_TEMPERATURE_MEASUREMENT_MEASUREDVALUE ZCL_TEMPERATURE_MEASUREMENT_ATTRID_MEASUREDVALUE

#include "clusters.h"

typedef struct {
  int id;
  const af_simple_descriptor_t* descriptor;
  int cluster_size;
  const zcl_specClusterInfo_t* cluster;
} endpoint_info_t;

endpoint_info_t* get_endpoints();

extern const af_simple_descriptor_t app_ep1Desc;

#endif /* SRC_INCLUDE_APP_ENDPOINT_CFG_H_ */
