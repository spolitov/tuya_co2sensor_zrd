#ifndef SRC_INCLUDE_APP_BOOTLOADER_H_
#define SRC_INCLUDE_APP_BOOTLOADER_H_

#define BOOTLOADER_ADDR         BOOT_LOADER_IMAGE_ADDR
#define IMAGE_OTA_ADDR1         0x70000
#define IMAGE_OTA_ADDR2         FLASH_ADDR_OF_OTA_IMAGE
#define IMAGE_OTA_ADDR_END      0xE6000                     /* for 1M flash module */
#define IMAGE_OTA_SIZE          0x68000                     /* tuya addr 0x70000, ota_end 0xe6000, e6000-70000=68000 */
#define BOOTLOAD_MARKER         ZCL_BASIC_MFG_NAME
#define BOOTLOAD_MARKER_ADDR    0x7ff0
#define BOOTLOAD_MARKER_SECTOR  0x7000

#ifndef BOOT_SIZE
#define BOOT_SIZE               20580
#endif

#define TL_MAGIC                "KNLT"
#define TL_SIG                  0x025D

typedef struct {
    u16 reset;
    u32 file_version;
    u16 sig;
    u8  magic[4];
    u32 addr_bin_code;
    u16 irq;
    u16 manuf_code;
    u16 image_type;
    u16 dummy1;
    u32 bin_size;
    u32 dummy2;
} tl_header_t;

void bootloader_check();

#endif /* SRC_INCLUDE_APP_BOOTLOADER_H_ */
