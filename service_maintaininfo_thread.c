
#include "../util/service_global.h"
#include "../include/ydt_log.h"
#include "../interface/service_protocol.h"
#include "service_sock_info.h"
#include "service_threadpoolmode_server.h"
#include "service_thread_pool.h"
#include "service_maintaininfo_thread.h"

#define MODIFY_LOG_LEVEL_FAILURE 0x00	//修改日记等级失败
#define MODIFY_LOG_LEVEL_SUCCEED 0x01	//修改日记等级成功

#define CLEAR_SRV_INFO_FAILURE 0x00		//清除服务器信息失败
#define CLEAR_SRV_INFO_SUCCEED 0x01		//清除服务器信息成功

extern Server_Conf_Info g_srv_conf_info;

extern Process_Thread_Counter g_process_thread_counter;

#define QUERY_SRV_INFO 0x00			//查询服务器信息
#define MODIFY_LOG_LEVEL 0x01		//修改日记级别
#define CLEAR_SRV_INFO 0x02			//清除服务器信息

#define CLEAR_QUEUE_INFO '1'		//清除队列信息

#define CLEAR_QUEUEIN '0'			//清除queuein信息
#define CLEAR_QUEUEOUT '1'			//清除queueout信息
#define CLEAR_QUEUEIN_QUEUEOUT '2'	//清除queuein和queueout信息

//数据库路由表
extern StDb_RoutingTable g_db_routingtable;

//临时的数据库路由表
extern StDb_RoutingTable g_db_tmp_routingtable;

extern StHeartbeat_Detect_Table g_heartbeat_detect_table;

extern Threads_Info g_threads_info;							//线程ID信息

extern StSave_ThreadRes_Hash g_save_threadres_hash;

/*该文件是用来处理运维客户端到来的消息的
  *
  *
  *
  */



