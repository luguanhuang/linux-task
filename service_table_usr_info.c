	
#include "../../util/service_global.h"
#include "../../interface/service_protocol.h"
#include "../../service/chat_service/service_create_cluster.h"

#include "service_table_usr_info.h"

/*****���ļ���Ҫ�����û���Ϣ���****************/

extern Awake_Thread_Flg g_awake_thread_flg;

//������;: ��ȡ�û���Ϣ������
//�������: �û�ID
//�������: ��ű����Ļ���
//����ֵ	: �û�ID�����˱�ķ�Χ,  ����FALSE��  ���߷���TRUE 

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

//������;: ��ȡ����ǩ���� ͼ��������ͼ������
//�������:�û�ID
//�������:��� ����ǩ��,  ͼ������,  ͼ�����͵Ļ���
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE

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
	//����������
	if (FALSE == nRet)
	{
		LOG_ERROR("GetSigAndPhotoIdx: Call GetUsrInfoTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}

	//��ȡ���ݿ�����
	MYSQL *pConn = GetConn(szTable_name);

	if (NULL == pConn)
	{
		LOG_ERROR("GetSigAndPhotoIdx: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	//���ݿ��Ĳ������
	sprintf(szBuf, "select BriefIntroduction, Image_System, Image_User_Define from %s where userid=%d", szTable_name, nUsr_id);

	nRet = mysql_query(pConn, szBuf);				//�������ݿ��
	if (nRet)
	{
		//���ݿ����
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
		nCount = (int)mysql_num_rows(pMysql_res);			//��ȡ���ݿ�����ݵ�����
		if (nCount != 0)
		{
			mysql_row =   mysql_fetch_row(pMysql_res);			//��ȡ���ݿ���һ������
			pBuf = mysql_row[0];	

			//�ж��ַ����Ƿ�Ϊ��
			nRet = IsStringNotEmpty(pBuf);

			if (TRUE == nRet)
			{
				nLen = strlen(pBuf);

				//��ȡ���ݳ���
				nLen = GetDataLen(SEL_SIGNATUER_LEN, nLen);
				strncpy(szSelfSig, pBuf, nLen);			//��ȡ����ǩ��
			}

			pBuf = mysql_row[2];					//��ȡ����ֶ�

			//�ж��ַ����Ƿ�Ϊ��
			nRet = IsStringNotEmpty(pBuf);
			if (FALSE == nRet)
			{
				pBuf = mysql_row[1];
				nRet = IsStringNotEmpty(pBuf);
				if (TRUE == nRet)
				{
					*pPhoto_idx = (BYTE)atoi(pBuf);	
					*pImage_type = SYSTEM_IMAGE;			//ϵͳͼ��
				}
			}
			else
			{
				*pPhoto_idx = 0;	
				*pImage_type = USR_DEFINE_IMAGE;			//�û��Զ���ͼ��
			}
		}

		//�ͷ��ڴ�
		mysql_free_result(pMysql_res);

		//�ͷ�����
		ReleaseConn(pConn, szTable_name);
		return TRUE;
	}

	return TRUE;
}

//������;: ��ȡͼ��������ͼ������
//�������:�û�ID
//�������:��� ͼ������,  ͼ�����͵Ļ���
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE

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

	
	MYSQL *pConn = GetConn(szTable_name);			//��ȡ����
	if (NULL == pConn)
	{
		LOG_ERROR("GetPhotoIdx: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	sprintf(szBuf, "select Image_System, Image_User_Define from %s where userid=%d", szTable_name, nUsr_id);

	
	nRet = mysql_query(pConn, szBuf);					//ִ�����ݿ����
	if (nRet)
	{
		//���ݿ����
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
		mysql_row =   mysql_fetch_row(pMysql_res);				//��ȡ���ݿ���һ��
		pBuf = mysql_row[1];	

		//�ж��ַ����Ƿ�Ϊ��
		nRet = IsStringNotEmpty(pBuf);
		
		if (FALSE == nRet)
		{
			pBuf = mysql_row[0];

			//�ж��ַ����Ƿ�Ϊ��
			nRet = IsStringNotEmpty(pBuf);
			if (TRUE == nRet)
			{
				*pPhoto_idx = (BYTE)atoi(pBuf);	
				*pImage_type = SYSTEM_IMAGE;					//ϵͳͼ��		
			}
		}
		else
		{
			*pPhoto_idx = 0;
			*pImage_type = USR_DEFINE_IMAGE;				//�û��Զ���ͼ��
		}

		mysql_free_result(pMysql_res);						//�ͷ��ڴ�
		ReleaseConn(pConn, szTable_name);					//�ͷ�����
		return TRUE;
	}

	return TRUE;
}

//������;:�����û���Ϣ���û���Ϣ����
//�������:�û�ID,  �Ա�
//�������:��
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE

int InsertUsrInfo(int nUsr_id, BYTE bSex)
{
	int nRet = 0;
	int nImage_idx = 0;
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char szBuf[1024] = {0};
	char szTime[100] = {0};
	time_t now;
	time(&now);
	struct tm *pTime = localtime(&now);				//��ǰ��ȡʱ��
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

	
	MYSQL *pConn = GetConn(szTable_name);			//��ȡ����
	if (NULL == pConn)
	{
		ERROR("InsertUsrInfo: Call GetConn error%s", "");
		return CONN_POOL_ISNOT_EXIST;
	}
	
	snprintf(szTime, sizeof(szTime) - 1, "%d-%d-%d %d:%d:%d", nYear, nMonth, nDay, nHour, nMinute, nSecond);

	//���ݿ��ִ�����
	snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(UserId, Sex, Image_System, AddTime) values(%d, %d, %d, '%s')", szTable_name, nUsr_id, bSex, nImage_idx, szTime);			

	nRet = mysql_query(pConn, szBuf);			//ִ�����ݿ����
	if (nRet)
	{
		//���ݿ����
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

//������;:�ж��Ƿ����㹻�Ĵ�ȺȨ��
//�������:�û�ID
//�������:��
/*����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  
  *	���û���㹻��Ȩ�޴���Ⱥ,  ����NOT_ENOUGH_CLUSTER_LEVEL, �û���Ϣ������,  ����FALSE,  ���߷���TRUE
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

	//��ȡ�û���Ϣ������
	nRet = GetUsrInfoTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("IsEnoughClusterLevel: Call GetUsrInfoTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}

	
	MYSQL *pConn = GetConn(szTable_name);			//��ȡ���ݿ�����
	if (NULL == pConn)
	{
		LOG_ERROR("IsEnoughClusterLevel: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

		
	snprintf(szBuf, sizeof(szBuf), "select Level from %s where userid=%d", szTable_name, nUsr_id);

	nRet = mysql_query(pConn, szBuf);				//ִ�����ݿ����
	if (nRet)
	{	
		//���ݿ����
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
		nCount = (int)mysql_num_rows(pMysql_res);			//��ȡ����
		if (nCount != 0)
		{
			mysql_row =   mysql_fetch_row(pMysql_res);		//��ȡ��
			pBuf = mysql_row[0];	

			//�ж��ַ����Ƿ�Ϊ��
			nRet = IsStringNotEmpty(pBuf);
			if (TRUE == nRet)
			{
				nCluster_level = atoi(pBuf);
				//�ж��Ƿ����㹻��Ȩ�޴���Ⱥ
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

//������;: ����ͼ�����ݵ��û���Ϣ����
//�������: �û�ID,  ͼ������,  ͼ���׺,  ͼ���С
//�������: ��
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE

int InsertImageData(int nUsr_id, BYTE *pImage_data, BYTE *pImage_postfix, int nImage_size)
{
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	int nRet = 0;
	char arrImage_data[2*MAX_IMAGE_SIZE+1] = {0};
	char szBuf[2*MAX_IMAGE_SIZE+1024] = {0};
	int nLen = 0;

	//��ȡ�û���Ϣ������
	nRet = GetUsrInfoTableName(nUsr_id, szTable_name);
	//����������
	if (FALSE == nRet)
	{
		LOG_ERROR("InsertImageData: Call GetUsrInfoTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}

	
	MYSQL *pConn = GetConn(szTable_name);		//��ȡ����
	if (NULL == pConn)
	{
		LOG_ERROR("InsertImageData: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	mysql_real_escape_string(pConn, arrImage_data, pImage_data, nImage_size);			//���ַ����е������ַ�����ת�崦��
	nLen = snprintf(szBuf, sizeof(szBuf) - 1, "update %s set ImageData='%s', ImagePostfix='%s' where UserId=%d", szTable_name, arrImage_data, pImage_postfix, nUsr_id);

	nRet = mysql_real_query(pConn, szBuf, nLen);			//ִ�����ݿ����
	if (nRet)
	{
		//���ݿ����
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
