
#include "../../util/service_global.h"
#include "../../interface/service_protocol.h"
#include "../wanwei_table/service_table_usr_connstatus.h"
#include "../wanwei_table/service_table_usrgps_log.h"
#include "../wanwei_table/service_table_busiarea.h"
#include "../wanwei_table/service_table_userinfo.h"
#include "../wanwei_table/service_table_task_a.h"
#include "../wanwei_table/service_table_businessinfo.h" 
#include "../wanwei_table/service_table_task_b.h" 
#include "service_handle_wanweitable.h"

#if defined(_WANWEI_LOGIN_SERVICE_) || defined(_WANWEI_PUSH_SERVICE_)
int H_HandlePhoneLogin(StPhone_LoginInfo *pPhone_logininfo)
{
	INFO("H_HandlePhoneLogin: func begin%s", "");
	if (NULL == pPhone_logininfo)
	{
		ERROR("H_HandlePhoneLogin: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = HandlePhoneLogin(pPhone_logininfo);
	DEBUG("H_HandlePhoneLogin: [return value]=%d", nRet);
	INFO("H_HandlePhoneLogin: func end%s", "");
	return nRet;
}
#endif


#ifdef _WANWEI_QUERY_SERVICE_
int H_InsertUsrGPSInfo(StUsr_GPS_Info *pUsr_gps_info)
{
	INFO("H_InsertUsrGPSInfo: func begin%s", "");
	if (NULL == pUsr_gps_info)
	{
		ERROR("H_InsertUsrGPSInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = InsertUsrGPSInfo(pUsr_gps_info);
	DEBUG("H_InsertUsrGPSInfo: [return value]=%d", nRet);
	INFO("H_InsertUsrGPSInfo: func end%s", "");
	return nRet;
}

int H_GetBusinessAreasInfo(void)
{
	INFO("H_GetBusinessAreasInfo: func begin%s", "");
	int nRet = GetBusinessAreasInfo();
	DEBUG("H_GetBusinessAreasInfo: [return value]=%d", nRet);
	INFO("H_GetBusinessAreasInfo: func end%s", "");
	return nRet;
}

int H_GetBusiAreaIDs(int nUUID, int **ppID, int *pNum)
{
	INFO("H_GetBusiAreaIDs: func begin%s", "");
	if (NULL == pNum)
	{
		ERROR("H_GetBusiAreaIDs: func param error%s", "");
		return FUNC_PARAM_ERROR;
		
	}
	
	int nRet = GetBusiAreaIDs(nUUID, ppID, pNum);
	DEBUG("H_GetBusiAreaIDs: [return value]=%d", nRet);
	INFO("H_GetBusiAreaIDs: func end%s", "");
	return nRet;
}


#endif

#ifdef _WANWEI_PUSH_SERVICE_ 
int H_TranslateBusiActivityTask(StActivity_Query_Info *pActivity_query_info, StBusi_Activity_PushData *pBusi_activity_pushdata)
{
	INFO("H_TranslateBusiActivityTask: func begin%s", "");
	if (NULL == pActivity_query_info || NULL == pBusi_activity_pushdata)
	{
		ERROR("H_TranslateBusiActivityTask: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = TranslateBusiActivityTask(pActivity_query_info, pBusi_activity_pushdata);
	DEBUG("H_TranslateBusiActivityTask: [return value]=%d", nRet);
	INFO("H_TranslateBusiActivityTask: func end%s", "");
	return nRet;
}

int H_GetBusiActivityInfo(StActivity_Query_Info *pActivity_query_info, int nActivity_id, StBusi_Activity_Info *pBusi_activity)
{
	INFO("H_GetBusiActivityInfo: func begin%s", "");
	if (NULL == pActivity_query_info || NULL == pBusi_activity)
	{
		ERROR("H_GetBusiActivityInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = GetBusiActivityInfo(pActivity_query_info, nActivity_id, pBusi_activity);
	DEBUG("H_GetBusiActivityInfo: [return value]=%d", nRet);
	INFO("H_GetBusiActivityInfo: func end%s", "");
	return nRet;
}

int H_GetBusiName(int nBusi_id, char *pBusi_name, int nLen, int nCity_id)
{
	INFO("H_GetBusiName: func begin%s", "");
	if (NULL == pBusi_name)
	{
		ERROR("H_GetBusiName: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = GetBusiName(nBusi_id, pBusi_name, nLen, nCity_id);
	DEBUG("H_GetBusiName: [return value]=%d", nRet);
	INFO("H_GetBusiName: func end%s", "");
	return nRet;
}

int H_InsertBusiActivityPushData(StBusi_Activity_Data *pActivity_data)
{
	INFO("H_InsertBusiActivityPushData: func begin%s", "");
	if (NULL == pActivity_data)
	{
		ERROR("H_InsertBusiActivityPushData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = InsertBusiActivityPushData(pActivity_data);
	DEBUG("H_InsertBusiActivityPushData: [return value]=%d", nRet);
	INFO("H_InsertBusiActivityPushData: func end%s", "");
	return nRet;
}

int H_DelPushTask(StDel_PushTask *pDel_pushtask)
{
	INFO("H_DelPushTask: func begin%s", "");
	if (NULL == pDel_pushtask)
	{
		ERROR("H_DelPushTask: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = DelPushTask(pDel_pushtask);
	DEBUG("H_DelPushTask: [return value]=%d", nRet);
	INFO("H_DelPushTask: func end%s", "");
	return nRet;
}

int H_GetTaskPushData(int nUUID, StPush_Task_Data *pPush_task_data)
{
	INFO("H_GetTaskPushData: func begin%s", "");
	if (NULL == pPush_task_data)
	{
		ERROR("H_GetTaskPushData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = GetTaskPushData(nUUID, pPush_task_data);
	DEBUG("H_GetTaskPushData: [return value]=%d", nRet);
	INFO("H_GetTaskPushData: func end%s", "");
	return nRet;
}

int H_GetUsrFollowBusiID(int nUUID, StFollow_Busi_Info *pFollow_busi_info)
{
	INFO("H_GetUsrFollowBusiID: func begin%s", "");
	if (NULL == pFollow_busi_info)
	{
		ERROR("H_GetUsrFollowBusiID: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = GetUsrFollowBusiID(nUUID, pFollow_busi_info);
	DEBUG("H_GetUsrFollowBusiID: [return value]=%d", nRet);
	INFO("H_GetUsrFollowBusiID: func end%s", "");
	return nRet;
}

int H_GetInterestBusiActivityInfo(StActivity_Query_Info *pActivity_query_info, int nActivity_id, StBusi_Activity_Info *pBusi_activity, StFollow_Busi_Info *pFollow_busi_info)
{
	INFO("H_GetInterestBusiActivityInfo: func begin%s", "");
	if (NULL == pActivity_query_info || NULL == pBusi_activity || NULL == pFollow_busi_info)
	{
		ERROR("H_GetInterestBusiActivityInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = GetInterestBusiActivityInfo(pActivity_query_info, nActivity_id, pBusi_activity, pFollow_busi_info);
	DEBUG("H_GetInterestBusiActivityInfo: [return value]=%d", nRet);
	INFO("H_GetInterestBusiActivityInfo: func end%s", "");
	return nRet;
}

int H_TranslateInterestBusiActivityTask(StActivity_Query_Info *pActivity_query_info, StBusi_Activity_PushData *pBusi_activity_pushdata)
{
	INFO("H_TranslateInterestBusiActivityTask: func begin%s", "");
	if (NULL == pActivity_query_info || NULL == pBusi_activity_pushdata)
	{
		ERROR("H_TranslateInterestBusiActivityTask: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = TranslateInterestBusiActivityTask(pActivity_query_info, pBusi_activity_pushdata);
	DEBUG("H_TranslateInterestBusiActivityTask: [return value]=%d", nRet);
	INFO("H_TranslateInterestBusiActivityTask: func end%s", "");
	return nRet;
}

int H_InsertTouchInfo(StTouch_Info *pTouch_info)
{
	INFO("H_InsertTouchInfo: func begin%s", "");
	if (NULL == pTouch_info)
	{
		ERROR("H_InsertTouchInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = InsertTouchInfo(pTouch_info);
	DEBUG("H_InsertTouchInfo: [return value]=%d", nRet);
	INFO("H_InsertTouchInfo: func end%s", "");
	return nRet;
}

int H_GetTouchInfo(StBusi_Touch_Info *pBusi_touch_info, StTouch_Match_Data *pTouch_match_data)
{
	INFO("H_GetTouchInfo: func begin%s", "");
	if (NULL == pBusi_touch_info || NULL == pTouch_match_data)
	{
		ERROR("H_GetTouchInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = GetTouchInfo(pBusi_touch_info, pTouch_match_data);
	DEBUG("H_GetTouchInfo: [return value]=%d", nRet);
	INFO("H_GetTouchInfo: func end%s", "");
	return nRet;
}

int H_GetBusiTouchInfo(StTouch_Info *pTouch_info, StTouch_Match_Data *pTouch_match_data)
{
	INFO("H_GetBusiTouchInfo: func begin%s", "");
	if (NULL == pTouch_info || NULL == pTouch_match_data)
	{
		ERROR("H_GetBusiTouchInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = GetBusiTouchInfo(pTouch_info, pTouch_match_data);
	DEBUG("H_GetBusiTouchInfo: [return value]=%d", nRet);
	INFO("H_GetBusiTouchInfo: func end%s", "");
	return nRet;
}

int H_GetUsrConnInfo(int *pUUID, int nUUID_count, StUsr_login_info *pUsr_login_info, int *pConn_cnt)
{
	INFO("H_GetUsrConnInfo: func begin%s", "");
	if (NULL == pUUID || nUUID_count <= 0 || NULL == pUsr_login_info || NULL == pConn_cnt)
	{
		ERROR("H_GetUsrConnInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = GetUsrConnInfo(pUUID, nUUID_count, pUsr_login_info, pConn_cnt);
	DEBUG("H_GetUsrConnInfo: [return value]=%d", nRet);
	INFO("H_GetUsrConnInfo: func end%s", "");
	return nRet;
}

int H_InsertBusiTouchInfo(StBusi_Touch_Info *pBusi_touch_info)
{
	INFO("H_InsertBusiTouchInfo: func begin%s", "");
	if (NULL == pBusi_touch_info)
	{
		ERROR("H_InsertBusiTouchInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = InsertBusiTouchInfo(pBusi_touch_info);
	DEBUG("H_InsertBusiTouchInfo: [return value]=%d", nRet);
	INFO("H_InsertBusiTouchInfo: func end%s", "");
	return nRet;
}

int H_GetConnInfo(int nUUID, StUsr_login_info *pUsr_login_info, int *pCount)
{
	INFO("H_GetConnInfo: func  begin%s", "");
	if (NULL == pUsr_login_info || NULL == pCount)
	{
		ERROR("H_GetConnInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = GetConnInfo(nUUID, pUsr_login_info, pCount);
	DEBUG("H_GetConnInfo: [return value]=%d", nRet);
	INFO("H_GetConnInfo: func end%s", "");
	return nRet;
}



#endif

