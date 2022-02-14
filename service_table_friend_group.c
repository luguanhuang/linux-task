#include "../../util/service_global.h"
#include "../../interface/service_protocol.h"
#include "service_table_friend_group.h"

/*****���ļ���Ҫ��������***************/


//������;: ��ȡ�������
//�������: �û�ID
//�������: ��ű����Ļ���
//����ֵ	: �û�ID�����˱�ķ�Χ,  ����FALSE��  ���߷���TRUE 

int GetGroupTableName(int nUsr_id, char *szTableName)
{
	char szBuf[MAX_TABLENAME_LEN] = {0};

	if (nUsr_id >= ONE && nUsr_id <= ONE_HUNDRED_THOUSAND)
	{
		sprintf(szBuf, "%s_%s", "friend_group", "1_100000");	
	}
	else if (nUsr_id > ONE_HUNDRED_THOUSAND && nUsr_id <= TWO_HUNDRED_THOUSAND)
	{
		sprintf(szBuf, "%s_%s", "friend_group", "100001_200000");		
	}
	else
	{
		LOG_ERROR("GetGroupTableName: table id out of range", FILE_NAME, FILE_LINE);
		return FALSE;	
	}
	
	
	strcpy(szTableName, szBuf);
	return TRUE;
}

//������;: �ж���ID�Ƿ����
//�������: �û�ID,  ��ID
//�������: ��
/*����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  
  *�����ID����,  ����TRUE,  ���߷���FALSE 
  */

