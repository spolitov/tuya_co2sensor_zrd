#ifndef SRC_INCLUDE_APP_MAIN_H_
#define SRC_INCLUDE_APP_MAIN_H_

#include "tl_common.h"
#include "zcl_include.h"
#include "bdb.h"
#include "ota.h"
#include "gp.h"

#include "app_zcl.h"
#include "app_utility.h"
#include "app_endpoint_cfg.h"
#include "app_bootloader.h"


typedef struct {
    u8 keyType; /* CERTIFICATION_KEY or MASTER_KEY key for touch-link or distribute network
                        SS_UNIQUE_LINK_KEY or SS_GLOBAL_LINK_KEY for distribute network */
    u8 key[16]; /* the key used */
} app_linkKey_info_t;

typedef struct {
  ev_timer_event_t *bdbFBTimerEvt;

  u32 short_poll;
  u32 long_poll;
  u32 current_poll;

  u32 timeout;

  u32 keyPressedTime;
  u8  keyPressed;

  app_linkKey_info_t tcLinkKey;
} app_ctx_t;

extern app_ctx_t g_appCtx;

extern const af_simple_descriptor_t app_ep1Desc;

status_t app_identifyCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload);
status_t app_groupCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload);
status_t app_sceneCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload);
status_t app_pollCtrlCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload);
status_t app_co2Cb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload);
status_t app_temperatureCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload);
status_t app_humidityCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload);

#define zcl_scene1AttrGet()         &g_zcl_scene1Attrs
#define zcl_scene2AttrGet()         &g_zcl_scene2Attrs
#define zcl_co2AttrGet()            &g_zcl_co2Attrs
#define zcl_temperatureAttrGet()    &g_zcl_temperatureAttrs
#define zcl_humidityAttrGet()       &g_zcl_humidityAttrs
#define zcl_fhydAttrGet()           &g_zcl_fhydAttrs
#define zcl_aInputAttrGet()         &g_zcl_aInputAttrs

#endif /* SRC_INCLUDE_APP_MAIN_H_ */
