
#include "../../util/service_global.h"
#include "../../include/ydt_log.h"
#include "../../interface/service_protocol.h"
#include "../handle_table/service_handle_table.h"
#include "service_table_task_b.h"
#include "service_sub_task_b.h"

static int InsertSysInformTaskData(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StConn_Info *pConn_info);
static int InsertSoftwareTaskData(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StConn_Info *pConn_info);
static int InsertBookTaskData(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StConn_Info *pConn_info);
static int InsertWeiboTaskData(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StConn_Info *pConn_info);
static int InsertClientUpdateTaskData(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StConn_Info *pConn_info);
static int InsertAdTaskData(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StConn_Info *pConn_info);
static int InsertNewPreTaskData(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StConn_Info *pConn_info);
static int InsertRecommUrlTaskData(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StConn_Info *pConn_info);

extern Server_Conf_Info g_srv_conf_info;

typedef int (*PProcTaskFunc)(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StConn_Info *pConn_info);

typedef struct
{
	BYTE bTask_type;
	PProcTaskFunc pProcTaskFunc;
}StProc_Task_Info;

static StTask_Sql_Info arrTask_sql_info[] = 
{
	{SYSTEM_INFORM, GetSysInformTaskSql},
	{SOFTWARE_TYPE, GetSoftwareTaskSql},
	{BOOK_TYPE, GetBookTaskSql},
	{BLOG_TYPE, GetWeiboTaskSql},
	{CLIENT_UPDATE_TYPE, GetClientUpdateTaskSql},
	{ADVERTISE_TYPE, GetAdTaskSql},
	{NEW_PREFERENCE, GetNewPreTaskSql},
	{RECOMM_URL, GetRecommUrlTaskSql}
};

static StProc_Task_Info arrProc_task_info[] = 
{
	{SYSTEM_INFORM, InsertSysInformTaskData},
	{SOFTWARE_TYPE, InsertSoftwareTaskData},
	{BOOK_TYPE, InsertBookTaskData},
	{BLOG_TYPE, InsertWeiboTaskData},
	{CLIENT_UPDATE_TYPE, InsertClientUpdateTaskData},
	{ADVERTISE_TYPE, InsertAdTaskData},
	{NEW_PREFERENCE, InsertNewPreTaskData},
	{RECOMM_URL, InsertRecommUrlTaskData}
};

/**************获取插入sql开始*******************************/

static void SetExecuteSqlInfo(StDB_Sql_Info *pDB_sql_info, char *pBuf, int nLen)
{
	INFO("SetExecuteSqlInfo: func begin%s", "");
	int nActual_len = MIN(nLen, pDB_sql_info->nCan_usr_len);
	//int nActual_len = nLen;
	char *pTmp_sql = pDB_sql_info->pSql + pDB_sql_info->nSql_cur_pos;
	memcpy(pTmp_sql, pBuf, nActual_len);
	pDB_sql_info->nSql_cur_pos += nActual_len;
	pDB_sql_info->nCan_usr_len -= nActual_len;	

	INFO("SetExecuteSqlInfo: [sql]=%s [actual len]=%d [total sql]=%s [sql cur pos]=%d [sql can usr len]=%d", \
		pBuf, nActual_len, pDB_sql_info->pSql, pDB_sql_info->nSql_cur_pos, pDB_sql_info->nCan_usr_len);
	INFO("SetExecuteSqlInfo: func end%s", "");
}

int GetSysInformTaskSql(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StDB_Sql_Info *pDB_sql_info)
{
	INFO("GetSysInformTaskSql: func begin%s", "");
	char szBuf[1024] = {0};
	static int nCount = 0;
	
	int nLen = snprintf(szBuf, sizeof(szBuf) - 1, " select %d, '%s', '%s', '%s', %d, '%s', '%s', "
		"%d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s' union", \
		pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, 0, pTask_a_info->arrInform_image_addr, \
		pTask_a_info->arrBegin_time, SYSTEM_INFORM, "", "", pTask_a_info->arrSys_inform_desc, \
		"", "", "", "", "", pRegister_info->arrDev_token);

	if (0 == (nCount % PRINT_TASK_INTERVAL_LINE))
	{
		DEBUG("GetSysInformTaskSql: [sql]=%s", szBuf);
	}

	nCount++;
	SetExecuteSqlInfo(pDB_sql_info, szBuf, nLen);
	INFO("GetSysInformTaskSql: func end%s", "");
	return TRUE;
}

int GetSoftwareTaskSql(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StDB_Sql_Info *pDB_sql_info)
{
	INFO("GetSoftwareTaskSql: func begin%s", "");
	char szBuf[12400] = {0};
	BYTE bEdition = pTask_a_info->bEdition;
	BYTE bPhone_type = pRegister_info->bPhone_type + 1;
	static int nCount = 0;

	if (0 != bEdition && bPhone_type != bEdition)
	{
		INFO("GetSoftwareTaskSql: current user phone type is not match the send edition"
			" [phone type]=%d [send edition]=%d [uuid]=%s", \
			bPhone_type, bEdition, pRegister_info->arrSys_uuid);
		return SND_TYPE_NOT_MATCH;
	}

	#if 0	
	StSoftware_Data *pSoftware_data = pTask_a_info->stRelative_task_data.pRelative_task_data;
	int nLen = snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
		"imageurl, begintime, Sendtype, name, Description, url, Size, type, Softversion, SoftImages, deviceToken) "
		"values(%d, '%s', '%s', '%s', %d, '%s', "
		"'%s', %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s');", \
		pDB_sql_info->arrTable_name, pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pSoftware_data->arrImage_addr, pTask_a_info->arrBegin_time, SOFTWARE_TYPE, \
		pSoftware_data->arrSoftware_name, pSoftware_data->arrSoftware_description, \
		pSoftware_data->arrSoftware_address, pSoftware_data->arrSoftware_size, \
		pSoftware_data->arrSoftware_type, pSoftware_data->arrSoftware_version, \
		pSoftware_data->arrNested_image_addr, pRegister_info->arrDev_token);
	#else
	StSoftware_Data *pSoftware_data = (StSoftware_Data *)pTask_a_info->stRelative_task_data.pRelative_task_data;
	int nLen = snprintf(szBuf, sizeof(szBuf) - 1, " select %d, '%s', '%s', '%s', %d, '%s', '%s', "
		"%d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s' union", \
		pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pSoftware_data->arrImage_addr, pTask_a_info->arrBegin_time, SOFTWARE_TYPE, \
		pSoftware_data->arrSoftware_name, "", pSoftware_data->arrSoftware_description, \
		pSoftware_data->arrSoftware_address, pSoftware_data->arrSoftware_size, \
		pSoftware_data->arrSoftware_type, pSoftware_data->arrSoftware_version, \
		pSoftware_data->arrNested_image_addr, pRegister_info->arrDev_token);
	#endif

	if (0 == (nCount % PRINT_TASK_INTERVAL_LINE))
	{
		DEBUG("GetSoftwareTaskSql: [sql]=%s", szBuf);
	}
	
	nCount++;
	SetExecuteSqlInfo(pDB_sql_info, szBuf, nLen);
	INFO("GetSoftwareTaskSql: func end%s", "");
	return TRUE;
}

