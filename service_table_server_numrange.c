
#include "../../util/service_global.h"
#include "../../include/ydt_log.h"
#include "../../interface/service_protocol.h"
#include "../handle_table/service_handle_table.h"
#include "service_table_server_numrange.h"


extern Server_Conf_Info g_srv_conf_info;
extern StUsrID_Info g_usrid_info;

//数据库路由表
extern StDb_RoutingTable g_db_routingtable;
static int GetServerNumRangeTableName(char *pTable_name, int nLen)
{
	INFO("GetServerNumRangeTableName: func  begin%s", "");
	strncpy(pTable_name, "server_numrange", nLen - 1);
	DEBUG("GetServerNumRangeTableName: [server num range table name]=%s", pTable_name);
	INFO("GetServerNumRangeTableName: func end%s", "");
	return TRUE;
}

#if 0
static int GetLocalIp(char *pIp, int nLen)
{
	INFO("GetLocalIp: func begin%s", "");
	char arrHost_name[128] = {0};
	int nRet = gethostname(arrHost_name, sizeof(arrHost_name));
	if (-1 == nRet)
	{
		ERROR("gethostname: Call gethostname error error[%d]=%s", \
			errno, strerror(errno));	
		return FALSE;
	}

	DEBUG("GetLocalIp: [host name]=%s", arrHost_name);

	struct hostent*pHost = NULL;
	pHost = gethostbyname(arrHost_name);
	if (NULL == pHost)
	{
		ERROR("GetLocalIp: Call gethostbyname error error[%d]=%s", \
			errno, strerror(errno));	
		return FALSE;
	}
	
	char *pTmp = inet_ntoa(*((struct in_addr *)pHost->h_addr_list[0]));
	strncpy(pIp, pTmp, nLen);
	DEBUG("GetLocalIp: [host name]=%s [ip]=%s", arrHost_name, pIp);
	
	INFO("GetLocalIp: func end%s", "");
	return TRUE;
}
#endif

void GetServerId(char *pServer_id, int nLen)
{
	INFO("GetServerId: func begin%s", "");
	if (NULL == pServer_id)
	{
		ERROR("GetServerId: func param error%s", "");
		return;
	}
	
	WORD wPort = 0;

	//GetLocalIp(arrIP, sizeof(arrIP));

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	wPort = g_srv_conf_info.wPort;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	snprintf(pServer_id, nLen - 1, "%s%d", g_srv_conf_info.arrServer_id, wPort);	

	DEBUG("GetServerId: [server id]=%s", pServer_id);
	INFO("GetServerId: func end%s", "");
}

