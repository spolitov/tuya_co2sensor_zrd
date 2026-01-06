#include "tl_common.h"
#include "zcl_include.h"

#include "app_main.h"

#ifndef ZCL_BASIC_SW_BUILD_ID
#define ZCL_BASIC_SW_BUILD_ID       {10,'0','1','2','2','0','5','2','0','1','7'}
#endif

#define R               ACCESS_CONTROL_READ
#define RW              ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE
#define RR              ACCESS_CONTROL_READ | ACCESS_CONTROL_REPORTABLE
#define RWR             ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE | ACCESS_CONTROL_REPORTABLE

/**
 *  @brief Defined for basic cluster attributes
 */
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

typedef struct{
  u8  nameSupport;
} zcl_groupAttr_t;

typedef struct{
  u8   sceneCount;
  u8   currentScene;
  u8   nameSupport;
  bool sceneValid;
  u16  currentGroup;
} zcl_sceneAttr_t;

typedef struct {
  s16 value;
} zcl_temperatureAttr_t;

typedef struct {
  s16 value;
} zcl_humidityAttr_t;

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

void InitZclString(u8* buffer, const char* input) {
  unsigned int len = strlen(input);
  buffer[0] = len;
  memcpy(buffer + 1, input, len);
}

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

const zclAttrInfo_t basic_attrTbl[] = {
    { ZCL_ATTRID_BASIC_ZCL_VER,             ZCL_DATA_TYPE_UINT8,      R,  (u8*)&g_zcl_basicAttrs.zclVersion  },
    { ZCL_ATTRID_BASIC_APP_VER,             ZCL_DATA_TYPE_UINT8,      R,  (u8*)&g_zcl_basicAttrs.appVersion  },
    { ZCL_ATTRID_BASIC_STACK_VER,           ZCL_DATA_TYPE_UINT8,      R,  (u8*)&g_zcl_basicAttrs.stackVersion},
    { ZCL_ATTRID_BASIC_HW_VER,              ZCL_DATA_TYPE_UINT8,      R,  (u8*)&g_zcl_basicAttrs.hwVersion   },
    { ZCL_ATTRID_BASIC_MFR_NAME,            ZCL_DATA_TYPE_CHAR_STR,   R,  (u8*)g_zcl_basicAttrs.manuName     },
    { ZCL_ATTRID_BASIC_MODEL_ID,            ZCL_DATA_TYPE_CHAR_STR,   R,  (u8*)g_zcl_basicAttrs.modelId      },
    { ZCL_ATTRID_BASIC_DATE_CODE,           ZCL_DATA_TYPE_CHAR_STR,   R,  (u8*)g_zcl_basicAttrs.dateCode     },
    { ZCL_ATTRID_BASIC_POWER_SOURCE,        ZCL_DATA_TYPE_ENUM8,      R,  (u8*)&g_zcl_basicAttrs.powerSource },
    { ZCL_ATTRID_BASIC_DEV_ENABLED,         ZCL_DATA_TYPE_BOOLEAN,    RW, (u8*)&g_zcl_basicAttrs.deviceEnable},
    { ZCL_ATTRID_BASIC_SW_BUILD_ID,         ZCL_DATA_TYPE_CHAR_STR,   R,  (u8*)&g_zcl_basicAttrs.swBuildId   },

    { ZCL_ATTRID_GLOBAL_CLUSTER_REVISION,   ZCL_DATA_TYPE_UINT16,     R,  (u8*)&zcl_attr_global_clusterRevision},

};

/* Identify */
zcl_identifyAttr_t g_zcl_identifyAttrs =
{
    .identifyTime   = 0x0000,
};

const zclAttrInfo_t identify_attrTbl[] =
{
    { ZCL_ATTRID_IDENTIFY_TIME,             ZCL_DATA_TYPE_UINT16,     RW, (u8*)&g_zcl_identifyAttrs.identifyTime},

    { ZCL_ATTRID_GLOBAL_CLUSTER_REVISION,   ZCL_DATA_TYPE_UINT16,     R,  (u8*)&zcl_attr_global_clusterRevision},
};

#define ZCL_IDENTIFY_ATTR_NUM           sizeof(identify_attrTbl) / sizeof(zclAttrInfo_t)

#ifdef ZCL_GROUP
/* Group */
zcl_groupAttr_t g_zcl_group1Attrs =
{
    .nameSupport    = 0,
};

const zclAttrInfo_t group_attr1Tbl[] =
{
    { ZCL_ATTRID_GROUP_NAME_SUPPORT,        ZCL_DATA_TYPE_BITMAP8,    R,  (u8*)&g_zcl_group1Attrs.nameSupport},

    { ZCL_ATTRID_GLOBAL_CLUSTER_REVISION,   ZCL_DATA_TYPE_UINT16,     R,  (u8*)&zcl_attr_global_clusterRevision},
};

#define ZCL_GROUP_1ATTR_NUM    sizeof(group_attr1Tbl) / sizeof(zclAttrInfo_t)

#endif

#ifdef ZCL_SCENE
/* Scene */
zcl_sceneAttr_t g_zcl_scene1Attrs = {
    .sceneCount     = 0,
    .currentScene   = 0,
    .currentGroup   = 0x0000,
    .sceneValid     = FALSE,
    .nameSupport    = 0,
};

