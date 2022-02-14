#include "../../util/service_global.h"
#include "../../interface/service_protocol.h"

#ifdef _IM_SERVICE_
#include "../chat_table/service_table_user_login.h"
#include "../chat_table/service_table_usr_connstatus.h"
#include "../chat_table/service_table_group_detail.h"
#include "../chat_table/service_table_usr_info.h"
#include "../chat_table/service_table_friend_group.h"
#include "../chat_table/service_table_belongto_which_cluster.h"
#include "../chat_table/service_table_cluster_detail.h"
#include "../chat_table/service_table_cluster.h"
#include "../chat_table/service_table_Cluster_type.h"
#endif


/****************公共的 表***************************/
#include "../common_table/service_table_server_numrange.h"
#include "service_handle_table.h"


/*********************此文件用来处理对数据库表的所有操作*************************/

#ifdef _IM_SERVICE_


int H_JudgeAccountAndPasswd(char *pUser_name, char *pPasswd, int *pUsr_id, char *pNickname)
{
	int nRet = JudgeAccountAndPasswd(pUser_name, pPasswd, pUsr_id, pNickname);
	return nRet;
}

int H_GetLoginTmpTableName(int nUsr_id, char *szTableName)
{
	int nRet = GetLoginTmpTableName(nUsr_id, szTableName);
	return nRet;
}

int H_IsUsrLogin(int nUsr_id)
{
	int nRet = IsUsrLogin(nUsr_id);
	return nRet;
}

int H_InsertLoginInfoIntoDB(int nUsr_id, int nSvr_seq, int nSock)
{
	int nRet = InsertLoginInfoIntoDB(nUsr_id, nSvr_seq, nSock);
	return nRet;
}

int H_GetGroupDetailTableName(int nUsr_id, char *szTableName)
{
	int nRet = GetGroupDetailTableName(nUsr_id, szTableName);
	return nRet;
}

int H_GetLoginTableName(int nUsr_id, char *szTableName)
{
	int nRet = GetLoginTableName(nUsr_id, szTableName);
	return nRet;
}
 
int H_GetUsrInfoTableName(int nUsr_id, char *szTableName)
{
	int nRet = GetUsrInfoTableName(nUsr_id, szTableName);
	return nRet;
}

int H_GetSigAndPhotoIdx(int nUsr_id, char *szSelfSig, BYTE *pPhoto_idx, EnImage_Type *pImage_tyoe)
{
	int nRet = GetSigAndPhotoIdx(nUsr_id, szSelfSig, pPhoto_idx, pImage_tyoe);
	return nRet;
}



int H_GetGroupTableName(int nUsr_id, char *szTableName)
{
	int nRet = GetGroupTableName(nUsr_id, szTableName);
	return nRet;
}


int H_IsTheSameUsrID(int nUsr_id)
{
	int nRet = IsTheSameUsrID(nUsr_id);
	return nRet;
}

int H_InsertUsrInfo(int nUsr_id, BYTE bSex)
{
	int nRet = InsertUsrInfo(nUsr_id, bSex);
	return nRet;
}

int H_InsertRegisInfoIntoDB(int nUsr_id, char *szNickname, char *szPasswd, Register_Type regis_type, char *szAccount)
{
	int nRet = InsertRegisInfoIntoDB(nUsr_id, szNickname, szPasswd, regis_type, szAccount);
	return nRet;
}


int H_IsTheSameAccount(char *szAccount)
{
	int nRet = IsTheSameAccount(szAccount);
	return nRet;
}

int H_IsUsrIDExistOrNot(int nUsr_id)
{
	int nRet = IsUsrIDExistOrNot(nUsr_id);
	return nRet;
}

int H_IsGroupIDExistOrNot(int nUsr_id, int nGroup_id)
{
	int nRet = IsGroupIDExistOrNot(nUsr_id, nGroup_id);
	return nRet;
}

int H_IsFriendDataExistOrNot(unsigned int nUsr_id, unsigned int nGroup_id, unsigned int nFriend_id)
{
	int nRet = IsFriendDataExistOrNot(nUsr_id, nGroup_id, nFriend_id);
	return nRet;
}

int H_InsertFriendData(unsigned int nUsr_id, unsigned int nGroup_id, unsigned int nFriend_id)
{
	int nRet = InsertFriendData(nUsr_id, nGroup_id, nFriend_id);
	return nRet;
}


int H_InsertFriGroupData(unsigned int nGroup_id, unsigned int nUsr_id, char *szGroup_name)
{
	int nRet = InsertFriGroupData(nGroup_id, nUsr_id, szGroup_name);
	return nRet;
}


int H_DeleteFriGroupData(unsigned int nGroup_id, unsigned int nUsr_id)
{
	int nRet = DeleteFriGroupData(nGroup_id, nUsr_id);
	return nRet;
}

int H_DeleteFriendData(unsigned int nGroup_id, unsigned int nUsr_id, unsigned int nFriend_id)
{
	int nRet = DeleteFriendData(nGroup_id, nUsr_id, nFriend_id);
	return nRet;
}



int H_FindMaxGroupID(unsigned int nUsr_id, unsigned int *pMax_group_id)
{
	int nRet = FindMaxGroupID(nUsr_id, pMax_group_id);
	return nRet;
}

//获取用户昵称
int H_GetNickname(int nUsr_id, char *pNickname)
{
	int nRet = GetNickname(nUsr_id, pNickname);
	return nRet;
}

