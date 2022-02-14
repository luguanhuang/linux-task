	
#include "../../util/service_global.h"
#include "../../interface/service_protocol.h"
#include "../../service/chat_service/service_create_cluster.h"

#include "service_table_usr_info.h"

/*****该文件主要操作用户信息表的****************/

extern Awake_Thread_Flg g_awake_thread_flg;

//函数用途: 获取用户信息表名称
//输入参数: 用户ID
//输出参数: 存放表名的缓存
//返回值	: 用户ID超出了表的范围,  返回FALSE，  否者返回TRUE 

int GetUsrInfoTableName(int nUsr_id, char *szTableName)
{
	char szBuf[MAX_TABLENAME_LEN] = {0};

	if (nUsr_id >= ONE && nUsr_id <= ONE_HUNDRED_THOUSAND)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s_%s", "user_info", "1_100000");	
	}
	else if (nUsr_id > ONE_HUNDRED_THOUSAND && nUsr_id <= TWO_HUNDRED_THOUSAND)
	{
		snprintf(szBuf, sizeof(szBuf), "%s_%s", "user_info", "100001_200000");		
	}
	else
	{
		LOG_ERROR("GetUsrInfoTableName: table id out of range", FILE_NAME, FILE_LINE);
		return FALSE;	
	}
	
	strcpy(szTableName, szBuf);
	return TRUE;
}

//函数用途: 获取个性签名， 图像索引和图像类型
//输入参数:用户ID
//输出参数:存放 个性签名,  图像索引,  图像类型的缓存
//返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  否者返回TRUE

