#include "../../util/service_global.h"
#include "../../interface/service_protocol.h"
#include "service_table_Cluster_type.h"
#include "service_table_cluster.h"

/*****该文件主要操作群表的*************/


//函数用途: 获取群表名称
//输入参数: 群ID
//输出参数: 存放表名的缓存
//返回值	: 用户ID超出了表的范围,  返回FALSE，  否者返回TRUE 


int GetClusterTableName(int nCluster_id, char *szTableName)
{
	char szBuf[MAX_TABLENAME_LEN] = {0};

	if (nCluster_id >= ONE && nCluster_id <= ONE_HUNDRED_THOUSAND)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s_%s", "cluster", "1_100000");	
	}
	else if (nCluster_id > ONE_HUNDRED_THOUSAND && nCluster_id <= TWO_HUNDRED_THOUSAND)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s_%s", "cluster", "100001_200000");		
	}
	else
	{
		LOG_ERROR("GetClusterTableName: table id out of range", FILE_NAME, FILE_LINE);
		return FALSE;	
	}

	
	strcpy(szTableName, szBuf);
	return TRUE;
}

//函数用途: 生成群号(随机数)
//输入参数: 无
//输出参数: 存放群号码的缓存
//返回值	:  如果生成成功,  返回TRUE,  否者返回FALSE 

int GenerateClusterNum(int *pCluster_num)
{
	if (NULL == pCluster_num)
	{
		LOG_ERROR("GenerateClusterNum: Param error", FILE_NAME, FILE_LINE);
		return FALSE;
	}

	int i = 0;

	char szCluster_num[100] = {0};

	for (i=0; i<CLUSTER_NUM_LEN; i++)
	{
		if (0 == i)
		{
			szCluster_num[i] = 48 + (int)(2.0 * rand() / (RAND_MAX + 1.0));  //暂时生成0-1的随机数字
		}
		else
		{
			szCluster_num[i] = 48 + (int)(10.0 * rand() / (RAND_MAX + 1.0)); //生成0-9的随机数字
		}
	}

	*pCluster_num = atoi(szCluster_num);
	return TRUE;
}

//函数用途: 获取群ID
//输入参数: 无
//输出参数: 存放群ID的缓存
//返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  否者返回TRUE

int GetClusterId(int *pCluster_id)
{
	if (NULL == pCluster_id)
	{
		LOG_ERROR("GetClusterId: Param error", FILE_NAME, FILE_LINE);
		return FALSE;
	}

	int nRet = 0;
	int nCluster_id = 0;
	WORD wCount = 0;
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char szBuf[1024] = {0};
	MYSQL_RES *pMysql_res = NULL;  
	MYSQL *pConn = NULL;

	for (;;)
	{
		memset(szTable_name, 0, sizeof(szTable_name));
		nCluster_id = 0;
		GenerateClusterNum(&nCluster_id);
		
		nRet = GetClusterTableName(nCluster_id, szTable_name);	
		if (FALSE == nRet)
		{
			LOG_ERROR("GetClusterId: Call GetClusterTableName error", FILE_NAME, FILE_LINE);	
			return TABLE_ID_OUTOF_RANGE;
		}
		
		pConn = GetConn(szTable_name);				//获取连接
		if (NULL == pConn)
		{
			LOG_ERROR("GetClusterId: Call GetConn error", FILE_NAME, FILE_LINE);
			return CONN_POOL_ISNOT_EXIST;
		}
		
		snprintf(szBuf, sizeof(szBuf) - 1, "select * from %s where ClusterId=%d", szTable_name, nCluster_id);

		nRet = mysql_query(pConn, szBuf);			//执行数据库表操作
		if (nRet)
		{
			if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
			{
				LOG_ERROR("GetClusterId: DB disconnect", FILE_NAME, FILE_LINE);
				ReconnDatabase(pConn, szTable_name);
				return DB_DISCONNECT;	
			}
		}
		else
		{
			pMysql_res = mysql_store_result(pConn);
			wCount = (WORD)mysql_num_rows(pMysql_res);
			if (0 == wCount)				//数据库表行数为零
			{
				*pCluster_id = nCluster_id;
				ReleaseConn(pConn, szTable_name);			//释放连接
				return TRUE;
			}
			else
			{
				ReleaseConn(pConn, szTable_name);
				continue;
			}
		}
	}
	
	return TRUE;
}

//函数用途:插入群信息到群表中
//输入参数: 群名称,   用户ID,  群类型
//输出参数: 存放群ID的缓存
//返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  否者返回TRUE

