#include "../../util/service_global.h"
#include "../../interface/service_protocol.h"

#ifdef _LIANTONG_SERVICE_
//联通定义文件头
#include "../liantong_table/service_table_phone_register.h"
#include "../liantong_table/service_table_Client_update.h"
#include "../liantong_table/service_table_Client_updaterecord.h"
#include "../liantong_table/service_table_task_b.h"
#include "../liantong_table/service_table_client_feedback.h"
#include "../liantong_table/service_table__task_touchrecord.h"
#include "../liantong_table/service_table_phone_register_view.h"
#include "../liantong_table/service_table_charge_manager.h"
#include "../liantong_table/service_table_charge_info.h"
#include "../liantong_table/service_table_system_tip.h"
#include "../liantong_table/service_table_user_connstatus.h"
#include "../liantong_table/service_table_snd_task_a.h"
#include "../liantong_table/service_sub_task_b.h"
#include "../liantong_table/service_table_new_task_b.h"
#include "../liantong_table/service_table_tagclick_record.h"
#include "../liantong_table/service_table_blacklist_info.h"
#include "../liantong_table/service_table_usershare_log.h"
#include "../liantong_table/service_table_task_apple.h"
#include "../liantong_table/service_table_test_conn.h"
#endif

#include "../common_table/service_table_server_numrange.h"
#include "service_handle_liantongtable.h"


/*********************此文件用来处理对数据库表的所有操作*************************/

#ifdef _LIANTONG_SERVICE_
/*********以下操作联通业务数据库表**********/
int H_BindUUIDAndPhoneNum(int nUUID, char *pPhone_num)
{
	INFO("H_BindUUIDAndPhoneNum: func begin%s", "");
	int nRet = BindUUIDAndPhoneNum(nUUID, pPhone_num);
	DEBUG("H_BindUUIDAndPhoneNum: [return value]=%d", nRet);
	INFO("H_BindUUIDAndPhoneNum: func end%s", "");
	return nRet;
}


int H_ReportAndroidInfo(StReport_Android_Info *pReport_android_info)
{
	INFO("H_ReportAndroidInfo: func begin%s", "");
	int nRet = ReportAndroidInfo(pReport_android_info);
	DEBUG("H_ReportAndroidInfo: [return value]=%d", nRet);
	INFO("H_ReportAndroidInfo: func end%s", "");
	return nRet;
}

int H_ReportIPhoneInfo(int nUUID, char *pPhone_type, char *pOS_version)
{
	INFO("H_ReportIPhoneInfo: func begin%s", "");
	int nRet = ReportIPhoneInfo(nUUID, pPhone_type, pOS_version);
	DEBUG("H_ReportIPhoneInfo: [return value]=%d", nRet);
	INFO("H_ReportIPhoneInfo: func end%s", "");
	return nRet;
}


int H_CheckAndroidSoftVersion(int nUUID, char *pVersion, BYTE *pUpdate_status, StSoftware_UpdateInfo **ppSoft_updateinfo)
{
	INFO("H_CheckAndroidSoftVersion: func begin%s", "");
	int nRet = CheckAndroidSoftVersion(nUUID, pVersion, pUpdate_status, ppSoft_updateinfo);
	DEBUG("H_CheckAndroidSoftVersion: [return value]=%d", nRet);
	INFO("H_CheckAndroidSoftVersion: func end%s", "");
	return nRet;
}


int H_RecordClientUpdateInfo(StSoftware_update_record *pUpdate_record)
{
	INFO("H_RecordClientUpdateInfo: func begin%s", "");
	int nRet = RecordClientUpdateInfo(pUpdate_record);
	DEBUG("H_RecordClientUpdateInfo: [return value]=%d", nRet);
	INFO("H_RecordClientUpdateInfo: func end%s", "");
	return nRet;
}