int H_GetClusterData(int nUsr_id, WORD *pCount, Cluster_Data *pCluster_data)
{
	int nRet = GetClusterData(nUsr_id, pCount, pCluster_data);
	return nRet;
}

int H_GetPhotoIdx(int nUsr_id, BYTE *pPhoto_idx, EnImage_Type *pImage_type)
{
	int nRet = GetPhotoIdx(nUsr_id, pPhoto_idx, pImage_type);
	return nRet;
}
int H_GetClusterDetailData(int nUsr_id, int nCluster_id, Cluster_Member_Data *pCluster_member_data, WORD *pCount)
{
	int nRet = GetClusterDetailData(nUsr_id, nCluster_id, pCluster_member_data, pCount);
	return nRet;
}


int H_DeleteItemByUsrId(unsigned int nUsr_id)
{
	int nRet = DeleteItemByUsrId(nUsr_id);
	return nRet;
}


int H_GetFriendInfoFromDB(int nUsr_id, int nGroup_id, Fri_List_Data *pFri_list_data, WORD *pCount)
{
	int nRet =  GetFriendInfoFromDB(nUsr_id, nGroup_id, pFri_list_data, pCount);
	return nRet;
}

int H_IsEnoughClusterLevel(int nUsr_id)
{
	int nRet = IsEnoughClusterLevel(nUsr_id);
	return nRet;
}



int H_InsertClusterInfo(char *pCluster_name, int nUsr_id, BYTE bCluster_type, int *pCluster_id)
{
	int nRet = InsertClusterInfo(pCluster_name, nUsr_id, bCluster_type, pCluster_id);
	return nRet;
}


int H_InsertOwnClusterInfo(int nUsr_id, int nCluster_id, char *pCluster_name, BYTE bStatus)
{
	int nRet = InsertOwnClusterInfo(nUsr_id, nCluster_id, pCluster_name, bStatus);
	return nRet;
}

int H_GetClusterTypeName(int nCluster_id, char *pType_name)
{
	int nRet = GetClusterTypeName(nCluster_id, pType_name);
	return nRet;
}


int H_GetFriendGroupInfo(int nUsr_id, WORD *pCount, Group_Data *pGroup_data)
{
	int nRet = GetFriendGroupInfo(nUsr_id, pCount, pGroup_data);
	return nRet;
}


int H_InsertClusterMember(int nCluster_id, int nUsr_id, BYTE bStatus)
{
	int nRet = InsertClusterMember(nCluster_id, nUsr_id, bStatus);
	return nRet;
}


int H_GetTypeName(BYTE bCluster_type, char *pType_name)
{
	int nRet = GetTypeName(bCluster_type, pType_name);
	return nRet;
}


int H_InsertImageData(int nUsr_id, BYTE *pImage_data, BYTE *pImage_postfix, int nImage_size)
{
	int nRet = InsertImageData(nUsr_id, pImage_data, pImage_postfix, nImage_size);
	return nRet;
}


int H_IsUsridAndPasswdCorrect(int nUsr_id, char *pPasswd, char *pNickname)
{
	int nRet = IsUsridAndPasswdCorrect(nUsr_id, pPasswd, pNickname);
	return nRet;
}

int H_DelClusterMember(int nCluster_id, int nUsr_id)
{
	int nRet = DelClusterMember(nCluster_id, nUsr_id);
	return nRet;
}


int H_DelOwnClusterInfo(int nUsr_id, int nCluster_id)
{	
	int nRet = DelOwnClusterInfo(nUsr_id, nCluster_id);
	return nRet;
}


int H_IsUsrHavePowerToDelCluster(int nUsr_id, int nCluster_id)
{
	int nRet = IsUsrHavePowerToDelCluster(nUsr_id, nCluster_id);
	return nRet;
}

int H_GetUsrsId(int nCluster_id, int **ppUsr_id, int *pCount)
{
	int nRet = GetUsrsId(nCluster_id, ppUsr_id, pCount);
	return nRet;
}



int H_DeleteUsrsOwnCluster(int nUsr_id, int nCluster_id)
{
	int nRet = DeleteUsrsOwnCluster(nUsr_id, nCluster_id);
	return nRet;
}


int H_DelAllClusterMember(int nCluster_id)
{
	int nRet = DelAllClusterMember(nCluster_id);
	return nRet;
}


int H_DeleteCluster(int nCluster_id)
{
	int nRet = DeleteCluster(nCluster_id);
	return nRet;
}

int H_GetGroupNum(int nUsr_id, int *pGroup_num)
{	
	int nRet = GetGroupNum(nUsr_id, pGroup_num);
	return nRet;
}

int H_GetMaxUsridInTheRange(int nUsrid_start, int nUsrid_end, int *pMax_usrid)
{
	INFO("H_GetMaxUsridInTheRange: func begin%s", "");
	int nRet = GetMaxUsridInTheRange(nUsrid_start, nUsrid_end, pMax_usrid);
	INFO("H_GetMaxUsridInTheRange: func end%s", "");
	return nRet;
}


#endif

#if 0
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
	INFO("H_FindMaxUsrIdEnd: func end%s", "");
	return nRet;
};


#endif
#endif

//公共的表
#ifdef _LIANTONG_SERVICE_
int H_GetRegisterStartIDInfo(void)
{
	INFO("H_GetRegisterStartIDInfo: func begin%s", "");
	int nRet = GetRegisterStartIDInfo();
	DEBUG("H_GetRegisterStartIDInfo: [return value]=%d", nRet);
	INFO("H_GetRegisterStartIDInfo: func end%s", "");
	return nRet;
}

#endif