int GetBookTaskSql(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StDB_Sql_Info *pDB_sql_info)
{
	INFO("GetBookTaskSql: func begin%s", "");
	char szBuf[8000] = {0};
	StBook_Data *pBook_data = (StBook_Data *)pTask_a_info->stRelative_task_data.pRelative_task_data;
	static int nCount = 0;

	#if 0
	int nLen = snprintf(szBuf, sizeof(szBuf ) - 1, "insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
		"imageurl, begintime, Sendtype, name, Author, Description, url, Size, type, deviceToken) \
		values(%d, '%s', '%s', '%s', %d, '%s', "
		"'%s', %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s');", \
		pDB_sql_info->arrTable_name, pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pBook_data->arrImage_addr, pTask_a_info->arrBegin_time, BOOK_TYPE, \
		pBook_data->arrBook_name, pBook_data->arrAuthor, pBook_data->arrDescription, \
		pBook_data->arrLink_addr, pBook_data->arrBook_size, \
		pBook_data->arrBook_type, pRegister_info->arrDev_token);
	#else
	int nLen = snprintf(szBuf, sizeof(szBuf ) - 1, " select %d, '%s', '%s', '%s', %d, '%s', '%s', "
		"%d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s' union", \
		pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pBook_data->arrImage_addr, pTask_a_info->arrBegin_time, BOOK_TYPE, \
		pBook_data->arrBook_name, pBook_data->arrAuthor, pBook_data->arrDescription, \
		pBook_data->arrLink_addr, pBook_data->arrBook_size, \
		pBook_data->arrBook_type, "", "", pRegister_info->arrDev_token);
	#endif

	
	if (0 == (nCount % PRINT_TASK_INTERVAL_LINE))
	{
		DEBUG("GetBookTaskSql: [sql]=%s", szBuf);
	}

	nCount++;
	SetExecuteSqlInfo(pDB_sql_info, szBuf, nLen);
	INFO("GetBookTaskSql: func end%s", "");
	return TRUE;
}

int GetWeiboTaskSql(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StDB_Sql_Info *pDB_sql_info)
{
	INFO("GetWeiboTaskSql: func begin%s", "");
	char szBuf[5000] = {0};
	StWeibo_Data *pWeibo_data = (StWeibo_Data *)pTask_a_info->stRelative_task_data.pRelative_task_data;
	static int nCount = 0;

	#if 0
	int nLen = snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
		"imageurl, begintime, Sendtype, name, Description, url, type, deviceToken) \
		values(%d, '%s', '%s', '%s', %d, '%s', "
		"'%s', %d, '%s', '%s', '%s', '%s', '%s');", \
		pDB_sql_info->arrTable_name, pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pWeibo_data->arrImage_addr, pTask_a_info->arrBegin_time, BLOG_TYPE, \
		pWeibo_data->arrWeibo_name, pWeibo_data->arrWeibo_Content, pWeibo_data->arrWeibo_address, \
		pWeibo_data->arrType, pRegister_info->arrDev_token);
	#else
	int nLen = snprintf(szBuf, sizeof(szBuf) - 1, " select %d, '%s', '%s', '%s', %d, '%s', '%s', "
		"%d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s' union", \
		pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pWeibo_data->arrImage_addr, pTask_a_info->arrBegin_time, BLOG_TYPE, \
		pWeibo_data->arrWeibo_name, "", pWeibo_data->arrWeibo_Content, pWeibo_data->arrWeibo_address, \
		"", pWeibo_data->arrType, "", "", pRegister_info->arrDev_token);
	#endif

	if (0 == (nCount % PRINT_TASK_INTERVAL_LINE))
	{
		DEBUG("GetWeiboTaskSql: [sql]=%s", szBuf);
	}

	nCount++;
	SetExecuteSqlInfo(pDB_sql_info, szBuf, nLen);
	INFO("GetWeiboTaskSql: func end%s", "");
	return TRUE;
}