int H_GetTaskSendQueryData(int nUUID, StTask_Send_Query_Data *pTask_snd_query_data, int *pTask_num)
{
	INFO("H_GetTaskSendQueryData: func begin%s", "");
	int nRet = GetTaskSendQueryData(nUUID, pTask_snd_query_data, pTask_num);
	DEBUG("H_GetTaskSendQueryData: [return value]=%d", nRet);
	INFO("H_GetTaskSendQueryData: func end%s", "");
	return nRet;
}



int H_InsertClientFeedbackInfo(StClient_Feedback *pClient_feedback)
{
	INFO("H_InsertClientFeedbackInfo: func begin%s", "");
	int nRet = InsertClientFeedbackInfo(pClient_feedback);
	DEBUG("H_InsertClientFeedbackInfo: [return value]=%d", nRet);
	INFO("H_InsertClientFeedbackInfo: func end%s", "");
	return nRet;
};



int H_InsertTaskTouchRecordInfo(int nUUID, char *pPhone_num, int nTask_id, BYTE bTask_type)
{
	INFO("H_InsertTaskTouchRecordInfo: func begin%s", "");
	int nRet = InsertTaskTouchRecordInfo(nUUID, pPhone_num, nTask_id, bTask_type);
	DEBUG("H_InsertTaskTouchRecordInfo: [return value]=%d", nRet);
	INFO("H_InsertTaskTouchRecordInfo: func end%s", "");
	return nRet;
}

