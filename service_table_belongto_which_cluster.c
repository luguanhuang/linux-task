#include "../../util/service_global.h"
#include "../../interface/service_protocol.h"
#include "../../service/chat_service/service_create_cluster.h"

#include "service_table_belongto_which_cluster.h"

/*****���ļ���Ҫ����Ⱥ�б��*************/


//������;: ��ȡȺ�б�����
//�������: �û�ID
//�������: ��ű����Ļ���
//����ֵ	: �û�ID�����˱�ķ�Χ,  ����FALSE��  ���߷���TRUE 


int GetBelongToClusterTableName(int nUsr_id, char *szTableName)
{
	char szBuf[MAX_TABLENAME_LEN] = {0};

	if (nUsr_id >= ONE && nUsr_id <= ONE_HUNDRED_THOUSAND)
	{
		sprintf(szBuf, "%s_%s", "user_belongto_which_cluster", "1_100000");	
	}
	else if (nUsr_id > ONE_HUNDRED_THOUSAND && nUsr_id <= TWO_HUNDRED_THOUSAND)
	{
		sprintf(szBuf, "%s_%s", "user_belongto_which_cluster", "100001_200000");		
	}
	else
	{
		LOG_ERROR("GetBelongToClusterTableName: table id out of range", FILE_NAME, FILE_LINE);
		return FALSE;	
	}
	
	strcpy(szTableName, szBuf);
	return TRUE;
}

//������;: ��ȡ����Ⱥ��Ϣ
//�������: �û�ID,
//�������: ���Ⱥ����,  Ⱥ���ݵĻ���
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE

int GetClusterData(int nUsr_id, WORD *pCount, Cluster_Data *pCluster_data)
{
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	MYSQL_ROW   mysql_row;
	MYSQL_RES *pMysql_res = NULL;   
	char szBuf[1024] = {0};
	char *pBuf = NULL;
	int nRes = 0;
	int nRet = 0;
	WORD wCount = 0;
	int i = 0;
	int nLen = 0;
	
	nRet = GetBelongToClusterTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("GetClusterData: Call GetBelongToClusterTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	MYSQL *pConn = GetConn(szTable_name);			//��ȡ���ݿ�����
	if (NULL == pConn)
	{
		LOG_ERROR("GetClusterData: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	snprintf(szBuf, sizeof(szBuf) - 1, "select * from %s where UserId = %d order by ClusterId", szTable_name, nUsr_id);

	nRet = mysql_query(pConn, szBuf);				//ִ�����ݿ�����
	if (nRet)
	{
		//���ݿ����
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			printf("GetClusterData: DB disconnect\n");
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
	}
	else
	{
		pMysql_res = mysql_store_result(pConn);
		wCount = (WORD)mysql_num_rows(pMysql_res);
		printf("GetClusterData: %d records found\n", wCount);

		if (wCount!= 0)						//���ݿ��������Ϊ��
		{
			*pCount = wCount;
			for (i=0; i<wCount; i++)
			{
				
				mysql_row =   mysql_fetch_row(pMysql_res);
				pBuf = mysql_row[1];				//��ȡȺID
				nRet = IsStringNotEmpty(pBuf);
				if (TRUE == nRet)
				{
					nRes = atoi(pBuf);
					pCluster_data[i].nCluster_id = htonl(nRes);		
				}

				pBuf = mysql_row[2];			//��ȡȺ����
				nRet = IsStringNotEmpty(pBuf);
				if (TRUE == nRet)
				{
					nLen = strlen(pBuf);
					nLen = GetDataLen(CLUSTER_NAME_LEN, nLen);
					strncpy(pCluster_data[i].arrCluster_name, pBuf, nLen);	
				}

				pBuf = mysql_row[3];					//��ȡȺ״̬
				nRet = IsStringNotEmpty(pBuf);
				if (TRUE == nRet)
				{
					pCluster_data[i].bStatus = (BYTE)atoi(pBuf);;		
				}
			}
		}

		mysql_free_result(pMysql_res);
		ReleaseConn(pConn, szTable_name);
		return TRUE;
	}

	return TRUE;
}

//������;: �����û�ӵ�е�Ⱥ��Ϣ
//�������: �û�ID, ȺID,  Ⱥ����,  �û���Ⱥ�����״̬
//�������: ���Ⱥ����,  Ⱥ���ݵĻ���
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE

int InsertOwnClusterInfo(int nUsr_id, int nCluster_id, char *pCluster_name, BYTE bStatus)
{
	char szTable_name[MAX_TABLENAME_LEN] = {0}; 
	char szBuf[1024] = {0};
	int nRet = 0;
	nRet = GetBelongToClusterTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("InsertOwnClusterInfo: Call GetBelongToClusterTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	MYSQL *pConn = GetConn(szTable_name);				//��ȡ���ݿ�����
	if (NULL == pConn)
	{
		LOG_ERROR("InsertOwnClusterInfo: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	snprintf(szBuf, sizeof(szBuf) - 1, "insert into %s(UserId, ClusterId, ClusterName, Status) values(%d, %d, '%s', %d)", szTable_name, nUsr_id, nCluster_id, pCluster_name, bStatus);
	nRet = mysql_query(pConn, szBuf);					//ִ�����ݿ�����
	if (nRet)
	{
		//���ݿ����
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("InsertOwnClusterInfo: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
		
	}
	else
	{
		ReleaseConn(pConn, szTable_name);				//�ͷ����ݿ�����
		return TRUE;
	}

	return TRUE;
}

//������;: ɾ���û�ӵ�е�Ⱥ��Ϣ
//�������: �û�ID, ȺID,  Ⱥ����
//�������: ��
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE


int DelOwnClusterInfo(int nUsr_id, int nCluster_id)
{
	char szBuf[1024] = {0};
	int nRet = 0;
	char szTable_name[MAX_TABLENAME_LEN] = {0}; 

	nRet = GetBelongToClusterTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("DelOwnClusterInfo: Call GetBelongToClusterTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	MYSQL *pConn = GetConn(szTable_name);					//��ȡ����
	if (NULL == pConn)
	{
		LOG_ERROR("DelOwnClusterInfo: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	snprintf(szBuf, sizeof(szBuf) - 1, "delete from %s where UserId=%d and ClusterId=%d", \
		szTable_name, nUsr_id, nCluster_id);
	nRet = mysql_query(pConn, szBuf);						//ִ�����ݿ�����
	if (nRet)
	{
		//���ݿ����
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("DelOwnClusterInfo: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
		
	}
	else
	{
		ReleaseConn(pConn, szTable_name);				//�ͷ����ݿ�����
		return TRUE;
	}

	return TRUE;
}

//������;: �ж��û��Ƿ����㹻��Ȩ�޴���Ⱥ
//�������: �û�ID, ȺID
//�������: ��
/*����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,
  *������㹻��Ȩ�޴���Ⱥ,  ����TURE,  ���߷���FALSE 
  */

int IsUsrHavePowerToDelCluster(int nUsr_id, int nCluster_id)
{
	char szBuf[1024] = {0};
	MYSQL_RES *pMysql_res = NULL;
	MYSQL_ROW mysql_row;
	int nRet = 0;
	int nCount = 0;
	int nStatus = 0;
	char szTable_name[MAX_TABLENAME_LEN] = {0}; 
	char *pBuf = NULL;
	
	nRet = GetBelongToClusterTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("IsUsrHavePowerToDelCluster: Call GetBelongToClusterTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	MYSQL *pConn = GetConn(szTable_name);				//��ȡ���ݿ�����
	if (NULL == pConn)
	{
		LOG_ERROR("IsUsrHavePowerToDelCluster: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	snprintf(szBuf, sizeof(szBuf) - 1, "select Status from %s where UserId=%d and ClusterId=%d", \
		szTable_name, nUsr_id, nCluster_id);
	nRet = mysql_query(pConn, szBuf);
	if (nRet)
	{
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("IsUsrHavePowerToDelCluster: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
		
	}
	else
	{
		pMysql_res = mysql_store_result(pConn);
		nCount = mysql_num_rows(pMysql_res);
		if (nCount != 0)
		{
			mysql_row = mysql_fetch_row(pMysql_res);
			pBuf = mysql_row[0];
			nRet = IsStringNotEmpty(pBuf);
			if (TRUE == nRet)
			{
				nStatus = atoi(pBuf);
				if (CLUSTER_CREATOR == nStatus)
				{
					mysql_free_result(pMysql_res);				//�ͷ��ڴ�
					ReleaseConn(pConn, szTable_name);			//�ͷ�����
					return TRUE;	
				}
				else
				{
					mysql_free_result(pMysql_res);				//�ͷ��ڴ� 
					ReleaseConn(pConn, szTable_name);			//�ͷ�����
					return FALSE;	
				}	
			}
		}
		
		mysql_free_result(pMysql_res);
		ReleaseConn(pConn, szTable_name);
		return FALSE;	
	}

	return TRUE;
}



/*int DeleteUsrsOwnCluster(int nCluster_id)
{
	char szBuf[1024] = {0};
	int nRet = 0;
	int i = 0;
	char szTable_name[MAX_TABLENAME_LEN] = {0}; 
	GetBelongToClusterTableName(nUsr_id, szTable_name);
	
	MYSQL *pConn = GetConn(szTable_name);

	snprintf(szBuf, sizeof(szBuf) - 1, "delete from %s where UserId=%d and ClusterId=%d", \
		szTable_name, nUsr_id, nCluster_id);
	nRet = mysql_query(pConn, szBuf);
	if (nRet)
	{
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("DelOwnClusterInfo: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
		
	}
	else
	{
		ReleaseConn(pConn, szTable_name);		
		return TRUE;
	}	
}*/

//������;: ɾ���û�ӵ�е�Ⱥ
//�������: �û�ID, ȺID
//�������: ��
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,

int DeleteUsrsOwnCluster(int nUsr_id, int nCluster_id)
{
	char szBuf[1024] = {0};
	int nRet = 0;
	char szTable_name[MAX_TABLENAME_LEN] = {0}; 
	
	nRet = GetBelongToClusterTableName(nUsr_id, szTable_name);
	if (FALSE == nRet)
	{
		LOG_ERROR("DeleteUsrsOwnCluster: Call GetBelongToClusterTableName error", FILE_NAME, FILE_LINE);	
		return TABLE_ID_OUTOF_RANGE;
	}
	
	
	MYSQL *pConn = GetConn(szTable_name);				//��ȡ���ݿ�����
	if (NULL == pConn)
	{
		LOG_ERROR("DeleteUsrsOwnCluster: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	snprintf(szBuf, sizeof(szBuf) - 1, "delete from %s where ClusterId=%d", \
		szTable_name, nCluster_id);

	printf("DeleteUsrsOwnCluster: szBuf=%s\n", szBuf);
	nRet = mysql_query(pConn, szBuf);				//ִ�����ݿ����
	if (nRet)
	{
		//���ݿ����
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("DeleteUsrsOwnCluster: DB disconnect", FILE_NAME, FILE_LINE);
			ReconnDatabase(pConn, szTable_name);
			return DB_DISCONNECT;	
		}
		
	}
	else
	{
		ReleaseConn(pConn, szTable_name);				//�ͷ����ݿ�����
		return TRUE;
	}	
	
	return TRUE;
}