int GetClientUpdateTaskSql(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StDB_Sql_Info *pDB_sql_info)
{
	INFO("GetClientUpdateTaskSql: func begin%s", "");
	char szBuf[6000] = {0};
	static int nCount = 0;
	
	BYTE bPhone_type = pRegister_info->bPhone_type;
	if (0 != bPhone_type)
	{
		INFO("GetClientUpdateTaskSql: current user phone type is not the android type"
			" so we will not translate the task [phone type]=%d ", bPhone_type);
		return SND_TYPE_NOT_MATCH;
	}
		
	StSof_Update_Data *pSoft_update_data = (StSof_Update_Data *)pTask_a_info->stRelative_task_data.pRelative_task_data;

	#if 0
	int nLen = snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
		"begintime, Sendtype, name, Description, url, Softversion, deviceToken) \
		values(%d, '%s', '%s', '%s', %d, '%s', "
		"%d, '%s', '%s', '%s', '%s', '%s');", \
		pDB_sql_info->arrTable_name, pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pTask_a_info->arrBegin_time, CLIENT_UPDATE_TYPE, \
		pSoft_update_data->arrSoftware_name, pSoft_update_data->arrSoftware_desc, \
		pSoft_update_data->arrDownload_addr, pSoft_update_data->arrSoftware_version, \
		pRegister_info->arrDev_token);
	#else
	int nLen = snprintf(szBuf, sizeof(szBuf) - 1, " select %d, '%s', '%s', '%s', %d, '%s', '%s', "
		"%d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s' union", \
		pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, "", \
		pTask_a_info->arrBegin_time, CLIENT_UPDATE_TYPE, \
		pSoft_update_data->arrSoftware_name, "", pSoft_update_data->arrSoftware_desc, \
		pSoft_update_data->arrDownload_addr, "", "", pSoft_update_data->arrSoftware_version, \
		"", pRegister_info->arrDev_token);	
	#endif

	if (0 == (nCount % PRINT_TASK_INTERVAL_LINE))
	{
		DEBUG("GetClientUpdateTaskSql: [sql]=%s", szBuf);	
	}

	nCount++;
	SetExecuteSqlInfo(pDB_sql_info, szBuf, nLen);
	INFO("GetClientUpdateTaskSql: func end%s", "");
	return TRUE;
}

int GetAdTaskSql(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StDB_Sql_Info *pDB_sql_info)
{
	INFO("GetAdTaskSql: func begin%s", "");
	char szBuf[7000] = {0};
	StAdvertise_Data *pAd_data = (StAdvertise_Data *)pTask_a_info->stRelative_task_data.pRelative_task_data;
	static int nCount = 0;

	#if 0
	int nLen = snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
		"imageurl, begintime, Sendtype, name, Description, url, deviceToken) \
		values(%d, '%s', '%s', '%s', %d, '%s', "
		"'%s', %d, '%s', '%s', '%s', '%s');", \
		pDB_sql_info->arrTable_name, pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pAd_data->arrAd_image_addr, pTask_a_info->arrBegin_time, ADVERTISE_TYPE, \
		pAd_data->arrTitle, pAd_data->arrAd_description, \
		pAd_data->arrAd_address, pRegister_info->arrDev_token);
	#else
	int nLen = snprintf(szBuf, sizeof(szBuf) - 1, " select %d, '%s', '%s', '%s', %d, '%s', '%s', "
		"%d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s' union", \
		pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pAd_data->arrAd_image_addr, pTask_a_info->arrBegin_time, ADVERTISE_TYPE, \
		pAd_data->arrTitle, "", pAd_data->arrAd_description, \
		pAd_data->arrAd_address, "", "", "", "", pRegister_info->arrDev_token);
	#endif

	if (0 == (nCount % PRINT_TASK_INTERVAL_LINE))
	{
		DEBUG("GetAdTaskSql: [sql]=%s", szBuf);
	}

	nCount++;
	SetExecuteSqlInfo(pDB_sql_info, szBuf, nLen);
	INFO("GetAdTaskSql: func end%s", "");
	return TRUE;
}

int GetNewPreTaskSql(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StDB_Sql_Info *pDB_sql_info)
{
	INFO("GetNewPreTaskSql: func begin%s", "");
	char szBuf[11000] = {0};
	StNew_Preference_Data *pNew_preference = (StNew_Preference_Data *)pTask_a_info->stRelative_task_data.pRelative_task_data;
	static int nCount = 0;

	#if 0
	nLen = snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
		"imageurl, begintime, Sendtype, name, Description, SoftImages, deviceToken) \
		values(%d, '%s', '%s', '%s', %d, '%s', "
		"'%s', %d, '%s', '%s', '%s', '%s');", \
		pDB_sql_info->arrTable_name, pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pNew_preference->arrImage_addr, pTask_a_info->arrBegin_time, NEW_PREFERENCE, \
		pNew_preference->arrTitle, pNew_preference->arrDesc, \
		pNew_preference->arrNested_image_addr, pRegister_info->arrDev_token);
	#else
	int nLen = snprintf(szBuf, sizeof(szBuf) - 1, " select %d, '%s', '%s', '%s', %d, '%s', '%s', "
		"%d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s' union", \
		pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pNew_preference->arrImage_addr, pTask_a_info->arrBegin_time, NEW_PREFERENCE, \
		pNew_preference->arrTitle, "", pNew_preference->arrDesc, "", "", "", "", \
		pNew_preference->arrNested_image_addr, pRegister_info->arrDev_token);
	#endif

	if (0 == (nCount % PRINT_TASK_INTERVAL_LINE))
	{
		DEBUG("GetNewPreTaskSql: [sql]=%s", szBuf);
	}

	nCount++;
	SetExecuteSqlInfo(pDB_sql_info, szBuf, nLen);
	INFO("GetNewPreTaskSql: func end%s", "");
	return TRUE;
}

int GetRecommUrlTaskSql(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StDB_Sql_Info *pDB_sql_info)
{
	INFO("GetRecommUrlTaskSql: func begin%s", "");
	char szBuf[12000] = {0};
	StRecomm_Url_Data *pRecomm_url_data = (StRecomm_Url_Data *)pTask_a_info->stRelative_task_data.pRelative_task_data;
	static int nCount = 0;

	#if 0
	int nLen = snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
		"imageurl, begintime, Sendtype, name, Description, url, SoftImages, deviceToken) \
		values(%d, '%s', '%s', '%s', %d, '%s', "
		"'%s', %d, '%s', '%s', '%s', '%s', '%s');", \
		pDB_sql_info->arrTable_name, pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pRecomm_url_data->arrimage_addr, pTask_a_info->arrBegin_time, RECOMM_URL, \
		pRecomm_url_data->arrTitle, pRecomm_url_data->arrDescription, \
		pRecomm_url_data->arrAd_addr, pRecomm_url_data->arrNested_image_addr, \
		pRegister_info->arrDev_token);
	#else
	int nLen = snprintf(szBuf, sizeof(szBuf) - 1, " select %d, '%s', '%s', '%s', %d, '%s', '%s', "
		"%d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s' union", \
		pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pRecomm_url_data->arrimage_addr, pTask_a_info->arrBegin_time, RECOMM_URL, \
		pRecomm_url_data->arrTitle, "", pRecomm_url_data->arrDescription, \
		pRecomm_url_data->arrAd_addr, "", "", "", pRecomm_url_data->arrNested_image_addr, \
		pRegister_info->arrDev_token);
	#endif

	if (0 == (nCount % PRINT_TASK_INTERVAL_LINE))
	{
		DEBUG("GetRecommUrlTaskSql: [sql]=%s", szBuf);
	}

	nCount++;
	SetExecuteSqlInfo(pDB_sql_info, szBuf, nLen);
	INFO("GetRecommUrlTaskSql: func end%s", "");
	return TRUE;
}


