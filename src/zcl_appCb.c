#include "tl_common.h"
#include "zcl_include.h"

#include "app_main.h"

static ev_timer_event_t *identifyTimerEvt = NULL;

s32 app_zclIdentifyTimerCb(void *arg)
{
	if(g_zcl_identifyAttrs.time <= 0){
		identifyTimerEvt = NULL;
		return -1;
	}
	g_zcl_identifyAttrs.time--;
	return 0;
}

void app_zclIdentifyTimerStop(void) {
  TL_ZB_TIMER_CANCEL(&identifyTimerEvt);
}

void app_zclIdentifyCmdHandler(u8 endpoint, u16 srcAddr, u16 identifyTime) {
	g_zcl_identifyAttrs.time = identifyTime;

	if(identifyTime == 0){
		app_zclIdentifyTimerStop();
	}else{
		if(!identifyTimerEvt){
			identifyTimerEvt = TL_ZB_TIMER_SCHEDULE(app_zclIdentifyTimerCb, NULL, SEC_TO_MS(1));
		}
	}
}

status_t app_identifyCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload) {
	if(pAddrInfo->dstEp == APP_ENDPOINT1){
		if(pAddrInfo->dirCluster == ZCL_FRAME_CLIENT_SERVER_DIR){
			switch(cmdId){
				case ZCL_CMD_IDENTIFY:
					app_zclIdentifyCmdHandler(pAddrInfo->dstEp, pAddrInfo->srcAddr, ((zcl_identifyCmd_t *)cmdPayload)->identifyTime);
					break;
				default:
					break;
			}
		}
	}

	return ZCL_STA_SUCCESS;
}

static void app_zclAddGroupRspCmdHandler(u8 ep, zcl_addGroupRsp_t *pAddGroupRsp) {

//    printf("app_zclAddGroupRspCmdHandler. ep: %d, status: %d, gid: %d\r\n", ep, pAddGroupRsp->status, pAddGroupRsp->groupId);
}

/*********************************************************************
 * @fn      app_zclViewGroupRspCmdHandler
 *
 * @brief   Handler for ZCL view group response command.
 *
 * @param   pViewGroupRsp
 *
 * @return  None
 */
static void app_zclViewGroupRspCmdHandler(zcl_viewGroupRsp_t *pViewGroupRsp)
{

}

/*********************************************************************
 * @fn      app_zclRemoveGroupRspCmdHandler
 *
 * @brief   Handler for ZCL remove group response command.
 *
 * @param   pRemoveGroupRsp
 *
 * @return  None
 */
static void app_zclRemoveGroupRspCmdHandler(zcl_removeGroupRsp_t *pRemoveGroupRsp)
{

}

/*********************************************************************
 * @fn      app_zclGetGroupMembershipRspCmdHandler
 *
 * @brief   Handler for ZCL get group membership response command.
 *
 * @param   pGetGroupMembershipRsp
 *
 * @return  None
 */
static void app_zclGetGroupMembershipRspCmdHandler(zcl_getGroupMembershipRsp_t *pGetGroupMembershipRsp)
{

}

/*********************************************************************
 * @fn      app_groupCb
 *
 * @brief   Handler for ZCL Group command.
 *
 * @param   pAddrInfo
 * @param   cmdId
 * @param   cmdPayload
 *
 * @return  status_t
 */
status_t app_groupCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload) {

//    printf("app_groupCb. ep: %d\r\n", pAddrInfo->dstEp);

	if(pAddrInfo->dstEp == APP_ENDPOINT1) {
		if(pAddrInfo->dirCluster == ZCL_FRAME_SERVER_CLIENT_DIR){
			switch(cmdId){
				case ZCL_CMD_GROUP_ADD_GROUP_RSP:
					app_zclAddGroupRspCmdHandler(pAddrInfo->dstEp, (zcl_addGroupRsp_t *)cmdPayload);
					break;
				case ZCL_CMD_GROUP_VIEW_GROUP_RSP:
					app_zclViewGroupRspCmdHandler((zcl_viewGroupRsp_t *)cmdPayload);
					break;
				case ZCL_CMD_GROUP_REMOVE_GROUP_RSP:
					app_zclRemoveGroupRspCmdHandler((zcl_removeGroupRsp_t *)cmdPayload);
					break;
				case ZCL_CMD_GROUP_GET_MEMBERSHIP_RSP:
					app_zclGetGroupMembershipRspCmdHandler((zcl_getGroupMembershipRsp_t *)cmdPayload);
					break;
				default:
					break;
			}
		}
	}

	return ZCL_STA_SUCCESS;
}

static void app_zclAddSceneRspCmdHandler(u8 cmdId, addSceneRsp_t *pAddSceneRsp)
{

}

/*********************************************************************
 * @fn      app_zclViewSceneRspCmdHandler
 *
 * @brief   Handler for ZCL view scene response command.
 *
 * @param   cmdId
 * @param   pViewSceneRsp
 *
 * @return  None
 */
