#include "app_endpoint_cfg.h"

#include "tl_common.h"
#include "zcl_include.h"

#include "app_utility.h"
#include "app_zcl.h"

#undef CLUSTER
#define CLUSTER(...) DEFINE_CLUSTER(__VA_ARGS__)
#include "clusters.h"

const u16 app_ep1_inClusterList[] = {
  ZCL_CLUSTER_GEN_BASIC,
  ZCL_CLUSTER_GEN_IDENTIFY,
  ZCL_CLUSTER_GEN_GROUPS,
  ZCL_CLUSTER_GEN_SCENES,
  ZCL_CLUSTER_MS_CO2_MEASUREMENT,
  ZCL_CLUSTER_MS_TEMPERATURE_MEASUREMENT,
  ZCL_CLUSTER_MS_RELATIVE_HUMIDITY,
};

const u16 app_ep1_outClusterList[] = {
  ZCL_CLUSTER_OTA,
};

const af_simple_descriptor_t app_ep1Desc = {
  .app_profile_id = HA_PROFILE_ID,
  .app_dev_id = HA_DEV_SIMPLE_SENSOR,
  .endpoint = APP_ENDPOINT1,
  .app_dev_ver = 2,
  .reserved = 0,
  .app_in_cluster_count = ARRAY_SIZE(app_ep1_inClusterList),
  .app_out_cluster_count = ARRAY_SIZE(app_ep1_outClusterList),
  .app_in_cluster_lst = (u16*)app_ep1_inClusterList,
  .app_out_cluster_lst = (u16*)app_ep1_outClusterList,
};

const zcl_specClusterInfo_t g_appEp1ClusterList[] = {
    {ZCL_CLUSTER_GEN_BASIC,                  MANUFACTURER_CODE_NONE, ARRAY_SIZE(      basic_attrTbl),       basic_attrTbl,                   zcl_basic_register,           NULL },
    {ZCL_CLUSTER_GEN_IDENTIFY,               MANUFACTURER_CODE_NONE, ARRAY_SIZE(   identify_attrTbl),    identify_attrTbl,                zcl_identify_register, app_identifyCb },
    {ZCL_CLUSTER_GEN_GROUPS,                 MANUFACTURER_CODE_NONE, ARRAY_SIZE(      group_attrTbl),       group_attrTbl,                   zcl_group_register,           NULL },
    {ZCL_CLUSTER_GEN_SCENES,                 MANUFACTURER_CODE_NONE, ARRAY_SIZE(      scene_attrTbl),       scene_attrTbl,                   zcl_scene_register,           NULL },
    {ZCL_CLUSTER_MS_CO2_MEASUREMENT,         MANUFACTURER_CODE_NONE, ARRAY_SIZE(        co2_attrTbl),         co2_attrTbl,         zcl_co2_measurement_register,           NULL },
    {ZCL_CLUSTER_MS_TEMPERATURE_MEASUREMENT, MANUFACTURER_CODE_NONE, ARRAY_SIZE(temperature_attrTbl), temperature_attrTbl, zcl_temperature_measurement_register,           NULL },
    {ZCL_CLUSTER_MS_RELATIVE_HUMIDITY,       MANUFACTURER_CODE_NONE,    ARRAY_SIZE(humidity_attrTbl),    humidity_attrTbl,    zcl_humidity_measurement_register,           NULL },
};

bool calibrateOnOff = false;

endpoint_info_t* get_endpoints() {
  init_zcl_string(g_zcl_basicAttrs.manuName, ZCL_BASIC_MFG_NAME);
  init_zcl_string(g_zcl_basicAttrs.modelId, "Mahtan_CO2_DIY");
  init_zcl_string(g_zcl_basicAttrs.dateCode, STRINGIFY(BUILD_DATE));
  init_zcl_string(g_zcl_basicAttrs.swBuildId, "0108012025");

  static endpoint_info_t result[] = {
    {
      .id = APP_ENDPOINT1,
      .descriptor = &app_ep1Desc,
      .cluster_size = ARRAY_SIZE(g_appEp1ClusterList),
      .cluster = g_appEp1ClusterList,
    },
    {
      .id = 0
    }
  };
  return result;
}
