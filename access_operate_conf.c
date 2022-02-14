
#include "access_global.h"
#include "mm_memorymonitor.h"
//#include "../include/ydt_log.h"
#include "../communication/access_routing_maintain.h"
#include "access_server_num.h"
#include "access_operate_conf.h"


CSerRouteInfo::CSerRouteInfo(void)
{
	m_mapSerConnInfo.clear();
}

CSerRouteInfo::MapType &CSerRouteInfo::GetMapConnInfo(void)
{
	return m_mapSerConnInfo;
}

int CSerRouteInfo::GetSrvSeq(BYTE bMainCode, BYTE bSubCode)
{
	stringstream strData;
	strData << (int)bMainCode << (int)bSubCode;
	MapIter iter = m_mapSerConnInfo.find(strData.str());
	if (iter == m_mapSerConnInfo.end())
	{
		return 0;	
	}

	int iSize = iter->second.size();
	int iSeq = (rand() % iSize) + 1;
	return iSeq;
}


CSerRouteInfo g_serRouteInfo;


/*���ļ���Ҫ����������ҵ������������ļ���
  *��Ҫ�Ĳ����ǻ�ȡҵ���������������Ϣ,  ���һ�ȡ
  *ҵ�������������ҵ��
  */

extern int g_iShmId;

map <string, Server_Info> g_mapSrvInfo;

//ҵ��·�ɱ�(���ҵ����������������)

StConf_File_Data g_conf_file_data;

extern StRouting_Table g_routing_table;

extern Service_Server_Num g_login_srv_num;				//��¼����������������Ϣ
extern Service_Server_Num g_register_srv_num;			//ҵ�����������������Ϣ
extern Service_Server_Num g_simplechat_srv_num;			//���ķ���������������Ϣ
extern Service_Server_Num g_clusterchat_srv_num;		//Ⱥ�ķ���������������Ϣ
extern Service_Server_Num g_liantong_srv_num;			//��ͨ����������������Ϣ

extern Service_Server_Num g_wanwei_loginsrv_num;			//��ͨ����������������Ϣ
extern Service_Server_Num g_wanwei_report_gps_srvnum;			//��ά��¼����������
extern Service_Server_Num g_wanwei_querypush_srvnum;		

Server_Conf_Info g_srv_conf_info;

//������;:��ȡ�����ļ���·��
//�������: ��
//�������: ����ļ�·���Ļ��� 
//����ֵ	: ��

static int GetConfFilePath(char *pFilePath, char *pFileName, int nLen)
{
	INFO("GetConfFilePath: func begin%s", "");
	if (NULL == pFilePath || NULL == pFileName || 0 == nLen)
	{
		ERROR("GetConfFilePath: func param error%s", "");
		return FALSE;
	}
	
	char szFilePath[FILE_PATH_LEN] = {0};
	char szPath[256] = {0};
	
	GetProgramPath(szPath);

	snprintf(szFilePath, sizeof(szFilePath) - 1, "%s/%s", szPath, pFileName);
	strncpy(pFilePath, szFilePath, nLen);

	DEBUG("GetConfFilePath: file path=%s", pFilePath);
	INFO("GetConfFilePath: func end%s", "");
	return TRUE;
}

//������;:��ȡ�Զ���Ϊ�ָ������ֶ�
//�������: pSrc_data, �ָ���cSeparator
//�������:  �������ݵĻ���ppDest
//����ֵ	: ��ȡ�¹�,  ����TRUE,   ��ȡʧ��,   ����FALSE;

int GetData(char *pSrc_data,char **ppDest, char cSeparator)
{
	int i = 0;
	int nCount = 0;
	int nData_len = 0;
	if (NULL == pSrc_data)
	{
		ERROR("GetData: param error%s", "");
		return FALSE;
	}

	
	nData_len = strlen(pSrc_data);
	
	for (i=0; i<nData_len; i++)
	{
		nCount++;
		if (cSeparator == pSrc_data[i])
		{
			break;
		}
	}
	
	*ppDest= (char *)MM_MALLOC_WITH_DESC(nCount, "GetData: Call func for get config file value");
	if (NULL == *ppDest)
	{
		FATAL("GetData: Call malloc error%s", "");
		return FALSE;
	}

	memset(*ppDest, 0, nCount);
	memcpy(*ppDest, pSrc_data, nCount - 1);
	return nCount;
}

