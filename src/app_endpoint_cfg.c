#include "app_endpoint_cfg.h"

#include "tl_common.h"
#include "zcl_include.h"

#include "app_utility.h"
#include "app_zcl.h"

#define ZCL_ATTRID_TEMPERATURE_MEASUREMENT_MEASUREDVALUE ZCL_TEMPERATURE_MEASUREMENT_ATTRID_MEASUREDVALUE

#ifndef ZCL_BASIC_SW_BUILD_ID
#define ZCL_BASIC_SW_BUILD_ID       {10,'0','1','2','2','0','5','2','0','1','7'}
#endif

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

#define EXTRACT_FIRST(A, B) A
#define EXTRACT_SECOND(A, B) B

#define R               ACCESS_CONTROL_READ
#define RW              ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE
#define RR              ACCESS_CONTROL_READ | ACCESS_CONTROL_REPORTABLE
#define RWR             ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE | ACCESS_CONTROL_REPORTABLE

#define ATTRIBUTE(TYPE, ACCESS, UNAME, LNAME) \
    { CAT4(ZCL_ATTRID_, EXTRACT_FIRST CLUSTER, _, UNAME), CAT(ZCL_DATA_TYPE_, TYPE), ACCESS, (u8*)&CAT3(g_zcl_, EXTRACT_SECOND CLUSTER, Attrs).LNAME  },
#define ATTRIBUTE_REVISION \
    { ZCL_ATTRID_GLOBAL_CLUSTER_REVISION,   ZCL_DATA_TYPE_UINT16,     R,  (u8*)&zcl_attr_global_clusterRevision},

typedef struct {
  u8  zclVersion;
  u8  appVersion;
  u8  stackVersion;
  u8  hwVersion;
  u8  manuName[ZCL_BASIC_MAX_LENGTH];
  u8  modelId[ZCL_BASIC_MAX_LENGTH];
  u8  dateCode[ZCL_BASIC_MAX_LENGTH];
  u8  powerSource;
  u8  genDevClass;                        //attr 8
  u8  genDevType;                         //attr 9
  u8  deviceEnable;
  u8  swBuildId[ZCL_BASIC_MAX_LENGTH];    //attr 4000
} zcl_basicAttr_t;

zcl_basicAttr_t g_zcl_basicAttrs = {
  .zclVersion     = 0x03,
  .appVersion     = APP_RELEASE,
  .stackVersion   = (STACK_RELEASE|STACK_BUILD),
  .hwVersion      = HW_VERSION,
  .manuName       = ZCL_BASIC_MFG_NAME,
  .modelId        = {},
  .dateCode       = {},
  .powerSource    = POWER_SOURCE_MAINS_1_PHASE,
  .swBuildId      = ZCL_BASIC_SW_BUILD_ID,
  .deviceEnable   = TRUE,
};

#define CLUSTER (BASIC, basic)

const zclAttrInfo_t basic_attrTbl[] = {
  ATTRIBUTE(   UINT8,  R,     ZCL_VER,   zclVersion)
  ATTRIBUTE(   UINT8,  R,      APP_VER,   appVersion)
  ATTRIBUTE(   UINT8,  R,    STACK_VER, stackVersion)
  ATTRIBUTE(   UINT8,  R,       HW_VER,    hwVersion)
  ATTRIBUTE(CHAR_STR,  R,     MFR_NAME,     manuName)
  ATTRIBUTE(CHAR_STR,  R,     MODEL_ID,      modelId)
  ATTRIBUTE(CHAR_STR,  R,    DATE_CODE,     dateCode)
  ATTRIBUTE(   ENUM8,  R, POWER_SOURCE,  powerSource)
  ATTRIBUTE( BOOLEAN, RW,  DEV_ENABLED, deviceEnable)
  ATTRIBUTE(CHAR_STR,  R,  SW_BUILD_ID,    swBuildId)

  ATTRIBUTE_REVISION
};

#undef CLUSTER

zcl_identifyAttr_t g_zcl_identifyAttrs = {
  .time = 0x0000,
};

#define CLUSTER (IDENTIFY, identify)

const zclAttrInfo_t identify_attrTbl[] = {
  ATTRIBUTE(UINT16, RW, TIME, time)

  ATTRIBUTE_REVISION
};

#undef CLUSTER

typedef struct{
  u8  nameSupport;
} zcl_groupAttr_t;

zcl_groupAttr_t g_zcl_groupAttrs = {
  .nameSupport    = 0,
};

#define CLUSTER (GROUP, group)

const zclAttrInfo_t group_attrTbl[] = {
  ATTRIBUTE(BITMAP8, R, NAME_SUPPORT, nameSupport)

  ATTRIBUTE_REVISION
};

#undef CLUSTER

typedef struct{
  u8   sceneCount;
  u8   currentScene;
  u8   nameSupport;
  bool sceneValid;
  u16  currentGroup;
} zcl_sceneAttr_t;

zcl_sceneAttr_t g_zcl_sceneAttrs = {
  .sceneCount     = 0,
  .currentScene   = 0,
  .currentGroup   = 0x0000,
  .sceneValid     = FALSE,
  .nameSupport    = 0,
};

#define CLUSTER (SCENE, scene)

const zclAttrInfo_t scene_attrTbl[] = {
  ATTRIBUTE(  UINT8, R,   SCENE_COUNT,   sceneCount)
  ATTRIBUTE(  UINT8, R, CURRENT_SCENE, currentScene)
  ATTRIBUTE( UINT16, R, CURRENT_GROUP, currentGroup)
  ATTRIBUTE(BOOLEAN, R,   SCENE_VALID,   sceneValid)
  ATTRIBUTE(BITMAP8, R,  NAME_SUPPORT,  nameSupport)

  ATTRIBUTE_REVISION
};

#undef CLUSTER

zcl_co2Attr_t g_zcl_co2Attrs = {
  .measured_value = 1014.0f,
  .calibration_value = 600,
  .last_calibration = 0,
};

#define CLUSTER (CO2_MEASUREMENT, co2)

const zclAttrInfo_t co2_attrTbl[] = {
  ATTRIBUTE(SINGLE_PREC, RR,     MEASUREDVALUE,    measured_value)
  ATTRIBUTE(     UINT16, RW, CALIBRATION_VALUE, calibration_value)
  ATTRIBUTE(        UTC, RR,  LAST_CALIBRATION,  last_calibration)

  ATTRIBUTE_REVISION
};

#undef CLUSTER

zcl_temperatureAttr_t g_zcl_temperatureAttrs = {
  .measured_value = 0x8000,
};

#define CLUSTER (TEMPERATURE_MEASUREMENT, temperature)

const zclAttrInfo_t temperature_attrTbl[] = {
  ATTRIBUTE(INT16, RR, MEASUREDVALUE, measured_value)

  ATTRIBUTE_REVISION
};

#undef CLUSTER

zcl_humidityAttr_t g_zcl_humidityAttrs = {
  .measured_value = 0x8000,
};

#define CLUSTER (RELATIVE_HUMIDITY, humidity)

const zclAttrInfo_t humidity_attrTbl[] = {
  ATTRIBUTE(UINT16, RR, MEASUREDVALUE, measured_value)

  ATTRIBUTE_REVISION
};

#undef CLUSTER

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
  init_zcl_string(g_zcl_basicAttrs.modelId, "Mahtan_CO2_DIY");
  init_zcl_string(g_zcl_basicAttrs.dateCode, STRINGIFY(BUILD_DATE));

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
