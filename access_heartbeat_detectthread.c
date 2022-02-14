
#include "../util/access_global.h"
#include "../interface/access_protocol.h"
//#include "../include/ydt_log.h"
#include "access_comm_client_rcv.h"
#include "access_epoll.h"
#include "access_comm_client_snd.h"
#include "access_heartbeat_detectthread.h"

/*���ļ���Ҫ�Ǵ�����������, ��ҪĿ����Ϊ�˱���ͻ�������������޷�
  *��epoll�е�socketɾ�����Լ��޷��Ͽ���ͻ���֮�������
  *
  */

//��������ϣ��, ����socket id, ֵ��ʱ��
//�˹�ϣ����Ҫ��Ϊ�˰����������������Ŀͻ���socket�رյ�
extern Server_Conf_Info g_srv_conf_info;


//epoll�ṹ,  ��Ҫ��������ͻ��˵Ĵ󲢷����ӵ�
extern Epoll_Fd g_epoll_fd;

extern StHeartbeat_Detect_Table g_heartbeatdetect_table;

//������;: ��������߳�,  ��Ҫ�Ǽ�ⳬʱ����ͻ������ӵ�socket(��Ҫ�Ƿ�ֹ�ͻ����������)
//�������: ��
//�������: ��
//����ֵ	: ��

void *HeartBeatDetectThread(void *pParam)
{
	INFO("HeartBeatDetectThread: func begin%s", "");
	int nHash_size = 0;
	int i = 0;
	StHash_Table *pHashtable = NULL;
	StHash_Item *pItem = NULL;
	StHash_Item *pLast_item = NULL;
	
	long lCur_time = 0;
	long *pPre_time = NULL;
	long lDiff = 0;
	int nSock = 0;
	char *pSock = NULL;
	int nPool_heartbeat_interval = 0;
	int nHeartbeat_timeout_time = 0;
	int nRet = 0;

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	nPool_heartbeat_interval = g_srv_conf_info.nPoll_heartbeat_interval;
	nHeartbeat_timeout_time = g_srv_conf_info.nHeartbeat_timeout_time;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	DEBUG("HeartBeatDetectThread: [poll heartbeat hashtable time interval]=%d [heartbeat packet timeout time]=%d", \
		nPool_heartbeat_interval, nHeartbeat_timeout_time);	

	//modify by luguanhuang 20111017 (reason: ��ǰ��������ϣ���ָ����ȡ����)
	pthread_mutex_lock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
	pHashtable = g_heartbeatdetect_table.pHeartbeat_detect_table;
	pthread_mutex_unlock(&g_heartbeatdetect_table.heartbeat_detect_mutex);

	while (TRUE)
	{
		sleep(nPool_heartbeat_interval);					//��ѯ��ʱ����
		pthread_mutex_lock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
		nHash_size = pHashtable->nBucket_size;
		
		//��ѯ������ϣ��
		for (i=0; i<nHash_size; i++)
		{
			pItem = pHashtable->ppItem[i];
			pLast_item = pItem;
			while (pItem != NULL)
			{		
				pPre_time = (long *)pItem->pMatch_msg;
				lCur_time = time((time_t *)NULL);
				lDiff = lCur_time - *pPre_time;				
				if (lDiff > nHeartbeat_timeout_time)
				{
					DEBUG("HeartBeatDetectThread: [heartbeat packet time different]=%d [heartbeattimeout time]=%ld", \
						lDiff, nHeartbeat_timeout_time);
					pSock = pItem->pKey;
					nSock = atoi(pSock);

					//modify by luguanhuang(reason: �ظ�ɾ�������������һ�����������)
					nRet = DelSockEventFromepoll(nSock);
					if (FALSE == nRet)
					{
						INFO("HeartBeatDetectThread: Call DelSockEventFromepoll error%s", "");
					}
					else
					{
						INFO("HeartBeatDetectThread: Call DelSockEventFromepoll succeed%s", "");
					}
	
					DelClientSockFromQueue(nSock);
					DelTmpClientSockFromHashtable(nSock);
					DelSndClientTmpSockFromHash(nSock);
					DelClientMsgBufFromHash(nSock);	
					close(nSock);								//�ر�socket

					//RecvClientMsgErrorProcess(nSock);

					//ɾ����ϣ�����
					if (pLast_item == pItem)
					{
						pHashtable->ppItem[i] = pItem->pNext;
						pLast_item = pItem->pNext;
						MM_FREE(pItem->pKey);
						MM_FREE(pItem->pMatch_msg);
						MM_FREE(pItem);	
						pItem = pLast_item;
					}
					else
					{
						pLast_item->pNext = pItem->pNext;
						MM_FREE(pItem->pKey);
						MM_FREE(pItem->pMatch_msg);
						MM_FREE(pItem);
						pItem = pLast_item->pNext;
					}

					if (pHashtable->nHashtable_len > 0)
					{
						pHashtable->nHashtable_len--;
					}
				}
				else
				{
					pLast_item = pItem;
					pItem = pItem->pNext;
				}
			}
		}

		pthread_mutex_unlock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
	}

	INFO("HeartBeatDetectThread: func end%s", "");
	return NULL;
}