//������;: ��ȡ�ļ�һ�������и����ֶ�(�Զ��ŷָ���)
//�������: ����ļ���һ�����ݵĻ���
//�������:  Server_Info ����ָ��,  ���ҵ���������Ϣ�Ļ���
//����ֵ	: ��ȡ�¹�,  ����TRUE,   ��ȡʧ��,   ����FALSE;

int GetLineFields(char *szLine, Server_Info *pSer_info)
{
	INFO("GetLineFields: func begin%s", "");
	int nCount = 0;
	int nTotal_count = 0;
	char *pData = NULL;
	char cComma = ',';

	nCount= GetData(szLine + nTotal_count, &pData, cComma);
	if (FALSE == nCount)
	{
		ERROR("GetLineFields : Call GetData error%s", "");
		return FALSE;
	}

	//��ȡ��ҵ����
	pSer_info->bMain_code = (BYTE)atoi(pData);
	nTotal_count += nCount;
	MM_FREE(pData);
	
	nCount= GetData(szLine + nTotal_count, &pData, cComma);
	if (FALSE == nCount)
	{
		ERROR("GetLineFields : Call GetData error%s", "");
		return FALSE;
	}

	//��ȡ��ҵ����
	pSer_info->bSub_code  = (BYTE)atoi(pData);
	nTotal_count += nCount;
	MM_FREE(pData);

	nCount= GetData(szLine + nTotal_count, &pData, cComma);
	if (FALSE == nCount)
	{
		ERROR("GetLineFields : Call GetData error%s", "");
		return FALSE;
	}

	//��ȡҵ����������
	pSer_info->wService_seq = (WORD)atoi(pData);
	nTotal_count += nCount;

	MM_FREE(pData);
	
	nCount= GetData(szLine + nTotal_count, &pData, cComma);
	if (FALSE == nCount)
	{
		ERROR("GetLineFields : Call GetData error%s", "");
		return FALSE;
	}


	//��ȡҵ���������IP
	strncpy(pSer_info->arrService_srv_ip, pData, sizeof(pSer_info->arrService_srv_ip));
	nTotal_count += nCount;

	MM_FREE(pData);
	
	nCount= GetData(szLine + nTotal_count, &pData, cComma);
	if (FALSE == nCount)
	{
		ERROR("GetLineFields : Call GetData error%s", "");
		return FALSE;
	}

	//��ȡҵ��������Ķ˿�
	pSer_info->wSrv_port = (WORD)atoi(pData);
	nTotal_count += nCount;

	MM_FREE(pData);
	pSer_info->nSock = 0;

	DEBUG("GetLineFields: [main service code]=%d [sub service code]=%d [service server seq]=%d [service ip addr]=%s [service server port]=%d [connect socket]=%d", pSer_info->bMain_code, pSer_info->bSub_code, pSer_info->wService_seq, pSer_info->arrService_srv_ip, pSer_info->wSrv_port, pSer_info->nSock);
	INFO("GetLineFields: func end%s", "");
	return TRUE;
}

//������;: ��ȡ�����ļ�, ���������ļ���Ϣ����ҵ��·�ɹ�ϣ����
//�������: ��
//�������:  ��
//����ֵ	: ��ȡ�ɹ�,  ����TRUE,  ��ȡʧ��,  ����FALSE

