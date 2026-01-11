#ifndef SRC_INCLUDE_APP_UTILITY_H_
#define SRC_INCLUDE_APP_UTILITY_H_

#include "types.h"
#include "zcl_include.h"

#define MS_TO_US(ms) ((ms)*1000)
#define SEC_TO_US(sec) ((sec)*1000000)

#define SEC_TO_MS(sec) ((sec)*1000)
#define MIN_TO_MS(min) ((min)*60*1000)

#define MODBUS_CRC_LEN 2
extern u16 modbus_crc(const u8* data, u32 len);
extern u8 sum_crc(const u8* data, u32 len);
extern bool check_sum_crc(const u8* data, u32 len);

inline u16 load_le16(const u8* start) {
  return *start + ((u16)start[1] << 8);
}

inline void store_le16(u8* out, u16 input) {
  out[0] = input & 0xff;
  out[1] = input >> 8;
}

inline u16 load_be16(const u8* start) {
  return start[1] + ((u16)start[0] << 8);
}

inline void store_be16(u8* out, u16 input) {
  out[0] = input >> 8;
  out[1] = input & 0xff;
}

extern void init_zcl_string(u8* buffer, const char* input);

typedef struct {
  const clusterInfo_t* cluster_info;
  const zclAttrInfo_t* attr_info;
} publish_info_t;

extern publish_info_t obtain_publish_info(u8 endpoint, u16 clusterId, u16 attrId);
extern void publish_attribute(publish_info_t info);

extern bool need_update(const reportCfgInfo_t* report_cfg, u16 default_internval, u16 min_interval, u32* state);

#endif /* SRC_INCLUDE_APP_UTILITY_H_ */
