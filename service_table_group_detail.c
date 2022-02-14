#include "../../util/service_global.h"
#include "../../interface/service_protocol.h"
#include "../handle_table/service_handle_table.h"
#include "service_table_group_detail.h"

/*****该文件主要操作组详表的***************/


//函数用途: 获取组详表名称
//输入参数: 用户ID
//输出参数: 存放表名的缓存
//返回值	: 用户ID超出了表的范围,  返回FALSE，  否者返回TRUE 


int GetGroupDetailTableName(int nUsr_id, char *szTableName)
{
	char szBuf[MAX_TABLENAME_LEN] = {0};

	if (nUsr_id >= ONE && nUsr_id <= ONE_HUNDRED_THOUSAND)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s_%s", "friend_group_detail", "1_100000");	
	}
	else if (nUsr_id > ONE_HUNDRED_THOUSAND && nUsr_id <= TWO_HUNDRED_THOUSAND)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s_%s", "friend_group_detail", "100001_200000");		
	}
	else
	{
		LOG_ERROR("GetGroupDetailTableName: table id out of range", FILE_NAME, FILE_LINE);
		return FALSE;	
	}
	
	strcpy(szTableName, szBuf);
	return TRUE;
}

//函数用途: 判断好友数据是否存在
//输入参数: 用户ID,  组ID,  好友ID
//输出参数: 无
/*返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  
  *如果好友数据已经存在,  返回TRUE,  否者返回FALSE 
  */