int ReadConfFile(void)
{
	INFO("ReadConfFile: func begin%s", "");
	char szLine[1024] = {0};
	char szBuf[100] = {0};
	char szFile_path[FILE_PATH_LEN] = {0};
	int nTotal_count = 0;
	Server_Info *pSer_info = NULL;
	int nLen = 0;
	int nRet = 0;
	stringstream strData;

	nRet = GetConfFilePath(szFile_path, CONFIG_FILE_NAME, FILE_PATH_LEN);
	if (FALSE == nRet)
	{
		ERROR("ReadConfFile: Call GetConfFilePath error%s", "");
		return FALSE;
	}
	
	FILE *fp = fopen(szFile_path, "r");
	if (NULL == fp)
	{
		ERROR("ReadConfFile : Call fopen error error[%d]=%s", errno, strerror(errno));
		return FALSE;
	}
	
	while (1)
	{
		nTotal_count = 0;
		memset (szLine, 0, sizeof(szLine));	
		if(NULL == fgets( szLine, sizeof(szLine), fp))
		{
			break;
		}		

		//���һ���ַ��ǻ��з�
		nLen = strlen(szLine);
		if (1 == nLen && '\n' == szLine[0])
		{
			WARN("ReadConfFile: the file line only contain one linefeed [value]=%d\n", szLine[0]);
			continue;
		}

		if ('\n' == szLine[nLen-1])		//���һ���ǻ��з�,  �ȸ�Ϊ������
		{
			szLine[nLen-1] = 0;
		}
		
		pSer_info = (Server_Info *)MM_MALLOC_WITH_DESC(sizeof(Server_Info), \
		"ReadConfFile: Call func for server info");
		if (NULL == pSer_info)
		{
			FATAL("ReadConfFile : Call malloc error%s", "");
			fclose(fp);
			return FALSE;
		} 
		memset(pSer_info, 0, sizeof(Server_Info));

		nRet = pthread_mutex_init(&pSer_info->mutex, NULL);
		if (0 != nRet)
		{
			ERROR("ReadConfFile: Call pthread_mutex_init error error[%d]=%s", errno, strerror(errno));
			MM_FREE(pSer_info);
			fclose(fp);
			return FALSE;
		}

		if (FALSE == GetLineFields(szLine, pSer_info))
		{
			ERROR("ReadConfFile: Call GetLineFields error%s", "");
			MM_FREE(pSer_info);
			fclose(fp);
			return FALSE;
		}

		StConnInfo stConnInfo;
		memset(&stConnInfo, 0, sizeof(stConnInfo));
		memcpy(&stConnInfo, &pSer_info->bMain_code, sizeof(stConnInfo));

		strData.str("");
		
		//������ֲ�ͬ���͵�ҵ�������������
		strData << (int)pSer_info->bMain_code << (int)pSer_info->bSub_code;
		CSerRouteInfo::MapType &mapRouteConnInfo = g_serRouteInfo.GetMapConnInfo();
		
		if (mapRouteConnInfo.end() == mapRouteConnInfo.find(strData.str()))
		{
			mapRouteConnInfo[strData.str()].reserve(100);
		}

		mapRouteConnInfo[strData.str()].push_back(stConnInfo);

		IncServiceSrvNum(pSer_info->bMain_code, pSer_info->bSub_code);
		
		memset(szBuf, 0, 100);
		snprintf(szBuf, sizeof(szBuf) - 1, "%d%d%d", pSer_info->bMain_code, pSer_info->bSub_code, pSer_info->wService_seq);

		g_mapSrvInfo[szBuf] = *pSer_info;
		
		//��ҵ���������������Ϣ�����ϣ����	
		nRet = HashInsert(&g_routing_table.pRouting_table, szBuf, pSer_info); 
		if (HASH_SAMEKEY_EXIST == nRet)
		{
 			StHash_Item *pItem = HashGetItem(g_routing_table.pRouting_table, szBuf);
			if (pItem != NULL)
			{
				pthread_mutex_lock(&g_routing_table.routing_mutex);
				MM_FREE(pItem->pMatch_msg);
				pItem->pMatch_msg = (void *)pSer_info;
				pthread_mutex_unlock(&g_routing_table.routing_mutex);
			}
		}
	}

	fclose(fp);
	
	CSerRouteInfo::MapType &mapRouteConnInfo = g_serRouteInfo.GetMapConnInfo();
	CSerRouteInfo::MapIter begin = mapRouteConnInfo.begin();
	while (begin != mapRouteConnInfo.end())
	{
		INFO("ReadConfFile: [key]=%s [size]=%d", \
			begin->first.c_str(), begin->second.size());
		vector <StConnInfo> &vecConnInfo = begin->second;	
		for (int i=0; i<vecConnInfo.size(); i++)
		{
			INFO("ReadConfFile: [MainCode]=%d [SubCode]=%d [wServiceSeq]=%d"
				" [arrIP]=%s [port]=%d [socket]=%d", vecConnInfo[i].bMainCode, \
				vecConnInfo[i].bSubCode, vecConnInfo[i].wServiceSeq, vecConnInfo[i].arrIP, \
				vecConnInfo[i].wPort, vecConnInfo[i].nSock);
		}
		begin++;
	}
	

	DEBUG("ReadConfFile: [liantong server num]=%d", g_liantong_srv_num.service_server_num);
	INFO("ReadConfFile: func end%s", "");
	return TRUE;
	
}	

