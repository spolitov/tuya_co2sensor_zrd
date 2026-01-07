/********************************************************************************************************
 * @file    stack_cfg.h
 *
 * @brief   This is the header file for stack_cfg
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

#define DEFAULT_CHANNEL                         20
#define NV_ENABLE                               1
#define SECURITY_ENABLE							1
#define	ZCL_CLUSTER_NUM_MAX						16
#define ZCL_REPORTING_TABLE_NUM					24
#define	ZCL_SCENE_TABLE_NUM						8
#define APS_GROUP_TABLE_NUM                   	8
#define APS_BINDING_TABLE_NUM                 	32

#if (COORDINATOR)
    #define ZB_ROUTER_ROLE                        1
    #define ZB_COORDINATOR_ROLE                   1
#elif (ROUTER)
    #define ZB_ROUTER_ROLE                        1
#elif (END_DEVICE)
    #define ZB_ED_ROLE                            1
#endif


#if ZB_ED_ROLE
	#if PM_ENABLE
		#define ZB_MAC_RX_ON_WHEN_IDLE			  0//must 0
	#endif

	#ifndef ZB_MAC_RX_ON_WHEN_IDLE
		#define ZB_MAC_RX_ON_WHEN_IDLE			  1//set 0 or 1
	#endif
#endif
