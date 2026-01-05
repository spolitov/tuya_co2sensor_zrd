#ifndef SRC_INCLUDE_APP_ENDPOINT_CFG_H_
#define SRC_INCLUDE_APP_ENDPOINT_CFG_H_

#define APP_ENDPOINT1 0x01
#define APP_ENDPOINT2 0x02
#define APP_ENDPOINT3 0x03
#define APP_ENDPOINT4 0x04
#define APP_ENDPOINT5 0x05

typedef struct {
  uint16_t identifyTime;
} zcl_identifyAttr_t;

typedef struct {
  float co2;
  int16_t temperature;
  uint16_t humidity;
} AppAttributes;

extern zcl_identifyAttr_t g_zcl_identifyAttrs;
extern AppAttributes attributes;

typedef struct {
  int id;
  const af_simple_descriptor_t* descriptor;
  int cluster_size;
  const zcl_specClusterInfo_t* cluster;
} EndpointInfo;

EndpointInfo* get_endpoints();

#endif /* SRC_INCLUDE_APP_ENDPOINT_CFG_H_ */