void *S_HeartBeatDetectThread(void *pParam)
{

	int nHash_size = 0;
	int i = 0;
	StHash_Table *pHashtable = NULL;
	StHash_Item *pItem = NULL;
	StHash_Item *pLast_item = NULL;

	StClient_Socket_Info *pClient_sock_info = NULL;
	long lCur_time = 0;
	long lPre_time = 0;
	pthread_t thread_id = 0;
	long lDiff = 0;
	int nSock = 0;
	char *pSock = NULL;
	int nPool_heartbeat_interval = 0;
	int nHeartbeat_timeout_time = 0;
	int nLen = 0;

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	nPool_heartbeat_interval = g_srv_conf_info.nPoll_heartbeat_interval;
	nHeartbeat_timeout_time = g_srv_conf_info.nHeartbeat_timeout_time;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	while (TRUE)
	{
		sleep(nPool_heartbeat_interval);					//��ѯ��ʱ����
		
		pthread_mutex_lock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
		pHashtable = g_heartbeatdetect_table.pHeartbeat_detect_table;
		nHash_size = pHashtable->nBucket_size;
		
		//��ѯ������ϣ��
		for (i=0; i<nHash_size; i++)
		{
			pItem = pHashtable->ppItem[i];
			pLast_item = pItem;
			while (pItem != NULL)
			{		
				pClient_sock_info = (StClient_Socket_Info *)pItem->pMatch_msg;
				lPre_time = pClient_sock_info->lTime;
				lCur_time = time((time_t *)NULL);
				lDiff = lCur_time - lPre_time;				
				//PrintContent("HeartBeatDetectThread: different time=%ld\n", lDiff);
				if (lDiff > nHeartbeat_timeout_time)
				{
					DEBUG("HeartBeatDetectThread: timeout diff=%ld", lDiff);
					thread_id = pClient_sock_info->thread_id;
					pSock = pItem->pKey;
					nSock = atoi(pSock);

					if (pthread_cancel(thread_id) != 0)
					{
						ERROR("CancelOperatorProc: Call pthread_cancel error%s", "");
						//return FALSE;
					}
					else
					{
						DEBUG("HeartBeatDetectThread: cancel thread succeed%s", "");	
					}
					
					//DeleteHeartbeatDetectItem(nSock);
					close(nSock);		

					//ɾ����ϣ�����
					if (pLast_item == pItem)
					{
						pHashtable->ppItem[i] = pItem->pNext;
						pLast_item = pItem->pNext;
						MM_FREE(pItem->pKey);
						MM_FREE(pItem->pMatch_msg);
						MM_FREE(pItem); 
						pItem = pLast_item;
					}
					else
					{
						pLast_item->pNext = pItem->pNext;
						MM_FREE(pItem->pKey);
						MM_FREE(pItem->pMatch_msg);
						MM_FREE(pItem); 
						pItem = pLast_item->pNext;
					}

					nLen = HashtableLen(g_heartbeatdetect_table.pHeartbeat_detect_table);
					DEBUG("HeartBeatDetectThread: nLen = %d", nLen);
				}
				else
				{
					pLast_item = pItem;
					pItem = pItem->pNext;
				}
			}
		}

		pthread_mutex_unlock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
	}

	return NULL;
}