/**************获取插入sql结束*******************************/
 
static int InsertTaskDataToTable(StConn_Info *pConn_info, char *pSQL)
{
	INFO("InsertTaskDataToTable: func begin%s", "");
	if (NULL == pConn_info || NULL == pSQL)
	{
		ERROR("InsertTaskDataToTable: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = 0;
	nRet = mysql_query(pConn_info->pConn, pSQL);
	if (nRet)
	{
		ERROR("InsertTaskDataToTable: Call mysql_query error%s", "");
		return EXECUTE_SQL_ERROR;
	}
	
	INFO("InsertTaskDataToTable: func end%s", "");
	return TRUE;
}

static int InsertSysInformTaskData(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StConn_Info *pConn_info)
{
	INFO("InsertSysInformTaskData: func begin%s", "");
	char szBuf[1024] = {0};

	snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
		"imageurl, begintime, Sendtype, Description, deviceToken) values(%d, '%s', '%s', '%s', %d, '%s', "
		"'%s', %d, '%s', '%s')", pConn_info->arrTable_name, pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, 0, pTask_a_info->arrInform_image_addr, \
		pTask_a_info->arrBegin_time, SYSTEM_INFORM, pTask_a_info->arrSys_inform_desc, pRegister_info->arrDev_token);

	int nRet = InsertTaskDataToTable(pConn_info, szBuf);
	if (TRUE != nRet)
	{
		ERROR("InsertSysInformTaskData: Call InsertDataToTable error%s", "");
		return nRet;
	}

	INFO("InsertSysInformTaskData: insert sys inform task data succeed%s", "");
	INFO("InsertSysInformTaskData: func end%s", "");
	return TRUE;
}

static int InsertSoftwareTaskData(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StConn_Info *pConn_info)
{
	INFO("InsertSoftwareTaskData: func begin%s", "");
	char szBuf[14400] = {0};
	static int nCount = 0;

	BYTE bEdition = pTask_a_info->bEdition;
	BYTE bPhone_type = pRegister_info->bPhone_type + 1;

	if (0 != bEdition && bPhone_type != bEdition)
	{
		INFO("InsertSoftwareTaskData: current user phone type is not match the send edition"
			" [phone type]=%d [send edition]=%d [uuid]=%s", \
			bPhone_type, bEdition, pRegister_info->arrSys_uuid);
		return TRUE;
	}

	StSoftware_Data *pSoftware_data = (StSoftware_Data *)pTask_a_info->stRelative_task_data.pRelative_task_data;
	
	snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
		"imageurl, begintime, Sendtype, name, Description, url, Size, type, Softversion, SoftImages, deviceToken) \
		values(%d, '%s', '%s', '%s', %d, '%s', "
		"'%s', %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')", \
		pConn_info->arrTable_name, pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pSoftware_data->arrImage_addr, pTask_a_info->arrBegin_time, SOFTWARE_TYPE, \
		pSoftware_data->arrSoftware_name, pSoftware_data->arrSoftware_description, \
		pSoftware_data->arrSoftware_address, pSoftware_data->arrSoftware_size, \
		pSoftware_data->arrSoftware_type, pSoftware_data->arrSoftware_version, \
		pSoftware_data->arrNested_image_addr, pRegister_info->arrDev_token);

	
	if (0 == (nCount % PRINT_TASK_INTERVAL_LINE))
	{
		DEBUG("InsertSoftwareTaskData: [sql]=%s", szBuf);		
	}

	nCount++;
	
	int nRet = InsertTaskDataToTable(pConn_info, szBuf);
	if (TRUE != nRet)
	{
		ERROR("InsertSoftwareTaskData: Call InsertTaskDataToTable error%s", "");
		return nRet;
	}

	INFO("InsertSoftwareTaskData: insert software task data succeed%s", "");
	INFO("InsertSoftwareTaskData: func end%s", "");
	return TRUE;	
}

static int InsertBookTaskData(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StConn_Info *pConn_info)
{
	INFO("InsertBookTaskData: func begin%s", "");
	int nRet = 0;
	char szBuf[8000] = {0};
	static int nCount = 0;
	StBook_Data *pBook_data = (StBook_Data *)pTask_a_info->stRelative_task_data.pRelative_task_data;

	snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
		"imageurl, begintime, Sendtype, name, Author, Description, url, Size, type, deviceToken) \
		values(%d, '%s', '%s', '%s', %d, '%s', "
		"'%s', %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s')", \
		pConn_info->arrTable_name, pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pBook_data->arrImage_addr, pTask_a_info->arrBegin_time, BOOK_TYPE, \
		pBook_data->arrBook_name, pBook_data->arrAuthor, pBook_data->arrDescription, \
		pBook_data->arrLink_addr, pBook_data->arrBook_size, \
		pBook_data->arrBook_type, pRegister_info->arrDev_token);

	if (0 == (nCount % PRINT_TASK_INTERVAL_LINE))
	{
		DEBUG("InsertBookTaskData: [sql]=%s", szBuf);
	}

	nCount++;

	nRet = InsertTaskDataToTable(pConn_info, szBuf);
	if (TRUE != nRet)
	{
		ERROR("InsertBookTaskData: Call InsertTaskDataToTable error%s", "");
		return nRet;
	}

	INFO("InsertBookTaskData: insert book task data succeed%s", "");
	INFO("InsertBookTaskData: func end%s", "");
	return TRUE;
}

