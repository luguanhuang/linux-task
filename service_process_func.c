

#include "../util/service_global.h"
#include "../include/ydt_log.h"
#include "../interface/service_protocol.h"

#ifdef _IM_SERVICE_
#include "../service/chat_service/service_account_register.h"
#include "../service/chat_service/service_add_friendgroup.h"
#include "../service/chat_service/service_addcluster_member.h"
#include "../service/chat_service/service_addfriend.h"
#include "../service/chat_service/service_agreeand_addfriend.h"
#include "../service/chat_service/service_application_exit.h"
#include "../service/chat_service/service_apply_notlogin_data.h"
#include "../service/chat_service/service_change_image.h"
#include "../service/chat_service/service_cluster.h"
#include "../service/chat_service/service_cluster_list.h"

#include "../service/chat_service/service_cluster_member.h"
#include "../service/chat_service/service_create_cluster.h"
#include "../service/chat_service/service_delcluster_member.h"
#include "../service/chat_service/service_delete_friend.h"
#include "../service/chat_service/service_delete_friendgroup.h"
#include "../service/chat_service/service_dismiss_cluster.h"
#include "../service/chat_service/service_exit_cluster.h"
#include "../service/chat_service/service_friendgroup.h"
#include "../service/chat_service/service_friendlist.h"
#include "../service/chat_service/service_heartbeat_detect.h"
#include "../service/chat_service/service_login.h"
#include "../service/chat_service/service_tattedcode.h"
#include "../service/chat_service/service_userID_register.h"
#include "../service/chat_service/service_usrid_login.h"
#endif

#ifdef _LIANTONG_SERVICE_
//联通 业务头文件定义

#include "../service/liantong_service/service_android_register.h"
#include "../service/liantong_service/service_iphone_register.h"
#include "../service/liantong_service/service_check_android_softver.h"
#include "../service/liantong_service/service_report_android_info.h"
#include "../service/liantong_service/service_report_iphone_info.h"
#include "../service/liantong_service/service_task_send_query.h"
#include "../service/liantong_service/service_report_software_update.h"
#include "../service/liantong_service/service_task_touch_record.h"
#include "../service/liantong_service/service_client_feedback.h"
#include "../service/liantong_service/service_delete_tasks_record.h"
#include "../service/liantong_service/service_charge_types.h"
#include "../service/liantong_service/service_record_charge_info.h"
#include "../service/liantong_service/service_get_image.h"
#include "../service/liantong_service/service_phone_login.h" 
#include "../service/liantong_service/service_req_hall_info.h" 
#include "../service/handle_service/service_handle_service.h"
#endif

//公共的业务
#include "../service/common_service/service_record_accesssrv_info.h"


#include "service_process_func.h"


#ifdef _IM_SERVICE_
static StProcFunc_MapTable arrChat_procfunc_maptable[] = 
{
	{APPLY_TATTEDCODE_MSG, HandleTattedCodeMsg},
	{LOGIN_MSG, HandleLoginMsg},
	{FRI_GROUP_MSG, HandleFriGroupMsg},
	{FRI_LIST_MSG, HandleFriListMsg},
	{USRID_REGISTER_MSG, HandleUsrRegisMsg},
	{ADD_FRIEND_GROUP_MSG, HandleAddFriendGroupMsg},
	{DEL_FRIEND_GROUP_MSG, HandleDelFriendGroupMsg},
	{ADD_FRINED_MSG, HandleAddFriendMsg},
	{DEL_FRIEND_MSG, HandleDelFriendMsg},
	{CREATE_CLUSTER, HandleCreateClusterMsg},
	{CLUSTER_LIST, HandleClusterListMsg},
	{CLUSTER_MEMBER_LIST, HandleClusterMemberMsg},
	{ADD_CLUSTER_MEMBER, HandleAddClusterMemberMsg},
	{DELETE_CLUSTER_MEMBER, HandleDelClusterMemberMsg},
	{DISMISS_CLUSTER, HandleDismissClusterMsg},
	{EXIT_CLUSTER, HandleExitClusterMsg},
	{CHANGE_IMAGE, HandleChangeImageMsg},
	{USRID_LOGIN_MSG, HandleUsrIdLoginMsg},
	{APPLICATION_EXIT, HandleApplicationExitMsg}
};
#endif

#ifdef _LIANTONG_SERVICE_
static StProcFunc_MapTable arrLiantong_procfunc_maptable[] = 
{
	{ANDROID_REGISTER_MSG, HandleAndriodRegisterMsg},
	{IPHONE_REGISTER_MSG, HandleIPhoneRegisterMsg},
	{REPORT_ANDROID_PHONEINFO, HandleReportAndroidInfoMsg},
	{PHONE_LOGIN_MSG, HandlePhoneLoginMsg},
	{REPORT_IPHONE_PHONEINFO, HandleReportIPhoneInfoMsg},
	{CHECK_ANDROID_VERSION, HandleCheckAndroidSoftVerMsg},
	{REPORT_VERSION_UPDATE, HandleReportSoftwareUpdateMsg},
	{TASKS_SEND_QUERY, HandleTasksSendQueryMsg},
	{CLIENT_FEEDBACK, HandleClientFeedback},
	{TASK_TOUCH_RECORD, HandleDiffTaskTouchRecordMsg},
	{DEL_TASK_RECORD, HandleDeleteTaskRecordMsg},
	{GET_CHARGE_TYPE, HandleGetChargeTypesMsg},
	{RECORD_CHARGE_INFO, HandleRecordClientChargeInfoMsg},
	{GET_IMAGE, HandleGetImageMsg},
	{REPORTIPHONE_MOREINFO, S_HandleReportIPhoneMoreInfoMsg},
	{BIND_UUID_SOCKETID, S_HandleBindUuidAndSocketIDMsg},
	{HEARTBEAT_DEC_PACKET, S_HandleHeartBeatDetMsg},
	{NEW_TASK_PUSH, S_HandleNewTasksSendMsg},
	{TAG_CLICK_RECORD, S_HandleTagClickRecordMsg},
	{USRSHARE_CLICKRECORD, S_HandleUsrShareClickRecordMsg},
	{RECORD_MALFUNC_REPORT, S_HandleRecordMalfuncReportMsg}
};