int GetSigAndPhotoIdx(int nUsr_id, char *szSelfSig, BYTE *pPhoto_idx, EnImage_Type *pImage_type)
{
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	MYSQL_ROW   mysql_row;
	MYSQL_RES *pMysql_res = NULL;   
	char szBuf[1024] = {0};
	int nCount = 0;
	int nRet = 0;
	int nLen = 0;
	char *pBuf = NULL;
	//DB_Conf_File_Fields *pConf_file_fields = NULL;

	nRet = GetUsrInfoTableName(nUsr_id, szTable_name);
	//表名不存在
	if (FALSE == nRet)
	{
		LOG_ERROR("GetSigAndPhotoIdx: Call GetUsrInfoTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}

	//获取数据库连接
	MYSQL *pConn = GetConn(szTable_name);

	if (NULL == pConn)
	{
		LOG_ERROR("GetSigAndPhotoIdx: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	//数据库表的操作语句
	sprintf(szBuf, "select BriefIntroduction, Image_System, Image_User_Define from %s where userid=%d", szTable_name, nUsr_id);

	nRet = mysql_query(pConn, szBuf);				//操作数据库表
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			printf("GetSigAndPhotoIdx: DB disconnect\n");
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		pMysql_res = mysql_store_result(pConn);
		nCount = (int)mysql_num_rows(pMysql_res);			//获取数据库表数据的行数
		if (nCount != 0)
		{
			mysql_row =   mysql_fetch_row(pMysql_res);			//获取数据库表的一行数据
			pBuf = mysql_row[0];	

			//判断字符串是否为空
			nRet = IsStringNotEmpty(pBuf);

			if (TRUE == nRet)
			{
				nLen = strlen(pBuf);

				//获取数据长度
				nLen = GetDataLen(SEL_SIGNATUER_LEN, nLen);
				strncpy(szSelfSig, pBuf, nLen);			//获取个性签名
			}

			pBuf = mysql_row[2];					//获取表的字段

			//判断字符串是否为空
			nRet = IsStringNotEmpty(pBuf);
			if (FALSE == nRet)
			{
				pBuf = mysql_row[1];
				nRet = IsStringNotEmpty(pBuf);
				if (TRUE == nRet)
				{
					*pPhoto_idx = (BYTE)atoi(pBuf);	
					*pImage_type = SYSTEM_IMAGE;			//系统图像
				}
			}
			else
			{
				*pPhoto_idx = 0;	
				*pImage_type = USR_DEFINE_IMAGE;			//用户自定义图像
			}
		}

		//释放内存
		mysql_free_result(pMysql_res);

		//释放连接
		ReleaseConn(pConn, szTable_name);
		return TRUE;
	}

	return TRUE;
}

//函数用途: 获取图像索引和图像类型
//输入参数:用户ID
//输出参数:存放 图像索引,  图像类型的缓存
//返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  否者返回TRUE

int GetPhotoIdx(int nUsr_id, BYTE *pPhoto_idx, EnImage_Type *pImage_type)
{
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	MYSQL_ROW   mysql_row;
	MYSQL_RES *pMysql_res = NULL;   
	char szBuf[1024] = {0};
	char *pBuf = NULL;
	int nRet = 0;

	nRet = GetUsrInfoTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("GetPhotoIdx: Call GetUsrInfoTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}

	
	MYSQL *pConn = GetConn(szTable_name);			//获取连接
	if (NULL == pConn)
	{
		LOG_ERROR("GetPhotoIdx: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	sprintf(szBuf, "select Image_System, Image_User_Define from %s where userid=%d", szTable_name, nUsr_id);

	
	nRet = mysql_query(pConn, szBuf);					//执行数据库操作
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			printf("GetPhotoIdx: DB disconnect\n");
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		pMysql_res = mysql_store_result(pConn);
		mysql_row =   mysql_fetch_row(pMysql_res);				//获取数据库表的一行
		pBuf = mysql_row[1];	

		//判断字符串是否为空
		nRet = IsStringNotEmpty(pBuf);
		
		if (FALSE == nRet)
		{
			pBuf = mysql_row[0];

			//判断字符串是否为空
			nRet = IsStringNotEmpty(pBuf);
			if (TRUE == nRet)
			{
				*pPhoto_idx = (BYTE)atoi(pBuf);	
				*pImage_type = SYSTEM_IMAGE;					//系统图像		
			}
		}
		else
		{
			*pPhoto_idx = 0;
			*pImage_type = USR_DEFINE_IMAGE;				//用户自定义图像
		}

		mysql_free_result(pMysql_res);						//释放内存
		ReleaseConn(pConn, szTable_name);					//释放连接
		return TRUE;
	}

	return TRUE;
}

//函数用途:插入用户信息到用户信息表中
//输入参数:用户ID,  性别
//输出参数:无
//返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  否者返回TRUE

int InsertUsrInfo(int nUsr_id, BYTE bSex)
{
	int nRet = 0;
	int nImage_idx = 0;
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char szBuf[1024] = {0};
	char szTime[100] = {0};
	time_t now;
	time(&now);
	struct tm *pTime = localtime(&now);				//当前获取时间
	int nYear = pTime->tm_year + 1900;
	int nMonth = pTime->tm_mon + 1;
	int nDay = pTime->tm_mday;
	int nHour = pTime->tm_hour;
	int nMinute = pTime->tm_min;
	int nSecond = pTime->tm_sec;

	
	nRet = GetUsrInfoTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("InsertUsrInfo: Call GetUsrInfoTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}

	
	MYSQL *pConn = GetConn(szTable_name);			//获取连接
	if (NULL == pConn)
	{
		ERROR("InsertUsrInfo: Call GetConn error%s", "");
		return CONN_POOL_ISNOT_EXIST;
	}
	
	snprintf(szTime, sizeof(szTime) - 1, "%d-%d-%d %d:%d:%d", nYear, nMonth, nDay, nHour, nMinute, nSecond);

	//数据库的执行语句
	snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(UserId, Sex, Image_System, AddTime) values(%d, %d, %d, '%s')", szTable_name, nUsr_id, bSex, nImage_idx, szTime);			

	nRet = mysql_query(pConn, szBuf);			//执行数据库操作
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("InsertUsrInfo: DB disconnect", FILE_NAME, FILE_LINE);
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

//函数用途:判断是否有足够的创群权限
//输入参数:用户ID
//输出参数:无
/*返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  
  *	如果没有足够的权限创建群,  返回NOT_ENOUGH_CLUSTER_LEVEL, 用户信息不存在,  返回FALSE,  否者返回TRUE
  */

int IsEnoughClusterLevel(int nUsr_id)
{
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	MYSQL_ROW   mysql_row;
	MYSQL_RES *pMysql_res = NULL;   
	char szBuf[1024] = {0};
	int nCount = 0;
	int nCluster_level = 0;
	int nRet = 0;
	char *pBuf = NULL;

	//获取用户信息表名称
	nRet = GetUsrInfoTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("IsEnoughClusterLevel: Call GetUsrInfoTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}

	
	MYSQL *pConn = GetConn(szTable_name);			//获取数据库连接
	if (NULL == pConn)
	{
		LOG_ERROR("IsEnoughClusterLevel: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

		
	snprintf(szBuf, sizeof(szBuf), "select Level from %s where userid=%d", szTable_name, nUsr_id);

	nRet = mysql_query(pConn, szBuf);				//执行数据库操作
	if (nRet)
	{	
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("IsEnoughClusterLevel: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		pMysql_res = mysql_store_result(pConn);
		nCount = (int)mysql_num_rows(pMysql_res);			//获取行数
		if (nCount != 0)
		{
			mysql_row =   mysql_fetch_row(pMysql_res);		//获取行
			pBuf = mysql_row[0];	

			//判断字符串是否为空
			nRet = IsStringNotEmpty(pBuf);
			if (TRUE == nRet)
			{
				nCluster_level = atoi(pBuf);
				//判断是否有足够的权限创建群
				if (nCluster_level < CLUSTER_LEVEL)
				{
					mysql_free_result(pMysql_res);
					ReleaseConn(pConn, szTable_name);
					return NOT_ENOUGH_CLUSTER_LEVEL;
				}
				else
				{
					mysql_free_result(pMysql_res);
					ReleaseConn(pConn, szTable_name);	
					return TRUE;	
				}	
			}
			
		}

		mysql_free_result(pMysql_res);
		ReleaseConn(pConn, szTable_name);
		return FALSE;
	}

	return TRUE;
}

//函数用途: 插入图像数据到用户信息表中
//输入参数: 用户ID,  图像数据,  图像后缀,  图像大小
//输出参数: 无
//返回值	:  表名不存在,  返回TABLE_ID_OUTOF_RANGE,  数据库断连,   返回DB_DISCONNECT,  否者返回TRUE

int InsertImageData(int nUsr_id, BYTE *pImage_data, BYTE *pImage_postfix, int nImage_size)
{
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	int nRet = 0;
	char arrImage_data[2*MAX_IMAGE_SIZE+1] = {0};
	char szBuf[2*MAX_IMAGE_SIZE+1024] = {0};
	int nLen = 0;

	//获取用户信息表名称
	nRet = GetUsrInfoTableName(nUsr_id, szTable_name);
	//表名不存在
	if (FALSE == nRet)
	{
		LOG_ERROR("InsertImageData: Call GetUsrInfoTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}

	
	MYSQL *pConn = GetConn(szTable_name);		//获取连接
	if (NULL == pConn)
	{
		LOG_ERROR("InsertImageData: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	mysql_real_escape_string(pConn, arrImage_data, pImage_data, nImage_size);			//对字符串中的特殊字符进行转义处理
	nLen = snprintf(szBuf, sizeof(szBuf) - 1, "update %s set ImageData='%s', ImagePostfix='%s' where UserId=%d", szTable_name, arrImage_data, pImage_postfix, nUsr_id);

	nRet = mysql_real_query(pConn, szBuf, nLen);			//执行数据库操作
	if (nRet)
	{
		//数据库断连
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			printf("InsertImageData: DB disconnect\n");
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