static int InsertWeiboTaskData(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StConn_Info *pConn_info)
{
	INFO("InsertWeiboTaskData: func  begin%s", "");
	int nRet = 0;
	char szBuf[5000] = {0};
	static int nCount = 0;
	
	StWeibo_Data *pWeibo_data = (StWeibo_Data *)pTask_a_info->stRelative_task_data.pRelative_task_data;

	snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
		"imageurl, begintime, Sendtype, name, Description, url, type, deviceToken) \
		values(%d, '%s', '%s', '%s', %d, '%s', "
		"'%s', %d, '%s', '%s', '%s', '%s', '%s')", \
		pConn_info->arrTable_name, pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pWeibo_data->arrImage_addr, pTask_a_info->arrBegin_time, BLOG_TYPE, \
		pWeibo_data->arrWeibo_name, pWeibo_data->arrWeibo_Content, pWeibo_data->arrWeibo_address, \
		pWeibo_data->arrType, pRegister_info->arrDev_token);

	if (0 == (nCount % PRINT_TASK_INTERVAL_LINE))
	{
		DEBUG("InsertWeiboTaskData: [sql]=%s", szBuf);
	}

	nCount++;
	
	nRet = InsertTaskDataToTable(pConn_info, szBuf);
	if (TRUE != nRet)
	{
		ERROR("InsertWeiboTaskData: Call InsertTaskDataToTable error%s", "");
		return nRet;
	}

	INFO("InsertWeiboTaskData: insert weibo task data succeed%s", "");
	INFO("InsertWeiboTaskData: func end%s", "");
	return TRUE;
}

static int InsertClientUpdateTaskData(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StConn_Info *pConn_info)
{
	INFO("InsertClientUpdateTaskData: func begin%s", "");

	int nRet = 0;
	char szBuf[6000] = {0};
	static int nCount = 0;
	
	BYTE bPhone_type = pRegister_info->bPhone_type;
	if (0 != bPhone_type)
	{
		INFO("InsertClientUpdateTaskData: current user phone type is not the android type"
			" so we will not translate the task [phone type]=%d ", bPhone_type);
		return TRUE;
	}
		
	StSof_Update_Data *pSoft_update_data = (StSof_Update_Data *)pTask_a_info->stRelative_task_data.pRelative_task_data;

	snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
		"begintime, Sendtype, name, Description, url, Softversion, deviceToken) \
		values(%d, '%s', '%s', '%s', %d, '%s', "
		"%d, '%s', '%s', '%s', '%s', '%s')", \
		pConn_info->arrTable_name, pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pTask_a_info->arrBegin_time, CLIENT_UPDATE_TYPE, \
		pSoft_update_data->arrSoftware_name, pSoft_update_data->arrSoftware_desc, \
		pSoft_update_data->arrDownload_addr, pSoft_update_data->arrSoftware_version, \
		pRegister_info->arrDev_token);

	if (0 == (nCount % PRINT_TASK_INTERVAL_LINE))
	{
		DEBUG("InsertClientUpdateTaskData: [sql]=%s", szBuf);
	}

	nCount++;
	
	nRet = InsertTaskDataToTable(pConn_info, szBuf);
	if (TRUE != nRet)
	{
		ERROR("InsertClientUpdateTaskData: Call InsertTaskDataToTable error%s", "");
		return nRet;
	}

	INFO("InsertClientUpdateTaskData: insert client update task data succeed%s", "");
	INFO("InsertClientUpdateTaskData: func end%s", "");
	return TRUE;
}

static int InsertAdTaskData(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StConn_Info *pConn_info)
{
	INFO("InsertAdTaskData: func begin%s", "");

	int nRet = 0;
	char szBuf[7000] = {0};
	static int nCount = 0;
	
	StAdvertise_Data *pAd_data = (StAdvertise_Data *)pTask_a_info->stRelative_task_data.pRelative_task_data;

	snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
		"imageurl, begintime, Sendtype, name, Description, url, deviceToken) \
		values(%d, '%s', '%s', '%s', %d, '%s', "
		"'%s', %d, '%s', '%s', '%s', '%s')", \
		pConn_info->arrTable_name, pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pAd_data->arrAd_image_addr, pTask_a_info->arrBegin_time, ADVERTISE_TYPE, \
		pAd_data->arrTitle, pAd_data->arrAd_description, \
		pAd_data->arrAd_address, pRegister_info->arrDev_token);

	if (0 == (nCount % PRINT_TASK_INTERVAL_LINE))
	{
		DEBUG("InsertAdTaskData: [sql]=%s", szBuf);
	}

	nCount++;
	
	nRet = InsertTaskDataToTable(pConn_info, szBuf);
	if (TRUE != nRet)
	{
		ERROR("InsertAdTaskData: Call InsertTaskDataToTable error%s", "");
		return nRet;
	}

	INFO("InsertAdTaskData: insert ad task data succeed%s", "");
	INFO("InsertAdTaskData: func end%s", "");
	return TRUE;
}

static int InsertNewPreTaskData(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StConn_Info *pConn_info)
{
	INFO("InsertNewPreTaskData: func begin%s", "");
	
	int nRet = 0;
	char szBuf[12000] = {0};
	static int nCount = 0;
	StNew_Preference_Data *pNew_preference = (StNew_Preference_Data *)pTask_a_info->stRelative_task_data.pRelative_task_data;

	snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
		"imageurl, begintime, Sendtype, name, Description, SoftImages, deviceToken) \
		values(%d, '%s', '%s', '%s', %d, '%s', "
		"'%s', %d, '%s', '%s', '%s', '%s')", \
		pConn_info->arrTable_name, pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pNew_preference->arrImage_addr, pTask_a_info->arrBegin_time, NEW_PREFERENCE, \
		pNew_preference->arrTitle, pNew_preference->arrDesc, \
		pNew_preference->arrNested_image_addr, pRegister_info->arrDev_token);

	if (0 == (nCount % PRINT_TASK_INTERVAL_LINE))
	{
		DEBUG("InsertNewPreTaskData: [sql]=%s", szBuf);
	}

	nCount++;
	
	nRet = InsertTaskDataToTable(pConn_info, szBuf);
	if (TRUE != nRet)
	{
		ERROR("InsertNewPreTaskData: Call InsertTaskDataToTable error%s", "");
		return nRet;
	}

	INFO("InsertNewPreTaskData: insert new preference task data succeed%s", "");
	INFO("InsertNewPreTaskData: func end%s", "");
	return TRUE;
}