//������;:ά�������ļ���Ϣ(��Ҫ����������ҵ����)
//�������: ��
//�������:  ��
//����ֵ	: �Ƿ�������ҵ��

int MaintainConfFile(void)
{
	INFO("MaintainConfFile: func begin%s", "");
	char szLine[1024] = {0};
	char szBuf[100] = {0};
	char szFile_path[FILE_PATH_LEN] = {0};
	long lPre_pos = 0;
	long lCur_pos = 0;
	int nCount = 0;
	int nTotal_count = 0;
	Server_Info *pSer_info = NULL;
	FILE *fp = NULL;
	int nLen = 0;
	int nAwake_flg = FALSE;
	int nRet = 0;
	
	nRet = GetConfFilePath(szFile_path, CONFIG_FILE_NAME, FILE_PATH_LEN);
	if (FALSE == nRet)
	{
		ERROR("MaintainConfFile: Call GetConfFilePath error%s", "");
		return MAINTAIN_CONF_FILE_ERROR;
	}
	
	fp = fopen(szFile_path, "r+");
	if (NULL == fp)
	{
		ERROR("MaintainConfFile: Call fopen error error[%d]=%s", errno, strerror(errno));
		return MAINTAIN_CONF_FILE_ERROR;
	}

	while (TRUE)
	{
		nTotal_count = 0;
		nCount = 0;
		
		memset(szLine, 0, sizeof(szLine));
		lPre_pos= ftell(fp);
		if (NULL == fgets(szLine, sizeof(szLine), fp))
		{
			break;	
		}

		lCur_pos = ftell(fp);
		nLen = strlen(szLine);

		//������µ�ҵ�������
		if ('1' == szLine[nLen-2])
		{
			//���Ƿ����µ�ҵ��������ֶ�����Ϊ�� 
			szLine[nLen-2] = '0';
			fseek(fp, lPre_pos - lCur_pos, SEEK_CUR);
			//��д·�������ļ�
			nRet = fputs(szLine, fp);
			if (EOF == nRet)
			{
				ERROR("MaintenanceConfFile: Call fputs error%s", "");
				fclose(fp);
				return MAINTAIN_CONF_FILE_ERROR;
			}

			pSer_info = (Server_Info *)MM_MALLOC_WITH_DESC(sizeof(Server_Info), \
			"MaintenanceConfFile: Call func for server info");
			if (NULL == pSer_info)
			{
				FATAL("MaintenanceConfFile: Call malloc error%s", "");
				fclose(fp);
				return MAINTAIN_CONF_FILE_ERROR;
			}
			memset(pSer_info, 0, sizeof(Server_Info));
			nRet = pthread_mutex_init(&pSer_info->mutex, NULL);
			if (0 != nRet)
			{
				ERROR("MaintenanceConfFile: Call pthread_mutex_init error%s", "");
				fclose(fp);
				MM_FREE(pSer_info);
				return FALSE;
			}

			szLine[nLen-1] = 0;			//���һ���ַ��ǻ��з�

			if (FALSE == GetLineFields(szLine, pSer_info))
			{
				ERROR("MaintainConfFile: Call GetLineFields error%s", "");
				fclose(fp);
				MM_FREE(pSer_info);
				return MAINTAIN_CONF_FILE_ERROR;
			}

			//���¸��ֲ�ͬ��ҵ�����������

			UpdateServiceSrvNum(pSer_info->bMain_code, pSer_info->bSub_code);
			
			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "%d%d%d", pSer_info->bMain_code, pSer_info->bSub_code, pSer_info->wService_seq);

			//��������ҵ�������������Ϣ���뵽��ϣ����
			nRet = HashInsert(&g_routing_table.pRouting_table, szBuf, pSer_info); 
			if (HASH_SAMEKEY_EXIST == nRet)
			{
				StHash_Item *pItem = HashGetItem(g_routing_table.pRouting_table, szBuf);
				if (NULL != pItem)
				{				
					pthread_mutex_lock(&g_routing_table.routing_mutex);
					MM_FREE(pItem->pMatch_msg);
					pItem->pMatch_msg = (void *)pSer_info;
					pthread_mutex_unlock(&g_routing_table.routing_mutex);
				}
			}

			nAwake_flg = TRUE;
		}
	}

	fclose(fp);

	if (TRUE == nAwake_flg)
	{
		DEBUG("MaintainConfFile: [service server num]=%d [liantong server num]=%d", \
					g_liantong_srv_num.service_server_num, g_srv_conf_info.nLiantong_srv_num);
	}
	
	INFO("MaintainConfFile: func end%s", "");
	return nAwake_flg;
}