int IsGroupIDExistOrNot(int nUsr_id, int nGroup_id)
{	
	int nCount = 0;
	int nRet = 0;
	char szTable_name[MAX_TABLENAME_LEN];
	MYSQL_RES *pMysql_res = NULL;   
	char szBuf[1024] = {0};
	memset(szBuf, 0, sizeof(szBuf));

	nRet = GetGroupTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("IsGroupIDExistOrNot: Call GetGroupTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}

	sprintf(szBuf, "select * from %s where GroupId=%d and Creator_UserId=%d", szTable_name, nGroup_id, nUsr_id);
	
	MYSQL *pConn = GetConn(szTable_name);				//��ȡ����
	if (NULL == pConn)
	{
		LOG_ERROR("IsGroupIDExistOrNot: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	nRet = mysql_query(pConn, szBuf);
	if (nRet)
	{
		//���ݿ����
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("IsGroupIDExistOrNot: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		pMysql_res = mysql_store_result(pConn);
		nCount= (int)mysql_num_rows(pMysql_res);					//��ȡ���ݿ�������
		printf("IsGroupIDExist: %d records found\n", nCount);

		//�ͷ�����
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

//������;: ������в���������ID
//�������: �û�ID
//�������: �����ID�Ļ���
/*����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  
  *���������Ϊ��,  ����FALSE,  ���߷���TRUE  
  */

int FindMaxGroupID(unsigned int nUsr_id, unsigned int *pMax_group_id)
{
	int nRet = 0;
	int nCount = 0;
	MYSQL_ROW   mysql_row;
	MYSQL_RES *pMysql_res = NULL;   
	char szBuf[1024] = {0};
	char *pBuf = NULL;
	MYSQL *pConn = NULL;
	char szTable_name[MAX_TABLENAME_LEN] = {0};

	
	nRet = GetGroupTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("FindMaxGroupID: Call GetGroupTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	pConn = GetConn(szTable_name);					//��ȡ����
	if (NULL == pConn)
	{
		LOG_ERROR("FindMaxGroupID: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	snprintf(szBuf, sizeof(szBuf) - 1, "select * from %s where Creator_UserId=%d and GroupId=(select max(GroupId) from %s where Creator_UserId=%d)", \
		szTable_name, nUsr_id, szTable_name, nUsr_id);
	nRet = mysql_query(pConn, szBuf);				//ִ�б����
	
	if (nRet)
	{	
		//���ݿ����
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("FindMaxGroupID: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		
		pMysql_res = mysql_store_result(pConn);
		nCount= (int)mysql_num_rows(pMysql_res);

		if (0 == nCount)				//��������Ϊ��ʱ
		{
			*pMax_group_id	= 1;
			mysql_free_result(pMysql_res);
			ReleaseConn(pConn, szTable_name);
			return FALSE;
		}
		else
		{
			mysql_row =   mysql_fetch_row(pMysql_res);
			pBuf = mysql_row[0];				//��ȡ������ID��
			nRet = IsStringNotEmpty(pBuf);
			if (TRUE == nRet)
			{
				*pMax_group_id = atoi(pBuf);		
			}
			
			mysql_free_result(pMysql_res);				//�ͷ��ڴ�
			ReleaseConn(pConn, szTable_name);			//�ͷ�����
			return TRUE;	
		}
	}

	return TRUE;
}

//������;:������� ��ȡ������
//�������: �û�ID
//�������: ����������Ļ���
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE 

int GetGroupNum(int nUsr_id, int *pGroup_num)
{
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char szBuf[1024] = {0};
	int nRet = 0;
	int nCount = 0;
	MYSQL_RES *pMysql_res = NULL;   

	//��ȡ�������
	nRet = GetGroupTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("GetGroupNum: Call GetGroupTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	

	MYSQL *pConn = GetConn(szTable_name);			//��ȡ����
	if (NULL == pConn)
	{
		LOG_ERROR("GetGroupNum: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	snprintf(szBuf, sizeof(szBuf) - 1, "select * from %s where Creator_UserId=%d", szTable_name, nUsr_id);

	nRet = mysql_query(pConn, szBuf);				//ִ�����ݿ�����

	if (nRet)
	{		
		//����
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("GetGroupNum: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		pMysql_res = mysql_store_result(pConn);
		nCount = (int)mysql_num_rows(pMysql_res);			//��ȡ������

		printf("GetGroupNum: nCount=%d\n", nCount);

		*pGroup_num = nCount;
		mysql_free_result(pMysql_res);
		ReleaseConn(pConn, szTable_name);
		return TRUE;
	}

	return TRUE;
}

//������;:������������ݵ������
//�������: ��ID,  �û�ID,  ������
//�������: ��
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE 

int InsertFriGroupData(unsigned int nGroup_id, unsigned int nUsr_id, char *szGroup_name)
{
	char szTable_name[MAX_TABLENAME_LEN];
	int nRet = 0;
	char szBuf[1024] = {0};
	memset(szBuf, 0, sizeof(szBuf));
	char szTime[100] = {0};
	time_t now;
	time(&now);
	struct tm *pTime = localtime(&now);			//��ȡ��ǰʱ��
	int nYear = pTime->tm_year + 1900;
	int nMonth = pTime->tm_mon + 1;
	int nDay = pTime->tm_mday;
	int nHour = pTime->tm_hour;
	int nMinute = pTime->tm_min;
	int nSecond = pTime->tm_sec;

	nRet = GetGroupTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("InsertFriGroupData: Call GetGroupTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	MYSQL *pConn = GetConn(szTable_name);			//��ȡ����
	if (NULL == pConn)
	{
		LOG_ERROR("InsertFriGroupData: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	snprintf(szTime, sizeof(szTime) - 1, "%d-%d-%d %d:%d:%d", nYear, nMonth, nDay, nHour, nMinute, nSecond);
	snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(GroupId, GroupName, Creator_UserId, AddTime) values(%d, '%s', %d, '%s')", szTable_name, nGroup_id, szGroup_name, nUsr_id, szTime);

	nRet = mysql_query(pConn, szBuf);			//ִ�����ݿ����
	if (nRet)
	{
		//����
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			printf("InsertFriGroupData: DB disconnect\n");
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

//������;:ɾ������������
//�������: ��ID,  �û�ID,
//�������: ��
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE 

int DeleteFriGroupData(unsigned int nGroup_id, unsigned int nUsr_id)
{
	int nRet = 0;
	char szBuf[1024] = {0};
	MYSQL *pConn = NULL;
	char szTable_name[MAX_TABLENAME_LEN] = {0};

	
	nRet = GetGroupTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("DeleteFriGroupData: Call GetGroupTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	pConn = GetConn(szTable_name);
	if (NULL == pConn)
	{
		LOG_ERROR("DeleteFriGroupData: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	sprintf(szBuf, "delete from %s where GroupId=%d and Creator_UserId=%d", szTable_name, nGroup_id, nUsr_id);

	printf("DeleteFriGroupData: szBuf=%s\n", szBuf);

	nRet = mysql_query(pConn, szBuf);			//ִ�����ݿ����
	if (nRet)
	{
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("DeleteFriGroupData: DB disconnect", FILE_NAME, FILE_LINE);
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

//������;:��ȡ�û���ӵ�е�����Ϣ
//�������: �û�ID
//�������: ����û���ӵ�е�������,  ������(��ID,  ������)�Ļ���
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE 

int GetFriendGroupInfo(int nUsr_id, WORD *pCount, Group_Data *pGroup_data)
{

	char szTable_name[MAX_TABLENAME_LEN] = {0};
	MYSQL_ROW   mysql_row;
	MYSQL_RES *pMysql_res = NULL;   
	char szBuf[500] = {0};
	char *pBuf = NULL;
	int nRes = 0;
	int nRet = 0;
	WORD wCount = 0;
	int i = 0;
	int nLen = 0;
	
	nRet = GetGroupTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("GetFriendGroupInfo: Call GetGroupTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	MYSQL *pConn = GetConn(szTable_name);				//��ȡ���ݿ�����
	if (NULL == pConn)
	{
		LOG_ERROR("GetFriendGroupInfo: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	snprintf(szBuf, sizeof(szBuf) - 1, "select GroupId, GroupName from %s where creator_userid = %d order by GroupId", szTable_name, nUsr_id);
	
	nRet = mysql_query(pConn, szBuf);
	if (nRet)
	{	
		//���ݿ����
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("GetFriendGroupInfo: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		pMysql_res = mysql_store_result(pConn);
		wCount = (WORD)mysql_num_rows(pMysql_res);						//���ݿ�������
		printf("GetFriendGroupInfo: %d records found\n", wCount);

		if (wCount!= 0)
		{
			*pCount = wCount;
			for (i=0; i<wCount; i++)
			{
				
				mysql_row =   mysql_fetch_row(pMysql_res);
				pBuf = mysql_row[0];				//��ȡ��ID
				nRet = IsStringNotEmpty(pBuf);
				if (TRUE == nRet)
				{
					nRes = atoi(pBuf);
					pGroup_data[i].nGroup_id = htonl(nRes);
				}
				
				pBuf = mysql_row[1];				//��ȡ���� 
				nRet = IsStringNotEmpty(pBuf);
				if (TRUE == nRet)
				{
					nLen = strlen(pBuf);
					nLen = GetDataLen(GROUP_NAME_LEN, nLen);
					strncpy(pGroup_data[i].arrGroup_name, pBuf, nLen);		
				}
			}
		}

		mysql_free_result(pMysql_res);				//�ͷ��ڴ�
		ReleaseConn(pConn, szTable_name);			//�ͷ�����
		return TRUE;
	}

	return TRUE;
}



