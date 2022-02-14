#include "../../util/service_global.h"
#include "../../interface/service_protocol.h"
#include "../handle_table/service_handle_table.h"
#include "service_table_cluster_detail.h"

/*****���ļ���Ҫ����Ⱥ����*************/


//������;: ��ȡȺ�������
//�������: ȺID
//�������: ��ű����Ļ���
//����ֵ	: �û�ID�����˱�ķ�Χ,  ����FALSE��  ���߷���TRUE 

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

//������;: ��ȡȺ��Ա����
//�������: �û�ID,  ȺID 
//�������: ���Ⱥ��Ա����,  ��Ա�����Ļ���
/*����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  
  *���Ⱥ��Ա������,  ����FALSE,  ���߷���TRUE 
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
	
	MYSQL *pConn = GetConn(szTable_name);					//��ȡ����
	if (NULL == pConn)
	{
		LOG_ERROR("GetClusterDetailData: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	
	snprintf(szBuf, sizeof(szBuf) - 1, "select * from  %s where ClusterId = %d and Member_UserId = %d", szTable_name, nCluster_id, nUsr_id);

	nRet = mysql_query(pConn, szBuf);					//ִ�����ݿ�����
	
	if (nRet)
	{
		//���ݿ����
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
		
		if (nCount!= 0)				//����������Ϊ��
		{
			sprintf(szBuf, "select * from  %s where ClusterId = %d", szTable_name, nCluster_id);

			nRet = mysql_query(pConn, szBuf);			//ִ�����ݿ�����
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
					pBuf = mysql_row[1];					//��ȡ�û�ID
					nRet = IsStringNotEmpty(pBuf);
					if (TRUE == nRet)
					{
						nUser_id = atoi(pBuf);		
						pCluster_member_data[i].nUsr_id = htonl(nUser_id);
					}
					
					pBuf = mysql_row[2];					//��ȡ��ע����
					nRet = IsStringNotEmpty(pBuf);	
					if (TRUE == nRet)
					{
						nLen = strlen(pBuf);
						nLen = GetDataLen(REMARK_NAME_LEN, nLen);
						strncpy(pCluster_member_data[i].arrRemarkName, pBuf, nLen);		
					}
					
					//��ȡ�ǳ�
					nRet = H_GetNickname(nUser_id, pCluster_member_data[i].arrNickname);
					if (DB_DISCONNECT == nRet)
					{
						LOG_ERROR("GetClusterDetailData: DB Disconnect", FILE_NAME, FILE_LINE);
						ReleaseConn(pConn, szTable_name);
						return FALSE;
					}

					//��ȡͼ������
					nRet = H_GetPhotoIdx(nUser_id, &pCluster_member_data[i].bPhoto_idx, &pCluster_member_data[i].enImage_type);
					if (DB_DISCONNECT == nRet)
					{
						LOG_ERROR("GetClusterDetailData: DB Disconnect", FILE_NAME, FILE_LINE);
						ReleaseConn(pConn, szTable_name);
						return FALSE;
					}
				}

				mysql_free_result(pMysql_res);					//�ͷ��ڴ�
				ReleaseConn(pConn, szTable_name);				//�ͷ�����
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

//������;: ����Ⱥ��Ա����
//�������: ȺID,  �û�ID,  �û���Ⱥ�����״̬
//�������: ��
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE 
  
int InsertClusterMember(int nCluster_id, int nUsr_id, BYTE bStatus)
{
	char szBuf[1024] = {0};
	char szTable_name[MAX_TABLENAME_LEN] = {0};
	char szNickname[20] = {0};
	int nRet = 0;
	MYSQL *pConn = NULL;

	//��ȡ�ǳ�
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
		//���ݿ����
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

//������;: ��ŶȺ�������ɾ��Ⱥ��Ա
//�������: ȺID,  �û�ID
//�������: ��
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE 

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
	
	pConn = GetConn(szTable_name);				//��ȡ����
	if (NULL == pConn)
	{
		LOG_ERROR("DelClusterMember: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	snprintf(szBuf, sizeof(szBuf) - 1, "delete from %s where ClusterId=%d and Member_UserId=%d", \
		szTable_name, nCluster_id, nUsr_id);
 	
	nRet = mysql_query(pConn, szBuf);			//ִ�����ݿ�����
	if (nRet)
	{
		//���ݿ����
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

//������;: ��ȡȺ�������е�Ⱥ��Ա ID
//�������: ȺID
//�������: ����û�ID,  Ⱥ��Ա�����������Ļ���
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE 

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

	nRet = mysql_query(pConn, szBuf);				//ִ�����ݿ����
	if (nRet)
	{
		//���ݿ����
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
		if (nCount != 0)				//���ݿ��������Ϊ��
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
				pBuf = mysql_row[0];				//��ȡ�û�ID
				nRet = IsStringNotEmpty(pBuf);		//�ж��ַ����Ƿ�Ϊ��
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

//������;: ɾ�����е�Ⱥ��Ա
//�������: ȺID
//�������: ��
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE 

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
	
	pConn = GetConn(szTable_name);				//��ȡ����
	if (NULL == pConn)
	{
		LOG_ERROR("DelAllClusterMember: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}
	
	snprintf(szBuf, sizeof(szBuf) - 1, "delete from %s where ClusterId=%d", szTable_name, nCluster_id);
	nRet = mysql_query(pConn, szBuf);			//ִ�����ݿ�����
	if (nRet)
	{
		//���ݿ����
		if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
		{
			LOG_ERROR("DelAllClusterMember: DB disconnect", FILE_NAME, FILE_LINE);
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