/*********��ͨר��*********/
//������;: ��ʼ�������������ļ���Ϣ
//�������: ��
//�������: �� 
//����ֵ	: ��ʼ���ɹ�,  ����TRUE�� ��ʼ��ʧ��,  ����FALSE;
int InitSrvConfFileInfo(void)
{
	INFO("InitSrvConfFileInfo: func begin%s", "");
	if (pthread_mutex_init(&g_srv_conf_info.conf_info_mutex, NULL) != 0)
	{
		ERROR("InitSrvConfFileInfo: Call pthread_mutex_init error%s", "");
		return FALSE;
	}

	INFO("InitSrvConfFileInfo: func end%s", "");
	return TRUE;
}
/********��ͨר��***********/

int GetValidStr(char *pStr)
{
	INFO("GetValidStr: func begin%s", "");
	if (NULL == pStr)
	{
	  ERROR("GetValidStr: func param error%s", "");
	  return FUNC_PARAM_ERROR;
	}

	int nLen = strlen(pStr);
	char *pTmp_str = (char *)MM_MALLOC_WITH_DESC(nLen + 1, \
	  "GetValidStr: Call func for tmp string");
	if (NULL == pTmp_str)
	{
	  FATAL("GetValidStr: out of memory%s", "");
	  return OUT_OF_MEMORY_ERROR;
	}

	memset(pTmp_str, 0, nLen + 1);
	int i = 0;
	int nBlank_begin = 0;
	int nBlank_end = 0;


	for (i=0; i<nLen; i++)
	{
	  if (' ' != pStr[i] && '\t' != pStr[i])
	  {
		  break;  
	  }
	}

	if (i >= nLen)			  //�շ�
	{
	  WARN("GetValidStr: string is empty%s", "");
	  return FALSE;   
	}

	nBlank_begin = i;
	for (i=nLen-1; i>nBlank_begin-1; i--)
	{
	  if (' ' != pStr[i] && '\t' != pStr[i])
	  {
		  break;  
	  }
	}

	nBlank_end = i;

	strncpy(pTmp_str, pStr + nBlank_begin, (nBlank_end - nBlank_begin + 1));
	memset(pStr, 0, nLen);
	strncpy(pStr, pTmp_str, (nBlank_end - nBlank_begin + 1));

	MM_FREE(pTmp_str);
	INFO("GetValidStr: func end%s", "");
	return TRUE;
}

