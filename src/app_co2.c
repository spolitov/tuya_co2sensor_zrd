#include "app_cfg.h"

#include "platform.h"
#include "zcl_include.h"

#include "app_endpoint_cfg.h"
#include "app_utility.h"
#include "app_zcl.h"

#define LOG_CO2(...) LOG_ON("CO2", __VA_ARGS__)

static u8 uart_data[0x200];
static u32 co2_last_calibration_request_time = 0;

static reportCfgInfo_t* co2_report_cfg = NULL;
static publish_info_t co2_last_calibration_publish_info;
#if CO2_MANUAL_MEASUREMENT
static u32 co2_measurement_time = 0;
#endif

static void co2_publish_last_calibration(void* arg) {
  publish_attribute(co2_last_calibration_publish_info);
}

static void uart_recv_cb() {
  // The only full packets are handled.
  // It is not a problem since we don't have any important information that could be lost because of
  // ignored incomplete packed.
  u32 len;
  memcpy(&len, uart_data, sizeof(len));
  const u8* data = uart_data + sizeof(len);
  switch (len) {
    case 4:
      if (load_le16(data) == 0x0116 && load_le16(data + 2) == 0xe603) {
        if (co2_last_calibration_request_time) {
          LOG_CO2("calibration accepted");
          zcl_co2.last_calibration = co2_last_calibration_request_time;
          co2_last_calibration_request_time = 0;
          tl_zbTaskPost(co2_publish_last_calibration, NULL);
        }
      }
      break;
    case 14:
      if (load_le16(data) == 0x6964) {
        u16 received_crc = load_le16(data + len - MODBUS_CRC_LEN);
        u16 expected_crc = modbus_crc(data, len - MODBUS_CRC_LEN);
        if (received_crc != expected_crc) {
          return;
        }

        zcl_co2.measured_value = load_le16(data + 4);
        LOG_CO2("single measurement: %d", (u32)zcl_co2.measured_value);
      }
      break;
    case 16:
      if (load_le16(data) == 0x4d42 && check_sum_crc(data, 16)) {
        zcl_co2.measured_value = load_be16(data + 6);
        LOG_CO2("background measurement: %d", (u32)zcl_co2.measured_value);
      }
      break;
  }
}

void co2_init() {
  drv_uart_pin_set(GPIO_UART_TX, GPIO_UART_RX);
  memset(uart_data, 0, sizeof(uart_data));
  drv_uart_init(9600, uart_data, sizeof(uart_data), uart_recv_cb);

  co2_report_cfg = zcl_reportCfgInfoEntryFind(
      APP_ENDPOINT1, ZCL_CLUSTER_MS_CO2_MEASUREMENT, ZCL_ATTRID_CO2_MEASUREMENT_MEASUREDVALUE);
  if (!co2_report_cfg) {
    LOG_CO2("Failed to find co2_report_cfg");
  }

  co2_last_calibration_publish_info = obtain_publish_info(
      APP_ENDPOINT1, ZCL_CLUSTER_MS_CO2_MEASUREMENT, ZCL_ATTRID_CO2_MEASUREMENT_LAST_CALIBRATION);
  if (!co2_last_calibration_publish_info.attr_info) {
    LOG_CO2("Failed to find co2_last_calibration_publish_info");
  }
}

void co2_update() {
#if CO2_MANUAL_MEASUREMENT
  if (!need_update(co2_report_cfg, 10, 2, &co2_measurement_time)) {
    return;
  }

  u8 packet[] = {0x64, 0x69, 0x03, 0x5E, 0x4E};
  drv_uart_tx_start(packet, ARRAY_SIZE(packet));
#endif
}

void co2_start_calibration(u32 time) {
  u16 value = zcl_co2.calibration_value;
  LOG_CO2("Calibrate with value: %d", value);

  u8 packet[] = {0x11, 0x03, 0x03, 0x00, 0x00, 0x00};
  u32 len = ARRAY_SIZE(packet);
  store_be16(packet + 3, value);
  u8 sum = 0;
  for (u8* i = packet; i != packet + len - 1; ++i) {
    sum += *i;
  }
  packet[len - 1] = 0x100 - sum;
  co2_last_calibration_request_time = time;
  if (!drv_uart_tx_start(packet, len)) {
    co2_last_calibration_request_time = 0;
  }
}