int GetDBConnPoolInfo(StConn_Pool_Info **ppConn_pool_info, int *pConn_pool_num)
{
	INFO("GetDBConnPoolInfo: func begin%s", "");
	if (NULL == pConn_pool_num)
	{
		ERROR("GetDBConnPoolInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	StHash_Item *pItem = NULL;
	int i = 0;
	int j = 0;
	int nHash_len = 0;
	int *pConn_pool_addr = NULL;
	int nConnpool_info_idx = 0;
	Conn_Pool *pConn_pool = NULL;
	int nUsed_conn_num = 0;
	int nNotused_conn_num = 0;
	int nUnconnect_conn_num = 0;
		
	pthread_mutex_lock(&g_db_tmp_routingtable.routing_mutex);
	StHash_Table *pHashtable = g_db_tmp_routingtable.pDb_Routing_table;
	nHash_len = HashtableLen(pHashtable);
	DEBUG("GetDBConnPoolInfo: [temp db routing table length]=%d", nHash_len);
	if (nHash_len > 0)
	{
		*ppConn_pool_info = (StConn_Pool_Info *)MM_MALLOC_WITH_DESC(sizeof(StConn_Pool_Info) * nHash_len, \
			"GetDBConnPoolInfo: Call func for connect pool info");
		if (NULL == *ppConn_pool_info)
		{
			FATAL("GetDBConnPoolInfo: Call MM_MALLOC_WITH_DESC eror%s", "");
			pthread_mutex_unlock(&g_db_tmp_routingtable.routing_mutex);
			return FALSE;
		}

		memset(*ppConn_pool_info, 0, sizeof(StConn_Pool_Info) * nHash_len);
	}
	
	int nSize = pHashtable->nBucket_size;	
	for (i=0; i<nSize; i++)
	{
		pItem = pHashtable->ppItem[i];
		while (NULL != pItem)
		{
			if (NULL != pItem->pMatch_msg)
			{
				pConn_pool_addr = (int *)pItem->pMatch_msg;
				pConn_pool = (Conn_Pool *)*pConn_pool_addr;

				(*ppConn_pool_info)[nConnpool_info_idx].nTotal_conn_num = htonl(pConn_pool->nConnect_num);
				for (j=0; j<pConn_pool->nConnect_num; j++)
				{
					switch (pConn_pool->arrConn_elem[j].is_used)
					{
						case NO:
							nNotused_conn_num++;
							break;
						case YES:
							nUsed_conn_num++;
							break;
						case UNCONNECT:
							nUnconnect_conn_num++;
							break;
						default:
							break;
					}
				}

				(*ppConn_pool_info)[nConnpool_info_idx].nNotused_conn_num = htonl(nNotused_conn_num);
				(*ppConn_pool_info)[nConnpool_info_idx].nUsed_conn_num = htonl(nUsed_conn_num);
				(*ppConn_pool_info)[nConnpool_info_idx].nUnconnect_conn_num = htonl(nUnconnect_conn_num);
				nConnpool_info_idx++;
			}
			
			pItem = pItem->pNext;
		}
	}
	
	pthread_mutex_unlock(&g_db_tmp_routingtable.routing_mutex);

	*pConn_pool_num = nConnpool_info_idx;
	INFO("GetDBConnPoolInfo: func end%s", "");
	return TRUE;
}


static int T_ProcQuerySrvInfoMsg(int nSock, void *pData)
{
	INFO("T_ProcQuerySrvInfoMsg: func begin%s", "");
	int i = 0;
	int nHash_size = 0;
	StHash_Item *pItem = NULL;
	StThread_Resource *pThread_res = NULL;
	int nTmp_queue_len = 0;
	int nIdx = 0;
	int nConn_pool_num = 0;

	Req_Query_Srv_Info *pQuery_srv_info = (Req_Query_Srv_Info *)pData;
	Res_T_Query_Srv_Info res_t_query_srv_info;
	memset(&res_t_query_srv_info, 0, sizeof(res_t_query_srv_info));

	GetDBConnPoolInfo(&res_t_query_srv_info.pConn_pool_info, &nConn_pool_num);
	DEBUG("T_ProcQuerySrvInfoMsg: [connect pool num]=%d", nConn_pool_num);
	for (i=0; i<nConn_pool_num; i++)
	{
		DEBUG("T_ProcQuerySrvInfoMsg: connect pool%d [total connect num]=%d [used connect num]=%d"
			" [not used connect num]=%d [unconnect connect num]=%d", (i + 1), \
			ntohl(res_t_query_srv_info.pConn_pool_info[i].nTotal_conn_num), \
			ntohl(res_t_query_srv_info.pConn_pool_info[i].nUsed_conn_num), \
			ntohl(res_t_query_srv_info.pConn_pool_info[i].nNotused_conn_num), \
			ntohl(res_t_query_srv_info.pConn_pool_info[i].nUnconnect_conn_num));
	}
	
	pthread_mutex_lock(&g_save_threadres_hash.hash_mutex);
	StHash_Table *pHashtable = g_save_threadres_hash.pSave_threadres_hash;
	int nHash_len = HashtableLen(pHashtable);
	if (nHash_len > 0)
	{
		res_t_query_srv_info.pMsg_queue_len = (StMsg_queue_len *)MM_MALLOC_WITH_DESC(nHash_len * sizeof(StMsg_queue_len), \
		"T_ProcQuerySrvInfoMsg: Call func for access server message queue length");
		if (NULL == res_t_query_srv_info.pMsg_queue_len)
		{
			FATAL("T_ProcQuerySrvInfoMsg: Call MM_MALLOC_WITH_DESC error%s", "");
			return OUT_OF_MEMORY_ERROR;
		}
		
		memset(res_t_query_srv_info.pMsg_queue_len, 0, nHash_len * sizeof(StMsg_queue_len));
		nHash_size = pHashtable->nBucket_size;
		for (i=0; i<nHash_size; i++)
		{
			pItem = pHashtable->ppItem[i];
			while (NULL != pItem)
			{
				pThread_res = (StThread_Resource *)(StThread_Resource *)pItem->pMatch_msg;
				if (NULL != pThread_res)
				{
					nTmp_queue_len = SeqQueueLength(pThread_res->stQueue_in.queue);
					res_t_query_srv_info.pMsg_queue_len[nIdx].nMsgin_len = htonl(nTmp_queue_len);
					nTmp_queue_len = SeqQueueLength(pThread_res->stQueue_out.queue);
					res_t_query_srv_info.pMsg_queue_len[nIdx].nMsgout_len = htonl(nTmp_queue_len);
					memcpy(res_t_query_srv_info.pMsg_queue_len[nIdx].arrAccess_srv_ip, pThread_res->arrAcccee_srv_ip, \
						sizeof(res_t_query_srv_info.pMsg_queue_len[nIdx].arrAccess_srv_ip) - 1);
					res_t_query_srv_info.pMsg_queue_len[nIdx].wAccess_srv_port = htons(pThread_res->wAccess_srv_port);
					res_t_query_srv_info.pMsg_queue_len[nIdx].nSock = htonl(pThread_res->nSock_id);
				}

				nIdx++;
				pItem = pItem->pNext;
			}
		}
	}
	pthread_mutex_unlock(&g_save_threadres_hash.hash_mutex);
	
	res_t_query_srv_info.bVersion = pQuery_srv_info->bVersion;
	res_t_query_srv_info.wCmd_id = pQuery_srv_info->wCmd_id;
	res_t_query_srv_info.nConn_pool_num = htonl(nConn_pool_num);
	res_t_query_srv_info.nAccess_srv_num = htonl(nHash_len);

	int nData_len = sizeof(Res_T_Query_Srv_Info) - sizeof(int *) * 2 \
		+ sizeof(StMsg_queue_len) * nHash_len + sizeof(StConn_Pool_Info) * nConn_pool_num;
	char *pSnd_data = (char *)MM_MALLOC_WITH_DESC(nData_len, \
	"T_ProcQuerySrvInfoMsg: Call func for send data");
	if (NULL == pSnd_data)
	{
		FATAL("T_ProcQuerySrvInfoMsg: Call MM_MALLOC_WITH_DESC error%s", "");
		return OUT_OF_MEMORY_ERROR;
	}
	memset(pSnd_data, 0, sizeof(nData_len));
	int nTmp_len = sizeof(Res_T_Query_Srv_Info) - 12;
	memcpy(pSnd_data, &res_t_query_srv_info, nTmp_len);
	memcpy(pSnd_data + nTmp_len, res_t_query_srv_info.pMsg_queue_len, \
		sizeof(StMsg_queue_len) * nHash_len);
	nTmp_len += sizeof(StMsg_queue_len) * nHash_len;
	memcpy(pSnd_data + nTmp_len, &res_t_query_srv_info.nConn_pool_num, sizeof(int));
	nTmp_len += sizeof(int);
	memcpy(pSnd_data + nTmp_len, res_t_query_srv_info.pConn_pool_info, \
		sizeof(StConn_Pool_Info) * nConn_pool_num);
	
	#if 0
	for (i=0; i<nHash_len; i++)
	{
		nTmp_len += i * sizeof(StMsg_queue_len);
		memcpy(pSnd_data + nTmp_len, &(res_t_query_srv_info.pMsg_queue_len[i]), sizeof(StMsg_queue_len));
	}
	#endif

	int nSnd_len = htonl(nData_len);
	send(nSock, (char *)&nSnd_len, sizeof(int), 0);
	send(nSock, pSnd_data, nData_len, 0);

	DEBUG("T_ProcQuerySrvInfoMsg: [version]=%d [command id]=%d [data length]=%d"
		" [access server num]=%d [connect pool num]=%d", \
		res_t_query_srv_info.bVersion, ntohs(res_t_query_srv_info.wCmd_id), nData_len, \
		nHash_len, ntohl(res_t_query_srv_info.nConn_pool_num));
	for (i=0; i<nHash_len; i++)
	{
		DEBUG("T_ProcQuerySrvInfoMsg: [message queuein%d length]=%d [message queueout%d length]=%d"
			" [access server%d ip]=%s [access server%d port]=%d [access server%d socket id]=%d [len]=%d", \
			(i + 1), ntohl(res_t_query_srv_info.pMsg_queue_len[i].nMsgin_len), (i + 1), \
			ntohl(res_t_query_srv_info.pMsg_queue_len[i].nMsgout_len), \
			(i + 1), res_t_query_srv_info.pMsg_queue_len[i].arrAccess_srv_ip, (i + 1), \
			ntohs(res_t_query_srv_info.pMsg_queue_len[i].wAccess_srv_port), \
			(i + 1), ntohl(res_t_query_srv_info.pMsg_queue_len[i].nSock), nHash_len);
	}

	for (i=0; i<nConn_pool_num; i++)
	{
		DEBUG("T_ProcQuerySrvInfoMsg: connect pool%d [total connect num]=%d [used connect num]=%d"
			" [not used connect num]=%d [unconnect connect num]=%d", \
			(i + 1), ntohl(res_t_query_srv_info.pConn_pool_info[i].nTotal_conn_num), \
			ntohl(res_t_query_srv_info.pConn_pool_info[i].nUsed_conn_num), \
			ntohl(res_t_query_srv_info.pConn_pool_info[i].nNotused_conn_num), \
			ntohl(res_t_query_srv_info.pConn_pool_info[i].nUnconnect_conn_num));
	}

	INFO("T_ProcQuerySrvInfoMsg: send message to maintain client succeed%s", "");

	MM_FREE(pSnd_data);
	MM_FREE(res_t_query_srv_info.pMsg_queue_len);
	MM_FREE(res_t_query_srv_info.pConn_pool_info);

	INFO("T_ProcQuerySrvInfoMsg: func end%s", "");
	return TRUE;	
}

#if 0
static int ProcQuerySrvInfoMsg(int nSock, void *pData)
{
	INFO("ProcQuerySrvInfoMsg: func begin%s", "");
	int nMax_procthread_num = 0;
	int nDB_routingtable_len = 0;
	int nTmpdb_routingtable_len = 0;
	int nHeartbeat_table_len = 0;
	int nMsg_queue_len = 0;
	int nCount = 0;
	int i = 0;

	Req_Query_Srv_Info *pQuery_srv_info = (Req_Query_Srv_Info *)pData;

	Res_Query_Srv_Info res_query_srv_info;
	memset(&res_query_srv_info, 0, sizeof(res_query_srv_info));
	
	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	nMax_procthread_num = g_srv_conf_info.nMax_process_thread_num;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	pthread_mutex_lock(&g_db_routing_table.routing_mutex);
	nDB_routingtable_len = YDT_Hashtable_Len(g_db_routing_table.pDb_Routing_table);
	nTmpdb_routingtable_len = YDT_Hashtable_Len(g_tmp_db_routing_table.pDb_Routing_table);
	nHeartbeat_table_len = YDT_Hashtable_Len(g_heartbeat_detect_table.pHeartbeat_detect_table);

	nCount = g_threads_info.nID_counter;
	res_query_srv_info.nAccess_srv_num = htonl(nCount);
	res_query_srv_info.pMsg_queue_len = (int *)MM_MALLOC_WITH_DESC(nCount, \
	"ProcQuerySrvInfoMsg: Call func for message queue length");
	if (NULL == res_query_srv_info.pMsg_queue_len)
	{
		FATAL("ProcQuerySrvInfoMsg: Call MM_MALLOC_WITH_DESC error%s", "");
		return FALSE;
	}

	memset(res_query_srv_info.pMsg_queue_len, 0, sizeof(int) * nCount);
	
	for (i=0; i<nCount; i++)
	{
		nMsg_queue_len = YDT_QueueLength(g_threads_info.pKeep_param[i]->queue);
		res_query_srv_info.pMsg_queue_len[i] = htonl(nMsg_queue_len);
	}
		
	pthread_mutex_unlock(&g_db_routing_table.routing_mutex);
	
	res_query_srv_info.bVersion = pQuery_srv_info->bVersion;
	res_query_srv_info.wCmd_id = pQuery_srv_info->wCmd_id;
	//res_query_srv_info.wMax_procthread_num = htons((WORD)nMax_procthread_num);
	res_query_srv_info.wCur_procthread_num = htons(g_process_thread_counter.nThread_counter);
	res_query_srv_info.nDB_routingtable_len = htonl(nDB_routingtable_len);
	res_query_srv_info.nTmpdb_routingtable_len = htonl(nTmpdb_routingtable_len);
	res_query_srv_info.nHeartbeat_table_len = htonl(nHeartbeat_table_len);
	//res_query_srv_info.nMsg_queue_len = htonl(nMsg_queue_len);

	int nData_len = sizeof(Res_Query_Srv_Info) - 4 + sizeof(int) * nCount;
	char *pSnd_data = (char *)MM_MALLOC_WITH_DESC(nData_len, \
	"ProcQuerySrvInfoMsg: Call func for send data");
	if (NULL == pSnd_data)
	{
		FATAL("ProcQuerySrvInfoMsg: Call MM_MALLOC_WITH_DESC error%s", "");
		return FALSE;
	}

	int nTmp_len = sizeof(Res_Query_Srv_Info) - 4;
	memcpy(pSnd_data, &res_query_srv_info, nTmp_len);

	for (i=0; i<nCount; i++)
	{
		nTmp_len += i * sizeof(int);
		memcpy(pSnd_data + nTmp_len, &(res_query_srv_info.pMsg_queue_len[i]), sizeof(int));
	}

	int nSnd_len = htonl(nData_len);
	send(nSock, (char *)&nSnd_len, sizeof(int), 0);
	send(nSock, pSnd_data, nData_len, 0);

	DEBUG("ProcQuerySrvInfoMsg: [version]=%d [command id]=%d [data length]=%d"
		" [database routingtable length]=%d [temp database routingtable length]=%d"
		" [heartbeat detect hashtable length]=%d", \
		res_query_srv_info.bVersion, ntohs(res_query_srv_info.wCmd_id), nData_len, nDB_routingtable_len, nTmpdb_routingtable_len, nHeartbeat_table_len);
	for (i=0; i<nCount; i++)
	{
		DEBUG("ProcQuerySrvInfoMsg: [message queue%d length]=%d", \
			(i + 1), ntohl(res_query_srv_info.pMsg_queue_len[i]));	
	}

	INFO("ProcQuerySrvInfoMsg: send message to maintain client succeed%s", "");

	MM_FREE(pSnd_data);
	MM_FREE(res_query_srv_info.pMsg_queue_len);

	INFO("ProcQuerySrvInfoMsg: func end%s", "");
	return TRUE;
}
#endif

static int ClearQueueIn(StSequence_Queue *pQueue_in, pthread_mutex_t *pQueue_mutex)
{
	INFO("ClearQueueIn: func begin%s", "");
	int nRet = 0;
	void *pMsg = NULL;
	char *pTmp_msg = NULL;
	BYTE bMsg_type = 0;
	Client_Server_Msg *pClient_server_msg = NULL;
	int nQueue_len = 0;
	
	pthread_mutex_lock(pQueue_mutex);
	nRet = GetMsgFromSeqQueue(pQueue_in, &pMsg);
	while (TRUE == nRet)
	{
		pTmp_msg = (char *)pMsg;
		pTmp_msg++;
		bMsg_type = *pTmp_msg;
		if (CLIENT_SERVER_MSG == bMsg_type)
		{
			INFO("ClearQueueIn: we will free the client server message buffer%s", "");
			pClient_server_msg = (Client_Server_Msg *)pMsg;
			MM_FREE(pClient_server_msg->pData);
			MM_FREE(pClient_server_msg);
		}

		nQueue_len = SeqQueueLength(*pQueue_in);
		DEBUG("ClearQueueIn: [queue length]=%d", nQueue_len);
		nRet = GetMsgFromSeqQueue(pQueue_in, &pMsg);		
	}


	pQueue_in->nFront = 0;
	pQueue_in->nRear = 0;
	pthread_mutex_unlock(pQueue_mutex);
	INFO("ClearQueueIn: func end%s", "");
	return TRUE;
}


int ClearQueueOut(StSequence_Queue *pQueue_out, pthread_mutex_t *pQueue_mutex)
{
	INFO("ClearQueueOut: func begin%s", "");
	int nRet = 0;
	int nQueue_len = 0;
	void *pMsg = NULL;
	char *pTmp_msg = NULL;
	BYTE bMsg_type = 0;
	Server_Client_Msg *pServer_client_msg = NULL;
	Forward_Srv_Client_Msg *pForward_srv_client_msg = NULL;

	pthread_mutex_lock(pQueue_mutex);
	nRet = GetMsgFromSeqQueue(pQueue_out, &pMsg);
	while (TRUE == nRet)
	{
		pTmp_msg = (char *)pMsg;
		pTmp_msg++;
		bMsg_type = *pTmp_msg;
		if (SERVER_CLIENT_MSG == bMsg_type)
		{
			INFO("ClearQueueOut: we will free the server client message buffer%s", "");
			pServer_client_msg = (Server_Client_Msg *)pMsg;
			MM_FREE(pServer_client_msg->pData);
			MM_FREE(pServer_client_msg);
		}
		else if (FORWARD_SRV_CLIENT_MSG == bMsg_type)
		{
			INFO("ClearQueueOut: we will free the forward server client message buffer%s", "");
			pForward_srv_client_msg = (Forward_Srv_Client_Msg *)pMsg;
			MM_FREE(pForward_srv_client_msg->pData);
			MM_FREE(pForward_srv_client_msg);
		}

		nQueue_len = SeqQueueLength(*pQueue_out);
		DEBUG("ClearQueueOut: [queue length]=%d", nQueue_len);
		nRet = GetMsgFromSeqQueue(pQueue_out, &pMsg);		
	}


	pQueue_out->nFront = 0;
	pQueue_out->nRear = 0;
	pthread_mutex_unlock(pQueue_mutex);
	INFO("ClearQueueOut: func end%s", "");
	return TRUE;
}

static int ProcClearServerInfoMsg(int nSock, void *pData)
{
	INFO("ProcClearServerinfo: func begin%s", "");

	int nLen = 0;
	int nData_len = 0;
	BYTE bClear_queue_type = 0;
	char szSock[100] = {0};
	int nRet = 0;
	int nTmp_sock = 0;
	
	if (NULL == pData)
	{
		ERROR("ProcClearServerinfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	Req_Clear_Srv_Info *pReq_clear_srvinfo = (Req_Clear_Srv_Info *)pData;
	BYTE bClear_type = pReq_clear_srvinfo->arrClear_info[0];
	
	DEBUG("ProcClearServerinfo: [clear info]=%s [clear type]=%c", \
		pReq_clear_srvinfo->arrClear_info, bClear_type);
	
	if (CLEAR_QUEUE_INFO == bClear_type)
	{
		nLen = strlen(pReq_clear_srvinfo->arrClear_info);
		bClear_queue_type = pReq_clear_srvinfo->arrClear_info[nLen-1];		
		memcpy(szSock, (pReq_clear_srvinfo->arrClear_info + 1), nLen - 2);
		DEBUG("ProcClearServerinfo: [socket id]=%s", szSock);
		nTmp_sock = atoi(szSock);
		StThread_Resource *pThread_res = GetThreadResourceFromHash(nTmp_sock);
		if (NULL == pThread_res)
		{
			ERROR("ProcClearServerinfo: the socket id is invalid we can't"
				" find the thread res by sock id [socket id]=%d", nSock);
		}
		else
		{
			if (CLEAR_QUEUEIN == bClear_queue_type)
			{
				nRet = ClearQueueIn(&pThread_res->stQueue_in.queue, &pThread_res->stQueue_in.mutex);
			}
			else if (CLEAR_QUEUEOUT == bClear_queue_type)
			{
				nRet = ClearQueueOut(&pThread_res->stQueue_out.queue, &pThread_res->stQueue_out.mutex);
			}
			else if (CLEAR_QUEUEIN_QUEUEOUT == bClear_queue_type)
			{
				nRet = ClearQueueIn(&pThread_res->stQueue_in.queue, &pThread_res->stQueue_in.mutex);	
				nRet = ClearQueueOut(&pThread_res->stQueue_out.queue, &pThread_res->stQueue_out.mutex);
			}
		}
		
	}

	Res_Clear_Srv_Info res_clear_srv_info;
	memset(&res_clear_srv_info, 0, sizeof(res_clear_srv_info));
	res_clear_srv_info.bVersion = pReq_clear_srvinfo->bVersion;
	res_clear_srv_info.wCmd_id = pReq_clear_srvinfo->wCmd_id;
	res_clear_srv_info.bClear_status = CLEAR_SRV_INFO_SUCCEED;

	nData_len = sizeof(Res_Clear_Srv_Info);
	nLen = htonl(nData_len);
	nRet = send(nSock, (char *)&nLen, sizeof(int), 0);

	if (0 == nRet)
	{
		ERROR("ProcClearServerinfo: Call send error error[%d]=%s", errno, strerror(errno));
		return FALSE;
	}
	
	nRet = send(nSock, (char *)&res_clear_srv_info, nData_len, 0);
	if (0 == nRet)
	{
		ERROR("ProcClearServerinfo: Call send error error[%d]=%s", errno, strerror(errno));
		return FALSE;			
	}

	DEBUG("ProcClearServerinfo: [version]=%d [command id]=%d [clear type]=%d"
		" [clear status]=%d", res_clear_srv_info.bVersion, ntohs(res_clear_srv_info.wCmd_id), \
		bClear_type, res_clear_srv_info.bClear_status);

	DEBUG("ProcClearServerinfo: send message to maintain client succeed%s", "");
	INFO("ProcClearServerinfo: func end%s", "");
	return TRUE;
}

static int ProcModifyLogLevel(int nSock, void *pData)
{
	INFO("ProcModifyLogLevel: func begin%s", "");
	if (NULL == pData)
	{
		ERROR("ProcModifyLogLevel: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	Req_Modify_Log_Level *pModify_log_level = (Req_Modify_Log_Level *)pData;	
	Res_Modify_Log_Level res_modify_log_level;
	memset(&res_modify_log_level, 0, sizeof(res_modify_log_level));

	res_modify_log_level.bVersion = pModify_log_level->bVersion;
	res_modify_log_level.wCmd_id = pModify_log_level->wCmd_id; 
	res_modify_log_level.bModify_status = MODIFY_LOG_LEVEL_SUCCEED;

	SetLogLevel((EnLevel)pModify_log_level->bLog_level);
	
	int nLen = sizeof(Res_Modify_Log_Level);
	nLen = htonl(nLen);
	send(nSock, (char *)&nLen, sizeof(int), 0);
	send(nSock, (char *)&res_modify_log_level, sizeof(res_modify_log_level), 0);

	DEBUG("ProcModifyLogLevel: [version]=%d [command id]=%d [log level]=%d", \
		pModify_log_level->bVersion, ntohs(pModify_log_level->wCmd_id), \
		pModify_log_level->bLog_level);

	DEBUG("ProcModifyLogLevel: send message to maintain client succeed%s", "");
	INFO("ProcModifyLogLevel: func end%s", "");
	return TRUE;
}

static int ProcessMaintainMsg(WORD wCmd_id, int nSock, void *pData)
{	
	INFO("ProcessMaintainMsg: func begin%s", "");
	if (NULL == pData)
	{
		ERROR("ProcessMaintainMsg: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = 0;
	DEBUG("ProcessMaintainMsg: [socket id]=%d [command id]=%d", nSock, wCmd_id);
	switch (wCmd_id)
	{
		case QUERY_SRV_INFO:
			nRet = T_ProcQuerySrvInfoMsg(nSock, pData);
			break;
		case MODIFY_LOG_LEVEL:
			nRet = ProcModifyLogLevel(nSock, pData);
			break;
		case CLEAR_SRV_INFO:
			nRet = ProcClearServerInfoMsg(nSock, pData);
			break;
		default:
			break;
	}

	INFO("ProcessMaintainMsg: func end%s", "");
	return TRUE;
}

static void *RecvMaintainClientMsg(void *pSock)
{
	INFO("RecvMaintainClientMsg: func begin%s", "");
	pthread_detach(pthread_self());
	int nRet = 0;
	WORD wCmd_id = 0;
	int nLen = 0;
	char *pData = NULL;
	int *pClient_sock = (int *)pSock;
	
	while (TRUE)
	{
		nRet = Recv(*pClient_sock, (char *)&nLen, sizeof(int));
		if (-1 == nRet || 0 == nRet)
		{
			ERROR("RecvMaintainClientMsg: Call Recv error [return value]=%d", nRet);
			close(*pClient_sock);
			MM_FREE(pClient_sock);
			pthread_exit(NULL);
			return NULL;
		}
		
		nLen = ntohl(nLen);
		pData = (char *)MM_MALLOC_WITH_DESC(nLen, \
		"RecvMaintainClientMsg: Call func for recv message");
		if (NULL == pData)
		{
			FATAL("RecvMaintainClientMsg: Call MM_MALLOC_WITH_DESC error%s", "");
			close(*pClient_sock);
			MM_FREE(pClient_sock);
			pthread_exit(NULL);
			return NULL;
		}

		nRet = Recv(*pClient_sock, pData, nLen);
		if (0 == nRet || -1 == nRet)
		{
		  ERROR("RecvMaintainClientMsg: Call Recv error [return value]=%d", nRet);
		  close(*pClient_sock);
		  MM_FREE(pClient_sock);
		  pthread_exit(NULL);
		  return NULL;
		}


		wCmd_id = *(WORD *)(pData + 1);
		wCmd_id = ntohs(wCmd_id);

		DEBUG("RecvMaintainClientMsg: [command id]=%d", wCmd_id);

		ProcessMaintainMsg(wCmd_id, *pClient_sock, (void *)pData);
		MM_FREE(pData);	
	}

	INFO("RecvMaintainClientMsg: func end%s", "");
	return NULL;
}


//函数用途:  该函数主要用来让客户端了解服务器当前的运行情况的
//输入参数:  无
//输出参数:  无
//返回值	  : 无

void *RunMaintainThread(void *pParam)
{
	INFO("RunMaintainThread: func begin%s", "");
	pthread_detach(pthread_self());

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	WORD wMaintain_port = g_srv_conf_info.wMaintain_port;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	DEBUG("RunMaintainThread: [maintain thread listen port]=%d", wMaintain_port);

	int nListen_sock = 0;
	int nClient_sock = 0;
	pthread_t thread_id = 0;

	int nRet = 0;
	int *pClient_sock = NULL;

	nRet = InitMaintainSockInfo(&nListen_sock, wMaintain_port);
	if (FALSE == nRet)
	{
		ERROR("RunMaintainThread: Call InitMaintainSockInfo error%s", "");
		pthread_exit(NULL);
		return NULL;
	}

	while (TRUE)
	{	
		nClient_sock = accept(nListen_sock, NULL, NULL);
		if (nClient_sock < 0)
		{
		  ERROR("RunMaintainThread: Call accept error sleep 3s and then accept again%s", "");
		  sleep(3);
		  continue;
		  //pthread_exit(NULL);
		  //return;
		}

		DEBUG("RunMaintainThread: accept client connect request succeed%s", "");

		pClient_sock = (int *)MM_MALLOC_WITH_DESC(sizeof(int), \
		"RunMaintainThread: Call func for client socket");
		if (NULL == pClient_sock)
		{
			FATAL("RunMaintainThread: Call MM_MALLOC_WITH_DESC error%s", "");
			pthread_exit(NULL);
			return NULL;

		}

		*pClient_sock = nClient_sock;
		nRet= pthread_create(&thread_id, NULL, RecvMaintainClientMsg, (void *)pClient_sock);
		//可能创建的线程数量太多, 等线程资源释放了,  再继续执行程序
		if (nRet != 0)
		{
			ERROR("RunMaintainThread: Call pthread_create error, wait 3s and then"
				" accept client connect request again%s", "");
			close(*pClient_sock);
			MM_FREE(pClient_sock);
			sleep(3);
			continue;
		}			
	}

	INFO("RunMaintainThread: func end%s", "");
	return NULL;
}