int H_InsertNewTaskTouchRecordInfo(StTask_Click_Info *pTask_click_info)
{
	INFO("H_InsertNewTaskTouchRecordInfo: func begin%s", "");
	if (NULL == pTask_click_info)
	{
		ERROR("H_InsertNewTaskTouchRecordInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = InsertNewTaskTouchRecordInfo(pTask_click_info);
	DEBUG("H_InsertNewTaskTouchRecordInfo: [return value]=%d", nRet);
	INFO("H_InsertNewTaskTouchRecordInfo: func end%s", "");
	return nRet;
}

int H_DelTasksSndRecord(int nUUID, char *pTime)
{
	INFO("H_DelTasksSndRecord: func begin%s", "");
	int nRet = DelTasksSndRecord(nUUID, pTime);
	DEBUG("H_DelTasksSndRecord: [return value]=%d", nRet);
	INFO("H_DelTasksSndRecord: func end%s", "");
	return nRet;
}

int H_IsUUIDExist(int nUUID)
{
	INFO("H_IsUUIDExist: func begin%s", "");
	int nRet = IsUUIDExist(nUUID);
	DEBUG("H_IsUUIDExist: [return value]=%d", nRet);
	INFO("H_IsUUIDExist: func end%s", "");
	return nRet;
}

int H_InsertPhoneRegisterInfo(EnPhoneRegisterType enRegis_type, char *pData, int nUUID)
{
	INFO("H_InsertPhoneRegisterInfo: func begin%s", "");
	int nRet = InsertPhoneRegisterInfo(enRegis_type, pData, nUUID);
	DEBUG("H_InsertPhoneRegisterInfo: [return value]=%d", nRet);	
	INFO("H_InsertPhoneRegisterInfo: func end%s", "");
	return nRet;
}

int H_GetChargeTypes(StCharge_Type **ppCharge_type, int *pCharge_type_num)
{
	INFO("H_GetChargeTypes: func begin%s", "");
	if (NULL == ppCharge_type || NULL == pCharge_type_num)
	{
		ERROR("H_GetChargeTypes: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = GetChargeTypes(ppCharge_type, pCharge_type_num);
	DEBUG("H_GetChargeTypes: [return value]=%d", nRet);
	INFO("H_GetChargeTypes: func end%s", "");
	return nRet;
}

int H_InsertClientChargeInfo(StCharge_Info *pCharge_info)
{
	INFO("H_InsertClientChargeInfo: func begin%s", "");
	if (NULL == pCharge_info)
	{
		ERROR("H_InsertClientChargeInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = InsertClientChargeInfo(pCharge_info);
	DEBUG("H_GetChargeTypes: [return value]=%d", nRet);
	INFO("H_InsertClientChargeInfo: func end%s", "");
	return nRet;
}

int H_InsertOldClientChargeInfo(StOld_Charge_Info *pCharge_info)
{
	INFO("H_InsertOldClientChargeInfo: func begin%s", "");
	if (NULL == pCharge_info)
	{
		ERROR("H_InsertOldClientChargeInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = InsertOldClientChargeInfo(pCharge_info);
	DEBUG("H_InsertOldClientChargeInfo: [return value]=%d", nRet);
	INFO("H_InsertOldClientChargeInfo: func end%s", "");
	return nRet;
}


void H_GetImageData(char **ppImage_data, int *pSize)
{
	INFO("H_GetImageData: func begin%s", "");
	if (NULL == pSize)
	{
		ERROR("H_GetImageData: func param error%s", "");
		return;
	}

	GetImageData(ppImage_data, pSize);
	//DEBUG("H_GetImageData: [return  vlaue]=%d", nRet);
	INFO("H_GetImageData: func end%s", "");
};

int H_GetMaxUUID(int nUsrid_begin, int *pMax_usrid)
{
	INFO("H_GetMaxUUID: func begin%s", "");
	if (NULL == pMax_usrid)
	{
		ERROR("H_GetMaxUUID: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = GetMaxUUID(nUsrid_begin, pMax_usrid);
	DEBUG("H_GetMaxUUID: [return value]=%d", nRet);
	INFO("H_GetMaxUUID: func end%s", "");
	return nRet;
}

int H_InsertUsrIdOutofrangeInfo(void)
{
	INFO("H_InsertUsrIdOutOfRangeInfo: func begin%s", "");
	int nRet = InsertUsrIdOutofrangeInfo();
	DEBUG("H_InsertUsrIdOutOfRangeInfo: [return value]=%d", nRet);
	INFO("H_InsertUsrIdOutOfRangeInfo: func end%s", "");
	return nRet;
}

int H_SetUsridFullUseFlag(int nBegin_usrid, int nEnd_usrid)
{
	INFO("H_SetUsridFullUseFlag: func begin%s", "");
	int nRet = SetUsridFullUseFlag(nBegin_usrid, nEnd_usrid);
	DEBUG("H_SetUsridFullUseFlag: [return value]=%d", nRet);
	INFO("H_SetUsridFullUseFlag: func end%s", "");
	return nRet;
}

int H_GetChineseCodeData(int nCode_id, StSystem_Tip_Info *pSystem_tip_info, BYTE *pHas_data)
{
	INFO("H_GetChineseCodeData: func begin%s", "");
	if (NULL == pSystem_tip_info || NULL == pHas_data)
	{
		ERROR("H_GetChineseCodeData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = GetChineseCodeData(nCode_id, pSystem_tip_info, pHas_data);
	DEBUG("H_GetChineseCodeData: [return value]=%d", nRet);
	INFO("H_GetChineseCodeData: func end%s", "");
	return nRet;
};

int H_FindMaxUsrIdEnd(int *pMax_usr_id)
{
	INFO("H_FindMaxUsrIdEnd: func begin%s", "");
	if (NULL == pMax_usr_id)
	{
		ERROR("H_FindMaxUsrIdEnd: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = FindMaxUsrIdEnd(pMax_usr_id);
	DEBUG("H_FindMaxUsrIdEnd: [return value]=%d", nRet);
	INFO("H_FindMaxUsrIdEnd: func end%s", "");
	return nRet;
};

int H_ReportIPhoneMoreInfo(StIPhone_MoreInfo *pIphone_moreinfo)
{
	INFO("H_ReportIPhoneMoreInfo: func begin%s", "");
	if (NULL == pIphone_moreinfo)
	{
		ERROR("H_ReportIPhoneMoreInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = ReportIPhoneMoreInfo(pIphone_moreinfo);
	DEBUG("H_ReportIPhoneMoreInfo: [return value]=%d", nRet);
	INFO("H_ReportIPhoneMoreInfo: func end%s", "");
	return nRet;
}


int H_RecordUsrLoginInfo(StUsr_Login_Info *pUsr_login_info)
{
	INFO("H_RecordUsrLoginInfo: func begin%s", "");
	if (NULL == pUsr_login_info)
	{
		ERROR("H_RecordUsrLoginInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = RecordUsrLoginInfo(pUsr_login_info);
	DEBUG("H_RecordUsrLoginInfo: [return value]=%d", nRet);
	INFO("H_RecordUsrLoginInfo: func end%s", "");
	return nRet;
}

int H_DelUsrLoginInfo(int nUUID)
{
	INFO("H_DelUsrLoginInfo: func begin%s", "");
	int nRet = DelUsrLoginInfo(nUUID);
	DEBUG("H_DelUsrLoginInfo: [return value]=%d", nRet);
	INFO("H_DelUsrLoginInfo: func end%s", "");
	return nRet;
}

int H_GetRegisterCount(int nUUID_begin, int *pCount)
{
	INFO("H_GetRegisterCount: func begin%s", "");
	if (NULL == pCount)
	{
		ERROR("H_GetRegisterCount: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = GetRegisterCount(nUUID_begin, pCount);
	DEBUG("H_GetRegisterCount: [return value]=%d", nRet);	
	INFO("H_GetRegisterCount: func end%s", "");
	return nRet;
}

int H_GetNewRegisterCount(int nUUID_begin, char *pCur_time, int *pCount)
{
	INFO("H_GetNewRegisterCount: func begin%s", "");
	if (NULL == pCount || NULL == pCur_time)
	{
		ERROR("H_GetNewRegisterCount: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = GetNewRegisterCount(nUUID_begin, pCur_time, pCount);
	DEBUG("H_GetNewRegisterCount: [return value]=%d", nRet);	
	INFO("H_GetNewRegisterCount: func end%s", "");
	return nRet;
}


int H_TranslateTaskPushData(int *pCount)
{
	INFO("H_TranslateTaskPushData: func begin%s", "");
	if (NULL == pCount)
	{
		ERROR("H_TranslateTaskPushData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = TranslateTaskPushData(pCount);
	DEBUG("H_TranslateTaskPushData: [return value]=%d", nRet);	
	INFO("H_TranslateTaskPushData: func end%s", "");
	return nRet;
}

int H_TransNewRegisUsrTaskPushData(char *pCur_time, int *pCount)
{
	INFO("H_TransNewRegisUsrTaskPushData: func begin%s", "");
	if (NULL == pCur_time || NULL == pCount)
	{
		ERROR("H_TransNewRegisUsrTaskPushData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = TransNewRegisUsrTaskPushData(pCur_time, pCount);
	DEBUG("H_TransNewRegisUsrTaskPushData: [return value]=%d", nRet);	
	INFO("H_TransNewRegisUsrTaskPushData: func end%s", "");
	return nRet;
}

int H_GetRegisterUsrsInfo(StRegis_table_info *pRegis_table_info, void *pTask_a_data, StRegister_Datas *pRegister_data)
{
	INFO("H_GetRegisterUsrsInfo: func begin%s", "");
	if (NULL == pRegis_table_info || NULL == pTask_a_data || NULL == pRegister_data)
	{
		ERROR("H_GetRegisterUsrsInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = GetRegisterUsrsInfo(pRegis_table_info, pTask_a_data, pRegister_data);
	DEBUG("H_GetRegisterUsrsInfo: [return value]=%d", nRet);	
	INFO("H_GetRegisterUsrsInfo: func end%s", "");
	return nRet;
}

int H_GetNewRegisUsrsInfo(StRegis_table_info *pRegis_table_info, void *pTask_a_data, char *pCur_time, StRegister_Datas *pRegister_data)
{
	INFO("H_GetNewRegisUsrsInfo: func begin%s", "");
	if (NULL == pRegis_table_info || NULL == pTask_a_data || NULL == pCur_time || NULL == pRegister_data)
	{
		ERROR("H_GetNewRegisUsrsInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	 
	int nRet = GetNewRegisUsrsInfo(pRegis_table_info, pTask_a_data, pCur_time, pRegister_data);
	DEBUG("H_GetNewRegisUsrsInfo: [return value]=%d", nRet);	
	INFO("H_GetNewRegisUsrsInfo: func end%s", "");
	return nRet;
}


int H_InsertTaskData(StTask_A_Datas *pTask_a_data, StRegister_Datas *pRegister_datas)
{
	INFO("H_InsertTaskData: func begin%s", "");
	if (NULL == pTask_a_data || NULL == pRegister_datas)
	{
		ERROR("H_InsertTaskData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = InsertTaskData(pTask_a_data, pRegister_datas);
	DEBUG("H_InsertTaskData: [return value]=%d", nRet);	
	INFO("H_InsertTaskData: func end%s", "");
	return nRet;
}

int H_BatchInsertTaskData(StTask_A_Datas *pTask_a_data, StRegister_Datas *pRegister_datas)
{
	INFO("H_BatchInsertTaskData: func begin%s", "");
	if (NULL == pTask_a_data || NULL == pRegister_datas)
	{
		ERROR("H_BatchInsertTaskData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = BatchInsertTaskData(pTask_a_data, pRegister_datas);
	DEBUG("H_BatchInsertTaskData: [return value]=%d", nRet);	
	INFO("H_BatchInsertTaskData: func end%s", "");
	return nRet;
}

int H_BatchInsertTaskAppleData(StTask_A_Datas *pTask_a_data, StRegister_Datas *pRegister_datas)
{
	INFO("H_BatchInsertTaskAppleData: func begin%s", "");
	if (NULL == pTask_a_data || NULL == pRegister_datas)
	{
		ERROR("H_BatchInsertTaskAppleData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = BatchInsertTaskAppleData(pTask_a_data, pRegister_datas);
	DEBUG("H_BatchInsertTaskAppleData: [return value]=%d", nRet);	
	INFO("H_BatchInsertTaskAppleData: func end%s", "");
	return nRet;
}

int H_InsertExactSndTaskAppleData(StTask_A_Info *pTask_a_info)
{
	INFO("H_InsertExactSndTaskAppleData: func begin%s", "");
	if (NULL == pTask_a_info)
	{
		ERROR("H_InsertExactSndTaskAppleData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}


	int nRet = InsertExactSndTaskAppleData(pTask_a_info);
	DEBUG("H_InsertExactSndTaskAppleData: [return value]=%d", nRet);	
	INFO("H_InsertExactSndTaskAppleData: func end%s", "");
	return nRet;
}

int H_GetSoftwareOneLineData(int nID, StSoftware_Data *pSoftware_data)
{
	INFO("H_GetSoftwareOneLineData: func bbegin%s", "");
	if (NULL == pSoftware_data)
	{
		ERROR("H_GetSoftwareOneLineData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = GetSoftwareOneLineData(nID, pSoftware_data);
	DEBUG("H_GetSoftwareOneLineData: [return value]=%d", nRet);	
	INFO("H_GetSoftwareOneLineData: func end%s", "");
	return nRet;
}

int H_GetBookOneLineData(int nID, StBook_Data *pBook_data)
{
	INFO("H_GetBookOneLineData: func begin%s", "");
	if (NULL == pBook_data)
	{
		ERROR("H_GetBookOneLineData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = GetBookOneLineData(nID, pBook_data);
	DEBUG("GetBookOneLineData: [return value]=%d", nRet);	
	INFO("H_GetBookOneLineData: func end%s", "");
	return nRet;
}

int H_GetWeiboOneLineData(int nID, StWeibo_Data *pWeibo_data)
{
	INFO("H_GetWeiboOneLineData: func begin%s", "");
	if (NULL == pWeibo_data)
	{
		ERROR("H_GetWeiboOneLineData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = GetWeiboOneLineData(nID, pWeibo_data);
	DEBUG("H_GetWeiboOneLineData: [return value]=%d", nRet);	
	INFO("H_GetWeiboOneLineData: func end%s", "");
	return nRet;
}

int H_GetClientUpdateOneLineData(int nID, StSof_Update_Data *pSoft_update_data)
{
	INFO("H_GetClientUpdateOneLineData: func  begin%s", "");
	if (NULL == pSoft_update_data)
	{
		ERROR("H_GetClientUpdateOneLineData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = GetClientUpdateOneLineData(nID, pSoft_update_data);
	DEBUG("H_GetClientUpdateOneLineData: [return value]=%d", nRet);
	INFO("H_GetClientUpdateOneLineData: func end%s", "");
	return nRet;
}

int H_GetAdvertiseOneLineData(int nID, StAdvertise_Data *pAd_data)
{
	INFO("H_GetAdvertiseOneLineData: func begin%s", "");
	if (NULL == pAd_data)
	{
		ERROR("H_GetAdvertiseOneLineData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = GetAdvertiseOneLineData(nID, pAd_data);
	DEBUG("H_GetAdvertiseOneLineData: [return value]=%d", nRet);	
	INFO("H_GetAdvertiseOneLineData: func end%s", "");
	return nRet;
}

int H_GetNewPreferenceData(int nID, StNew_Preference_Data *pNew_preference)
{
	INFO("H_GetNewPreferenceData: func begin%s", "");
	if (NULL == pNew_preference)
	{
		ERROR("H_GetNewPreferenceData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = GetNewPreferenceData(nID, pNew_preference);
	DEBUG("H_GetNewPreferenceData: [return value]=%d", nRet);	
	INFO("H_GetNewPreferenceData: func end%s", "");
	return nRet;
}

int H_GetRecommUrlOneLineData(int nID, StRecomm_Url_Data *pRecomm_url_data)
{
	INFO("H_GetRecommUrlOneLineData: func begin%s", "");
	if (NULL == pRecomm_url_data)
	{
		ERROR("H_GetRecommUrlOneLineData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = GetRecommUrlOneLineData(nID, pRecomm_url_data);
	DEBUG("H_GetNewPreferenceData: [return value]=%d", nRet);	
	INFO("H_GetRecommUrlOneLineData: func end%s", "");
	return nRet;
}

int H_GetNewTaskSendData(int nUUID, StTask_Snd_Datas *pTask_snd_query_data, int *pTask_num)
{
	INFO("H_GetNewTaskSendData: func begin%s", "");
	if (NULL == pTask_snd_query_data)
	{
		ERROR("H_GetNewTaskSendData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = GetNewTaskSendData(nUUID, pTask_snd_query_data, pTask_num);
	DEBUG("H_GetNewTaskSendData: [return value]=%d", nRet);	
	INFO("H_GetNewTaskSendData: func end%s", "");
	return nRet;
}

int H_InsertTagClickRecordInfo(StTag_Click_Info *pTag_click_info)
{
	INFO("H_InsertTagClickRecordInfo: func begin%s", "");
	if (NULL == pTag_click_info)
	{
		ERROR("H_InsertTagClickRecordInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = InsertTagClickRecordInfo(pTag_click_info);
	DEBUG("H_InsertTagClickRecordInfo: [return value]=%d", nRet);	
	INFO("H_InsertTagClickRecordInfo: func end%s", "");
	return nRet;
}

int H_GetUsrLoginInfos(StLogin_Table_Info *pLogin_table_info)
{	
	INFO("H_GetUsrLoginInfos: func begin%s", "");
	if (NULL == pLogin_table_info)
	{
		ERROR("H_GetUsrLoginInfos: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = GetUsrLoginInfos(pLogin_table_info);
	DEBUG("H_GetUsrLoginInfos: [return value]=%d", nRet);	
	INFO("H_GetUsrLoginInfos: func end%s", "");
	return nRet;
}

int H_GetExactSndRegisInfo(int nUUID_begin, StPhone_Num_Datas *pPhone_num_datas, void *pParam)
{
	INFO("H_GetExactSndRegisInfo: func begin%s", "");
	if (NULL == pPhone_num_datas || NULL == pParam)
	{
		ERROR("H_GetExactSndRegisInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = GetExactSndRegisInfo(nUUID_begin, pPhone_num_datas, pParam);
	DEBUG("H_GetExactSndRegisInfo: [return value]=%d", nRet);	
	INFO("H_GetExactSndRegisInfo: func end%s", "");
	return nRet;
}

int H_InsertExactSndTaskData(StTask_A_Info *pTask_a_info)
{
	INFO("H_InsertExactSndTaskData: func begin%s", "");
	if (NULL == pTask_a_info)
	{
		ERROR("H_InsertExactSndTaskData: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = InsertExactSndTaskData(pTask_a_info);
	DEBUG("H_InsertExactSndTaskData: [return value]=%d", nRet);	
	INFO("H_InsertExactSndTaskData: func end%s", "");
	return nRet;
}

int H_GetBlackLstInfo(StTask_A_Datas *pTask_a_datas)
{
	INFO("H_GetBlackLstInfo: func begin%s", "");
	if (NULL == pTask_a_datas)
	{
		ERROR("H_GetBlackLstInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = GetBlackLstInfo(pTask_a_datas);
	DEBUG("H_GetBlackLstInfo: [return value]=%d", nRet);	
	INFO("H_GetBlackLstInfo: func end%s", "");
	return nRet;
}

int H_InsertUsrShareClickRecordInfo(StUsrShare_ClickInfo *pUsrshare_clickinfo)
{
	INFO("H_InsertUsrShareClickRecordInfo: func begin%s", "");
	if (NULL == pUsrshare_clickinfo)
	{
		ERROR("H_InsertUsrShareClickRecordInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = InsertUsrShareClickRecordInfo(pUsrshare_clickinfo);
	DEBUG("H_InsertUsrShareClickRecordInfo: [return value]=%d", nRet);	
	INFO("H_InsertUsrShareClickRecordInfo: func end%s", "");
	return nRet;
}

int H_InsertMalfuncReportInfo(StMalfunc_Charge_Info *pMalfunc_charge_info)
{
	INFO("H_InsertMalfuncReportInfo: func begin%s", "");
	if (NULL == pMalfunc_charge_info)
	{
		ERROR("H_InsertMalfuncReportInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = InsertMalfuncReportInfo(pMalfunc_charge_info);
	DEBUG("H_InsertMalfuncReportInfo: [return value]=%d", nRet);	
	INFO("H_InsertMalfuncReportInfo: func end%s", "");
	return nRet;
}

int H_GetUsrLoginNum(int *pCount)
{
	INFO("H_GetUsrLoginNum: func begin%s", "");
	if (NULL == pCount)
	{
		ERROR("H_GetUsrLoginNum: func param erro%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = GetUsrLoginNum(pCount);
	DEBUG("H_GetUsrLoginNum: [return value]=%d", nRet);
	INFO("H_GetUsrLoginNum: func end%s", "");
	return nRet;
}

int H_TestTableNormal(StConn_Info *pConn_info)
{
	INFO("H_TestTableNormal: func begin%s", "");
	if (NULL == pConn_info)
	{
		ERROR("H_TestTableNormal: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = TestTableNormal(pConn_info);
	DEBUG("H_TestTableNormal: [return value]=%d", nRet);
	INFO("H_TestTableNormal: func end%s", "");
	return nRet;
}

#endif

