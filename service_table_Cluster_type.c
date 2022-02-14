#include "../../util/service_global.h"
#include "../../interface/service_protocol.h"
#include "service_table_Cluster_type.h"

/*****���ļ���Ҫ����Ⱥ�������Ʊ��*************/


//������;:��ȡȺ�������Ʊ��
//�������: Ⱥ����
//�������: ���Ⱥ�������ƵĻ���
//����ֵ	:  ����������,  ����TABLE_ID_OUTOF_RANGE,  ���ݿ����,   ����DB_DISCONNECT,  ���߷���TRUE 


int GetTypeName(BYTE bCluster_type, char *pType_name)
{
	char szBuf[1024] = {0};
	char szTable_name[100] = {0};
	char szType_name[200] = {0};
	char arrType_name[10][100];
	BYTE bTmpType = bCluster_type;
	int nRet = 0;
	int nParent_id = 0;
	int i = 0;
	int nIdx = 0;
	int nCur_len = 0;
	WORD wCount;
	char *pBuf = NULL;
	MYSQL_ROW   mysql_row;
	MYSQL_RES *pMysql_res = NULL;  
	
	strcpy(szTable_name, "Cluster_type");
	
	MYSQL *pConn = GetConn(szTable_name);						//��ȡ����
	if (NULL == pConn)
	{
		LOG_ERROR("GetTypeName: Call GetConn error", FILE_NAME, FILE_LINE);
		return CONN_POOL_ISNOT_EXIST;
	}

	for (i=0; i<10; i++)
	{
		memset(arrType_name[i], 0, sizeof(arrType_name[i]));
	}

	for (;;)
	{
		if (-1 == nParent_id)
		{
			ReleaseConn(pConn, szTable_name);
			break;	
		}
	
		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "select * from Cluster_type where id=%d", bTmpType);	
		nRet = mysql_query(pConn, szBuf);					//ִ�����ݿ�����

		if (nRet)
		{
			//���ݿ����
			if (CR_SERVER_LOST == nRet || CR_SERVER_GONE_ERROR == nRet)
			{
				LOG_ERROR("GetTypeName: DB disconnect", FILE_NAME, FILE_LINE);
				ReconnDatabase(pConn, szTable_name);
				return DB_DISCONNECT;	
			}	
		}
		else
		{
			pMysql_res = mysql_store_result(pConn);
			wCount = (WORD)mysql_num_rows(pMysql_res);				//���ݿ�������
			if (wCount > 0)
			{
				mysql_row =   mysql_fetch_row(pMysql_res);
				pBuf = mysql_row[1];								//��ȡ��������
				if (pBuf != NULL)
				{
					strcpy(arrType_name[nIdx], pBuf);	
				}
				
				pBuf = mysql_row[2];					//��ȡ���ڵ��ID
				nParent_id = atoi(pBuf);
				bTmpType = nParent_id;
				nIdx++;
			}
		}
	}

	
	//��ȡȺ���͵����
	for (i=nIdx-1; i>=0; i--)
	{
		if (0 == i)
		{
			snprintf(pType_name + nCur_len, sizeof(szType_name) - 1 - nCur_len, "%s", arrType_name[i]);
		}
		else
		{
			snprintf(pType_name + nCur_len, sizeof(szType_name) - 1 - nCur_len, "%s-", arrType_name[i]);
			nCur_len += strlen(arrType_name[i]) + 1;
		}
	}
	
	return TRUE;
}