#endif

static StProcFunc_MapTable arrDef_procfunc_maptable[] = 
{
	{AFTER_ACEESS_SRV_CONN, HandleRecordAccessSrvInfoMsg}
};

int YDTProcessFunc(WORD wCmd_id, Client_Server_Msg *pClient_srv_msg, StSequence_Queue *pQueue, pthread_mutex_t *pMutex, pthread_cond_t *pCond)
{
	int nRet = 0;
	#ifdef _IM_SERVICE_
	INFO("YDTProcessFunc: func begin%s", "");
	DEBUG("YDTProcessFunc: [command id]=%d", wCmd_id);
	int nSize = sizeof(arrChat_procfunc_maptable)/ sizeof(arrChat_procfunc_maptable[0]);
	int i = 0;
	char szLog[1024] = {0};
	
	for (i=0; i<nSize; i++)
	{
		if (wCmd_id == arrChat_procfunc_maptable[i].wCmd_id)
		{
			nRet = arrChat_procfunc_maptable[i].pProcessFunc(pClient_srv_msg, pQueue, pMutex, pCond);
			break;
		}
	}

	if (i == nSize)
	{
		WARN("YDTProcessFunc: we can't find the chat process func%s", "");	
		return FALSE;
	}

	if (TRUE != nRet)
	{
		snprintf(szLog, sizeof(szLog) - 1, "LTProcessFunc: Call arrChat_procfunc_maptable.pProcessFunc error cmd_id=%d", wCmd_id);
		ERROR("%s", szLog);
	}
	
	INFO("YDTProcessFunc: func end%s", "");
	#endif
	return nRet;
}

int LTProcessFunc(WORD wCmd_id, Client_Server_Msg *pClient_srv_msg, StSequence_Queue *pQueue, pthread_mutex_t *pMutex, pthread_cond_t *pCond)
{
	int nRet = 0;
	#ifdef _LIANTONG_SERVICE_
	INFO("LTProcessFunc: func begin%s", "");
	DEBUG("LTProcessFunc: [command id]=%d [req hall info]=%d", \
		wCmd_id, REQ_HALL_INFO);
	char szLog[1024] = {0};
	int nSize = sizeof(arrLiantong_procfunc_maptable) / sizeof(arrLiantong_procfunc_maptable[0]);
	int i = 0;

	if (REQ_HALL_INFO == wCmd_id)
	{
		CReqHallInfo reqHallInfo;
		reqHallInfo.ProReqHallInfoMsg(pClient_srv_msg, pQueue, pMutex, pCond);
	}
	else
	{
		for (i=0; i<nSize; i++)
		{
			if (wCmd_id == arrLiantong_procfunc_maptable[i].wCmd_id)
			{
				nRet = arrLiantong_procfunc_maptable[i].pProcessFunc(pClient_srv_msg, pQueue, pMutex, pCond);
				break;
			}
		}		
	}
	
	if (i == nSize)
	{
		WARN("LTProcessFunc: we can't find the liantong process func%s", "");	
		return FALSE;
	}

	if (TRUE != nRet)
	{
		snprintf(szLog, sizeof(szLog) - 1, "LTProcessFunc: Call LTProcessFunc error [command id]=%d [return value]=%d", \
			wCmd_id, nRet);
		ERROR("%s", szLog);
	}

	INFO("LTProcessFunc: func end%s", "");
	#endif
	return nRet;
}

int DefProcessFunc(WORD wCmd_id, Client_Server_Msg *pClient_srv_msg, StSequence_Queue *pQueue, pthread_mutex_t *pMutex, pthread_cond_t *pCond)
{
	INFO("DefProcessFunc: func begin%s", "");
	int nRet = 0;
	DEBUG("DefProcessFunc: [command id]=%d", wCmd_id);
	char szLog[1024] = {0};
	int nSize = sizeof(arrDef_procfunc_maptable) / sizeof(arrDef_procfunc_maptable[0]);
	int i = 0;
	
	for (i=0; i<nSize; i++)
	{
		if (wCmd_id == arrDef_procfunc_maptable[i].wCmd_id)
		{
			nRet = arrDef_procfunc_maptable[i].pProcessFunc(pClient_srv_msg, pQueue, pMutex, pCond);
			break;
		}
	}
	
	if (i == nSize)
	{
		WARN("DefProcessFunc: we can't find the default process func%s", "");	
		return FALSE;
	}

	if (TRUE != nRet)
	{
		snprintf(szLog, sizeof(szLog) - 1, "DefProcessFunc: Call LTProcessFunc error [command id]=%d [return value]=%d", \
			wCmd_id, nRet);
		ERROR("%s", szLog);
	}

	INFO("DefProcessFunc: func end%s", "");
	return nRet;
}

