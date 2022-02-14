#include "../../util/service_global.h"
#include "../../interface/service_protocol.h"
#include "../handle_table/service_handle_table.h"
#include "service_table_group_detail.h"

/*****���ļ���Ҫ����������***************/


//������;: ��ȡ���������
//�������: �û�ID
//�������: ��ű����Ļ���
//����ֵ	: �û�ID�����˱�ķ�Χ,  ����FALSE��  ���߷���TRUE 


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

//������;: �жϺ��������Ƿ����
//�������: �û�ID,  ��ID,  ����ID
//�������: ��
/*����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  
  *������������Ѿ�����,  ����TRUE,  ���߷���FALSE 
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

	nRet = mysql_query(pConn, szBuf);				//ִ�����ݿ�����
	if (nRet)
	{
		//���ݿ����
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
		nCount= (int)mysql_num_rows(pMysql_ret);					//��ȡ���ݿ�������
		printf("IsFriendDataExistOrNot: %d records found\n", nCount);

		mysql_free_result(pMysql_ret);				//�ͷ��ڴ�
		ReleaseConn(pConn, szTable_name);			//�ͷ�����
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

//������;: ����������� ���������
//�������: �û�ID,  ��ID,  ����ID
//�������: ��
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,   ���߷���TRUE 

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

	
	MYSQL *pConn = GetConn(szTable_name);					//��ȡ����
	if (NULL == pConn)
	{
		LOG_ERROR("InsertFriendData: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	memset(szBuf, 0, sizeof(szBuf));

	//��������
	sprintf(szBuf, "insert into %s(Creator_UserId, GroupId, Friend_UserId) values(%d, %d, %d)", szTable_name, nUsr_id, nGroup_id, nFriend_id);


	
	nRet = mysql_query(pConn, szBuf);				//�������ݿ��
	
	if (nRet)
	{
		//���ݿ����
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

//������;: ���������ɾ����������
//�������: �û�ID,  ��ID,  ����ID
//�������: ��
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,   ���߷���TRUE 


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
	
	MYSQL *pConn = GetConn(szTable_name);					//��ȡ���ݿ�����
	if (NULL == pConn)
	{
		LOG_ERROR("DeleteFriendData: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	snprintf(szBuf, sizeof(szBuf) - 1, "delete from %s where Creator_UserId=%d and GroupId=%d and Friend_UserId=%d", szTable_name, nUsr_id, nGroup_id, nFriend_id);

	nRet = mysql_query(pConn, szBuf);					//ִ�б����
	
	if (nRet)
	{
		//���ݿ����
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			printf("DeleteFriendData: DB disconnect\n");
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

//������;: ��������л�ȡ������Ϣ
//�������: �û�ID,  ��ID
//�������: ��ź�����Ϣ,  ���������������Ļ���
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,   ���߷���TRUE 

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
		//���ݿ����
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
				mysql_row =   mysql_fetch_row(pMysql_res);			//��ȡһ������
				pBuf = mysql_row[1];								//��ȡ��ע����
				nRet = IsStringNotEmpty(pBuf);
				if (TRUE == nRet)
				{
					nLen = strlen(pBuf);
					nLen = GetDataLen(REMARK_NAME_LEN, nLen);
					strncpy(pFri_list_data[i].arrRemark, pBuf, nLen);	
				}
				
				pBuf = mysql_row[0];					//��ȡ�û�ID
				nRet = IsStringNotEmpty(pBuf);
				if (TRUE == nRet)
				{
					nUser_id = atoi(pBuf);
					pFri_list_data[i].nUsr_id = htonl(nUser_id);	
				}
				
				
				nRet = H_GetNickname(nUser_id, pFri_list_data[i].arrNickname);				//��ȡ�ǳ�
				if (DB_DISCONNECT == nRet)
				{
					LOG_ERROR("GetFriendInfoFromDB: DB Disconnect", FILE_NAME, FILE_LINE);
					ReleaseConn(pConn, szTable_name);		
					return nRet;
				}

				//��ȡ����ǩ����ͼ������				
				nRet = H_GetSigAndPhotoIdx(nUser_id, pFri_list_data[i].arrSelf_signature, &bPhoto_idx, &enImage_type);
				if (DB_DISCONNECT == nRet)
				{
					LOG_ERROR("GetFriendInfoFromDB: DB Disconnect", FILE_NAME, FILE_LINE);
					ReleaseConn(pConn, szTable_name);
					return nRet;
				}
				
				pFri_list_data[i].bPhoto_idx = bPhoto_idx;
				pFri_list_data[i].enImage_type = enImage_type;

				//�ж��û��Ƿ��Ѿ���¼ϵͳ
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




