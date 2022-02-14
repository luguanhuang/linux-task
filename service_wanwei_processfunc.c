

#include "../util/service_global.h"
#include "../include/ydt_log.h"
#include "../interface/service_protocol.h"


#ifdef _WANWEI_PUSH_SERVICE_
	#include "../service/wanwei_service/service_wanwei_activity_query.h"
	#include "../service/wanwei_service/service_wanwei_del_pushtask.h"
	#include "../service/wanwei_service/service_wanwei_task_push.h"
	#include "../service/wanwei_service/service_wanwei_interest_activity_query.h"
	#include "../service/wanwei_service/service_report_info.h"
#endif

#ifdef _WANWEI_QUERY_SERVICE_
#include "../service/wanwei_service/service_wanwei_report_gps.h"
#endif

#ifdef _WANWEI_LOGIN_SERVICE_
#include "../service/wanwei_service/service_wanwei_phonelogin.h"
#endif

#include "../service/handle_service/service_handle_service.h"
#include "service_wanwei_processfunc.h"

static StProcFunc_MapTable arrWanwei_procfunc_maptable[] = 
{
	#ifdef _WANWEI_LOGIN_SERVICE_
	{WANWEI_PHONE_LOGIN, HandleWanweiLoginMsg},
	#endif
	#ifdef _WANWEI_QUERY_SERVICE_
	{WANWEI_REPORT_GPS_INFO, HandleReportGPSMsg},
	#endif
	#ifdef _WANWEI_PUSH_SERVICE_
	{WANWEI_ACTIVITY_QUERY, HandleActivityQueryMsg},
	{DEL_PUSH_TASK, HandleDelPushTaskMsg},
	{TASK_PUSH_QUERY, HandleTaskPushMsg},
	{WANWEI_INTEREST_ACTIVITY_QUERY, HandleInterestActivityQueryMsg},
	{WANWEI_REPORT_INFO, S_HandleReportInfoMsg},
	{WANWEI_REPORT_BUSI_INFO, S_HandleReportBusiInfoMsg},
	{WANWEI_PHONE_LOGIN, S_HandleWanweiLoginMsg},
	{WANWEI_CONFIRM_SIGNIN, S_HandleConfirmSigninMsg}
	#endif
};


int WanweiProcessFunc(WORD wCmd_id, Client_Server_Msg *pClient_srv_msg, StSequence_Queue *pQueue, pthread_mutex_t *pMutex, pthread_cond_t *pCond)
{
	int nRet = 0;
	INFO("WanweiProcessFunc: func begin%s", "");
	DEBUG("WanweiProcessFunc: [command id]=%d", wCmd_id);
	char szLog[1024] = {0};
	int nSize = sizeof(arrWanwei_procfunc_maptable) / sizeof(arrWanwei_procfunc_maptable[0]);
	int i = 0;
	
	for (i=0; i<nSize; i++)
	{
		if (wCmd_id == arrWanwei_procfunc_maptable[i].wCmd_id)
		{
			nRet = arrWanwei_procfunc_maptable[i].pProcessFunc(pClient_srv_msg, pQueue, pMutex, pCond);
			break;
		}
	}
	
	if (i == nSize)
	{
		WARN("WanweiProcessFunc: we can't find the wanwei process func%s", "");	
		return FALSE;
	}

	if (TRUE != nRet)
	{
		snprintf(szLog, sizeof(szLog) - 1, "WanweiProcessFunc: Call WanweiProcessFunc error [command id]=%d [return value]=%d", \
			wCmd_id, nRet);
		ERROR("%s", szLog);
	}

	INFO("WanweiProcessFunc: func end%s", "");
	return nRet;
}


