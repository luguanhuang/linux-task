
#include "../util/service_global.h"
#include "../include/ydt_log.h"
#include "../interface/service_protocol.h"
#include "service_simplemode_server.h"
#include "service_sock_info.h"
#include "service_maintaininfo_thread.h"
#include "service_simplemode_server.h"
#include "service_communication.h"
#include "service_process_func.h"
#include "service_thread_pool.h"
#include "../util/service_xml.h"
#include "../DB/handle_table/service_handle_table.h"
#include "service_maintain_chargetypes.h"
#include "../thread/service_handle_clientdisconn_thread.h"
#include "../thread/service_maintain_busiarea_thread.h"
#include "../thread/service_translate_data_thread.h"
#include "../thread/service_signal_translate_thread.h"
#include "../thread/service_handle_pushmsg_thread.h"
#include "../thread/service_send_pushmsg_thread.h"
#include "service_threadpoolmode_server.h"


extern Server_Conf_Info g_srv_conf_info;
extern Threads_Info g_threads_info;							//线程ID信息
extern StSave_ThreadRes_Hash g_save_threadres_hash;
extern StThread_Ids_Info g_thread_ids_info;
extern StThread_Ids_Info g_thread_ids_info;
extern StCharge_Types_Info g_charge_types_info;
extern Server_Conf_Info g_srv_conf_info;

#define OPEN_TRANS_THREAD 0x01
#define OPEN_PUSHMSG_THREAD 0x01

//int ProcessMessageFunc(void *pClient_srv_msg, StQueue *pQueue_out)
int ProcessMessageFunc(void *pMsg, StQueue *pQueue_out)
{
	INFO("ProcessMessageFunc: func begin%s", "");
	char *pData = NULL;
	WORD wCmd_id = 0;
	int nRet = 0;
	char *pTmp_msg = (char *)pMsg;
	pTmp_msg++;
	BYTE bMsg_type = *pTmp_msg;

	DEBUG("ProcessMessageFunc: [msg type]=%d", bMsg_type);

	if (CLIENT_SERVER_MSG == bMsg_type)
	{
		Client_Server_Msg *pClient_srv_msg = (Client_Server_Msg *)pMsg;
		pData = pClient_srv_msg->pData;
		pData++;
		wCmd_id = ntohs(*(WORD *)pData);	
		DEBUG("ProcessMessageFunc: [version]=%d", *pClient_srv_msg->pData);
		DEBUG("ProcessMessageFunc: [service command id]=%d [lower byte]=%d [higher byte]=%d", \
			wCmd_id, (wCmd_id & 0x00ff), (wCmd_id & 0xff00) >> 8);

		
		if(wCmd_id >= 0x01 && wCmd_id <= 0x28)
		{
			nRet = YDTProcessFunc(wCmd_id, pClient_srv_msg, &pQueue_out->queue, &pQueue_out->mutex, &pQueue_out->cond);
		}
		else if (wCmd_id >= 0x100 && wCmd_id <= 0x1FF)
		{
			nRet = LTProcessFunc(wCmd_id, pClient_srv_msg, &pQueue_out->queue, &pQueue_out->mutex, &pQueue_out->cond);	
		}
		else if (wCmd_id >= 0x200 && wCmd_id <= 0x2FF)
		{
			#if defined(_WANWEI_PUSH_SERVICE_) || defined(_WANWEI_QUERY_SERVICE_) || defined(_WANWEI_LOGIN_SERVICE_)
					nRet = WanweiProcessFunc(wCmd_id, pClient_srv_msg, &pQueue_out->queue, &pQueue_out->mutex, &pQueue_out->cond);	
			#endif
		}
		else 
		{
			nRet = DefProcessFunc(wCmd_id, pClient_srv_msg, &pQueue_out->queue, &pQueue_out->mutex, &pQueue_out->cond);
		}
	}
	else if (PUSHTO_CLIENT_RESPONSE == bMsg_type)
	{
		//wCmd_id = 0;
		//DEBUG("ProcessMessageFunc: push to client res msg%s", "");
	}

	//根据不同的消息类型调用不同的的处理函数
	
	INFO("ProcessMessageFunc: func end%s", "");
	return nRet;
}


void DelThreadResFromHash(int nSock_id)
{
	char szSock_id[100] = {0};
	snprintf(szSock_id, sizeof(szSock_id) - 1, "%d", nSock_id);

	void *pData = (StThread_Resource *)HashDel(&g_save_threadres_hash.pSave_threadres_hash, szSock_id);
	if (NULL != pData)
	{
		MM_FREE(pData);	
		INFO("DelThreadResFromHash: delete thread resource from hashtable succeed%s", "");
	}
	else
	{
		WARN("DelThreadResFromHash: delete thread resource from hashtable fail%s", "");
	}
}