static int InsertRecommUrlTaskData(StTask_A_Info *pTask_a_info, StRegister_Info *pRegister_info, StConn_Info *pConn_info)
{
	INFO("InsertRecommUrlTaskData: func begin%s", "");

	int nRet = 0;
	char szBuf[14000] = {0};
	static int nCount = 0;
	
	StRecomm_Url_Data *pRecomm_url_data = (StRecomm_Url_Data *)pTask_a_info->stRelative_task_data.pRelative_task_data;

	snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
		"imageurl, begintime, Sendtype, name, Description, url, SoftImages, deviceToken) \
		values(%d, '%s', '%s', '%s', %d, '%s', "
		"'%s', %d, '%s', '%s', '%s', '%s', '%s')", \
		pConn_info->arrTable_name, pTask_a_info->nTask_id, pRegister_info->arrSys_uuid, \
		pRegister_info->arrPhone_num, pTask_a_info->arrTitle, pTask_a_info->bEdition, \
		pRecomm_url_data->arrimage_addr, pTask_a_info->arrBegin_time, RECOMM_URL, \
		pRecomm_url_data->arrTitle, pRecomm_url_data->arrDescription, \
		pRecomm_url_data->arrAd_addr, pRecomm_url_data->arrNested_image_addr, \
		pRegister_info->arrDev_token);


	if (0 == (nCount % PRINT_TASK_INTERVAL_LINE))
	{
		DEBUG("InsertRecommUrlTaskData: [sql]=%s", szBuf);
	}

	nCount++;
	
	nRet = InsertTaskDataToTable(pConn_info, szBuf);
	if (TRUE != nRet)
	{
		ERROR("InsertRecommUrlTaskData: Call InsertTaskDataToTable error%s", "");
		return nRet;
	}

	INFO("InsertRecommUrlTaskData: insert recomm url task data succeed%s", "");
	INFO("InsertRecommUrlTaskData: func end%s", "");
	return TRUE;
}

int InsertTaskData(StTask_A_Datas *pTask_a_data, StRegister_Datas *pRegister_datas)
{
	INFO("InsertTaskData: func begin%s", "");
	int nSize = sizeof(arrProc_task_info) / sizeof(arrProc_task_info[0]);
	int nTask_idx = 0;
	int nRegis_info_idx = 0;
	int nProc_func_idx = 0;
	
	int nRet = 0;
	BYTE bTask_type = 0;
	int nTask_count = pTask_a_data->nCount;
	int nRegister_count = pRegister_datas->nCount;

	StConn_Info stConn_info;
	memset(&stConn_info, 0, sizeof(stConn_info));

	nRet = GetLTTaskBConnInfo(&stConn_info);
	if (TRUE != nRet)
	{
		ERROR("InsertTaskData: Call GetLTTaskBConnInfo error%s", "");
		return nRet;
	}

	DEBUG("InsertTaskData: [task count]=%d [register count]=%d", \
		nTask_count, nRegister_count);

	for (nTask_idx=0; nTask_idx<nTask_count; nTask_idx++)
	{
		if (TRUE != pTask_a_data->arrTask_a_info[nTask_idx].bIs_exact_send)
		{
			bTask_type = pTask_a_data->arrTask_a_info[nTask_idx].bTask_type;
			for (nRegis_info_idx=0; nRegis_info_idx<nRegister_count; nRegis_info_idx++)
			{
				for (nProc_func_idx=0; nProc_func_idx<nSize; nProc_func_idx++)
				{
					if (bTask_type == arrProc_task_info[nProc_func_idx].bTask_type)
					{
						nRet = arrProc_task_info[nProc_func_idx].pProcTaskFunc(&pTask_a_data->arrTask_a_info[nTask_idx], \
							pRegister_datas->arrRegister_info + nRegis_info_idx, &stConn_info);
						break;
					}
				}

				if (nProc_func_idx == nSize)
				{
					WARN("InsertTaskData: we can't find the proc func according to the task type"
					" [task type]=%d", bTask_type);
					continue;
				}

				if (TRUE != nRet)
				{
					if (EXECUTE_SQL_ERROR == nRet)
					{
						ERROR("InsertTaskData: task_b's db connection occur error now%s", "");
						nRet = HandleDBError(stConn_info.pConn, stConn_info.arrTable_name);	
						return nRet;
					}
					else
					{
						ERROR("InsertTaskData: Call insert task proc func error%s", "");
						ReleaseConn(stConn_info.pConn, stConn_info.arrTable_name);
						return nRet;
					}
				}
			}
		}
	}
	
	ReleaseConn(stConn_info.pConn, stConn_info.arrTable_name);
	INFO("InsertTaskData: func end%s", "");
	return TRUE;
}