int IsFriendDataExistOrNot(unsigned int nUsr_id, unsigned int nGroup_id, unsigned int nFriend_id)
{
	int nRet = 0;
	int nCount = 0;
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	MYSQL_RES *pMysql_ret = NULL;   
	char szBuf[1024] = {0};
	memset(szBuf, 0, sizeof(szBuf));

	
	nRet = GetGroupDetailTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("IsFriendDataExistOrNot: Call GetGroupDetailTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	
	MYSQL *pConn = GetConn(szTable_name);
	if (NULL == pConn)
	{
		LOG_ERROR("IsFriendDataExistOrNot: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	snprintf(szBuf, sizeof(szBuf) - 1, "select * from %s where Creator_UserId=%d and GroupId=%d and Friend_UserId=%d", \
		szTable_name, nUsr_id, nGroup_id, nFriend_id);

	nRet = mysql_query(pConn, szBuf);				//执行数据库表操作
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			printf("IsFriendDataExistOrNot: DB disconnect\n");
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		pMysql_ret = mysql_store_result(pConn);
		nCount= (int)mysql_num_rows(pMysql_ret);					//获取数据库表的行数
		printf("IsFriendDataExistOrNot: %d records found\n", nCount);

		mysql_free_result(pMysql_ret);				//释放内存
		ReleaseConn(pConn, szTable_name);			//释放连接
		if (nCount != 0)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	return TRUE;
}

//函数用途: 插入好友数据 到组详表中
//输入参数: 用户ID,  组ID,  好友ID
//输出参数: 无
//返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,   否者返回TRUE 

int InsertFriendData(unsigned int nUsr_id, unsigned int nGroup_id, unsigned int nFriend_id)
{
	int nRet = 0;
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char szBuf[1024] = {0};

	nRet = GetGroupDetailTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("InsertFriendData: Call GetGroupDetailTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}

	
	MYSQL *pConn = GetConn(szTable_name);					//获取连接
	if (NULL == pConn)
	{
		LOG_ERROR("InsertFriendData: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	memset(szBuf, 0, sizeof(szBuf));

	//表操作语句
	sprintf(szBuf, "insert into %s(Creator_UserId, GroupId, Friend_UserId) values(%d, %d, %d)", szTable_name, nUsr_id, nGroup_id, nFriend_id);


	
	nRet = mysql_query(pConn, szBuf);				//操作数据库表
	
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			printf("InsertFriendData: DB disconnect\n");
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

//函数用途: 从组详表中删除好友数据
//输入参数: 用户ID,  组ID,  好友ID
//输出参数: 无
//返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,   否者返回TRUE 


int DeleteFriendData(unsigned int nGroup_id, unsigned int nUsr_id, unsigned int nFriend_id)
{
	int nRet = 0;
	char szBuf[1024] = {0};
	char szTable_name[MAX_TABLENAME_LEN] = {0};

	
	nRet = GetGroupDetailTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("DeleteFriendData: Call GetGroupDetailTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	MYSQL *pConn = GetConn(szTable_name);					//获取数据库连接
	if (NULL == pConn)
	{
		LOG_ERROR("DeleteFriendData: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	snprintf(szBuf, sizeof(szBuf) - 1, "delete from %s where Creator_UserId=%d and GroupId=%d and Friend_UserId=%d", szTable_name, nUsr_id, nGroup_id, nFriend_id);

	nRet = mysql_query(pConn, szBuf);					//执行表操作
	
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			printf("DeleteFriendData: DB disconnect\n");
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		ReleaseConn(pConn, szTable_name);				//释放连接
		return TRUE;	
	}

	return TRUE;
}

//函数用途: 从组详表中获取好友信息
//输入参数: 用户ID,  组ID
//输出参数: 存放好友信息,  好友数量计数器的缓存
//返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,   否者返回TRUE 

int GetFriendInfoFromDB(int nUsr_id, int nGroup_id, Fri_List_Data *pFri_list_data, WORD *pCount)
{
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	MYSQL_ROW   mysql_row;
	MYSQL_RES *pMysql_res = NULL;   
	char szBuf[1024] = {0};
	char *pBuf = NULL;
	int nUser_id = 0;
	int nRet = 0;
	WORD wCount = 0;
	int i = 0;
	int nLen = 0;
	BYTE bPhoto_idx = 0;
	EnImage_Type enImage_type = SYSTEM_IMAGE;

	nRet = GetGroupDetailTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("GetFriendInfoFromDB: Call GetGroupDetailTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	MYSQL *pConn = GetConn(szTable_name);
	if (NULL == pConn)
	{
		LOG_ERROR("GetFriendInfoFromDB: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	snprintf(szBuf, sizeof(szBuf) - 1, "select Friend_UserId, RemarkName from  %s where creator_userid = %d and GroupId = %d", szTable_name, nUsr_id, nGroup_id);

	nRet = mysql_query(pConn, szBuf);
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("GetFriendInfoFromDB: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		pMysql_res = mysql_store_result(pConn);
		wCount = (int)mysql_num_rows(pMysql_res);
		printf("GetFriendInfoFromDB: nGroup_id=%d wCount=%d\n", nGroup_id, wCount);
		
		if (wCount!= 0)
		{
			*pCount = wCount;
			for (i=0; i<wCount; i++)
			{
				mysql_row =   mysql_fetch_row(pMysql_res);			//获取一行数据
				pBuf = mysql_row[1];								//获取备注名称
				nRet = IsStringNotEmpty(pBuf);
				if (TRUE == nRet)
				{
					nLen = strlen(pBuf);
					nLen = GetDataLen(REMARK_NAME_LEN, nLen);
					strncpy(pFri_list_data[i].arrRemark, pBuf, nLen);	
				}
				
				pBuf = mysql_row[0];					//获取用户ID
				nRet = IsStringNotEmpty(pBuf);
				if (TRUE == nRet)
				{
					nUser_id = atoi(pBuf);
					pFri_list_data[i].nUsr_id = htonl(nUser_id);	
				}
				
				
				nRet = H_GetNickname(nUser_id, pFri_list_data[i].arrNickname);				//获取昵称
				if (DB_DISCONNECT == nRet)
				{
					LOG_ERROR("GetFriendInfoFromDB: DB Disconnect", FILE_NAME, FILE_LINE);
					ReleaseConn(pConn, szTable_name);		
					return nRet;
				}

				//获取个性签名和图像索引				
				nRet = H_GetSigAndPhotoIdx(nUser_id, pFri_list_data[i].arrSelf_signature, &bPhoto_idx, &enImage_type);
				if (DB_DISCONNECT == nRet)
				{
					LOG_ERROR("GetFriendInfoFromDB: DB Disconnect", FILE_NAME, FILE_LINE);
					ReleaseConn(pConn, szTable_name);
					return nRet;
				}
				
				pFri_list_data[i].bPhoto_idx = bPhoto_idx;
				pFri_list_data[i].enImage_type = enImage_type;

				//判断用户是否已经登录系统
				nRet = H_IsUsrLogin(nUser_id);
				if (DB_DISCONNECT == nRet)
				{
					LOG_ERROR("GetFriendInfoFromDB: DB Disconnect", FILE_NAME, FILE_LINE);
					ReleaseConn(pConn, szTable_name);
					return nRet;
				}

				if (TRUE == nRet)
				{
					pFri_list_data[i].bStatus = USR_ONLINE;
				}
				else if (FALSE == nRet)
				{
					pFri_list_data[i].bStatus = USR_OFFLINE;
				}
			}
		}

		mysql_free_result(pMysql_res);
		ReleaseConn(pConn, szTable_name);		
		return TRUE;
	}

	return TRUE;
}




