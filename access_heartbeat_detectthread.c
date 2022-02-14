
#include "../util/access_global.h"
#include "../interface/access_protocol.h"
//#include "../include/ydt_log.h"
#include "access_comm_client_rcv.h"
#include "access_epoll.h"
#include "access_comm_client_snd.h"
#include "access_heartbeat_detectthread.h"

/*此文件主要是处理心跳包的, 主要目的是为了避免客户端意外崩溃而无法
  *把epoll中的socket删除掉以及无法断开与客户端之间的连接
  *
  */

//心跳检测哈希表, 键是socket id, 值是时间
//此哈希表主要是为了把与接入服务器断连的客户端socket关闭掉
extern Server_Conf_Info g_srv_conf_info;


//epoll结构,  主要用来处理客户端的大并发连接的
extern Epoll_Fd g_epoll_fd;

extern StHeartbeat_Detect_Table g_heartbeatdetect_table;

//函数用途: 心跳检测线程,  主要是检测超时的与客户端连接的socket(主要是防止客户端意外崩溃)
//输入参数: 无
//输出参数: 无
//返回值	: 无

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

	//modify by luguanhuang 20111017 (reason: 提前把心跳哈希表的指针提取出来)
	pthread_mutex_lock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
	pHashtable = g_heartbeatdetect_table.pHeartbeat_detect_table;
	pthread_mutex_unlock(&g_heartbeatdetect_table.heartbeat_detect_mutex);

	while (TRUE)
	{
		sleep(nPool_heartbeat_interval);					//轮询的时间间隔
		pthread_mutex_lock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
		nHash_size = pHashtable->nBucket_size;
		
		//轮询心跳哈希表
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

					//modify by luguanhuang(reason: 重复删除心跳包里面的一项而导致死锁)
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
					close(nSock);								//关闭socket

					//RecvClientMsgErrorProcess(nSock);

					//删除哈希表的项
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
		sleep(nPool_heartbeat_interval);					//轮询的时间间隔
		
		pthread_mutex_lock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
		pHashtable = g_heartbeatdetect_table.pHeartbeat_detect_table;
		nHash_size = pHashtable->nBucket_size;
		
		//轮询心跳哈希表
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

					//删除哈希表的项
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

//函数用途:  插入简单模式心跳检查包(键是socket id,  值是当前时间)
//输入参数:  socket  id
//输出参数:  无
//返回值	: 插入成功,  返回TRUE,  插入失败,  返回FALSE

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
	else if (HASH_SAMEKEY_EXIST == nRet)				//如果键值已存在
	{
		pTmp_client_sock_info = (StClient_Socket_Info *)HashData(g_heartbeatdetect_table.pHeartbeat_detect_table, szSock);
		if (pTmp_client_sock_info != NULL)
		{
			pthread_mutex_lock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
			*pTmp_client_sock_info = *pClient_sock_info;					//更新时间
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

//函数用途:  插入心跳检查包(键是socket id,  值是当前时间)
//输入参数:  socket  id
//输出参数:  无
//返回值	: 插入成功,  返回TRUE,  插入失败,  返回FALSE

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
	
	if (HASH_SAMEKEY_EXIST == nRet)				//如果键值已存在
	{
		pTmp_time = (long *)HashData(g_heartbeatdetect_table.pHeartbeat_detect_table, szSock);
		if (pTmp_time != NULL)
		{
			pthread_mutex_lock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
			*pTmp_time = *pCur_time;					//更新时间
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

//函数用途:  从心跳检测哈希表里面删除特定的项
//输入参数: 心跳检测的键:  socket id
//输出参数:  无
//返回值	: 删除成功,  返回TRUE,  删除失败,  返回FALSE

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

//函数用途:  改变心跳包的时间
//输入参数: socket id
//输出参数:  无
//返回值	: 改变成功,  返回TRUE，  改变失败,  返回FALSE；

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
		//这里在外面加锁的原因是在做大并发的时候, 
		//如果不加的话, 会崩溃
		*pTime = lCur_time;				//改变心跳包的时间

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