int BatchInsertTaskData(StTask_A_Datas *pTask_a_data, StRegister_Datas *pRegister_datas)
{
	INFO("BatchInsertTaskData: func begin%s", "");
	int nSize = sizeof(arrTask_sql_info) / sizeof(arrTask_sql_info[0]);
	int nTask_idx = 0;
	int nRegis_info_idx = 0;
	int nProc_func_idx = 0;
	int nRet = 0;
	BYTE bTask_type = 0;
	int nTask_count = pTask_a_data->nCount;
	int nRegister_count = pRegister_datas->nCount;
	int nCur_sql_count = 0;
	int nTotal_sql_count = 0;
	int nSql_len = 0;

	//DEBUG("BatchInsertTaskData: max insert num=%d", g_srv_conf_info.nMax_batchinsert_num);

	StConn_Info stConn_info;
	memset(&stConn_info, 0, sizeof(stConn_info));

	nRet = GetLTTaskBConnInfo(&stConn_info);
	if (TRUE != nRet)
	{
		ERROR("BatchInsertTaskData: Call GetLTTaskBConnInfo error%s", "");
		return nRet;
	}

	StDB_Sql_Info stDB_sql_info;
	memset(&stDB_sql_info, 0, sizeof(stDB_sql_info));

	stDB_sql_info.pSql = (char *)MM_MALLOC_WITH_DESC(MAX_BATCHINSERT_SQL_SIZE, \
		"BatchInsertTaskData: Call func for batch insert sql");
	if (NULL == stDB_sql_info.pSql)
	{
		ERROR("BatchInsertTaskData: out of memory%s", "");
		ReleaseConn(stConn_info.pConn, stConn_info.arrTable_name);
		return OUT_OF_MEMORY_ERROR;
	}

	memset(stDB_sql_info.pSql, 0, MAX_BATCHINSERT_SQL_SIZE);
	
	stDB_sql_info.nCan_usr_len = MAX_BATCHINSERT_SQL_SIZE;
	stDB_sql_info.nSql_cur_pos = 0;

	int nLen = snprintf(stDB_sql_info.pSql, MAX_BATCHINSERT_SQL_SIZE - 1, \
		"insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
		"imageurl, begintime, Sendtype, name, Author, Description, url, Size, "
		"type, Softversion, SoftImages, deviceToken)", \
		stConn_info.arrTable_name);

	int nInit_sql_len = strlen(stDB_sql_info.pSql);

	stDB_sql_info.nSql_cur_pos += nLen;
	stDB_sql_info.nCan_usr_len -= nLen;

	DEBUG("BatchInsertTaskData: [sql]=%s", stDB_sql_info.pSql);
	DEBUG("BatchInsertTaskData: [task count]=%d [register count]=%d", \
		nTask_count, nRegister_count);

	for (nTask_idx=0; nTask_idx<nTask_count; nTask_idx++)
	{
		if (TRUE != pTask_a_data->arrTask_a_info[nTask_idx].bIs_exact_send)
		{
			bTask_type = pTask_a_data->arrTask_a_info[nTask_idx].bTask_type;
			for (nRegis_info_idx=0; nRegis_info_idx<nRegister_count; nRegis_info_idx++)
			{
				for (nProc_func_idx=0; nProc_func_idx<nSize; nProc_func_idx++)
				{
					if (bTask_type == arrTask_sql_info[nProc_func_idx].bTask_type)
					{
						nRet = arrTask_sql_info[nProc_func_idx].pProcTaskSqlFunc(&pTask_a_data->arrTask_a_info[nTask_idx], \
							pRegister_datas->arrRegister_info + nRegis_info_idx, &stDB_sql_info);
						break;
					}
				}

				if (nProc_func_idx == nSize)
				{
					WARN("BatchInsertTaskData: we can't find the proc func according to the task type"
					" [task type]=%d", bTask_type);
					continue;
				}

				if (TRUE != nRet && SND_TYPE_NOT_MATCH != nRet)
				{
					if (EXECUTE_SQL_ERROR == nRet)
					{
						ERROR("BatchInsertTaskData: task_b's db connection occur error now%s", "");
						MM_FREE(stDB_sql_info.pSql);
						nRet = HandleDBError(stConn_info.pConn, stConn_info.arrTable_name);	
						return nRet;
					}
					else
					{
						ERROR("BatchInsertTaskData: Call get task sql func error%s", "");
						MM_FREE(stDB_sql_info.pSql);
						ReleaseConn(stConn_info.pConn, stConn_info.arrTable_name);
						return nRet;
					}
				}

				if (TRUE == nRet)
				{
					nCur_sql_count++;	
				}
				
				nTotal_sql_count++;	
					
				//执行sql语句
				if ((nInit_sql_len != strlen(stDB_sql_info.pSql))
					&& (g_srv_conf_info.nMax_batchinsert_num == nCur_sql_count 
					|| nTotal_sql_count == nTask_count * nRegister_count))
				{	
					nSql_len = strlen(stDB_sql_info.pSql);
					nSql_len -= strlen("union");
					stDB_sql_info.pSql[nSql_len] = '\0';
					DEBUG("BatchInsertTaskData: [insert sql]=%s", stDB_sql_info.pSql);
					nRet = InsertDataToTable(&stConn_info, stDB_sql_info.pSql);
					if (TRUE != nRet)
					{
						ERROR("BatchInsertTaskData: Call InsertDataToTable error%s", "");
						MM_FREE(stDB_sql_info.pSql);
						//nRet = HandleDBError(stConn_info.pConn, stConn_info.arrTable_name);	
						return nRet;
					}

					nCur_sql_count = 0;
					stDB_sql_info.nCan_usr_len = MAX_BATCHINSERT_SQL_SIZE;
					stDB_sql_info.nSql_cur_pos = 0;
					memset(stDB_sql_info.pSql, 0, MAX_BATCHINSERT_SQL_SIZE);
					nLen = snprintf(stDB_sql_info.pSql, MAX_BATCHINSERT_SQL_SIZE - 1, "insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
					"imageurl, begintime, Sendtype, name, Author, Description, url, Size, type, Softversion, SoftImages, deviceToken)", \
					stConn_info.arrTable_name);
					stDB_sql_info.nSql_cur_pos += nLen;
					stDB_sql_info.nCan_usr_len -= nLen;
					DEBUG("BatchInsertTaskData: [sql]=%s", stDB_sql_info.pSql);
					
				}
			}
		}
	}

	MM_FREE(stDB_sql_info.pSql);
	ReleaseConn(stConn_info.pConn, stConn_info.arrTable_name);
	INFO("BatchInsertTaskData: func end%s", "");
	return TRUE;
}

