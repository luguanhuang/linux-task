#include "../../util/service_global.h"
#include "../../interface/service_protocol.h"
#include "../handle_table/service_handle_table.h"
#include "service_table_cluster_detail.h"

/*****该文件主要操作群详表的*************/


//函数用途: 获取群详表名称
//输入参数: 群ID
//输出参数: 存放表名的缓存
//返回值	: 用户ID超出了表的范围,  返回FALSE，  否者返回TRUE 

int GetClusterDetailTableName(int nCluster_id, char *szTableName)
{
	char szBuf[MAX_TABLENAME_LEN] = {0};

	if (nCluster_id >= ONE && nCluster_id <= FIFTY_THOUSAND)
	{
		sprintf(szBuf, "%s_%s", "cluster_detail", "1_50000");	
	}
	else if (nCluster_id > FIFTY_THOUSAND && nCluster_id <= ONE_HUNDRED_THOUSAND)
	{
		sprintf(szBuf, "%s_%s", "cluster_detail", "50001_100000");		
	}
	else if (nCluster_id > ONE_HUNDRED_THOUSAND && nCluster_id <=ONE_HUNDRED_FIFTY_THOUSAND)
	{
		sprintf(szBuf, "%s_%s", "cluster_detail", "100001_150000");	
	}
	else if (nCluster_id > ONE_HUNDRED_FIFTY_THOUSAND && nCluster_id <= TWO_HUNDRED_THOUSAND)
	{
		sprintf(szBuf, "%s_%s", "cluster_detail", "150001_200000");		
	}
		else
	{
		LOG_ERROR("GetClusterDetailTableName: table id out of range", FILE_NAME, FILE_LINE);
		return FALSE;	
	}
	
	strcpy(szTableName, szBuf);
	return TRUE;
}

//函数用途: 获取群成员数据
//输入参数: 用户ID,  群ID 
//输出参数: 存放群成员数据,  成员总数的缓存
/*返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  
  *如果群成员不存在,  返回FALSE,  否者返回TRUE 
  */