//������;:  �����ģʽ��������(����socket id,  ֵ�ǵ�ǰʱ��)
//�������:  socket  id
//�������:  ��
//����ֵ	: ����ɹ�,  ����TRUE,  ����ʧ��,  ����FALSE

int S_InsertHeartbeatDetectItem(int nSock, pthread_t thread_id)
{
	int nRet = 0;
	StClient_Socket_Info *pClient_sock_info = (StClient_Socket_Info *)MM_MALLOC_WITH_DESC(sizeof(StClient_Socket_Info), \
	"S_InsertHeartbeatDetectItem: Call func for client sock info");
	if (NULL == pClient_sock_info)
	{
		FATAL("S_InsertHeartbeatDetectItem: Call malloc error%s", "");
		return FALSE;
	}

	memset(pClient_sock_info, 0, sizeof(StClient_Socket_Info));

	char szSock[100] = {0};
	snprintf(szSock, sizeof(szSock) - 1, "%d", nSock);

	StClient_Socket_Info *pTmp_client_sock_info = NULL;

	pClient_sock_info->lTime = time((time_t *)NULL);
	pClient_sock_info->thread_id = thread_id;
	nRet = HashInsert(&g_heartbeatdetect_table.pHeartbeat_detect_table, szSock, (void *)pClient_sock_info);
	if (FALSE == nRet)
	{
		MM_FREE(pClient_sock_info);
		return FALSE;
		
	}
	else if (HASH_SAMEKEY_EXIST == nRet)				//�����ֵ�Ѵ���
	{
		pTmp_client_sock_info = (StClient_Socket_Info *)HashData(g_heartbeatdetect_table.pHeartbeat_detect_table, szSock);
		if (pTmp_client_sock_info != NULL)
		{
			pthread_mutex_lock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
			*pTmp_client_sock_info = *pClient_sock_info;					//����ʱ��
			pthread_mutex_unlock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
		}

		MM_FREE(pClient_sock_info); 
	}	
	else if (TRUE == nRet)
	{
		DEBUG("InsertHeartbeatDetectItem: insert succeed%s", "");
	}
	
	return TRUE;
}

//������;:  ������������(����socket id,  ֵ�ǵ�ǰʱ��)
//�������:  socket  id
//�������:  ��
//����ֵ	: ����ɹ�,  ����TRUE,  ����ʧ��,  ����FALSE

int InsertHeartbeatDetectItem(int nSock)
{
	INFO("InsertHeartbeatDetectItem: func begin%s", "");
	int nRet = 0;
	long *pCur_time = (long *)MM_MALLOC_WITH_DESC(sizeof(long), \
	"InsertHeartbeatDetectItem: Call func for current time");
	if (NULL == pCur_time)
	{
		FATAL("InsertHeartbeatDetectItem: call malloc error%s", "");
		return OUT_OF_MEMORY_ERROR;
	}

	char szSock[100] = {0};
	snprintf(szSock, sizeof(szSock) - 1, "%d", nSock);

	long *pTmp_time = NULL;

	*pCur_time = time((time_t *)NULL);
	int nTmp_time = *pCur_time;
	nRet = HashInsert(&g_heartbeatdetect_table.pHeartbeat_detect_table, szSock, (void *)pCur_time);
	if (TRUE != nRet && HASH_SAMEKEY_EXIST != nRet)
	{
		ERROR("InsertHeartbeatDetectItem: insert heartbeat detect item error [return value]=%d", nRet);
		MM_FREE(pCur_time); 	
		return FALSE;
	}
	
	if (HASH_SAMEKEY_EXIST == nRet)				//�����ֵ�Ѵ���
	{
		pTmp_time = (long *)HashData(g_heartbeatdetect_table.pHeartbeat_detect_table, szSock);
		if (pTmp_time != NULL)
		{
			pthread_mutex_lock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
			*pTmp_time = *pCur_time;					//����ʱ��
			pthread_mutex_unlock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
		}

		MM_FREE(pCur_time); 

		INFO("InsertHeartbeatDetectItem:  heartbeat detect item exist yest in the hashtable then replace"
			" the previous value [socket id]=%d [time]=%d", nSock, nTmp_time);
	}	
	else if (TRUE == nRet)
	{
		INFO("InsertHeartbeatDetectItem: insert heartbeat detect packet succeed "
			" [socket id]=%d [time]=%d", nSock, nTmp_time);
	}
	
	INFO("InsertHeartbeatDetectItem: func end%s", "");
	return TRUE;
}

