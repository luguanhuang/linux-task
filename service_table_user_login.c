#include "../../util/service_global.h"
#include "../../interface/service_protocol.h"
#include "service_table_user_login.h"

/*****该文件主要操作登录表的 ****************/


//数据库路由表
extern StDb_RoutingTable g_db_routingtable;

//函数用途: 获取用户获取登录表名称
//输入参数: 用户ID
//输出参数: 存放表名的缓存
//返回值	: 用户ID超出了表的范围,  返回FALSE，  否者返回TRUE 

int GetLoginTableName(int nUsr_id, char *szTableName)
{
	char szBuf[MAX_TABLENAME_LEN] = {0};

	if (nUsr_id >= ONE && nUsr_id <= ONE_HUNDRED_THOUSAND)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s_%s", "user_login", "1_100000");	
	}
	else if (nUsr_id > ONE_HUNDRED_THOUSAND && nUsr_id <= TWO_HUNDRED_THOUSAND)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s_%s", "user_login", "100001_200000");		
	}
	else
	{
		LOG_ERROR("GetLoginTableName: table id out of range", FILE_NAME, FILE_LINE);
		return FALSE;	
	}
	
	strcpy(szTableName, szBuf);
	return TRUE;
}

//函数用途: 获取数据库路由表变量
//输入参数: 无
//输出参数: 无
//返回值	: 返回数据库路由表变量

StHash_Table *GetDbRoutingTable(void)
{
	StHash_Table *pRouting_table = NULL;

	pthread_mutex_lock(&g_db_routingtable.routing_mutex);
	pRouting_table = g_db_routingtable.pDb_Routing_table;
	pthread_mutex_unlock(&g_db_routingtable.routing_mutex);

	return pRouting_table;
}

//函数用途: 判断用户ID和密码是否正确
//输入参数: 用户ID,  密码
//输出参数: 存放昵称的缓存
/*返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  
  *密码错误,   返回FALSE,  否者返回TRUE 
  */
  