static void app_zclViewSceneRspCmdHandler(u8 cmdId, viewSceneRsp_t *pViewSceneRsp)
{

}

/*********************************************************************
 * @fn      app_zclRemoveSceneRspCmdHandler
 *
 * @brief   Handler for ZCL remove scene response command.
 *
 * @param   pRemoveSceneRsp
 *
 * @return  None
 */
static void app_zclRemoveSceneRspCmdHandler(removeSceneRsp_t *pRemoveSceneRsp)
{

}

/*********************************************************************
 * @fn      app_zclRemoveAllSceneRspCmdHandler
 *
 * @brief   Handler for ZCL remove all scene response command.
 *
 * @param   pRemoveAllSceneRsp
 *
 * @return  None
 */
static void app_zclRemoveAllSceneRspCmdHandler(removeAllSceneRsp_t *pRemoveAllSceneRsp)
{

}

/*********************************************************************
 * @fn      app_zclStoreSceneRspCmdHandler
 *
 * @brief   Handler for ZCL store scene response command.
 *
 * @param   pStoreSceneRsp
 *
 * @return  None
 */
static void app_zclStoreSceneRspCmdHandler(storeSceneRsp_t *pStoreSceneRsp)
{

}

/*********************************************************************
 * @fn      app_zclGetSceneMembershipRspCmdHandler
 *
 * @brief   Handler for ZCL get scene membership response command.
 *
 * @param   pGetSceneMembershipRsp
 *
 * @return  None
 */
static void app_zclGetSceneMembershipRspCmdHandler(getSceneMemRsp_t *pGetSceneMembershipRsp)
{

}

/*********************************************************************
 * @fn      app_sceneCb
 *
 * @brief   Handler for ZCL Scene command.
 *
 * @param   pAddrInfo
 * @param   cmdId
 * @param   cmdPayload
 *
 * @return  status_t
 */
status_t app_sceneCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload)
{
	if(pAddrInfo->dstEp == APP_ENDPOINT1){
		if(pAddrInfo->dirCluster == ZCL_FRAME_SERVER_CLIENT_DIR){
			switch(cmdId){
				case ZCL_CMD_SCENE_ADD_SCENE_RSP:
				case ZCL_CMD_SCENE_ENHANCED_ADD_SCENE_RSP:
					app_zclAddSceneRspCmdHandler(cmdId, (addSceneRsp_t *)cmdPayload);
					break;
				case ZCL_CMD_SCENE_VIEW_SCENE_RSP:
				case ZCL_CMD_SCENE_ENHANCED_VIEW_SCENE_RSP:
					app_zclViewSceneRspCmdHandler(cmdId, (viewSceneRsp_t *)cmdPayload);
					break;
				case ZCL_CMD_SCENE_REMOVE_SCENE_RSP:
					app_zclRemoveSceneRspCmdHandler((removeSceneRsp_t *)cmdPayload);
					break;
				case ZCL_CMD_SCENE_REMOVE_ALL_SCENE_RSP:
					app_zclRemoveAllSceneRspCmdHandler((removeAllSceneRsp_t *)cmdPayload);
					break;
				case ZCL_CMD_SCENE_STORE_SCENE_RSP:
					app_zclStoreSceneRspCmdHandler((storeSceneRsp_t *)cmdPayload);
					break;
				case ZCL_CMD_SCENE_GET_SCENE_MEMSHIP_RSP:
					app_zclGetSceneMembershipRspCmdHandler((getSceneMemRsp_t *)cmdPayload);
					break;
				default:
					break;
			}
		}
	}

	return ZCL_STA_SUCCESS;
}

status_t app_co2Cb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload) {
//    printf("app_co2Cb(). pAddrInfo->dirCluster: %0x%x, cmdId: 0x%x\r\n", pAddrInfo->dirCluster, cmdId);

  status_t status = ZCL_STA_SUCCESS;

  return status;
}

/*********************************************************************
 * @fn      app_temperatureCb
 *
 * @brief   Handler for ZCL Temperature command.
 *
 * @param   pAddrInfo
 * @param   cmdId
 * @param   cmdPayload
 *
 * @return  status_t
 */
status_t app_temperatureCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload) {

//    printf("app_tepmeratureCb. cmd: 0x%x\r\n", cmdId);

    return ZCL_STA_SUCCESS;
}


/*********************************************************************
 * @fn      app_humidityCb
 *
 * @brief   Handler for ZCL Humidity command.
 *
 * @param   pAddrInfo
 * @param   cmdId
 * @param   cmdPayload
 *
 * @return  status_t
 */
status_t app_humidityCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload) {

//    printf("app_humidityCb. cmd: 0x%x\r\n", cmdId);

    return ZCL_STA_SUCCESS;
}