//������;:  ����������ϣ������ɾ���ض�����
//�������: �������ļ�:  socket id
//�������:  ��
//����ֵ	: ɾ���ɹ�,  ����TRUE,  ɾ��ʧ��,  ����FALSE

int DeleteHeartbeatDetectItem(int nSock)
{
	INFO("DeleteHeartbeatDetectItem: func begin%s", "");
	char szSock[100] = {0};
	snprintf(szSock, sizeof(szSock) - 1, "%d", nSock);
	void *pValue = HashDel(&(g_heartbeatdetect_table.pHeartbeat_detect_table), szSock);

	if (NULL != pValue)
	{
		MM_FREE(pValue);
		INFO("DeleteHeartbeatDetectItem: delete heartbeat table item from heartbeat detect table succeed%s", "");
	}
	else
	{
		INFO("DeleteHeartbeatDetectItem: delete heartbeat table item from heart detect table fail%s", "");	
	}

	INFO("DeleteHeartbeatDetectItem: func end%s", "");
	return TRUE;
}

//������;:  �ı���������ʱ��
//�������: socket id
//�������:  ��
//����ֵ	: �ı�ɹ�,  ����TRUE��  �ı�ʧ��,  ����FALSE��

int ChangeHeartBeatPacketTime(int nSock)
{
	INFO("ChangeHeartBeatPacketTime: func begin%s", "");
	long *pTime = NULL;
	StHash_Item *pItem = NULL;
	long lCur_time = 0;
	int nRet = 0;
	char szSock[100] = {0};
	snprintf(szSock, sizeof(szSock) - 1, "%d", nSock);

	pthread_mutex_lock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
	pItem = HashGetItem(g_heartbeatdetect_table.pHeartbeat_detect_table, szSock);
	lCur_time = time((time_t *)NULL);
	if (NULL != pItem)
	{
		pTime = (long *)pItem->pMatch_msg;	
		//���������������ԭ���������󲢷���ʱ��, 
		//������ӵĻ�, �����
		*pTime = lCur_time;				//�ı���������ʱ��

		INFO("ChangeHeartBeatPacketTime: heartbeat packet exist in the hashtable,"
			" change heartbeat time succeed [socket id]=%d [time]=%d", nSock, lCur_time);
	}
	else
	{
		long *pCur_time = (long *)MM_MALLOC_WITH_DESC(sizeof(long), \
		"ChangeHeartBeatPacketTime: Call func for current time");	
		if (NULL == pCur_time)
		{
			FATAL("ChangeHeartBeatPacketTime: Call malloc error%s", "");
			pthread_mutex_unlock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
			return OUT_OF_MEMORY_ERROR;
		}
		
		*pCur_time = lCur_time;
		nRet = HashInsert(&(g_heartbeatdetect_table.pHeartbeat_detect_table), szSock, (void *)pCur_time);
		if (TRUE != nRet && HASH_SAMEKEY_EXIST != nRet)
		{
			ERROR("ChangeHeartBeatPacketTime: Call HashInsert error%s", "");
			pthread_mutex_unlock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
			return FALSE;
		}

		INFO("ChangeHeartBeatPacketTime: heartbeat packet not exist in the hashtable,"
			" change heartbeat time succeed [socket id]=%d [time]=%d", nSock, lCur_time);
	}

	INFO("ChangeHeartBeatPacketTime: func end%s", "");
	pthread_mutex_unlock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
	return TRUE;
}