int IsUsridAndPasswdCorrect(int nUsr_id, char *pPasswd, char *pNickname)
{	
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char szBuf[1024] = {0};
	MYSQL *pConn = NULL;
	MYSQL_ROW mysql_row;
	MYSQL_RES *pMysql_res = NULL;
	int nRet = 0;
	int nCount = 0;
	int nLen = 0;
	
	char *pBuf = NULL;

	//获取登录表名称
	nRet = GetLoginTableName(nUsr_id, szTable_name);

	//表名不存在
	if (FALSE == nRet)
	{
		LOG_ERROR("IsUsridAndPasswdCorrect: Call GetLoginTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	pConn = GetConn(szTable_name);				//获取数据库连接
	if (NULL == pConn)
	{
		LOG_ERROR("IsUsridAndPasswdCorrect: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	//数据库表的操作语句
	snprintf(szBuf, sizeof(szBuf) - 1, "select * from %s where UserId=%d and password='%s'", szTable_name, \
		nUsr_id, pPasswd);

	nRet = mysql_query(pConn, szBuf);					//执行数据库操作
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("IsUsridAndPasswdCorrect: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		pMysql_res = mysql_store_result(pConn);
		nCount = (int)mysql_num_rows(pMysql_res);					//获取数据库表数据的行数

		printf("IsUsridAndPasswdCorrect: count=%d\n", nCount);			

		if (nCount != 0)
		{
			mysql_row = mysql_fetch_row(pMysql_res);				//获取数据库表的一行数据

			//获取昵称
			pBuf = mysql_row[1];

			//判断字符串是否为空
			nRet = IsStringNotEmpty(pBuf);
			if (TRUE == nRet)
			{
				nLen = strlen(pBuf);
				nLen = GetDataLen(NICKNAME_LEN, nLen);
				strncpy(pNickname, pBuf, nLen);				
			}

			mysql_free_result(pMysql_res);				//释放内存
			ReleaseConn(pConn, szTable_name);			//释放连接
			return TRUE;

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

//函数用途: 判断用户名和密码是否正确
//输入参数: 用户名,  密码
//输出参数: 存放用户ID, 昵称的缓存
/*返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  
  *密码错误,  返回FALSE,  否者返回TRUE 
  */

int JudgeAccountAndPasswd(char *pUser_name, char *pPasswd, int *pUsr_id, char *pNickname)
{
	int nRet = 0;
	int i = 0;
	int nCount = 0;
	int nLen = 0;
	char szTable_name[MAX_TABLENAME_LEN] = {0}; 
	char szBuf[1024] = {0};
	MYSQL *pConn = NULL;
	DB_Conf_File_Fields *pConf_file_fields = NULL;
	StHash_Table *pRouting_table = NULL;
	StHash_Item *pItem = NULL;
	MYSQL_RES *pMysql_res = NULL; 	
	MYSQL_ROW	mysql_row;
	char *pBuf = NULL;
	
	strcpy(szTable_name, "user_login");

	pRouting_table = GetDbRoutingTable();

	//遍历数据库路由表, 查找登陆表
	for (i=0; i<pRouting_table->nBucket_size; i++)
	{
		pItem = pRouting_table->ppItem[i];
		while (pItem != NULL)
		{
			pConf_file_fields = (DB_Conf_File_Fields *)pItem->pMatch_msg;

			//比较表名
			if (0 == memcmp(pConf_file_fields->szTable_name, szTable_name, strlen(szTable_name)))
			{
				memset(szBuf, 0, sizeof(szBuf));
				pConn = GetConn(pConf_file_fields->szTable_name);				//获取连接
				if (NULL == pConn)
				{
					LOG_ERROR("JudgeAccountAndPasswd: Call GetConn error", FILE_NAME, FILE_LINE);
					return CONN_POOL_ISNOT_EXIST;
				}
			
				//数据库操作语句
				snprintf(szBuf, sizeof(szBuf) - 1, "select * from %s where username='%s' and password='%s'", pConf_file_fields->szTable_name, pUser_name, pPasswd);

				nRet = mysql_query(pConn, szBuf);				//执行数据库操作
				if (nRet)
				{
					LOG_ERROR("JudgeAccountAndPasswd: DB Disconnect", FILE_NAME, FILE_LINE);

					//数据库断连
					if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
					{
						ReconnDatabase(pConn, pConf_file_fields->szTable_name);
						//DBReconnectOper(pConf_file_fields->szTable_name);			//进行数据库重连操作
						return DB_DISCONNECT;	
					}
				}
				else
				{
					pMysql_res = mysql_store_result(pConn);
					nCount = (int)mysql_num_rows(pMysql_res);					//获取数据库表数据的行数
					printf("JudgeAccountAndPasswd: nCount=%d\n", nCount);
					if (nCount != 0)
					{
						mysql_row = mysql_fetch_row(pMysql_res);			//获取数据库表的一行数据
						pBuf = mysql_row[0];				//获取用户ID
						nRet = IsStringNotEmpty(pBuf);		//判读字符串是否为空
						if (TRUE == nRet)
						{
							*pUsr_id = atoi(pBuf);	
						}

						pBuf = mysql_row[1];				//获取昵称
						nRet = IsStringNotEmpty(pBuf);
						if (TRUE == nRet)
						{
							nLen = strlen(pBuf);
							nLen = GetDataLen(NICKNAME_LEN, nLen);
							strncpy(pNickname, mysql_row[1], nLen);		
						}

						mysql_free_result(pMysql_res);								//释放内存
						ReleaseConn(pConn, pConf_file_fields->szTable_name);		//释放连接
						return TRUE;
					}
					else
					{
						mysql_free_result(pMysql_res);
						ReleaseConn(pConn, pConf_file_fields->szTable_name);
					}
				}
			}
			
			pItem = pItem->pNext;
		}
	}

	return FALSE;
}

//函数用途:判断用户ID是否已经存在
//输入参数: 用户ID
//输出参数: 无
/*返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  
  *用户ID存在,  返回TRUE,  否者返回FALSE 
  */

int IsTheSameUsrID(int nUsr_id)
{	
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	MYSQL_RES *pMysql_res = NULL;   
	char szBuf[1024];
	int nCount = 0;
	int nRet = 0;

	//获取登陆表名称
	nRet = GetLoginTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("IsTheSameUsrID: Call GetLoginTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}

	
	MYSQL *pConn = GetConn(szTable_name);			//获取数据库连接
	if (NULL == pConn)
	{
		LOG_ERROR("IsTheSameUsrID: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	snprintf(szBuf, sizeof(szBuf) - 1, "select * from %s where userid = %d", szTable_name, nUsr_id);

	nRet = mysql_query(pConn, szBuf);				//执行数据库操作
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("IsTheSameUsrID: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		pMysql_res = mysql_store_result(pConn);
		nCount = (int)mysql_num_rows(pMysql_res);					//获取数据库表的行数
		printf("IsTheSameUsrID: %d records found\n", nCount);

		mysql_free_result(pMysql_res);
		ReleaseConn(pConn, szTable_name);

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


//函数用途:插入注册信息到登录表中
//输入参数: 用户ID,  昵称, 密码, 注册类型,  帐号
//输出参数: 无
//返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  否者返回TRUE 

int InsertRegisInfoIntoDB(int nUsr_id, char *szNickname, char *szPasswd, Register_Type regis_type, char *szAccount)
{
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char szBuf[1024] = {0};
	int nRet = 0;

	nRet = GetLoginTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("InsertRegisInfoIntoDB: Call GetLoginTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}

	
	MYSQL *pConn = GetConn(szTable_name);				//获取数据库连接
	if (NULL == pConn)
	{
		LOG_ERROR("InsertUsrInfoIntoDB: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	//根据不同的注册类型而执行不同的表操作
	if (USR_ID_REGISTER == regis_type)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(userid, nickname, password) values(%d, '%s', '%s')", szTable_name, nUsr_id, szNickname, szPasswd);	
	}
	else if (USR_ACC_REGISTER == regis_type)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(UserId, nickname, username, password) values(%d, '%s', '%s', '%s')", szTable_name, nUsr_id, szNickname, szAccount, szPasswd);				
	}

	nRet = mysql_query(pConn, szBuf);							//执行数据库操作
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("InsertUsrInfoIntoDB: DB disconnect", FILE_NAME, FILE_LINE);
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

//函数用途:判断帐号是否相同
//输入参数: 帐号
//输出参数: 无
/*返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT, 
  *如果帐号相同,  返回TRUE,  否者返回FALSE 
  */

int IsTheSameAccount(char *szAccount)
{
	int nRet = 0;
	int i = 0;
	int nCount = 0;
	char szTable_name[MAX_TABLENAME_LEN] = {0}; 
	char szBuf[1024] = {0};
	MYSQL *pConn = NULL;
	DB_Conf_File_Fields *pConf_file_fields = NULL;
	StHash_Table *pRouting_table = NULL;
	StHash_Item *pItem = NULL;
	MYSQL_RES *pMysql_res = NULL; 	
	strcpy(szTable_name, "user_login");

	pRouting_table = GetDbRoutingTable();

	//轮询数据库路由表, 查找所有的登录表
	for (i=0; i<pRouting_table->nBucket_size; i++)
	{
		pItem = pRouting_table->ppItem[i];
		while (pItem != NULL)
		{
			pConf_file_fields = (DB_Conf_File_Fields *)pItem->pMatch_msg;

			//比较字符串
			if (0 == memcmp(pConf_file_fields->szTable_name, szTable_name, strlen(szTable_name)))
			{
				memset(szBuf, 0, sizeof(szBuf));
				pConn = GetConn(pConf_file_fields->szTable_name);					//获取数据库连接
				if (NULL == pConn)
				{
					LOG_ERROR("IsTheSameAccount: Call GetConn error", FILE_NAME, FILE_LINE);
					return CONN_POOL_ISNOT_EXIST;
				}
				
				snprintf(szBuf, sizeof(szBuf) - 1,"select * from %s where username = '%s'", pConf_file_fields->szTable_name, szAccount);
				
				nRet = mysql_query(pConn, szBuf);					//执行数据库操作
				if (nRet)
				{
					//数据库断连
					if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
					{
						printf("IsTheSameAccount: DB disconnect\n");
						ReconnDatabase(pConn, pConf_file_fields->szTable_name);
						return DB_DISCONNECT;	
					}
				}
				else
				{
					pMysql_res = mysql_store_result(pConn);
					nCount = (int)mysql_num_rows(pMysql_res);					//获取数据库表的行数
					printf("IsTheSameAccount: nCount=%d\n", nCount);
					if (nCount != 0)
					{
						mysql_free_result(pMysql_res);
						ReleaseConn(pConn, pConf_file_fields->szTable_name);
						return TRUE;
					}
					else
					{
						mysql_free_result(pMysql_res);
						ReleaseConn(pConn, pConf_file_fields->szTable_name);
					}
				}
			}
			
			pItem = pItem->pNext;
		}
	}

	return FALSE;	
}

//函数用途:判断用户ID是否存在
//输入参数: 用户ID
//输出参数: 无
/*返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT, 
  *如果用户ID存在,  返回TRUE,  否者返回FALSE 
  */

int IsUsrIDExistOrNot(int nUsr_id)
{
	int nCount = 0;
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	MYSQL_RES *pMysql_res = NULL;   
	char szBuf[1024] = {0};
	int nRet = 0;

	//获取登录表
	nRet = GetLoginTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("IsUsrIDExistOrNot: Call GetLoginTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	MYSQL *pConn = GetConn(szTable_name);				//获取连接
	if (NULL == pConn)
	{
		LOG_ERROR("IsUsrIDExistOrNot: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	memset(szBuf, 0, sizeof(szBuf));
	snprintf(szBuf, sizeof(szBuf) - 1, "select * from %s where UserId=%d", szTable_name, nUsr_id);

	nRet = mysql_query(pConn, szBuf);				//执行数据库操作
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			printf("IsUsrIDExistOrNot: DB disconnect\n");
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		pMysql_res = mysql_store_result(pConn);
		nCount= (int)mysql_num_rows(pMysql_res);							//获取数据库表的行数
		printf("IsUsrIDExistOrNot: %d records found\n", nCount);
	
		mysql_free_result(pMysql_res);						//释放内存
		ReleaseConn(pConn, szTable_name);					//释放连接
		
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

//函数用途:获取用户昵称
//输入参数: 用户ID
//输出参数: 存放昵称的缓存
//返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  否者返回TRUE 

int GetNickname(int nUsr_id, char *szNickname)
{
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	MYSQL_ROW mysql_row;
	MYSQL_RES *pMysql_res = NULL;   
	char szBuf[1024] = {0};
	int nCount = 0;
	char *pBuf = NULL;
	int nRet = 0;
	int nLen = 0;

	nRet = GetLoginTableName(nUsr_id, szTable_name);
	//表名不存在
	if (FALSE == nRet)
	{
		LOG_ERROR("GetNickname: Call GetLoginTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}

	
	MYSQL *pConn = GetConn(szTable_name);
	if (NULL == pConn)
	{
		LOG_ERROR("GetNickname: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
		
	snprintf(szBuf, sizeof(szBuf) - 1, "select nickname from %s where userid=%d", szTable_name, nUsr_id);

	nRet = mysql_query(pConn, szBuf);
	
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			printf("GetNickname: DB disconnect\n");
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		pMysql_res = mysql_store_result(pConn);
		nCount= (int)mysql_num_rows(pMysql_res);

		if (nCount != 0)					//表行数不为零
		{
			mysql_row =   mysql_fetch_row(pMysql_res);
			pBuf = mysql_row[0];				//获取昵称
			nRet = IsStringNotEmpty(pBuf);
			if (TRUE == nRet)
			{
				nLen = strlen(pBuf);
				nLen = GetDataLen(NICKNAME_LEN, nLen);
				strncpy(szNickname, pBuf, nLen);	
			}
		}

		mysql_free_result(pMysql_res);				//释放内存
		ReleaseConn(pConn, szTable_name);			//释放连接
		return TRUE;
	}

	return TRUE;
}

int GetMaxUsridInTheRange(int nUsrid_start, int nUsrid_end, int *pMax_usrid)
{
	INFO("GetMaxUsridInTheRange: func begin%s", "");
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	MYSQL_RES *pMysql_res = NULL;
	MYSQL_ROW mysql_row = NULL;
	char szSQL[1024] = {0};
	int nCount = 0;
	int nRet = 0; 

	nRet = GetLoginTableName(nUsrid_end, szTable_name);
	if (FALSE == nRet)
	{
		ERROR("GetMaxUsridInTheRange: Call GetLoginTableName error  db table is not exist%s", "");	
		return TABLE_ID_OUTOF_RANGE;
	}

	MYSQL *pConn = GetConn(szTable_name);			//获取连接
	if (NULL == pConn)
	{
		ERROR("GetMaxUsridInTheRange: Call GetConn error db connection  is not exits%s", "");
		return CONN_POOL_ISNOT_EXIST;
	}

	snprintf(szSQL, sizeof(szSQL) - 1, "select UserId from %s where UserId between %d and %d order by UserId desc limit 0, 1", \
		szTable_name, nUsrid_start, nUsrid_end);
	
	DEBUG("GetMaxUsridInTheRange: [sql]=%s", szSQL);

	nRet = mysql_query(pConn, szSQL);
	if (nRet)
	{
		ERROR("GetMaxUsridInTheRange: Call mysql_query error%s", "");
		nRet = HandleDBError(pConn, szTable_name);
		return nRet;
	}

	pMysql_res = mysql_store_result(pConn);
	if (NULL == pMysql_res)
	{
		ERROR("GetMaxUsridInTheRange: Call mysql_store_result error%s", "");
		nRet = HandleDBError(pConn, szTable_name);
		return nRet;
	}

	nCount = (int)mysql_num_rows(pMysql_res);
	DEBUG("GetMaxUsridInTheRange: [count]=%d", nCount);

	if (0 == nCount)
	{
		INFO("GetMaxUsridInTheRange: max usrid is not exist in the user login table in the certain range%s", "");
		nRet = FALSE;
	}
	else
	{
		mysql_row = mysql_fetch_row(pMysql_res);	
		DEBUG("GetMaxUsridInTheRange: [max uuid]=%s", mysql_row[0]);
		*pMax_usrid = atoi(mysql_row[0]);
		nRet = TRUE;	
	}

	mysql_free_result(pMysql_res);
	ReleaseConn(pConn, szTable_name);
	
	INFO("GetMaxUsridInTheRange: func end%s", "");
	return nRet;
}

