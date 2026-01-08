CLUSTER(
  BASIC, basic,
  ((   UINT8,  R,      ZCL_VER,   zclVersion, 0x03))
  ((   UINT8,  R,      APP_VER,   appVersion, APP_RELEASE))
  ((   UINT8,  R,    STACK_VER, stackVersion, STACK_RELEASE | STACK_BUILD))
  ((   UINT8,  R,       HW_VER,    hwVersion, HW_VERSION))
  ((CHAR_STR,  R,     MFR_NAME,     manuName, {}))
  ((CHAR_STR,  R,     MODEL_ID,      modelId, {}))
  ((CHAR_STR,  R,    DATE_CODE,     dateCode, {}))
  ((   ENUM8,  R, POWER_SOURCE,  powerSource, POWER_SOURCE_MAINS_1_PHASE))
  (( BOOLEAN, RW,  DEV_ENABLED, deviceEnable, TRUE))
  ((CHAR_STR,  R,  SW_BUILD_ID,    swBuildId, {}))
)

CLUSTER(
  IDENTIFY, identify,
  ((UINT16, RW, TIME, time, 0))
)

CLUSTER(
  GROUP, group,
  ((BITMAP8, R, NAME_SUPPORT, nameSupport, 0))
)

CLUSTER(
  SCENE, scene,
  ((  UINT8, R,   SCENE_COUNT,   sceneCount, 0))
  ((  UINT8, R, CURRENT_SCENE, currentScene, 0))
  (( UINT16, R, CURRENT_GROUP, currentGroup, 0))
  ((BOOLEAN, R,   SCENE_VALID,   sceneValid, FALSE))
  ((BITMAP8, R,  NAME_SUPPORT,  nameSupport, 0))
)

CLUSTER(
  CO2_MEASUREMENT, co2,
  ((SINGLE_PREC, RR,     MEASUREDVALUE,    measured_value, 1014.0f))
  ((     UINT16, RW, CALIBRATION_VALUE, calibration_value, 600))
  ((        UTC, RR,  LAST_CALIBRATION,  last_calibration, 0))
)

CLUSTER(
  TEMPERATURE_MEASUREMENT, temperature,
  ((INT16, RR, MEASUREDVALUE, measured_value, 0x8000))
)

CLUSTER(
  RELATIVE_HUMIDITY, humidity,
  ((UINT16, RR, MEASUREDVALUE, measured_value, 0xffff))
)