static int GetConfFileFieldValue(char *pLine)
{
	INFO("GetConfFileFieldValue: func begin%s", "");
	if (NULL == pLine)
	{
		ERROR("GetConfFileFieldValue: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nKey_size = 0;
	unsigned int nIdx = g_conf_file_data.nIdx;
	StConfLine_Data *pTmp_confline_data = (g_conf_file_data.arrConf_line_data + nIdx);
	
	char *pPos = strchr(pLine, '=');
	if (NULL == pPos)
	{
		WARN("GetConfFileFieldValue: we can't find the equal symbol%s", "");
		return FALSE;


	}

	int nLen = pPos - pLine;
	nKey_size = sizeof(pTmp_confline_data->arrKey);
	nLen = MIN(nLen, nKey_size - 1);
	
	memcpy(pTmp_confline_data->arrKey, pLine, nLen);
	GetValidStr(pTmp_confline_data->arrKey);

	int nValue_size = sizeof(pTmp_confline_data->arrValue);
	memcpy(pTmp_confline_data->arrValue, pPos + 1, nValue_size - 1);
	GetValidStr(pTmp_confline_data->arrValue);

	DEBUG("GetConfFileFieldValue: [key]=%s [value]=%s", \
		pTmp_confline_data->arrKey, pTmp_confline_data->arrValue);

	g_conf_file_data.nIdx++;
	g_conf_file_data.nCount++;
	INFO("GetConfFileFieldValue: func end%s", "");
	return TRUE;
}

static int GetConfFileKeyValue(char *pKey, char *pValue, int nValue_size)
{
	INFO("GetConfFileKeyValue: func begin%s", "");
	int i = 0;
	StConfLine_Data *pTmp_confline_data = NULL;

	for (i=0; i<g_conf_file_data.nCount; i++)
	{
		pTmp_confline_data = g_conf_file_data.arrConf_line_data + i;
		if (0 == strcmp(pKey, pTmp_confline_data->arrKey))
		{
			strncpy(pValue, pTmp_confline_data->arrValue, nValue_size - 1);

			break;
		}

	}

	if (i == g_conf_file_data.nCount)
	{
		WARN("GetConfFileKeyValue: we can't find the value according to the key [key]=%s", \
			pKey);	
	}
	
	INFO("GetConfFileKeyValue: func end%s", "");
	return TRUE;
}

void GetConfFileValues(void)
{
	
	INFO("GetConfFileValues: func begin%s", "");
	char arrValue[100] = {0};
	int nSize = sizeof(arrValue);
	
	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("port", arrValue, nSize);
	g_srv_conf_info.wPort =  (WORD)atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("liantong_server_num", arrValue, nSize);
	g_srv_conf_info.nLiantong_srv_num =  atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("min_thread_num", arrValue, nSize);
	g_srv_conf_info.nMin_thread_num =  atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("max_thread_num", arrValue, nSize);
	g_srv_conf_info.nMax_thread_num =  atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("poll_heartbeat_interval", arrValue, nSize);
	g_srv_conf_info.nPoll_heartbeat_interval =  atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("heartbeat_timeout_time", arrValue, nSize);
	g_srv_conf_info.nHeartbeat_timeout_time =  atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("poll_srvconfig_interval", arrValue, nSize);
	g_srv_conf_info.nPool_srvconfig_interval =  atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("poll_clientmsg_interval", arrValue, nSize);
	g_srv_conf_info.nPool_clientmsg_interval =  atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("clientmsg_timeout_time", arrValue, nSize);
	g_srv_conf_info.nClientmsg_timeout_time =  atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("msg_queue_size", arrValue, nSize);
	g_srv_conf_info.nMsg_queue_size = atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("service_routingtable_size", arrValue, nSize);
	g_srv_conf_info.nRoutingtable_size =  atoi(arrValue);

	
	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("heartbeat_detect_table_size", arrValue, nSize);
	g_srv_conf_info.nHeartbeat_table_size =	atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("clientmsg_buf_table_size", arrValue, nSize);
	g_srv_conf_info.nClient_msgbuf_tablesize =	atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("clientmsginfo_table_size", arrValue, nSize);
	g_srv_conf_info.nClient_msginfo_tablesize = atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("detect_reconnect_servicesrv_interval", arrValue, nSize);
	g_srv_conf_info.nDec_reconnect_servicesrv_interval = atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("maintain_listen_port", arrValue, nSize);
	g_srv_conf_info.wMaintain_port = (WORD)atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("poll_heartbeat_interval", arrValue, nSize);
	g_srv_conf_info.nPoll_heartbeat_interval = atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("epoll_size", arrValue, nSize);
	g_srv_conf_info.nEpoll_size = atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("server_mode", arrValue, nSize);
	g_srv_conf_info.nServer_mode = atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("max_clientmsg_thread_num", arrValue, nSize);
	g_srv_conf_info.nMax_clientmsg_thread_num = atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("log_print_level", arrValue, nSize);
	g_srv_conf_info.bLog_print_level = (BYTE)atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("tmp_clientsock_hash_size", arrValue, nSize);
	g_srv_conf_info.nTmp_clientsock_hash_size = atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("sndmsgto_servicesrv_threadnum", arrValue, nSize);
	g_srv_conf_info.nSndmsgto_servicesrv_threadnum = atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("sndmsgto_client_threadnum", arrValue, nSize);
	g_srv_conf_info.nSndmsgto_client_threadnum = atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("accessserver_sequence", arrValue, nSize);
	g_srv_conf_info.wAccess_srv_seq = (WORD)atoi(arrValue);

	memset(arrValue, 0, sizeof(arrValue));
	GetConfFileKeyValue("max_worker", arrValue, nSize);
	g_srv_conf_info.iMaxWorker = (WORD)atoi(arrValue);
			
	INFO("GetConfFileValues: func end%s", "");
}



//������;:��ȡ��ͨ�����������ļ���Ϣ
//�������: ��
//�������: ��
//����ֵ	: ��ȡ�ɹ�, ����TRUE, ��ȡʧ��, ����FALSE

int ReadServerConfFile(void)
{
	INFO("ReadServerConfFile: func begin%s", "");
	if (FALSE == InitSrvConfFileInfo())
	{
		ERROR("ReadServerConfFile: Call InitSrvConfFileInfo error%s", "");
		return FALSE;
	}
	
	FILE *fp = NULL;
	char szLine[1024] = {0};
	char szFile_path[FILE_PATH_LEN] = {0};
	int nLen = 0;
	int nRet = 0;
	
	nRet = GetConfFilePath(szFile_path, SERVER_CONFFILE_NAME, FILE_PATH_LEN);
	if (FALSE == nRet)
	{
		ERROR("ReadServerConfFile: Call GetConfFilePath error%s", "");
		return FALSE;
	}

	fp = fopen(szFile_path, "r+");
	if (NULL == fp)
	{
		ERROR("ReadServerConfFile: Call fopen error%s", "");
		return MAINTAIN_CONF_FILE_ERROR;
	}	

	memset(&g_conf_file_data, 0, sizeof(g_conf_file_data));

	while (TRUE)
	{
		memset(szLine, 0, sizeof(szLine));
		
		if (NULL == fgets(szLine, sizeof(szLine), fp))
		{
			break;	
		}

		nLen = strlen(szLine);
		if (1 == nLen && '\n' == szLine[0])
		{
			WARN("ReadServerConfFile: the file line only contain one linefeed [value]=%d", szLine[0]);
			continue;
		}
		
		if ('\n' == szLine[nLen-1])
		{
			szLine[nLen-1] = 0;					//���һ���ַ��ǻ��з�		
		}
			
		GetConfFileFieldValue(szLine);
	}

	DEBUG("ReadServerConfFile: [file count]=%d", g_conf_file_data.nCount);
	GetConfFileValues();

	DEBUG("ReadServerConfFile: [port]=%d [liantong srv num]=%d [min thread num]=%d"
		" [max thread num]=%d [poll heartbeat interval]=%d [heartbeat timeout time]=%d"
		" [pool srv config interval]=%d [pool client msg interval]=%d [client msg timeout time]=%d"
		" [msg queue size]=%d [routingtable size]=%d"
		" [heartbeat table size]=%d [client msg buf table size]=%d [client msg info table size]=%d"
		" [detect reconnect service srv interval]=%d [maintain port]=%d [epoll size]=%d"
		" [server mode]=%d [max client msg thread num]=%d [tmp client sock hashtable size]=%d"
		" [log print level]=%d [sndmsgto servicesrv threadnum]=%d [sndmsgto client threadnum]=%d [access server seq]=%d [max worker]=%d", \
		g_srv_conf_info.wPort, g_srv_conf_info.nLiantong_srv_num, \
		g_srv_conf_info.nMin_thread_num, g_srv_conf_info.nMax_thread_num, g_srv_conf_info.nPoll_heartbeat_interval, \
		g_srv_conf_info.nHeartbeat_timeout_time, g_srv_conf_info.nPool_srvconfig_interval, \
		g_srv_conf_info.nPool_clientmsg_interval, g_srv_conf_info.nClientmsg_timeout_time, g_srv_conf_info.nMsg_queue_size, \
		g_srv_conf_info.nRoutingtable_size, g_srv_conf_info.nHeartbeat_table_size, \
		g_srv_conf_info.nClient_msgbuf_tablesize, g_srv_conf_info.nClient_msginfo_tablesize, \
		g_srv_conf_info.nDec_reconnect_servicesrv_interval, g_srv_conf_info.wMaintain_port, \
		g_srv_conf_info.nEpoll_size, g_srv_conf_info.nServer_mode, g_srv_conf_info.nMax_clientmsg_thread_num, \
		g_srv_conf_info.nTmp_clientsock_hash_size, g_srv_conf_info.bLog_print_level, \
		g_srv_conf_info.nSndmsgto_servicesrv_threadnum, g_srv_conf_info.nSndmsgto_client_threadnum, \
		g_srv_conf_info.wAccess_srv_seq, g_srv_conf_info.iMaxWorker);

	INFO("ReadServerConfFile: func end%s", "");
	return TRUE;
}


