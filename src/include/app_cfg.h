/********************************************************************************************************
 * @file    app_cfg.h
 *
 * @brief   This is the header file for app_cfg
 *
 * @author  Zigbee Group
 * @date    2021
 *
 * @par     Copyright (c) 2021, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *			All rights reserved.
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/

#pragma once

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif

#include "types.h"

#define ON                      1
#define OFF                     0

#ifndef MCU_CORE_8258
#define MCU_CORE_8258   1
#endif

#define REPORTING_MIN       300             /* 5 min            */
#define REPORTING_MAX       3600            /* 60 min           */

#define ZCL_BASIC_MFG_NAME     "Mahtan-DIY"

#include "version_cfg.h"

#define	UART_PRINTF_MODE                OFF
#define ZBHCI_UART                      OFF
#define PM_ENABLE					            	OFF
#define PA_ENABLE						            OFF
#define TOUCHLINK_SUPPORT				        ON

#define BOARD_826x_EVK                  0
#define BOARD_826x_DONGLE               1
#define BOARD_826x_DONGLE_PA            2
#define BOARD_8258_EVK                  3
#define BOARD_8258_EVK_V1P2             4//C1T139A30_V1.2
#define BOARD_8258_DONGLE               5
#define BOARD_8278_EVK                  6
#define BOARD_8278_DONGLE               7
#define BOARD_B91_EVK                   8
#define BOARD_B91_DONGLE                9
#define BOARD_TUYA_ZT3L                 10

/* Board define */
#if (CHIP_TYPE == TLSR_8258_1M)
#define FLASH_CAP_SIZE_1M           1
#endif
#define BOARD                       BOARD_TUYA_ZT3L
#define CLOCK_SYS_CLOCK_HZ          48000000
#define NV_ITEM_APP_USER_CFG        (NV_ITEM_APP_GP_TRANS_TABLE + 1)    // see sdk/proj/drivers/drv_nv.h

#include "board_zt3l.h"

#define MODULE_WATCHDOG_ENABLE						ON

#define ZCL_GROUP_SUPPORT                           ON
#define ZCL_SCENE_SUPPORT                           ON
#define ZCL_OTA_SUPPORT                             ON
#define ZCL_GP_SUPPORT                              ON
#define ZCL_CO2_MEASUREMENT_SUPPORT                 ON
#define ZCL_TEMPERATURE_MEASUREMENT_SUPPORT         ON
#define ZCL_HUMIDITY_MEASUREMENT_SUPPORT            ON
#define ZCL_ZLL_COMMISSIONING_SUPPORT               ON

#define ENABLE_DHT22  ON
#define ENABLE_DS1820 OFF

#include "stack_cfg.h"

typedef enum{
	EV_POLL_ED_DETECT,
	EV_POLL_HCI,
  EV_POLL_IDLE,
	EV_POLL_MAX,
}ev_poll_e;

#define LOG_OFF(...) do {} while(false)

#if UART_PRINTF_MODE
#define LOG_ON(prefix, fmt, ...) do { printf(prefix ": " fmt "\n", ##__VA_ARGS__); } while(false)
#else
#define LOG_ON(...) LOG_OFF(__VA_ARGS__)
#endif

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
