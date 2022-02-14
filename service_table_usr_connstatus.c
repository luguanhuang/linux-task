#include "../../util/service_global.h"
#include "../../interface/service_protocol.h"
#include "service_table_usr_connstatus.h"

/*****���ļ���Ҫ������¼��ʱ��� ****************/


//������;: ��ȡ��¼��ʱ������
//�������: �û�ID
//�������: ��ű����Ļ���
//����ֵ	: �û�ID�����˱�ķ�Χ,  ����FALSE��  ���߷���TRUE 

int GetLoginTmpTableName(int nUsr_id, char *szTableName)
{
	char szBuf[MAX_TABLENAME_LEN] = {0};

	if (nUsr_id >= ONE && nUsr_id <= ONE_HUNDRED_THOUSAND)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s_%s", "user_connectstatus_temporary_table", "1_100000");	
	}
	else if (nUsr_id > ONE_HUNDRED_THOUSAND && nUsr_id <= TWO_HUNDRED_THOUSAND)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s_%s", "user_connectstatus_temporary_table", "100001_200000");		
	}
	else
	{
		LOG_ERROR("GetLoginTmpTableName: table id out of range", FILE_NAME, FILE_LINE);
		return FALSE;	
	}
	
	
	strcpy(szTableName, szBuf);
	return TRUE;
}

//������;:�ж��û��Ƿ��Ѿ���¼ϵͳ
//�������:�û�ID
//�������:��
/*����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  
  *�û�û�е�¼ϵͳ,  ����FALSE,  ���߷���TRUE
  */

int IsUsrLogin(int nUsr_id)
{
	int nRet = 0;
	int nCount = 0;
	MYSQL_RES *pMysql_res = NULL;   
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char szBuf[1024] = {0};

	nRet = GetLoginTmpTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("IsUsrLogin: Call GetLoginTmpTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}

	
	MYSQL *pConn = GetConn(szTable_name);				//��ȡ���ݿ�����
	if (NULL == pConn)
	{
		LOG_ERROR("IsUsrLogin: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	memset(szBuf, 0, sizeof(szBuf));
	snprintf(szBuf, sizeof(szBuf) - 1, "select * from %s where userid=%d", szTable_name, nUsr_id);
	nRet = mysql_query(pConn, szBuf);					//ִ�����ݿ����
	
	if (nRet)
	{
		//���ݿ����
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("IsUsrLogin: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		pMysql_res = mysql_store_result(pConn);
		nCount = (int)mysql_num_rows(pMysql_res);					//��ȡ���ݿ�����ݵ�����
		printf("IsUsrLogin: usr_id=%d %d records found\n", nUsr_id, nCount);
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

//������;: �����¼��Ϣ����¼��ʱ����
//�������:�û�ID,  ҵ����������,  �ͻ�������������֮���socket id
//�������:��
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE

int InsertLoginInfoIntoDB(int nUsr_id, int nSvr_seq, int nSock)
{	
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char szBuf[2024] = {0};
	memset(szBuf, 0, sizeof(szBuf));
	int nRet = 0;


	nRet = GetLoginTmpTableName(nUsr_id, szTable_name);
	//����������
	if (FALSE == nRet)
	{
		LOG_ERROR("InsertLoginInfoIntoDB: Call GetLoginTmpTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}

	
	MYSQL *pConn = GetConn(szTable_name);			//��ȡ���ݿ�����
	if (NULL == pConn)
	{
		LOG_ERROR("InsertLoginInfoIntoDB: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	//���ݿ���ִ�� ���
	snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(userid, AccessServer_ID, Socket_Client_ConnectWith_AccessServer) values(%d, %d, %d)", szTable_name, nUsr_id, nSvr_seq, nSock);

	nRet = mysql_query(pConn, szBuf);				//ִ�����ݿ����
	if (nRet)
	{
		//���ݿ����
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("InsertLoginInfoIntoDB: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		ReleaseConn(pConn, szTable_name);				//�ͷ�����
		return TRUE;
	}

	return TRUE;
}

//������;: ɾ���û��ĵ�¼��Ϣ
//�������:�û�ID
//�������:��
//����ֵ	: ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE

int DeleteItemByUsrId(unsigned int nUsr_id)
{
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char szBuf[1024] = {0}; 
	MYSQL *pConn = NULL;
	int nRet = 0;

	//��ȡ��¼��ʱ������
	nRet = GetLoginTmpTableName(nUsr_id, szTable_name);

	//����������
	if (FALSE == nRet)
	{
		LOG_ERROR("DeleteItemByUsrId: Call GetLoginTmpTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}

	
	pConn = GetConn(szTable_name);				//��ȡ���ݿ�����
	if (NULL == pConn)
	{
		LOG_ERROR("DeleteItemByUsrId: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	snprintf(szBuf, sizeof(szBuf) - 1, "delete from %s where userid=%d", szTable_name, nUsr_id);			//���ݿ���ִ�����
	nRet = mysql_query(pConn, szBuf);
	if (nRet)
	{
		//���ݿ����
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			printf("DeleteItemByUsrId: DB disconnect\n");
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

