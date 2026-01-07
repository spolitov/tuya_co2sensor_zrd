#ifndef SRC_INCLUDE_APP_UTILITY_H_
#define SRC_INCLUDE_APP_UTILITY_H_

#include "types.h"
#include "zcl_include.h"

#define MS_TO_US(ms) ((ms)*1000)
#define SEC_TO_US(sec) ((sec)*1000000)

#define SEC_TO_MS(sec) ((sec)*1000)
#define MIN_TO_MS(min) ((min)*60*1000)

#define MODBUS_CRC_LEN 2
extern u16 modbus_crc(const u8 *dataarray, u16 datalen);

inline u16 load_le16(const u8* start) {
  return *start + ((u16)start[1] << 8);
}

inline void store_be16(u8* out, u16 input) {
  out[0] = input >> 8;
  out[1] = input & 0xff;
}

inline void store_le16(u8* out, u16 input) {
  out[0] = input & 0xff;
  out[1] = input >> 8;
}

extern void init_zcl_string(u8* buffer, const char* input);

#define CAT_II(p, res) res
#define CAT_I(a, b) CAT_II(~, a ## b)
#define CAT(a, b) CAT_I(a, b)
#define CAT3(a, b, c) CAT(CAT(a, b), c)
#define CAT4(a, b, c, d) CAT(CAT(CAT(a, b), c), d)

typedef struct {
  const clusterInfo_t* cluster_info;
  const zclAttrInfo_t* attr_info;
} publish_info_t;

extern publish_info_t obtain_publish_info(u8 endpoint, u16 clusterId, u16 attrId);
extern void publish_attribute(publish_info_t info);

#endif /* SRC_INCLUDE_APP_UTILITY_H_ */