int InsertExactSndTaskData(StTask_A_Info *pTask_a_info)
{
	INFO("InsertExactSndTaskData: func begin%s", "");
	StConn_Info stConn_info;
	memset(&stConn_info, 0, sizeof(stConn_info));

	int nRet = 0;
	int nExact_snd_idx = 0;
	int nProc_func_idx = 0;
	int nSize = sizeof(arrTask_sql_info) / sizeof(arrTask_sql_info[0]);
	BYTE  bTask_type = 0;
	StRegister_Info *pTmp_regis_info = NULL;
	int nCur_sql_count = 0;
	int nTotal_sql_count = 0;
	int nSql_len = 0;

	nRet = GetLTTaskBConnInfo(&stConn_info);
	if (TRUE != nRet)
	{
		ERROR("InsertExactSndTaskData: Call GetLTTaskBConnInfo error%s", "");
		return nRet;
	}	

	StDB_Sql_Info stDB_sql_info;
	memset(&stDB_sql_info, 0, sizeof(stDB_sql_info));

	stDB_sql_info.pSql = (char *)MM_MALLOC_WITH_DESC(MAX_BATCHINSERT_SQL_SIZE, \
		"InsertExactSndTaskData: Call func for exact snd batch insert sql");
	if (NULL == stDB_sql_info.pSql)
	{
		ERROR("InsertExactSndTaskData: out of memory%s", "");
		ReleaseConn(stConn_info.pConn, stConn_info.arrTable_name);
		return OUT_OF_MEMORY_ERROR;
	}

	memset(stDB_sql_info.pSql, 0, MAX_BATCHINSERT_SQL_SIZE);
	
	stDB_sql_info.nCan_usr_len = MAX_BATCHINSERT_SQL_SIZE;
	stDB_sql_info.nSql_cur_pos = 0;

	int nLen = snprintf(stDB_sql_info.pSql, MAX_BATCHINSERT_SQL_SIZE - 1, \
		"insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
		"imageurl, begintime, Sendtype, name, Author, Description, url, Size, "
		"type, Softversion, SoftImages, deviceToken)", \
		stConn_info.arrTable_name);

	int nInit_sql_len = strlen(stDB_sql_info.pSql);

	stDB_sql_info.nSql_cur_pos += nLen;
	stDB_sql_info.nCan_usr_len -= nLen;

	DEBUG("InsertExactSndTaskData: [sql]=%s", stDB_sql_info.pSql);
	int nExact_snd_cnt = pTask_a_info->stExact_snd_info.nCount;
	bTask_type = pTask_a_info->bTask_type;
	DEBUG("InsertExactSndTaskData: [task type]=%d [exact snd cnt]=%d", \
		bTask_type, nExact_snd_cnt);
	for (nExact_snd_idx=0; nExact_snd_idx<nExact_snd_cnt; nExact_snd_idx++)
	{
		for (nProc_func_idx=0; nProc_func_idx<nSize; nProc_func_idx++)
		{
			if (bTask_type == arrTask_sql_info[nProc_func_idx].bTask_type)
			{
				pTmp_regis_info = &pTask_a_info->stExact_snd_info.pRegister_info[nExact_snd_idx];
				nRet = arrTask_sql_info[nProc_func_idx].pProcTaskSqlFunc(pTask_a_info, \
					pTmp_regis_info, &stDB_sql_info);
				break;
			}
		}

		if (nProc_func_idx == nSize)
		{
			WARN("InsertExactSndTaskData: we can't find the proc func according to the task type"
				" [task type]=%d", bTask_type);
			continue;
		}

		//if (TRUE != nRet)
		if (TRUE != nRet && SND_TYPE_NOT_MATCH != nRet)
		{
			if (EXECUTE_SQL_ERROR == nRet)
			{
				ERROR("InsertExactSndTaskData: task_b's db connection occur error now%s", "");
				MM_FREE(stDB_sql_info.pSql);
				nRet = HandleDBError(stConn_info.pConn, stConn_info.arrTable_name);	
				return nRet;
			}
			else
			{
				ERROR("InsertExactSndTaskData: Call insert task proc func error%s", "");
				MM_FREE(stDB_sql_info.pSql);
				ReleaseConn(stConn_info.pConn, stConn_info.arrTable_name);
				return nRet;
			}
		}

		if (TRUE == nRet)
		{
			nCur_sql_count++;
		}
		
		nTotal_sql_count++;
		if ((nInit_sql_len != strlen(stDB_sql_info.pSql))
		&& (g_srv_conf_info.nMax_batchinsert_num == nCur_sql_count || nTotal_sql_count == nExact_snd_cnt))
		{
			
			nSql_len = strlen(stDB_sql_info.pSql);
			nSql_len -= strlen("union");
			stDB_sql_info.pSql[nSql_len] = '\0';
			DEBUG("InsertExactSndTaskData: [insert sql]=%s", stDB_sql_info.pSql);
			nRet = InsertDataToTable(&stConn_info, stDB_sql_info.pSql);
			if (TRUE != nRet)
			{
				ERROR("InsertExactSndTaskData: Call InsertDataToTable error%s", "");
				MM_FREE(stDB_sql_info.pSql);
				nRet = HandleDBError(stConn_info.pConn, stConn_info.arrTable_name); 
				return nRet;
			}

			nCur_sql_count = 0;
			stDB_sql_info.nCan_usr_len = MAX_BATCHINSERT_SQL_SIZE;
			stDB_sql_info.nSql_cur_pos = 0;
			memset(stDB_sql_info.pSql, 0, MAX_BATCHINSERT_SQL_SIZE);
			nLen = snprintf(stDB_sql_info.pSql, MAX_BATCHINSERT_SQL_SIZE - 1, "insert into %s(taskid, Sysuuid, Phone, Title, Edition, "
			"imageurl, begintime, Sendtype, name, Author, Description, url, Size, type, Softversion, SoftImages, deviceToken)", \
			stConn_info.arrTable_name);
			stDB_sql_info.nSql_cur_pos += nLen;
			stDB_sql_info.nCan_usr_len -= nLen;
			DEBUG("InsertExactSndTaskData: [sql]=%s", stDB_sql_info.pSql);
		}
	}

	MM_FREE(stDB_sql_info.pSql);
	ReleaseConn(stConn_info.pConn, stConn_info.arrTable_name);
	INFO("InsertExactSndTaskData: func end%s", "");
	return TRUE;
}