const zclAttrInfo_t scene_attr1Tbl[] = {
    { ZCL_ATTRID_SCENE_SCENE_COUNT,         ZCL_DATA_TYPE_UINT8,      R,  (u8*)&g_zcl_scene1Attrs.sceneCount   },
    { ZCL_ATTRID_SCENE_CURRENT_SCENE,       ZCL_DATA_TYPE_UINT8,      R,  (u8*)&g_zcl_scene1Attrs.currentScene },
    { ZCL_ATTRID_SCENE_CURRENT_GROUP,       ZCL_DATA_TYPE_UINT16,     R,  (u8*)&g_zcl_scene1Attrs.currentGroup },
    { ZCL_ATTRID_SCENE_SCENE_VALID,         ZCL_DATA_TYPE_BOOLEAN,    R,  (u8*)&g_zcl_scene1Attrs.sceneValid   },
    { ZCL_ATTRID_SCENE_NAME_SUPPORT,        ZCL_DATA_TYPE_BITMAP8,    R,  (u8*)&g_zcl_scene1Attrs.nameSupport  },

    { ZCL_ATTRID_GLOBAL_CLUSTER_REVISION,   ZCL_DATA_TYPE_UINT16,     R,  (u8*)&zcl_attr_global_clusterRevision},
};

#define ZCL_SCENE_1ATTR_NUM   sizeof(scene_attr1Tbl) / sizeof(zclAttrInfo_t)

#endif

AppAttributes attributes = {
  .co2 = 0.001014,
  .co2_calibration_value = 600,
  .co2_last_calibration = 0xffffffff,
  .temperature = 0x8000,
  .humidity = 0xffff,
};

const zclAttrInfo_t co2_attrTbl[] = {
  { ZCL_CO2_MEASUREMENT_ATTRID_MEASUREDVALUE,     ZCL_DATA_TYPE_SINGLE_PREC, RR, (u8*)&attributes.co2 },
  { ZCL_CO2_MEASUREMENT_ATTRID_CALIBRATION_VALUE,      ZCL_DATA_TYPE_UINT16, RW, (u8*)&attributes.co2_calibration_value },
  { ZCL_CO2_MEASUREMENT_ATTRID_LAST_CALIBRATION,          ZCL_DATA_TYPE_UTC, RR, (u8*)&attributes.co2_last_calibration },
  { ZCL_ATTRID_GLOBAL_CLUSTER_REVISION,                ZCL_DATA_TYPE_UINT16,  R, (u8*)&zcl_attr_global_clusterRevision },
};

const zclAttrInfo_t temperature_attrTbl[] = {
  { ZCL_TEMPERATURE_MEASUREMENT_ATTRID_MEASUREDVALUE, ZCL_DATA_TYPE_INT16, RR, (u8*)&attributes.temperature },
  { ZCL_ATTRID_GLOBAL_CLUSTER_REVISION,               ZCL_DATA_TYPE_UINT16, R, (u8*)&zcl_attr_global_clusterRevision },
};

const zclAttrInfo_t humidity_attrTbl[] = {
  { ZCL_ATTRID_HUMIDITY_MEASUREDVALUE,  ZCL_DATA_TYPE_UINT16, RR, (u8*)&attributes.humidity },
  { ZCL_ATTRID_GLOBAL_CLUSTER_REVISION, ZCL_DATA_TYPE_UINT16, R,  (u8*)&zcl_attr_global_clusterRevision },
};

/**
 *  @brief Definition for simple switch ZCL specific cluster
 */
const zcl_specClusterInfo_t g_appEp1ClusterList[] = {
    {ZCL_CLUSTER_GEN_BASIC,                  MANUFACTURER_CODE_NONE, ARRAY_SIZE(      basic_attrTbl),       basic_attrTbl,                   zcl_basic_register,         app_basicCb },
    {ZCL_CLUSTER_GEN_IDENTIFY,               MANUFACTURER_CODE_NONE, ARRAY_SIZE(   identify_attrTbl),    identify_attrTbl,                zcl_identify_register,      app_identifyCb },
    {ZCL_CLUSTER_GEN_GROUPS,                 MANUFACTURER_CODE_NONE, ARRAY_SIZE(     group_attr1Tbl),      group_attr1Tbl,                   zcl_group_register,                NULL },
    {ZCL_CLUSTER_GEN_SCENES,                 MANUFACTURER_CODE_NONE, ARRAY_SIZE(     scene_attr1Tbl),      scene_attr1Tbl,                   zcl_scene_register,         app_sceneCb },
    {ZCL_CLUSTER_MS_CO2_MEASUREMENT,         MANUFACTURER_CODE_NONE, ARRAY_SIZE(        co2_attrTbl),         co2_attrTbl,         zcl_co2_measurement_register,           app_co2Cb },
    {ZCL_CLUSTER_MS_TEMPERATURE_MEASUREMENT, MANUFACTURER_CODE_NONE, ARRAY_SIZE(temperature_attrTbl), temperature_attrTbl, zcl_temperature_measurement_register,   app_temperatureCb },
    {ZCL_CLUSTER_MS_RELATIVE_HUMIDITY,       MANUFACTURER_CODE_NONE,    ARRAY_SIZE(humidity_attrTbl),    humidity_attrTbl,    zcl_humidity_measurement_register,      app_humidityCb },
};

bool calibrateOnOff = false;

EndpointInfo* get_endpoints() {
  InitZclString(g_zcl_basicAttrs.modelId, "Mahtan_CO2_DIY");
  InitZclString(g_zcl_basicAttrs.dateCode, STRINGIFY(BUILD_DATE));

  static EndpointInfo result[] = {
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