int InsertClusterInfo(char *pCluster_name, int nUsr_id, BYTE bCluster_type, int *pCluster_id)
{		
	int nRet = 0;
	int nCluster_id = 0;
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char szType_name[200] = {0};
	char szBuf[1024] = {0};
	MYSQL *pConn = NULL;
	char szTime[100] = {0};
	time_t now;

	nRet = GetClusterId(&nCluster_id);
	
	if (DB_DISCONNECT == nRet)
	{
		LOG_ERROR("InsertClusterInfo: DB Disconnect", FILE_NAME, FILE_LINE);
		return nRet;
	}

	//获取类型名称
	nRet = GetTypeName(bCluster_type, szType_name);
	if (DB_DISCONNECT == nRet)
	{
		LOG_ERROR("InsertClusterInfo: DB Disconnect", FILE_NAME, FILE_LINE);
		return nRet;
	}

	nRet = GetClusterTableName(nCluster_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("InsertClusterInfo: Call GetClusterTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	pConn = GetConn(szTable_name);				//获取连接
	if (NULL == pConn)
	{
		LOG_ERROR("InsertClusterInfo: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	time(&now);
	struct tm *pTime = localtime(&now);			//获取当前时间
	int nYear = pTime->tm_year + 1900;
	int nMonth = pTime->tm_mon + 1;
	int nDay = pTime->tm_mday;
	int nHour = pTime->tm_hour;
	int nMinute = pTime->tm_min;
	int nSecond = pTime->tm_sec;

	snprintf(szTime, sizeof(szTime) - 1, "%d-%d-%d %d:%d:%d", nYear, nMonth, nDay, nHour, nMinute, nSecond);
	
	snprintf(szBuf, sizeof(szBuf) - 1, \
		"insert into %s(ClusterId, ClusterName, Creator_UserId, ClusterMainType, Alltypename, Addtime) values(%d, '%s', %d, %d, '%s', '%s')", \
		szTable_name, nCluster_id, pCluster_name, nUsr_id, bCluster_type, szType_name, szTime);

	nRet = mysql_query(pConn, szBuf);				//执行数据库表操作
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("InsertClusterInfo: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		ReleaseConn(pConn, szTable_name);
		*pCluster_id = nCluster_id;
		return TRUE;
	}

	return TRUE;	
}

//函数用途: 获取群类型名称
//输入参数: 群ID
//输出参数:  存放群名称的缓存
/*返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  
  *群类型不存在,   返回FALSE,  否者返回TRUE 
  */

int GetClusterTypeName(int nCluster_id, char *pType_name)
{
	if (NULL == pType_name)
	{
		LOG_ERROR("GetClusterTypeName: param error", FILE_NAME, FILE_LINE);
		return FALSE;
	}

	int nRet = 0;
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char szBuf[1024] = {0};
	nRet = GetClusterTableName(nCluster_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("GetClusterTypeName: Call GetClusterTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	MYSQL *pConn = GetConn(szTable_name);
	if (NULL == pConn)
	{
		LOG_ERROR("GetClusterTypeName: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	MYSQL_ROW mysql_row;
	MYSQL_RES *pMysql_res = NULL;	
	WORD wCount = 0;
	char *pBuf = NULL;
	int nLen = 0;

	
	snprintf(szBuf, sizeof(szBuf) - 1, "select Alltypename from %s where ClusterId=%d", szTable_name, nCluster_id);
	nRet = mysql_query(pConn, szBuf);			//执行数据库操作
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("GetClusterTypeName: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		pMysql_res = mysql_store_result(pConn);
		wCount = (WORD)mysql_num_rows(pMysql_res);
		if (wCount > 0)						//数据库表行号为零
		{
			mysql_row = mysql_fetch_row(pMysql_res);
			pBuf = mysql_row[0];				//获取群类型名称	
			nRet = IsStringNotEmpty(pBuf);
			if (TRUE == nRet)
			{
				nLen = strlen(pBuf);
				nLen = GetDataLen(ALLTYPE_NAME_LEN, nLen);
				strncpy(pType_name, mysql_row[0], nLen);		
				mysql_free_result(pMysql_res);				//释放内存
				ReleaseConn(pConn, szTable_name);			//释放连接
				return TRUE;
			}
		}

		mysql_free_result(pMysql_res);
		ReleaseConn(pConn, szTable_name);
		return FALSE;
	}

	return TRUE;
}

//函数用途: 删除群
//输入参数: 群ID
//输出参数:  无
//返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  否者返回TRUE
  


int DeleteCluster(int nCluster_id)
{
	char szBuf[1024] = {0};
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	int nRet = 0;
	MYSQL *pConn = NULL;
	
	nRet = GetClusterTableName(nCluster_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("DeleteCluster: Call GetClusterTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	pConn = GetConn(szTable_name);			//获取连接
	if (NULL == pConn)
	{
		LOG_ERROR("DeleteCluster: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	snprintf(szBuf, sizeof(szBuf) - 1, "delete from %s where ClusterId=%d", \
		szTable_name, nCluster_id);
 	
	nRet = mysql_query(pConn, szBuf);			//执行数据库表操作
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("DeleteCluster: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		ReleaseConn(pConn, szTable_name);			//释放连接
		return TRUE;	
	}

	return TRUE;
}