int InsertThreadResourceToHash(StThread_Resource *pThread_resource, int nSock_id)
{
	INFO("InsertThreadResourceToHash: func begin%s", "");
	if (NULL == pThread_resource)
	{
		ERROR("InsertThreadResourceToHash: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	StHash_Item *pItem = NULL;
	int nRet = 0;
	char szSock[100] = {0};
	snprintf(szSock, sizeof(szSock) - 1, "%d", nSock_id);

	nRet = HashInsert(&g_save_threadres_hash.pSave_threadres_hash, szSock, pThread_resource);
	if (FALSE == nRet)
	{
		ERROR("InsertThreadResourceToHash: insert thread resource to hashtable error [return value]=%d", nRet);
		return FALSE;
	}
	else if (HASH_SAMEKEY_EXIST == nRet)
	{
		pItem = HashGetItem(g_save_threadres_hash.pSave_threadres_hash, szSock);
		if (NULL != pItem)
		{
			pthread_mutex_lock(&g_save_threadres_hash.hash_mutex);
			MM_FREE(pItem->pMatch_msg);
			pItem->pMatch_msg = pThread_resource;
			pthread_mutex_unlock(&g_save_threadres_hash.hash_mutex);
		}

		DEBUG("InsertThreadResourceToHash:  thread resource hash item exist yest in the hashtable then replace"
			" the previous value [socket id]=%d", nSock_id);
	}
	else if (TRUE == nRet)
	{
		DEBUG("InsertThreadResourceToHash: insert thread resource to hashtable succeed"
			" [socket id]=%d", nSock_id);	
	}

	INFO("InsertThreadResourceToHash: func end%s", "");
	return TRUE;
}

int InitMsgQueuesInfo(StQueue *pQueue_in, StQueue *pQueue_out)
{
	INFO("InitMsgQueuesInfo: func begin%s", "");
	if (NULL == pQueue_in || NULL == pQueue_out)
	{
		ERROR("InitMsgQueuesInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	int nMsg_queuein_size = g_srv_conf_info.nMsg_queuein_size;
	int nMsg_queueout_size = g_srv_conf_info.nMsg_queueout_size;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);
	int nRet = 0;
	
	//初始化互斥变量	
	nRet = pthread_mutex_init(&pQueue_in->mutex, NULL);
	if (nRet != 0)
	{
		ERROR("InitMsgQueuesInfo: Call pthread_mutex_init error error[%d]=%s", \
			errno, strerror(errno));
		return FALSE;
	}

		//初始化同步变量
	nRet = pthread_cond_init(&pQueue_in->cond, NULL);
	if (nRet != 0)
	{
		ERROR("InitMsgQueuesInfo: Call pthread_cond_init error error[%d]=%s", \
			errno, strerror(errno));
		return FALSE;
	}	

	//初始化消息队列
	pthread_mutex_lock(&pQueue_in->mutex);
	DEBUG("InitMsgQueuesInfo: [message queuein size]=%d", nMsg_queuein_size);
	//nRet = YDT_InitQueue(&pQueue_in->queue, nMsg_queuein_size, FreeMsgBuffer);
	nRet = InitSeqQueue(&pQueue_in->queue, nMsg_queuein_size, FreeMsgBuffer);
	if (FALSE == nRet)
	{
		ERROR("InitMsgQueuesInfo: Call YDT_InitQueue error%s", "");
		pthread_mutex_unlock(&pQueue_in->mutex);
		return FALSE;
	}

	pthread_mutex_unlock(&pQueue_in->mutex);

	nRet = pthread_mutex_init(&pQueue_out->mutex, NULL);
		//初始化互斥变量	
	if (nRet != 0)
	{
		ERROR("InitMsgQueuesInfo: call pthread_mutex_init error%s", "");
		DestroySeqQueue(&pQueue_in->queue);
		return FALSE;
	}
	
	//初始化同步变量
	nRet = pthread_cond_init(&pQueue_out->cond, NULL);
	if (nRet != 0)
	{
		ERROR("InitMsgQueuesInfo: call pthread_cond_init error%s", "");
		DestroySeqQueue(&pQueue_in->queue);
		return FALSE;
	}	

	//初始化消息队列
	pthread_mutex_lock(&pQueue_out->mutex);
	DEBUG("InitMsgQueuesInfo: [message queueout size]=%d", nMsg_queueout_size);
	nRet = InitSeqQueue(&pQueue_out->queue, nMsg_queueout_size, FreeMsgBuffer);
	if (FALSE == nRet)
	{
		ERROR("InitMsgQueuesInfo: Call YDT_InitQueue error%s", "");
		DestroySeqQueue(&pQueue_in->queue);
		pthread_mutex_unlock(&pQueue_out->mutex);
		return FALSE;
	}

	pthread_mutex_unlock(&pQueue_out->mutex);
	INFO("InitMsgQueuesInfo: func end%s", "");
	return TRUE;
}
void DestroyQueues(int nSock_id, Threads_Id threads_id)
{
	INFO("DestroyQueues: func begin%s", "");
	EnThread_Status enThread_status = THREAD_NOT_EXIST;

	DEBUG("DestroyQueues: [socket id]=%d [send thread id]=%d [recv thread id]=%d", \
		nSock_id, threads_id.snd_id, threads_id.recv_id);
	StThread_Resource *pThread_resource = GetThreadResourceFromHash(nSock_id);
	if (NULL == pThread_resource)
	{
		ERROR("DestroyQueues: we can't find the thread resource in the thread resource hashtable"
			" so we will exit the DestroyQueues function [socket id]=%d", nSock_id);
		return;
	}

	if (0 != threads_id.recv_id)
	{	
		enThread_status = pThread_resource->recv_thread_info.enThread_status;
		while (THREAD_EXIT != enThread_status)
		{
			WARN("DestroyQueues: recv thread is not exit yet so sleep 2s and then judge recv thread exit or not%s", "");
			sleep(2);
			enThread_status = pThread_resource->recv_thread_info.enThread_status;
		}
	}

	if (0 != threads_id.snd_id)
	{
		INFO("DestroyQueues: we will send a signal to awake send thread to exit the thread%s", "");
		pthread_cond_signal(&pThread_resource->stQueue_out.cond);
		enThread_status = pThread_resource->snd_thread_info.enThread_status;
		while (THREAD_EXIT != enThread_status)
		{
			WARN("DestroyQueues: send thread is not exit yet so sleep 2s send a signal and  then judge send thread exit or not%s", "");				
			sleep(2);
			pthread_cond_signal(&pThread_resource->stQueue_out.cond);
			enThread_status = pThread_resource->snd_thread_info.enThread_status;		
		}
	}

	Thread_Pool *pThread_pool = &pThread_resource->thread_pool;
	int nThread_num = pThread_resource->thread_pool.nMin_thread_num;
	int i = 0;
	pthread_mutex_lock(&pThread_pool->thread_pool_mutex);
	for (i=0; i<nThread_num; i++)
	{
		if (0 != pThread_pool->pThread_info[i].thread_id)
		{
			pThread_pool->pThread_info[i].bIs_thread_exit = TRUE;
			pthread_cond_signal(&pThread_resource->stQueue_in.cond);	
		}
		
	}
	
	if (i == nThread_num)
	{
		INFO("DestroyQueues: we set all threads exit flag are true [thread num]=%d", nThread_num);	
	}
	pthread_mutex_unlock(&pThread_pool->thread_pool_mutex);

	INFO("DestroyQueues: we will wait 2s and then check if all threads are exit or not [thread num]=%d", nThread_num);

	BYTE bIs_allthread_exit = FALSE;
	while (TRUE)
	{
		bIs_allthread_exit = TRUE;
		for (i=0; i<nThread_num; i++)
		{
			if (0 != pThread_pool->pThread_info[i].thread_id)
			{
				enThread_status = pThread_pool->pThread_info[i].enThread_status;	
				if (THREAD_EXIT != enThread_status)
				{
					WARN("DestroyQueues: thread is not exit now so we will send a signal to awake it to let it exit[thread id]=%d", \
						pThread_pool->pThread_info[i].thread_id);
					pthread_cond_signal(&pThread_resource->stQueue_in.cond);
					bIs_allthread_exit = FALSE;
					break;
				}	
			}
				
		}
		
		if (FALSE == bIs_allthread_exit)
		{
			WARN("DestroyQueues: not all the threads exit now so we will wait 2s and then check"
				" if all the threads exit now%s", "");	
			sleep(2);
		}
		else
		{
			INFO("DestroyQueues: all the thread eixt yet now%s", "");
			break;	
		}
		
	}

	MM_FREE(pThread_pool->pThread_info);
	
	INFO("DestroyQueues: all threadpool exit yet so we will destroy the queues%s", "");

	DestroySeqQueue(&pThread_resource->stQueue_in.queue);
	DestroySeqQueue(&pThread_resource->stQueue_out.queue);
	
	INFO("DestroyQueues: func end%s", "");
}	

int GetAccessSrvConnInfo(Client_Server_Msg *pClient_server_msg, WORD *pCmd_id, WORD *pAccess_srv_seq)
{
	INFO("GetAccessSrvConnInfo: func begin%s", "");
	if (NULL == pClient_server_msg || NULL == pCmd_id || NULL == pAccess_srv_seq)
	{
		ERROR("GetAccessSrvConnInfo: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	WORD wCmd_id = 0;
	WORD wAccess_srv_seq = 0;

	wAccess_srv_seq = ntohs(pClient_server_msg->wAccess_server_seq);
	char *pData = pClient_server_msg->pData;
	pData++;
	wCmd_id = *(WORD *)pData;
	wCmd_id = ntohs(wCmd_id);

	*pCmd_id = wCmd_id;
	*pAccess_srv_seq = wAccess_srv_seq;

	DEBUG("GetAccessSrvConnInfo: [cmd id]=%d [access server sequence]=%d", \
		*pCmd_id, *pAccess_srv_seq);

	INFO("GetAccessSrvConnInfo: func end%s", "");
	return TRUE;
}

void *RecvAccessSrvMsgThread(void* pThread_param)
{
	INFO("RecvAccessSrvMsgThread: func begin%s", "");
	pthread_detach(pthread_self());
	if (NULL == pThread_param)
	{
		ERROR("RecvAccessSrvMsgThread: func param error%s", "");
		pthread_exit(NULL);
		return NULL;
	}

	int *pSock_id = (int *)pThread_param;
	int nRet = 0;
	Threads_Id threads_id = {0, 0};
	Client_Server_Msg *pClient_server_msg = NULL;
	StThread_Resource *pThread_resource = NULL;
	BYTE bIs_thread_exit = FALSE;
	
	SetCancelThreadFlg();
	pThread_resource = GetThreadResourceFromHash(*pSock_id);
	if (NULL == pThread_resource)
	{
		ERROR("RecvAccessSrvMsgThread: we can't find the thread resource in the thread resource hashtable"
			" so we will exit the RecvAccessSrvMsgThread function [socket id]=%d", *pSock_id);
		close(*pSock_id);
		//释放内存
		MM_FREE(pSock_id);
		pthread_exit(NULL);
		return NULL;
	}
	
	while (TRUE)
	{
		pthread_mutex_lock(&pThread_resource->res_mutex);
		bIs_thread_exit = pThread_resource->recv_thread_info.bIs_thread_exit;
		pthread_mutex_unlock(&pThread_resource->res_mutex);
		if (TRUE == bIs_thread_exit)
		{
			INFO("RecvAccessSrvMsgThread: recv thread will exit later [thread id]=%d", \
				pThread_resource->recv_thread_info.thread_id);
			MM_FREE(pSock_id);
			pThread_resource->recv_thread_info.enThread_status = THREAD_EXIT;
			pthread_exit(NULL);
			return NULL;	
		}
		pClient_server_msg = (Client_Server_Msg *)MM_MALLOC_WITH_DESC(sizeof(Client_Server_Msg), \
		"RecvAccessSrvMsgThread: Call func for client server message");
		if (NULL == pClient_server_msg)
		{
			FATAL("RecvAccessSrvMsgThread: Call MM_MALLOC_WITH_DESC error%s", "");
			
			pthread_mutex_lock(&pThread_resource->res_mutex);
			pThread_resource->snd_thread_info.bIs_thread_exit = TRUE;
			pthread_mutex_unlock(&pThread_resource->res_mutex);
			threads_id.recv_id = 0;
			threads_id.snd_id = pThread_resource->snd_thread_info.thread_id;
			DestroyQueues(*pSock_id, threads_id);
			close(*pSock_id);
			DelThreadResFromHash(*pSock_id);
			//释放内存
			MM_FREE(pSock_id);
			pThread_resource->recv_thread_info.enThread_status = THREAD_EXIT;
			pthread_exit(NULL);
			return NULL;
		}

		memset(pClient_server_msg, 0, sizeof(Client_Server_Msg));
		nRet = RecvAccessSrvMsg(*pSock_id, (void *)pClient_server_msg);
		if (FALSE == nRet)
		{
			ERROR("RecvAccessSrvMsgThread: Call RecvMsg error [return value]=%d", FALSE);
			pthread_mutex_lock(&pThread_resource->res_mutex);
			bIs_thread_exit = pThread_resource->recv_thread_info.bIs_thread_exit;
			pthread_mutex_unlock(&pThread_resource->res_mutex);

			if (TRUE == bIs_thread_exit)
			{
				INFO("RecvAccessSrvMsgThread: recv thread will exit later [thread id]=%d", pThread_resource->recv_thread_info.thread_id);
				MM_FREE(pSock_id);
				MM_FREE(pClient_server_msg);
				pThread_resource->recv_thread_info.enThread_status = THREAD_EXIT;
				pthread_exit(NULL);
				return NULL;	
			}
			
			pthread_mutex_lock(&pThread_resource->res_mutex);
			pthread_cond_signal(&pThread_resource->stQueue_out.cond);
			pThread_resource->snd_thread_info.bIs_thread_exit = TRUE;
			pthread_mutex_unlock(&pThread_resource->res_mutex);
			threads_id.recv_id = 0;
			threads_id.snd_id = pThread_resource->snd_thread_info.thread_id;
			DestroyQueues(*pSock_id, threads_id);
			close(*pSock_id);
			DelThreadResFromHash(*pSock_id);
			//释放内存
			MM_FREE(pSock_id);
			MM_FREE(pClient_server_msg);
			pThread_resource->recv_thread_info.enThread_status = THREAD_EXIT;
			pthread_exit(NULL);
			//退出线程
			return NULL;
		}

		PutMsgToSeqQueue(&pThread_resource->stQueue_in.queue, (void *)pClient_server_msg);
		pthread_cond_signal(&pThread_resource->stQueue_in.cond);
		
	}
	
	INFO("RecvAccessSrvMsgThread: func end%s", "");
	return NULL;
}

void *SndMsgToAccessSrvThread(void* pThread_param)
{
	INFO("SndMsgToAccessSrvThread: func begin%s", "");
	pthread_detach(pthread_self());	
	if (NULL == pThread_param)
	{
		ERROR("SndMsgToAccessSrvThread: func param error%s", "");
		pthread_exit(NULL);
		return NULL;
	}
		
	int *pSock_id = (int *)pThread_param;
	Server_Client_Msg *pServer_client_msg = NULL;
	int nRet = 0;
	WORD wCmd_id = 0;
	char *pData = NULL;
	StThread_Resource *pThread_resource = NULL;
	StQueue *pQueue_out = NULL;
	pthread_t current_thread_id = 0;
	Threads_Id threads_id = {0, 0};
	BYTE bIs_thread_exit = FALSE;
	
	SetCancelThreadFlg();
	pThread_resource = GetThreadResourceFromHash(*pSock_id);
	if (NULL == pThread_resource)
	{
		ERROR("SndMsgToAccessSrvThread: we can't find the thread resource in the thread resource hashtable"
			" so we will exit the SndMsgToAccessSrvThread function [socket id]=%d", *pSock_id);
		close(*pSock_id);											//关闭socket
		//释放内存
		MM_FREE(pSock_id);
		pthread_exit(NULL);
		return NULL;
	}

	pQueue_out = &(pThread_resource->stQueue_out);
	current_thread_id = pthread_self(); 

	for(;;)
	{
		pthread_mutex_lock(&pQueue_out->mutex);
		//判断消息队列是否为空
		nRet = IsSeqQueueEmpty(&pQueue_out->queue);
		while (TRUE == nRet)
		{
			INFO("SndMsgToAccessSrvThread: message queue is empty, please wait to get message from queue%s", "");
			pthread_cond_wait(&pQueue_out->cond, &pQueue_out->mutex);
			bIs_thread_exit = pThread_resource->snd_thread_info.bIs_thread_exit;
			if (TRUE == bIs_thread_exit)
			{
				INFO("SndMsgToAccessSrvThread: send thread will exit later [thread_id]=%u", \
					(unsigned int)pThread_resource->snd_thread_info.thread_id);		
				pThread_resource->snd_thread_info.enThread_status = THREAD_EXIT;
				MM_FREE(pSock_id);
				pthread_mutex_unlock(&pQueue_out->mutex);
				pthread_exit(NULL);
				return NULL;
			}
			nRet = IsSeqQueueEmpty(&pQueue_out->queue);
		}
		
		//从消息队列中取出消息
		nRet = GetMsgFromSeqQueue(&pQueue_out->queue, (void **)&pServer_client_msg);
		pthread_mutex_unlock(&pQueue_out->mutex);

		//发送消息

		#if 0
		#ifdef _WANWEI_PUSH_SERVICE_
		nRet = SndPushServiceMsg((void *)pServer_client_msg);
		#else
		nRet = SendMsg(*pSock_id, (void *)pServer_client_msg);
		#endif
		#endif
		#ifdef MORE_SEND_THREAD
		pthread_mutex_lock(&pThread_resource->sock_mutex);
		#endif
		nRet = SendMsg(*pSock_id, (void *)pServer_client_msg);
		#ifdef MORE_SEND_THREAD
		pthread_mutex_unlock(&pThread_resource->sock_mutex);
		#endif
		if (FALSE == nRet)
		{
			INFO("SndMsgToAccessSrvThread: Call SendMsg error [return value]=%d", FALSE);
			//加上这句话的目的是为了避免发送和接收线程同时往下执行
			//避免同时销毁线程资源
			pthread_mutex_lock(&pThread_resource->res_mutex);
			bIs_thread_exit = pThread_resource->snd_thread_info.bIs_thread_exit;
			pthread_mutex_unlock(&pThread_resource->res_mutex);
			if (TRUE == bIs_thread_exit)
			{
				INFO("SndMsgToAccessSrvThread: send thread will exit later [thread_id]=%d", \
					pThread_resource->snd_thread_info.thread_id);		
				MM_FREE(pSock_id);
				pThread_resource->snd_thread_info.enThread_status = THREAD_EXIT;
				pthread_exit(NULL);
				return NULL;
			}

			pthread_mutex_lock(&pThread_resource->res_mutex);
			pThread_resource->recv_thread_info.bIs_thread_exit = TRUE;
			pthread_mutex_unlock(&pThread_resource->res_mutex);
			
			threads_id.snd_id = 0;
			threads_id.recv_id = pThread_resource->recv_thread_info.thread_id;
			DestroyQueues(*pSock_id, threads_id);					
			close(*pSock_id);											//关闭socket
			DelThreadResFromHash(*pSock_id);
			
			//释放内存
			MM_FREE(pSock_id);
			MM_FREE(pServer_client_msg->pData);
			MM_FREE(pServer_client_msg);
			pThread_resource->snd_thread_info.enThread_status = THREAD_EXIT;
			pthread_exit(NULL);											//退出线程
			return NULL;
		}

		pData = pServer_client_msg->pData;
		wCmd_id = *(WORD *)(pData + 1);
		wCmd_id = ntohs(wCmd_id);

		char arrAcc_checkcode[5] = {0};
		memcpy(arrAcc_checkcode, pServer_client_msg->arrCheck_code, 4);
		
		DEBUG("SndMsgToAccessSrvThread: [version]=%d [msg type]=%d [main service code]=%d [sub service code]=%d"
		" [access server seq]=%d [msg counter]=%d [access check code]=%s [data len]=%d [cmd_id]=%d", pServer_client_msg->bVersion, pServer_client_msg->bMsg_type, \
		pServer_client_msg->bMain_service_code, pServer_client_msg->bSub_service_code, ntohs(pServer_client_msg->wAccess_server_seq), \
		ntohl(pServer_client_msg->nMsg_counter), arrAcc_checkcode, ntohl(pServer_client_msg->nData_len), wCmd_id);	
		INFO("SendFunc: send msg to access server succeed%s", "");
		
		MM_FREE(pServer_client_msg->pData);
		MM_FREE(pServer_client_msg);
	}

	INFO("SndMsgToAccessSrvThread: func end%s", "");
	return NULL;
}

int CreateSndRecvThread(int nSock_id, pthread_t *pRecv_thread_id, pthread_t *pSnd_thread_id)
{
	INFO("CreateSndRecvThread: func begin%s", "");

	if (NULL == pRecv_thread_id || NULL == pSnd_thread_id)
	{
		ERROR("CreateSndRecvThread: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int *pSnd_Sockid = NULL;
	int *pRecv_sockid = NULL;
	
	int nRet = 0;
	pthread_t recv_thread_id = 0;
	pthread_t snd_thread_id = 0;
	
	pRecv_sockid = (int *)MM_MALLOC_WITH_DESC(sizeof(int), \
	"CreateSndRecvThread: Call func for recv socket id");
	if (NULL == pRecv_sockid)
	{
		FATAL("CreateSndRecvThread: Call malloc error%s", "");
		return FALSE;
	}

	*pRecv_sockid = nSock_id;
	nRet = pthread_create(&recv_thread_id, NULL, RecvAccessSrvMsgThread, (void *)pRecv_sockid);	
	if(nRet != 0)
	{
		ERROR("CreateSndRecvThread: Call pthread_create error error[%d]=%s", errno, strerror(errno));
		MM_FREE(pRecv_sockid);
		return FALSE;
	}

	pSnd_Sockid = (int *)MM_MALLOC_WITH_DESC(sizeof(int), \
	"CreateSndRecvThread: Call func for send socket id");
	if (NULL == pSnd_Sockid)
	{
		FATAL("CreateSndRecvThread: Call MM_MALLOC_WITH_DESC error%s", "");
		MM_FREE(pRecv_sockid);
		nRet = pthread_cancel(recv_thread_id);
		if (0 != nRet)
		{
			ERROR("CreateSndRecvThread: terminate recv thread error error[%d]=%s", errno, strerror(errno));	
		}
		else
		{
			INFO("CreateSndRecvThread: terminate recv thread succeed%s", "");	
		}
		return FALSE;
	}

	*pSnd_Sockid = nSock_id;
	
	nRet = pthread_create(&snd_thread_id, NULL, SndMsgToAccessSrvThread, (void *)pSnd_Sockid);	
	if(nRet != 0)
	{
		ERROR("CreateSndRecvThread: Call pthread_create error%s", "");
		MM_FREE(pRecv_sockid);
		MM_FREE(pSnd_Sockid);
		nRet = pthread_cancel(recv_thread_id);
		if (0 != nRet)
		{
			ERROR("CreateSndRecvThread: terminate recv thread error error[%d]=%s", errno, strerror(errno));	
		}
		else
		{
			INFO("CreateSndRecvThread: terminate recv thread succeed%s", "");	
		}
		return FALSE;
	}

	*pRecv_thread_id = recv_thread_id;
	*pSnd_thread_id = snd_thread_id;
	INFO("CreateSndRecvThread: func end%s", "");
	return TRUE;
}

int RunServerFunc(void)
{
	INFO("RunServerFunc: func begin%s", "");

	#ifdef _WANWEI_PUSH_SERVICE_
	pthread_t thread_id = 0;
	int nRet = pthread_create(&thread_id, NULL, (void *)SndPushMsgThread, NULL);
	if (0 != nRet)
	{
		ERROR("RunServerFunc: Call SndPushMsgThread thread error error[%d]=%s", errno, strerror(errno));
		return FALSE;
	}
	#endif

	#ifdef _WANWEI_QUERY_SERVICE_
	int nRet = TRUE;
	pthread_t thread_id = 0;
	nRet= pthread_create(&thread_id, NULL, (void *)MaintainBusiAreaThread, NULL);
	if (nRet != 0)
	{
		ERROR("RunServerFunc: Call pthread_create error error[%d]=%s", errno, strerror(errno));
		return FALSE;
	}
	#endif

	#ifdef _WANWEI_LOGIN_SERVICE_ 
	int nRet = TRUE;
	pthread_t thread_id = 0;
	nRet= pthread_create(&thread_id, NULL, (void *)HandleDisconnClientThread, NULL);
	if (nRet != 0)
	{
		ERROR("RunServerFunc: Call pthread_create error error[%d]=%s", errno, strerror(errno));
		return FALSE;
	}
	#endif

	#ifdef _LIANTONG_SERVICE_
	#if 1
	int nRet = TRUE;
	pthread_t thread_id = 0;
	BOOL bOpen_transthread_flg = g_srv_conf_info.bOpen_transthread_flg;
	BOOL bOpen_pushthread_flg = g_srv_conf_info.bOpen_pushthread_flg;

	#if 0
	nRet= pthread_create(&thread_id, NULL, (void *)MaintainChargeTypesThread, NULL);
	if (nRet != 0)
	{
		ERROR("RunServerFunc: Call pthread_create error error[%d]=%s", errno, strerror(errno));
		return FALSE;
	}
	#endif
	
	//sleep(3);
	nRet = H_GetRegisterStartIDInfo();
	if (TRUE != nRet)
	{
		ERROR("RunServerFunc: Call H_GetRegisterStartIDInfo error%s", "");
	}
	#endif

	#if 0
	if (OPEN_TRANS_THREAD == bOpen_transthread_flg)
	{
		nRet= pthread_create(&thread_id, NULL, (void *)TranslateDataThread, NULL);
		if (nRet != 0)
		{
			ERROR("RunServerFunc: Call TranslateDataThread thread error error[%d]=%s", errno, strerror(errno));
			return FALSE;
		}
		
		nRet= pthread_create(&thread_id, NULL, (void *)TransNewRegisUsrDataThread, NULL);
		if (nRet != 0)
		{
			ERROR("RunServerFunc: Call TransNewRegisUsrDataThread thread error error[%d]=%s", errno, strerror(errno));
			return FALSE;
		}			
	}

	if (OPEN_PUSHMSG_THREAD == bOpen_pushthread_flg)
	{
		nRet= pthread_create(&thread_id, NULL, (void *)HeartBeatDetectThread, NULL);
		if (nRet != 0)
		{
			ERROR("RunServerFunc: Call pthread_create error error[%d]=%s", errno, strerror(errno));
			return FALSE;
		}

		nRet = pthread_create(&thread_id, NULL, (void *)SndPushMsgThread, NULL);
		if (0 != nRet)
		{
			ERROR("RunServerFunc: Call SndPushMsgThread thread error error[%d]=%s", errno, strerror(errno));
			return FALSE;
		}

		nRet = pthread_create(&thread_id, NULL, (void *)HandlePushMsgThread, NULL);
		if (0 != nRet)
		{
			ERROR("RunServerFunc: Call SndPushMsgThread thread error error[%d]=%s", errno, strerror(errno));
			return FALSE;
		}
	}
	#endif
	
	#if 0
	nRet = pthread_create(&thread_id, NULL, (void *)SignalTranslateThread, NULL);
	if (0 != nRet)
	{
		ERROR("RunServerFunc: Call SignalTranslateThread thread error error[%d]=%s", \
			errno, strerror(errno));
		return FALSE;
	}
	#endif
	#endif
	
	INFO("RunServerFunc: func end%s", "");
	return TRUE;
}

int RunThreadPoolModeServer(void)
{
	INFO("RunThreadPoolModeServer: func begin%s", "");
	int nListen_sock = 0;
	int nConn_sock = 0;
	pthread_t thread_id = 0;
	int nRet = 0;
	StThread_Resource *pThread_resource = NULL;
	Threads_Id threads_id = {0, 0};
	pthread_t recv_thread_id = 0;
	pthread_t snd_thread_id = 0;
	char szIP[50] = {0};
	WORD wPort = 0;

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	wPort = g_srv_conf_info.wPort;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);
		
	nRet = InitListenSockInfo(&nListen_sock, wPort);
	if (FALSE == nRet)
	{
		ERROR("RunThreadPoolModeServer: Call InitListenSockInfo error%s", "");
		return FALSE;	
	}

	INFO("RunThreadPoolModeServer: service server listening......%s", "");

	//创建运行维护线程
	nRet= pthread_create(&thread_id, NULL, RunMaintainThread, NULL);
	if (nRet != 0)
	{
		ERROR("RunThreadPoolModeServer: Call pthread_create error error[%d]=%s", errno, strerror(errno));
		return FALSE;
	}

	nRet = RunServerFunc();
	if (TRUE != nRet)
	{
		ERROR("RunThreadPoolModeServer: Call RunServerFunc error%s", "");
		return nRet;
	}
	
	while (TRUE)
	{
		//接收
		memset(szIP, 0, sizeof(szIP));
		nRet = T_Accept(nListen_sock, &nConn_sock, szIP, &wPort);
		if (FALSE == nRet)
		{	
			ERROR("RunThreadPoolModeServer: Call Accept error sleep 1s and then accept client connection request again%s", "");
			sleep(1);
			continue;  
		}

		DEBUG("RunThreadPoolModeServer: accept client connection request succeed [socket id]=%d", nConn_sock);	

		pThread_resource = (StThread_Resource *)MM_MALLOC_WITH_DESC(sizeof(StThread_Resource), \
		"RunThreadPoolModeServer: Call func for thread resource");
		if (NULL == pThread_resource)
		{
			ERROR("RunThreadPoolModeServer: Call MM_MALLOC_WITH_DESC error%s", "");
			close(nConn_sock);
			continue;
		}

		memset(pThread_resource, 0, sizeof(StThread_Resource));
		pThread_resource->nSock_id = nConn_sock;
		memcpy(pThread_resource->arrAcccee_srv_ip, szIP, sizeof(pThread_resource->arrAcccee_srv_ip) - 1);
		pThread_resource->wAccess_srv_port = wPort;
		
		if (0 != pthread_mutex_init(&pThread_resource->res_mutex, NULL))
		{
			ERROR("RunThreadPoolModeServer: Call pthread_mutex_init error error[%d]=%s", errno, strerror(errno));
			return FALSE;
		}

		if (0 != pthread_mutex_init(&pThread_resource->sock_mutex, NULL))
		{
			ERROR("RunThreadPoolModeServer: Call pthread_mutex_init error error[%d]=%s", errno, strerror(errno));
			return FALSE;
		}
		
		nRet = InitMsgQueuesInfo(&pThread_resource->stQueue_in, &pThread_resource->stQueue_out);
		if (TRUE != nRet)
		{
			ERROR("RunThreadPoolModeServer: Call InitMsgQueuesInfo error%s", "");
			MM_FREE(pThread_resource);
			close(nConn_sock);
			continue;
		}

		nRet = InsertThreadResourceToHash(pThread_resource, nConn_sock);
		if (TRUE != nRet)
		{
			ERROR("RunThreadPoolModeServer: Call InsertThreadResourceToHash error%s", "");
			DestroySeqQueue(&pThread_resource->stQueue_in.queue);
			DestroySeqQueue(&pThread_resource->stQueue_out.queue);
			MM_FREE(pThread_resource);
			close(nConn_sock);
			continue;
		}

		nRet = CreateSndRecvThread(nConn_sock, &recv_thread_id, &snd_thread_id);
		if (TRUE != nRet)
		{
			ERROR("Call CreateSndRecvThread error%s", "");
			DestroyQueues(nConn_sock, threads_id);
			//注: 这里不条用FREEIF(pThread_resource);的原因是
			//DelThreadResFromHash函数内部已经把线程资源释放掉了
			DelThreadResFromHash(nConn_sock);
			close(nConn_sock);
			continue;
		}

		nRet = CreateThreadPool(ProcessMessageFunc, nConn_sock);
		if (TRUE != nRet)
		{
			ERROR("RunThreadPoolModeServer: Call CreateThreadPool error [return value]=%d", nRet);	
			DestroyQueues(nConn_sock, threads_id);
			//注: 这里不条用FREEIF(pThread_resource);的原因是
			//DelThreadResFromHash函数内部已经把线程资源释放掉了
			DelThreadResFromHash(nConn_sock);
			close(nConn_sock);
			continue;
		}

		pThread_resource->recv_thread_info.thread_id = recv_thread_id;
		pThread_resource->recv_thread_info.bIs_thread_exit = FALSE;
		pThread_resource->recv_thread_info.enThread_status = THREAD_NOT_EXIT;
		
		pThread_resource->snd_thread_info.thread_id = snd_thread_id;
		pThread_resource->snd_thread_info.bIs_thread_exit = FALSE;
		pThread_resource->snd_thread_info.enThread_status = THREAD_NOT_EXIT;
	}
	
	INFO("RunThreadPoolModeServer: func end%s", "");
	return TRUE;
}