int GetClusterDetailData(int nUsr_id, int nCluster_id, Cluster_Member_Data *pCluster_member_data, WORD *pCount)
{
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	MYSQL_ROW   mysql_row;
	MYSQL_RES *pMysql_res = NULL;   
	char szBuf[1024] = {0};
	char *pBuf = NULL;
	int nUser_id = 0;
	int nCount = 0;
	int i = 0;
	int nRet = 0;
	int nLen = 0;

	nRet = GetClusterDetailTableName(nCluster_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("GetClusterDetailData: Call GetClusterDetailTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	MYSQL *pConn = GetConn(szTable_name);					//获取连接
	if (NULL == pConn)
	{
		LOG_ERROR("GetClusterDetailData: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	
	snprintf(szBuf, sizeof(szBuf) - 1, "select * from  %s where ClusterId = %d and Member_UserId = %d", szTable_name, nCluster_id, nUsr_id);

	nRet = mysql_query(pConn, szBuf);					//执行数据库表操作
	
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			printf("GetClusterDetailData: DB disconnect\n");
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		pMysql_res = mysql_store_result(pConn);
		nCount = (int)mysql_num_rows(pMysql_res);
		
		if (nCount!= 0)				//当表行数不为零
		{
			sprintf(szBuf, "select * from  %s where ClusterId = %d", szTable_name, nCluster_id);

			nRet = mysql_query(pConn, szBuf);			//执行数据库表操作
			if (nRet)
			{
				if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
				{
					printf("GetClusterDetailData: DB disconnect\n");
					ReconnDatabase(pConn, szTable_name);
					return DB_DISCONNECT;	
				}
			}
			else
			{
				pMysql_res = mysql_store_result(pConn);
				nCount = (int)mysql_num_rows(pMysql_res);

				*pCount = nCount;
				for (i=0; i<nCount; i++)
				{
					mysql_row =   mysql_fetch_row(pMysql_res);
					pBuf = mysql_row[1];					//获取用户ID
					nRet = IsStringNotEmpty(pBuf);
					if (TRUE == nRet)
					{
						nUser_id = atoi(pBuf);		
						pCluster_member_data[i].nUsr_id = htonl(nUser_id);
					}
					
					pBuf = mysql_row[2];					//获取备注名称
					nRet = IsStringNotEmpty(pBuf);	
					if (TRUE == nRet)
					{
						nLen = strlen(pBuf);
						nLen = GetDataLen(REMARK_NAME_LEN, nLen);
						strncpy(pCluster_member_data[i].arrRemarkName, pBuf, nLen);		
					}
					
					//获取昵称
					nRet = H_GetNickname(nUser_id, pCluster_member_data[i].arrNickname);
					if (DB_DISCONNECT == nRet)
					{
						LOG_ERROR("GetClusterDetailData: DB Disconnect", FILE_NAME, FILE_LINE);
						ReleaseConn(pConn, szTable_name);
						return FALSE;
					}

					//获取图像索引
					nRet = H_GetPhotoIdx(nUser_id, &pCluster_member_data[i].bPhoto_idx, &pCluster_member_data[i].enImage_type);
					if (DB_DISCONNECT == nRet)
					{
						LOG_ERROR("GetClusterDetailData: DB Disconnect", FILE_NAME, FILE_LINE);
						ReleaseConn(pConn, szTable_name);
						return FALSE;
					}
				}

				mysql_free_result(pMysql_res);					//释放内存
				ReleaseConn(pConn, szTable_name);				//释放连接
				return TRUE;
			}
		}
		else
		{
			mysql_free_result(pMysql_res);
			ReleaseConn(pConn, szTable_name);
			return FALSE;	
		}
	}

	return TRUE;
}

//函数用途: 插入群成员数据
//输入参数: 群ID,  用户ID,  用户在群里面的状态
//输出参数: 无
//返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  否者返回TRUE 
  
int InsertClusterMember(int nCluster_id, int nUsr_id, BYTE bStatus)
{
	char szBuf[1024] = {0};
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char szNickname[20] = {0};
	int nRet = 0;
	MYSQL *pConn = NULL;

	//获取昵称
	nRet = H_GetNickname(nUsr_id, szNickname);
	if (DB_DISCONNECT == nRet)
	{
		LOG_ERROR("InsertClusterMember: DB Disconnect", FILE_NAME, FILE_LINE);
		return FALSE;
	}
	
	nRet = GetClusterDetailTableName(nCluster_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("InsertClusterMember: Call GetClusterDetailTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	pConn = GetConn(szTable_name);
	if (NULL == pConn)
	{
		LOG_ERROR("InsertClusterMember: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	snprintf(szBuf, sizeof(szBuf) - 1, \
		"insert into %s(ClusterId, Member_UserId, Member_NickName_In_Cluster, Status) values(%d, %d, '%s', %d)", \
		szTable_name, nCluster_id, nUsr_id, szNickname, bStatus);

	//printf("InsertClusterMember: szBuf=%s\n", szBuf);
	
	nRet = mysql_query(pConn, szBuf);
	
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("InsertClusterMember: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		ReleaseConn(pConn, szTable_name);
		return TRUE;	
	}

	return TRUE;
}

//函数用途: 从哦群详表里面删除群成员
//输入参数: 群ID,  用户ID
//输出参数: 无
//返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  否者返回TRUE 

int DelClusterMember(int nCluster_id, int nUsr_id)
{
	char szBuf[1024] = {0};
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	int nRet = 0;
	MYSQL *pConn = NULL;
	
	nRet = GetClusterDetailTableName(nCluster_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("DelClusterMember: Call GetClusterDetailTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	pConn = GetConn(szTable_name);				//获取连接
	if (NULL == pConn)
	{
		LOG_ERROR("DelClusterMember: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	snprintf(szBuf, sizeof(szBuf) - 1, "delete from %s where ClusterId=%d and Member_UserId=%d", \
		szTable_name, nCluster_id, nUsr_id);
 	
	nRet = mysql_query(pConn, szBuf);			//执行数据库表操作
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("DelClusterMember: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		ReleaseConn(pConn, szTable_name);
		return TRUE;	
	}

	return TRUE;
}

//函数用途: 获取群里面所有的群成员 ID
//输入参数: 群ID
//输出参数: 存放用户ID,  群成员总数计数器的缓存
//返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  否者返回TRUE 

int GetUsrsId(int nCluster_id, int **ppUsr_id, int *pCount)
{
	char szBuf[1024] = {0};
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	int nRet = 0;
	MYSQL *pConn = NULL;
	MYSQL_ROW   mysql_row;
	MYSQL_RES *pMysql_res = NULL;   
	int nCount = 0;
	int i = 0;
	char *pBuf = NULL;
	int nUsr_id = 0;
	int nIdx = 0;
	
	nRet = GetClusterDetailTableName(nCluster_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("GetUsrsId: Call GetClusterDetailTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	pConn = GetConn(szTable_name);
	if (NULL == pConn)
	{
		LOG_ERROR("GetUsrsId: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	snprintf(szBuf, sizeof(szBuf) - 1, "select Member_UserId from %s where ClusterId=%d order by Member_UserId", \
		szTable_name, nCluster_id);

	nRet = mysql_query(pConn, szBuf);				//执行数据库操作
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("DelUsrsBelongToWhichCluster: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		pMysql_res = mysql_store_result(pConn);
		nCount = (int)mysql_num_rows(pMysql_res);
		if (nCount != 0)				//数据库表行数不为零
		{
			*pCount = nCount; 
			*ppUsr_id = (int *)malloc(sizeof(int) * nCount);
			if (NULL == ppUsr_id)
			{
				LOG_ERROR("GetUsrsId: out of memory", FILE_NAME, FILE_LINE);
				return FALSE;
			}
			
			for (i=0; i<nCount; i++)
			{
				mysql_row =   mysql_fetch_row(pMysql_res);
				pBuf = mysql_row[0];				//获取用户ID
				nRet = IsStringNotEmpty(pBuf);		//判断字符串是否为零
				if (TRUE == nRet)
				{
					nUsr_id = atoi(pBuf);
					(*ppUsr_id)[nIdx] = nUsr_id;
					nIdx++;
				}
			}

			*pCount = nIdx; 
		}

		mysql_free_result(pMysql_res);
		ReleaseConn(pConn, szTable_name);
		return TRUE;	
	}

	return TRUE;
}

//函数用途: 删除所有的群成员
//输入参数: 群ID
//输出参数: 无
//返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  否者返回TRUE 

int DelAllClusterMember(int nCluster_id)
{
	char szBuf[1024] = {0};
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	int nRet = 0;
	MYSQL *pConn = NULL;
	
	nRet = GetClusterDetailTableName(nCluster_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("DelAllClusterMember: Call GetClusterDetailTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	pConn = GetConn(szTable_name);				//获取连接
	if (NULL == pConn)
	{
		LOG_ERROR("DelAllClusterMember: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	snprintf(szBuf, sizeof(szBuf) - 1, "delete from %s where ClusterId=%d", szTable_name, nCluster_id);
	nRet = mysql_query(pConn, szBuf);			//执行数据库表操作
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("DelAllClusterMember: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		ReleaseConn(pConn, szTable_name);				//释放数据库连接
		return TRUE;	
	}	

	return TRUE;
}