int IsServerIDExist(MYSQL *pConn, int *pUsrid_begin, int *pUsrid_end)
{
	INFO("IsServerIDExist: func begin%s", "");
	if (NULL == pConn || NULL == pUsrid_begin || NULL == pUsrid_end)
	{
		ERROR("IsServerIDExist: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char arrServer_id[200] = {0};
	char arrSQL[1024] = {0};
	int nRet = 0;
	int nCount = 0;
	MYSQL_ROW mysql_row = NULL;
	char *pBuf = NULL;
	MYSQL_RES *pMysql_res = NULL;

	GetServerNumRangeTableName(szTable_name, MAX_TABLENAME_LEN);
	
	GetServerId(arrServer_id, sizeof(arrServer_id));
	DEBUG("IsServerIDExist: [server id]=%s", arrServer_id);
	
	snprintf(arrSQL, sizeof(arrSQL) - 1, "select useridStart, useridEnd, serverID from %s where fullUseFlg=%d and serverID='%s'", \
		szTable_name, 0, arrServer_id);	

	DEBUG("IsServerIDExist: [sql]=%s", arrSQL);
	nRet = mysql_query(pConn, arrSQL);					//执行数据库操作
	if (nRet)
	{
		ERROR("IsServerIDExist: Call mysql_query error%s", "");
		return EXECUTE_SQL_ERROR;
	}
	
	pMysql_res = mysql_store_result(pConn);
	if (NULL == pMysql_res)
	{
		ERROR("IsServerIDExist: Call mysql_store_result error%s", "");
		return EXECUTE_SQL_ERROR;
	}
		
	nCount = (int)mysql_num_rows(pMysql_res);					//获取数据库表数据的行数				
	if (0 == nCount)
	{
		INFO("IsServerIDExist: server id is not exist [server id]=%s count=%d", arrServer_id, nCount);
		nRet = FALSE;
	}
	else
	{
		INFO("IsServerIDExist: server id exist [server id]=%s nCount=%d", arrServer_id, nCount);
		mysql_row = mysql_fetch_row(pMysql_res);
		pBuf = mysql_row[0];
		nRet = IsStringNotEmpty(pBuf);
		if (TRUE == nRet)
		{
			*pUsrid_begin = atoi(pBuf);		
		}

		pBuf = mysql_row[1];
		nRet = IsStringNotEmpty(pBuf);
		if (TRUE == nRet)
		{
			*pUsrid_end = atoi(pBuf);
		}

		nRet = TRUE;
	}

	mysql_free_result(pMysql_res);							//释放内存
	INFO("IsServerIDExist: func end%s", "");
	return nRet;
}

int FindMaxUsrIdEnd(int *pMax_usr_id)
{
	INFO("FindMaxUsrIdEnd: func begin%s", "");

	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char arrSQL[1024] = {0};
	int nRet = 0;	
	int nCount = 0;
	MYSQL_ROW   mysql_row = NULL;
	MYSQL_RES *pMysql_res = NULL;
	char *pBuf = NULL;
	MYSQL *pConn = NULL;
	
	GetServerNumRangeTableName(szTable_name, MAX_TABLENAME_LEN); 
	pConn = GetConn(szTable_name);			//获取连接
	if (NULL == pConn)
	{
		ERROR("FindMaxUsrIdEnd: Call GetConn error db connection  is not exits%s", "");
		return CONN_POOL_ISNOT_EXIST;
	}
	
	snprintf(arrSQL, sizeof(arrSQL) - 1, "select useridEnd from %s order by useridEnd desc limit 0, 1", \
		szTable_name);

	DEBUG("FindMaxUsrIdEnd: [sql]=%s", arrSQL);
	nRet = mysql_query(pConn, arrSQL);
	if (nRet)
	{
		ERROR("FindMaxUsrIdEnd: Call mysql_query error%s", "");
		nRet = HandleDBError(pConn, szTable_name);
		return nRet;
		
	}

	pMysql_res = mysql_store_result(pConn);
	if (NULL == pMysql_res)
	{
		ERROR("FindMaxUsrIdEnd: Call mysql_store_result error%s", "");	
		nRet = HandleDBError(pConn, szTable_name);
		return nRet;
	}

	nCount = (int)mysql_num_rows(pMysql_res);					//获取数据库表数据的行数		
	DEBUG("FindMaxUsrIdEnd: [count]=%d", nCount);
	
	if (0 == nCount)
	{
		INFO("FindMaxUsrIdEnd: server num range table is empty now%s", "");
		*pMax_usr_id = 0;
		nRet = FALSE;	
	}
	else
	{	
		mysql_row = mysql_fetch_row(pMysql_res);
		pBuf = mysql_row[0];
		nRet = IsStringNotEmpty(pBuf);
		if (TRUE == nRet)
		{
			*pMax_usr_id = atoi(mysql_row[0]);
			DEBUG("FindMaxUsrIdEnd: [max usrid]=%d", *pMax_usr_id);
			nRet = TRUE;	
		}
	}
	
	mysql_free_result(pMysql_res);							//释放内存
	ReleaseConn(pConn, szTable_name);
	INFO("FindMaxUsrIdEnd: func end%s", "");
	return nRet;
	
}


static int S_FindMaxUsrIdEnd(MYSQL *pConn, int *pMax_usr_id)
{
	INFO("S_FindMaxUsrIdEnd: func begin%s", "");
	if (NULL == pConn || NULL == pMax_usr_id)
	{
		ERROR("S_FindMaxUsrIdEnd: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char arrSQL[1024] = {0};
	int nRet = 0;	
	int nCount = 0;
	MYSQL_ROW   mysql_row = NULL;
	MYSQL_RES *pMysql_res = NULL;
	char *pBuf = NULL;
	
	GetServerNumRangeTableName(szTable_name, MAX_TABLENAME_LEN); 
	snprintf(arrSQL, sizeof(arrSQL) - 1, "select useridEnd from %s order by useridEnd desc limit 0, 1", \
		szTable_name);

	DEBUG("S_FindMaxUsrIdEnd: [sql]=%s", arrSQL);
	nRet = mysql_query(pConn, arrSQL);
	if (nRet)
	{
		ERROR("S_FindMaxUsrIdEnd: Call mysql_query error%s", "");
		return EXECUTE_SQL_ERROR;
	}

	pMysql_res = mysql_store_result(pConn);
	if (NULL == pMysql_res)
	{
		ERROR("S_FindMaxUsrIdEnd: Call mysql_store_result error%s", "");	
		return EXECUTE_SQL_ERROR;
	}

	nCount = (int)mysql_num_rows(pMysql_res);					//获取数据库表数据的行数		
	DEBUG("S_FindMaxUsrIdEnd: [count]=%d", nCount);
	
	if (0 == nCount)
	{
		INFO("S_FindMaxUsrIdEnd: server num range table is empty now%s", "");
		*pMax_usr_id = 0;
		nRet = FALSE;	
	}
	else
	{	
		mysql_row = mysql_fetch_row(pMysql_res);
		pBuf = mysql_row[0];
		nRet = IsStringNotEmpty(pBuf);
		if (TRUE == nRet)
		{
			*pMax_usr_id = atoi(mysql_row[0]);
			DEBUG("S_FindMaxUsrIdEnd: [max usrid]=%d", *pMax_usr_id);
			nRet = TRUE;	
		}
	}
	
	mysql_free_result(pMysql_res);							//释放内存
	
	INFO("S_FindMaxUsrIdEnd: func end%s", "");
	return nRet;
}
 
int InsertServerNumRangeInfo(MYSQL *pConn, int nUsrid_start, int nUsrid_end)
{
	INFO("InsertServerNumRangeInfo: func begin%s", "");
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char arrSQL[1024] = {0};
	int nRet = 0;	
	char szServer_id[200] = {0};
	char szTime[100] = {0};

	GetLocalTime(szTime, sizeof(szTime));
	GetServerNumRangeTableName(szTable_name, MAX_TABLENAME_LEN); 

	GetServerId(szServer_id, sizeof(szServer_id));
	snprintf(arrSQL, sizeof(arrSQL) - 1, "insert into %s(useridStart, useridEnd, fullUseFlg, serverID, Addtime)"
		" values(%d, %d, %d, '%s', '%s')", \
		szTable_name, nUsrid_start, nUsrid_end, 0, szServer_id, szTime);

	DEBUG("InsertServerNumRangeInfo: [sql]=%s", arrSQL);
	nRet = mysql_query(pConn, arrSQL);
	if (nRet)
	{
		ERROR("InsertServerNumRangeInfo: Call mysql_query error%s", "");
		return EXECUTE_SQL_ERROR;
	}

	INFO("InsertServerNumRangeInfo: func end%s", "");
	return TRUE;
}


#if 0
#ifdef _IM_SERVICE_
static int GetChatRegisterStartIDInfo(void)
{
	INFO("GetChatRegisterStartIDInfo: func begin%s", "");
	int nUsrid_begin = 0;
	int nUsrid_end = 0;
	int nMax_usrid = 0;
	char szNew_regis_tablename[MAX_TABLENAME_LEN] = {0};
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	MYSQL *pConn = NULL;
	Conn_Pool *pConn_pool = NULL;

	GetServerNumRangeTableName(szTable_name, MAX_TABLENAME_LEN); 
	pConn = GetConn(szTable_name);			//获取连接

	int nRet = IsServerIDExist(&nUsrid_begin, &nUsrid_end);
	if (TRUE != nRet && FALSE !=nRet)
	{
		ERROR("GetChatRegisterStartIDInfo: Call IsServerIDExist error [return value]=%d", nRet);
		return nRet;
	}

	if (FALSE == nRet)
	{
		nRet = FindMaxUsrId(&nMax_usrid);
		if (TRUE != nRet && FALSE != nRet)
		{
			ERROR("GetChatRegisterStartIDInfo: Call FindMaxUsrId error [return value]=%d", nRet);
			return nRet;
		}

		if (FALSE == nRet)
		{
			nUsrid_begin = ONE;
			nUsrid_end = ONE_HUNDRED_THOUSAND;
			DEBUG("GetChatRegisterStartIDInfo: [usrid start]=%d [usrid end]=%d", \
				nUsrid_begin, nUsrid_end);

			memset(szNew_regis_tablename, 0, sizeof(szNew_regis_tablename));
			snprintf(szNew_regis_tablename, sizeof(szNew_regis_tablename) - 1, \
				"user_login_%d_%d", nUsrid_begin, nUsrid_end);
			pConn_pool = GetConnPool(szNew_regis_tablename);
			if (NULL == pConn_pool)
			{
				ERROR("GetChatRegisterStartIDInfo: db connection pool is not exist, so we will notify admin to add table [table name]=%s", \
					szNew_regis_tablename);
				pthread_mutex_lock(&g_usrid_info.usrid_mutex);
				g_usrid_info.nUsr_id = 0;
				g_usrid_info.enUsrid_stauts = USRID_UNNORMAL;
				pthread_mutex_unlock(&g_usrid_info.usrid_mutex);
				return CONN_POOL_ISNOT_EXIST;
			}
			
			nRet = InsertServerNumRangeInfo(nUsrid_begin, nUsrid_end);
			if (TRUE != nRet)
			{
				ERROR("GetChatRegisterStartIDInfo: Call InsertServerNumRangeInfo error [return value]=%d", \
					nRet);
				return nRet;
			}

			pthread_mutex_lock(&g_usrid_info.usrid_mutex);
			g_usrid_info.nUsr_id = ONE;
			g_usrid_info.enUsrid_stauts = USRID_NORMAL;
			pthread_mutex_unlock(&g_usrid_info.usrid_mutex);
			DEBUG("GetChatRegisterStartIDInfo: usrid=%d", g_usrid_info.nUsr_id);
			
		}
		else if (TRUE == nRet)
		{
			nUsrid_begin = nMax_usrid + 1;
			nUsrid_end = nUsrid_begin + ONE_HUNDRED_THOUSAND - 1;

			DEBUG("GetChatRegisterStartIDInfo: [usrid start]=%d [usrid end]=%d", \
				nUsrid_begin, nUsrid_end);
			memset(szNew_regis_tablename, 0, sizeof(szNew_regis_tablename));
			snprintf(szNew_regis_tablename, sizeof(szNew_regis_tablename) - 1, \
				"user_login_%d_%d", nUsrid_begin, nUsrid_end);
			pConn_pool = GetConnPool(szNew_regis_tablename);
			if (NULL == pConn_pool)
			{
				ERROR("GetChatRegisterStartIDInfo: db connection pool is not exist, so we will notify admin to add table [table name]=%s", \
					szNew_regis_tablename);
				pthread_mutex_lock(&g_usrid_info.usrid_mutex);
				g_usrid_info.nUsr_id = 0;
				g_usrid_info.enUsrid_stauts = USRID_UNNORMAL;
				pthread_mutex_unlock(&g_usrid_info.usrid_mutex);
				return CONN_POOL_ISNOT_EXIST;
			}
			
			nRet = InsertServerNumRangeInfo(nUsrid_begin, nUsrid_end);
			if (TRUE != nRet)
			{
				ERROR("GetChatRegisterStartIDInfo: Call InsertServerNumRangeInfo error%s", "");
				pthread_mutex_lock(&g_usrid_info.usrid_mutex);
				g_usrid_info.nUsr_id = nUsrid_begin;
				g_usrid_info.enUsrid_stauts = USRID_NORMAL;
				pthread_mutex_unlock(&g_usrid_info.usrid_mutex);
				DEBUG("GetRegisterIDInfo: usrid=%d", g_usrid_info.nUsr_id);
				return nRet;
			}

			pthread_mutex_lock(&g_usrid_info.usrid_mutex);
			g_usrid_info.nUsr_id = nUsrid_begin;
			g_usrid_info.enUsrid_stauts = USRID_NORMAL;
			pthread_mutex_unlock(&g_usrid_info.usrid_mutex);
			DEBUG("GetChatRegisterStartIDInfo: usrid=%d", g_usrid_info.nUsr_id);
		}
	}
	else if (TRUE == nRet)
	{
		memset(szNew_regis_tablename, 0, sizeof(szNew_regis_tablename));
		snprintf(szNew_regis_tablename, sizeof(szNew_regis_tablename) - 1, \
			"user_login_%d_%d", nUsrid_begin, nUsrid_end);
		pConn_pool = GetConnPool(szNew_regis_tablename);
		if (NULL == pConn_pool)
		{
			ERROR("GetChatRegisterStartIDInfo: db connection pool is not exist, so we will notify admin to add table [table name]=%s", \
				szNew_regis_tablename);
			pthread_mutex_lock(&g_usrid_info.usrid_mutex);
			g_usrid_info.nUsr_id = 0;
			g_usrid_info.enUsrid_stauts = USRID_UNNORMAL;
			pthread_mutex_unlock(&g_usrid_info.usrid_mutex);
			return CONN_POOL_ISNOT_EXIST;
		}
	
		nRet = H_GetMaxUsridInTheRange(nUsrid_begin, nUsrid_end, &nMax_usrid);
		if (TRUE != nRet && FALSE != nRet)
		{
			ERROR("GetChatRegisterStartIDInfo: Call H_GetMaxUsridInTheRange error [return value]=%d", nRet);
			return nRet;
		}

		if (FALSE == nRet)
		{
			pthread_mutex_lock(&g_usrid_info.usrid_mutex);
			g_usrid_info.nUsr_id = 1;
			g_usrid_info.enUsrid_stauts = USRID_NORMAL;
			pthread_mutex_unlock(&g_usrid_info.usrid_mutex);
			INFO("GetChatRegisterStartIDInfo: max usrid is not exist so current usrid=%d", g_usrid_info.nUsr_id);
		}
		else if (TRUE == nRet)
		{
			pthread_mutex_lock(&g_usrid_info.usrid_mutex);
			g_usrid_info.nUsr_id = nMax_usrid + 1;
			g_usrid_info.enUsrid_stauts = USRID_NORMAL;
			pthread_mutex_unlock(&g_usrid_info.usrid_mutex);
			INFO("GetChatRegisterStartIDInfo: max usrid is exist so current usrid=%d", g_usrid_info.nUsr_id);
		}
	}
	
	INFO("GetChatRegisterStartIDInfo: func end%s", "");
	return TRUE;
}
#endif
#endif

#ifdef _LIANTONG_SERVICE_
int IsLiantongRegisterTableExist(int nUsrid_begin, int nUsrid_end)
{
	INFO("IsLiantongRegisterTableExist: func begin%s", "");
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	int nRet = FALSE; 

	snprintf(szTable_name, sizeof(szTable_name) - 1, \
		"phone_register%d_%d", nUsrid_begin, nUsrid_end);

	DEBUG("IsLiantongRegisterTableExist: [register table name]=%s", szTable_name);
	StHash_Item *pItem = HashGetItem(g_db_routingtable.pDb_Routing_table, szTable_name);
	if (NULL == pItem)
	{
		WARN("IsLiantongRegisterTableExist: register table is not exist now yet"
			" [register table name]=%s", szTable_name);	
		nRet = FALSE;
	}
	else
	{
		INFO("IsLiantongRegisterTableExist: register table is exist"
			" [register table name]=%s", szTable_name);	
		nRet = TRUE;		
	}

	INFO("IsLiantongRegisterTableExist: func end%s", "");
	return nRet;	
}

#endif

static int LockTable(MYSQL *pConn, char *pTable_name)
{
	INFO("LockTable: func begin%s", "");
	if (NULL == pConn || NULL == pTable_name)
	{
		ERROR("LockTable: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	char szLock_table[200] = {0};
	int nRet = 0;
	snprintf(szLock_table, sizeof(szLock_table) - 1, "lock table %s write", pTable_name);
	DEBUG("LockTable: [lock table]=%s", szLock_table);
	nRet = mysql_query(pConn, szLock_table);
	if (nRet)
	{
		ERROR("LockTable: Call mysql_query error%s", "");
		return EXECUTE_SQL_ERROR;
	}

	INFO("LockTable: func end%s", "");
	return TRUE;
}

static int UnlockTable(MYSQL *pConn, char *pTable_name)
{
	INFO("UnlockTable: func begin%s", "");
	char szUnlock_table[200] = {0};
	int nRet = 0;
	
	if (NULL == pConn || NULL == pTable_name)
	{
		ERROR("UnlockTable: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	snprintf(szUnlock_table, sizeof(szUnlock_table) - 1, "unlock table");
	DEBUG("UnlockTable: [unlock table]=%s", szUnlock_table);
	
	nRet = mysql_query(pConn, szUnlock_table);
	if (nRet)
	{
		ERROR("UnlockTable: Call mysql_query error%s", "");
		return EXECUTE_SQL_ERROR;
	}
	
	INFO("UnlockTable: func end%s", "");
	return TRUE;
}


void SetUsrIdInfoValue(unsigned int nUsr_id, unsigned int nUsrid_begin, unsigned int nUsrid_end, EnUsrId_Status enUsrid_status)
{
		pthread_mutex_lock(&g_usrid_info.usrid_mutex);
		g_usrid_info.nUsr_id = nUsr_id;
		g_usrid_info.nUsrid_begin = nUsrid_begin;
		g_usrid_info.nUsrid_end = nUsrid_end;
		g_usrid_info.enUsrid_stauts = enUsrid_status;
		pthread_mutex_unlock(&g_usrid_info.usrid_mutex);
}

static int HandleExecuteDBError(MYSQL *pConn, char *pTable_name)
{
	INFO("HandleExecuteDBError: func begin%s", "");
	if (NULL == pConn)
	{
		ERROR("HandleExecuteDBError: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nMysql_errno = mysql_errno(pConn);
	if (CR_SERVER_LOST == nMysql_errno || CR_SERVER_GONE_ERROR == nMysql_errno)
	{
		ERROR("HandleExecuteDBError: DB connection is disconnect mysql_error[%d]=%s", \
									mysql_errno(pConn), mysql_error(pConn));
		ReconnDatabase(pConn, pTable_name);
		INFO("HandleExecuteDBError: func end%s", "");
		return DB_DISCONNECT;	
	}
	else
	{
		ERROR("HandleExecuteDBError: DB have other error mysql_error[%d]=%s", \
		mysql_errno(pConn), mysql_error(pConn));
		INFO("HandleExecuteDBError: func end%s", "");
		return DB_OTHER_ERROR;
					
	}
}


#ifdef _LIANTONG_SERVICE_
static int GetLiantongRegisterStartIDInfo(void)
{	
	INFO("GetLiantongRegisterStartIDInfo: func begin%s", "");
	MYSQL *pConn = NULL;
	int nUsrid_begin = 0;
	int nUsrid_end = 0;
	int nMax_usrid = 0;
	int nUsr_id = 0;
	int nRet = 0;
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	
	GetServerNumRangeTableName(szTable_name, MAX_TABLENAME_LEN); 
	pConn = GetConn(szTable_name);			//获取连接
	if (NULL == pConn)
	{
		ERROR("GetLiantongRegisterStartIDInfo: Call GetConn error db connection  is not exits%s", "");
		return CONN_POOL_ISNOT_EXIST;
	}

	//判断服务器表示标识存在
	nRet = IsServerIDExist(pConn, &nUsrid_begin, &nUsrid_end);
	if (TRUE != nRet && FALSE != nRet)
	{
		ERROR("GetLiantongRegisterStartIDInfo: Call IsServerIDExist error%s", "");
		if (EXECUTE_SQL_ERROR == nRet)
		{
			nRet = HandleDBError(pConn, szTable_name);
		}

		return nRet;	
	}

	if (FALSE == nRet)					//如果服务器标识不存在
	{
		nRet = LockTable(pConn, szTable_name);
		if (TRUE != nRet)
		{
			ERROR("GetLiantongRegisterStartIDInfo: Call LockTable error%s", "");
			if (EXECUTE_SQL_ERROR == nRet)
			{
				nRet = HandleDBError(pConn, szTable_name);	
			}
			
			return nRet;
		}
		
		nRet = S_FindMaxUsrIdEnd(pConn, &nMax_usrid);				//在号段表里面查找最大的号段
		if (TRUE != nRet && FALSE != nRet)
		{
			ERROR("GetLiantongRegisterStartIDInfo: Call S_FindMaxUsrIdEnd error%s", "");
			if (EXECUTE_SQL_ERROR == nRet)
			{
				nRet = HandleExecuteDBError(pConn, szTable_name);
				if (DB_DISCONNECT != nRet)			//如果不是数据库断连
				{
					UnlockTable(pConn, szTable_name);
				}
			}

			ReleaseConn(pConn, szTable_name);
			return nRet;	
		}

		if (FALSE == nRet)					//如果当前号段表为空
		{
			INFO("GetLiantongRegisterStartIDInfo: we cann't find the max usrid from num range table%s", "");
			nUsrid_begin = USRID_BEGIN;
			nUsrid_end = USRID_BEGIN + ONE_MILLION - 1;
		}
		else if (TRUE == nRet)
		{
			INFO("GetLiantongRegisterStartIDInfo: we can find the max usrid from num range table%s", "");			
			nUsrid_begin = nMax_usrid + 1;
			nUsrid_end = nUsrid_begin + ONE_MILLION - 1;		
		}

		
		DEBUG("GetLiantongRegisterStartIDInfo: [usrid begin]=%d [usrid end]=%d", \
						nUsrid_begin, nUsrid_end);

		
		nRet = IsLiantongRegisterTableExist(nUsrid_begin, nUsrid_end);
		if (FALSE == nRet)
		{
			SetUsrIdInfoValue(0, 0, 0, USRID_UNNORMAL);
			UnlockTable(pConn, szTable_name);
			ReleaseConn(pConn, szTable_name);
			return DB_TABLE_ISNOT_EXIST;
		}

		
		nRet = H_GetMaxUUID(nUsrid_begin, &nMax_usrid); 		//在注册表中查找最大的UUID
		if (TRUE != nRet && FALSE != nRet)
		{
			ERROR("GetLiantongRegisterStartIDInfo: Call H_GetMaxUUID error [return value]=%s", nRet);	
			SetUsrIdInfoValue(0, 0, 0, USRID_UNNORMAL);
			UnlockTable(pConn, szTable_name);
			ReleaseConn(pConn, szTable_name);
			return nRet;
		}
		
		if (TRUE == nRet)			//如果注册表中有数据
		{
			INFO("GetLiantongRegisterStartIDInfo: we can find the usrid from register table%s", "");
			nMax_usrid++;
			nUsr_id = (nUsrid_begin > nMax_usrid) ? nUsrid_begin : nMax_usrid;
			SetUsrIdInfoValue(nUsr_id, nUsrid_begin, nUsrid_end, USRID_NORMAL);
		}
		else if (FALSE == nRet) 	//如果注册表中没有数据
		{
			INFO("GetLiantongRegisterStartIDInfo: we cann't find the usrid from register table%s", "");
			SetUsrIdInfoValue(nUsrid_begin, nUsrid_begin, nUsrid_end, USRID_NORMAL);	
		}

		
		nRet = InsertServerNumRangeInfo(pConn, nUsrid_begin, nUsrid_end);		//插入服务器信息
		if (EXECUTE_SQL_ERROR == nRet || FUNC_PARAM_ERROR == nRet)
		{
			ERROR("GetLiantongRegisterStartIDInfo: Call InsertServerNumRangeInfo error%s", "");
			if (EXECUTE_SQL_ERROR == nRet)
			{
				nRet = HandleExecuteDBError(pConn, szTable_name); 
				if (DB_DISCONNECT != nRet)			//如果不是数据库断连
				{
					UnlockTable(pConn, szTable_name);
				}
			}

			SetUsrIdInfoValue(0, 0, 0, USRID_UNNORMAL);
			ReleaseConn(pConn, szTable_name);
			return nRet;
		}

		nRet = UnlockTable(pConn, szTable_name);
		if (EXECUTE_SQL_ERROR == nRet)
		{
			ERROR("GetLiantongRegisterStartIDInfo: Call UnlockTable error%s", "");
			nRet = HandleDBError(pConn, szTable_name);
			return nRet;
		}
		
	}
	else if (TRUE == nRet)				//如果服务器标识存在
	{
		INFO("GetLiantongRegisterStartIDInfo: server id exist in the server numrange table%s", "");
		nRet = IsLiantongRegisterTableExist(nUsrid_begin, nUsrid_end);
		if (FALSE == nRet)
		{
			SetUsrIdInfoValue(0, 0, 0, USRID_UNNORMAL);
			ReleaseConn(pConn, szTable_name);
			return DB_TABLE_ISNOT_EXIST;
		}

		nRet = H_GetMaxUUID(nUsrid_begin, &nMax_usrid);
		if (TRUE != nRet && FALSE != nRet)
		{
			ERROR("GetLiantongRegisterStartIDInfo: Call H_GetMaxUUID error [return value]=%s", nRet);	
			ReleaseConn(pConn, szTable_name);
			return nRet;
		}

		if (FALSE == nRet)
		{	
			INFO("GetLiantongRegisterStartIDInfo: we can't find the usrid from the register table%s", "");
			SetUsrIdInfoValue(nUsrid_begin, nUsrid_begin, nUsrid_end, USRID_NORMAL);
		}
		else if (TRUE == nRet)
		{
			INFO("GetLiantongRegisterStartIDInfo: we can find the usrid from the register table%s", "");
			nMax_usrid++;
			nUsr_id = (nUsrid_begin > nMax_usrid) ? nUsrid_begin : nMax_usrid;
			SetUsrIdInfoValue(nUsr_id, nUsrid_begin, nUsrid_end, USRID_NORMAL); 
		}
	}

	
	DEBUG("GetLiantongRegisterStartIDInfo: [current usrid]=%d", g_usrid_info.nUsr_id);
	ReleaseConn(pConn, szTable_name);
	INFO("GetLiantongRegisterStartIDInfo: func end%s", "");
	return TRUE;
}
#endif

int GetRegisterStartIDInfo(void)
{
	INFO("GetRegisterStartIDInfo: func begin%s", "");	
	int nRet = 0;
	
	#if defined(_IM_SERVICE_)
	//nRet = GetChatRegisterStartIDInfo();
	nRet = 0;
	#elif defined(_LIANTONG_SERVICE_)
	nRet = GetLiantongRegisterStartIDInfo();
	if (TRUE != nRet)
	{
		ERROR("GetRegisterStartIDInfo: Call GetLiantongRegisterStartIDInfo error"
			" [return value]=%d", nRet);
		return nRet;		
	}
	#endif
	
	INFO("GetRegisterStartIDInfo: func end%s", "");
	return TRUE;
}


int SetUsridFullUseFlag(int nBegin_usrid, int nEnd_usrid)
{	
	INFO("SetUsridFullUseFlag: func begin%s", "");
	MYSQL *pConn = NULL;
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char szSQL[1024] = {0};
	int nRet = 0;
	
	GetServerNumRangeTableName(szTable_name, MAX_TABLENAME_LEN); 
	pConn = GetConn(szTable_name);			//获取连接
	if (NULL == pConn)
	{
		ERROR("SetUsridFullUseFlag: Call GetConn error db connection  is not exits%s", "");
		return CONN_POOL_ISNOT_EXIST;
	}

	snprintf(szSQL, sizeof(szSQL) - 1, "update %s set fullUseFlg=1 where useridStart=%d"
		" and useridEnd=%d", szTable_name, nBegin_usrid, nEnd_usrid);
	DEBUG("SetUsridFullUseFlag: [sql]=%s", szSQL);
	nRet = mysql_query(pConn, szSQL);
	if (nRet)
	{
		ERROR("SetUsridFullUseFlag: Call mysql_query error%s", "");
		nRet = HandleDBError(pConn, szTable_name);
		return nRet;
	}

	ReleaseConn(pConn, szTable_name);
	INFO("SetUsridFullUseFlag: update fulluse flg succeed%s", "");
	INFO("SetUsridFullUseFlag func end%s", "");
	return TRUE;
};

