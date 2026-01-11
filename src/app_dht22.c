#include "app_cfg.h"

#include "platform.h"
#include "zcl_include.h"

#include "app_endpoint_cfg.h"
#include "app_utility.h"
#include "app_zcl.h"

#define LOG_DHT(...) LOG_ON("DHT", __VA_ARGS__)

const u32 PIN_DHT = GPIO_PC2;

static unsigned long dht22_measurement_time = 0;
static reportCfgInfo_t* temp_report_cfg = NULL;
static reportCfgInfo_t* hum_report_cfg = NULL;

static void dht22_read() {
  sleep_us(30);

  if (gpio_read(PIN_DHT)) {
    LOG_DHT("no response");
    return;
  }

  u8 byte = 0;
  u8 bytes[5];
  const u32 deadline_us = 300;
  for (int i = 0; i != 41; ++i) {
    u32 read_start = clock_time();
    byte <<= 1;
    while (!gpio_read(PIN_DHT)) {
      if (clock_time_exceed(read_start, deadline_us)) {
        LOG_DHT("time out waiting low on step %d", i);
        return;
      }
    }
    u32 this_start = clock_time();
    while (gpio_read(PIN_DHT)) {
      if (clock_time_exceed(read_start, deadline_us)) {
        LOG_DHT("time out waiting high on step %d", i);
        return;
      }
    }
    if (i == 0) {
      continue;
    }
    if (clock_time_exceed(this_start, 50)) {
      byte |= 1;
    }
    if ((i & 7) == 0) {
      bytes[(i - 1) >> 3] = byte;
      byte = 0;
    }
  }

  if (!check_sum_crc(bytes, 5)) {
    LOG_DHT("crc failure");
    return;
  }

  zcl_humidity.measured_value = load_be16(bytes) * 10;
  u16 temp_raw = load_be16(bytes + 2);
  if (temp_raw & 0x8000) {
    zcl_temperature.measured_value = -(s16)(temp_raw ^ 0x8000) * 10;
  } else {
    zcl_temperature.measured_value = temp_raw * 10;
  }
  LOG_DHT("humidity: %d, temperature: %d", zcl_humidity.measured_value, zcl_temperature.measured_value);
}

static int dht22_read_start(void* arg) {
  LOG_DHT("start");

  drv_gpio_output_en(PIN_DHT, 0);
  drv_gpio_input_en(PIN_DHT, 1);

  dht22_read();

  drv_gpio_input_en(PIN_DHT, 0);
  return -1;
}

void dht22_update() {
  unsigned int period_sec = 0;
  if (temp_report_cfg && temp_report_cfg->minInterval) {
    period_sec = temp_report_cfg->minInterval;
  }
  if (hum_report_cfg && hum_report_cfg->minInterval) {
    if (!period_sec || hum_report_cfg->minInterval < period_sec) {
      period_sec = hum_report_cfg->minInterval;
    }
  }
  if (period_sec == 0) {
    period_sec = 10;
  } else if (period_sec < 2) {
    period_sec = 2;
  }
  if (!clock_time_exceed(dht22_measurement_time, SEC_TO_US(period_sec))) {
    return;
  }
  dht22_measurement_time = clock_time();

  drv_gpio_input_en(PIN_DHT, 0);
  drv_gpio_output_en(PIN_DHT, 1);
  drv_gpio_write(PIN_DHT, 0);
  TL_ZB_TIMER_SCHEDULE(dht22_read_start, NULL, 1);
}

void dht22_init() {
  drv_gpio_func_set(PIN_DHT);
  drv_gpio_output_en(PIN_DHT, 0);
  drv_gpio_input_en(PIN_DHT, 1);

  temp_report_cfg = zcl_reportCfgInfoEntryFind(
      APP_ENDPOINT1, ZCL_CLUSTER_MS_TEMPERATURE_MEASUREMENT, ZCL_ATTRID_TEMPERATURE_MEASUREMENT_MEASUREDVALUE);
  if (!temp_report_cfg) {
    LOG_DHT("Failed to find temp_report_cfg");
  }

  hum_report_cfg = zcl_reportCfgInfoEntryFind(
      APP_ENDPOINT1, ZCL_CLUSTER_MS_RELATIVE_HUMIDITY, ZCL_ATTRID_RELATIVE_HUMIDITY_MEASUREDVALUE);
  if (!hum_report_cfg) {
    LOG_DHT("Failed to find hum_report_cfg");
  }
}
