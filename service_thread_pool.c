
#include "../util/service_global.h"
#include "../include/ydt_log.h"
#include "../interface/service_protocol.h"
#include "service_thread_pool.h"

#define CLIENT_SOCK_EXIST 0x02

/*该文件主要是用来创建线程池的
  *在程序启动的时候就创建好规定的线程
  *该线程池主要是用来接收客户端消息的
  *当线程池创建好后, 各个线程就从存放
  *客户端消息包的消息队列里面取出sockid
  *, 根据得到的sockid来接收客户端的消息
  *
  *
  */

extern Server_Conf_Info g_srv_conf_info;
extern StSave_ThreadRes_Hash g_save_threadres_hash;

StThread_Resource *GetThreadResourceFromHash(int nSock_id)
{
	INFO("GetThreadResourceFromHash: func begin%s", "");
	char szSock_id[100] = {0};
	snprintf(szSock_id, sizeof(szSock_id) - 1, "%d", nSock_id);
	DEBUG("GetThreadResourceFromHash: [socket id]=%d", nSock_id);

	StThread_Resource *pThread_resource = (StThread_Resource *)HashData(g_save_threadres_hash.pSave_threadres_hash, szSock_id);
	INFO("GetThreadResourceFromHash: func end%s", "");
	return pThread_resource;
}

//函数用途: 根据线程ID获取线程序号
//输入参数: 线程ID
//输出参数: 无
//返回值: 线程序号

static int GetThreadSeqById(int nId, Thread_Pool *pThread_pool)
{
	INFO("GetThreadSeqById: func begin%s", "");
	pthread_mutex_lock(&(pThread_pool->thread_pool_mutex));
	int i = 0;
	for (i=0; i<pThread_pool->nCur_thread_num; i++)
	{
		if (nId == pThread_pool->pThread_info[i].thread_id)			//比较线程ID
		{
			pthread_mutex_unlock(&(pThread_pool->thread_pool_mutex));
			DEBUG("GetThreadSeqById: [thread id]=%u [thread sequence]=%d", (unsigned int)nId, i);
			INFO("GetThreadSeqById: func end%s", "");
			return i;	
		}
	}

	pthread_mutex_unlock(&(pThread_pool->thread_pool_mutex));

	WARN("GetThreadSeqById: we can't get the thread seq by thread id [thread id]=%u", (unsigned int)nId);
	INFO("GetThreadSeqById: func end%s", "");
	return -1;
}

//函数用途: 工作线程函数, 负责从消息队列里面获取socket, 并根据socket id来接收客户端的消息包
//输入参数: 无
//输出参数: 无
//返回值:  无

static void *WorkThread(void *pParam)
{
	INFO("WorkThread: func begin%s", "");

	int nRet = 0;
	pthread_t thread_id = pthread_self();
	//创建分离线程
	pthread_detach(thread_id);
	SetCancelThreadFlg();
	int *pSock_id = (int *)pParam;
	void *pMsg = NULL;
	char *pTmp_msg = NULL;
	BYTE bMsg_type = 0;
	
	BYTE bIs_thread_exit = FALSE;

	StThread_Resource *pThread_resource = GetThreadResourceFromHash(*pSock_id);
	if (NULL == pThread_resource)
	{
		WARN("WorkThread: we can't find the thread resource in the thread resource hashtable"
			" so we will exit the WorkThread function [socket id]=%d", *pSock_id);
		MM_FREE(pSock_id);
		pthread_exit(NULL);
		return NULL;
	}

	Thread_Pool *pThread_pool = &(pThread_resource->thread_pool); 
	int nSeq = pThread_pool->pGetThreadSeqById(thread_id, pThread_pool);
	if (nSeq < 0)
	{
		ERROR("WorkThread: we can't find the thread seq by thread id and then exit the thread [return value]=%d", nSeq);
		MM_FREE(pSock_id);		
		return NULL;
	}

	DEBUG("WorkThread: current thread [socket id]=%d [thread id]=%u", *pSock_id, (unsigned int)thread_id);

	while (TRUE)
	{
		pthread_mutex_lock(&pThread_resource->stQueue_in.mutex);
		nRet = IsSeqQueueEmpty(&pThread_resource->stQueue_in.queue);
		while (TRUE == nRet)
		{
			INFO("WorkThread: current message queuein is empty so we will wait a signal to get message"
				" from message queuein%s", "");
			pthread_cond_wait(&pThread_resource->stQueue_in.cond, &pThread_resource->stQueue_in.mutex);

			INFO("WorkThread: we recv a signal to get item from message queuein%s", "");
			bIs_thread_exit = pThread_pool->pThread_info[nSeq].bIs_thread_exit;
			if (TRUE == bIs_thread_exit)
			{
				INFO("WorkThread: current thread will exit later [thread_id]=%u", (unsigned int)thread_id);
				pThread_pool->pThread_info[nSeq].enThread_status = THREAD_EXIT;
				pthread_mutex_unlock(&pThread_resource->stQueue_in.mutex);
				MM_FREE(pSock_id);
				pthread_exit(NULL);
				return NULL;
			}
			nRet = IsSeqQueueEmpty(&pThread_resource->stQueue_in.queue);	
		}

		GetMsgFromSeqQueue(&pThread_resource->stQueue_in.queue, (void **)&pMsg);
		pthread_mutex_unlock(&pThread_resource->stQueue_in.mutex);

		nRet = pThread_pool->pThread_info[nSeq].pProcessFunc(pMsg, &pThread_resource->stQueue_out);
		if (TRUE != nRet)
		{
			ERROR("WorkThread: Handle client message fail%s", "");
		}

		pTmp_msg = (char *)pMsg;
		pTmp_msg++;
		bMsg_type = *pTmp_msg;
		if (CLIENT_SERVER_MSG == bMsg_type)
		{
			//MM_FREE(pClient_srv_msg->pData);
			//MM_FREE(pClient_srv_msg);		
			MM_FREE(((Client_Server_Msg *)pMsg)->pData);
			MM_FREE(pMsg);
		}
		else if (PUSHTO_CLIENT_RESPONSE == bMsg_type)
		{
			MM_FREE(((PushToClient_Response_Msg *)pMsg)->pData);
			MM_FREE(pMsg);				
		}
	}
	

	INFO("WorkThread: func end%s", "");
	return NULL;
}


//函数用途: 初始化线程池函数(初始化线程的同步和互斥变量,   并且创建相应的线程)
//输入参数: 无
//输出参数: 无
//返回值: 如果初始化成功, 返回TRUE,  如果初始化失败, 返回FALSE

int InitThreadPool(int nSock_id, Thread_Pool *pThread_pool)
{
	INFO("InitThreadPool: func begin%s", "");
	int nMin_thread_num = pThread_pool->nMin_thread_num;

	DEBUG("InitThreadPool: [initial thread num]=%d [socket id]=%d", nMin_thread_num, nSock_id);
	
	int i = 0;
	int nRet = 0;
	int *pSock = NULL;
	int nStack_size = 1024 * 1024 * 2;
	pthread_attr_t tattr;
	int nThread_counter = 0;

	nRet = pthread_attr_init(&tattr);
	if (0 != nRet)
	{
		ERROR("InitThreadPool: Call pthread_attr_init error error[%d]=%s", \
			errno, strerror(errno));	
	}
	
	nRet = pthread_attr_setstacksize(&tattr, nStack_size);
	if (0 != nRet)
	{
		ERROR("InitThreadPool: Call pthread_attr_setstacksize error error[%d]=%s", \
			errno, strerror(errno));		
	}

	for (i=0; i<nMin_thread_num; i++)
	{
		nRet = pthread_cond_init(&(pThread_pool->pThread_info[i].thread_cond), NULL);
		if (nRet != 0)
		{
			ERROR("InitThreadPool: Call pthread_cond_init error error[%d]=%s", \
				errno, strerror(errno));
			return FALSE;
		}

		//初始化线程互斥变量
		nRet = pthread_mutex_init(&(pThread_pool->pThread_info[i].thread_mutex), NULL);
		if (nRet != 0)
		{
			ERROR("InitThreadPool: Call pthread_mutex_init error error[%d]=%s", \
				errno, strerror(errno));	
			return FALSE;
		}

		pSock = (int *)MM_MALLOC_WITH_DESC(sizeof(int), \
		"InitThreadPool: Call func for socket");
		if (NULL == pSock)
		{
			FATAL("InitThreadPool: Call MM_MALLOC_WITH_DESC error%s", "");
			return FALSE;
		}
		
		*pSock = nSock_id;
		
		//创建工作线程
		nRet = pthread_create(&(pThread_pool->pThread_info[i].thread_id), &tattr, WorkThread, (void *)pSock);
		if (nRet != 0)
		{
			pThread_pool->pThread_info[i].thread_id = 0;	
			ERROR("InitThreadPool: Call pthread_create error error[%d]=%s", \
				errno, strerror(errno));
			MM_FREE(pSock);
			return FALSE;
		}

		nThread_counter++;
	}

	INFO("InitThreadPool: func end%s", "");
	return TRUE;
}

//函数用途: 线程函数的默认处理函数
//输入参数: socket id
//输出参数: 无
//返回值: 整形(目前还没用到返回值)


static int ProcessFunc(void *pClient_srv_msg, StQueue *pQueue)
{
	return TRUE;
}


//函数用途: 创建线程池
//输入参数: 线程函数的默认处理函数
//输出参数: 无
//返回值: 如果创建线程池成功,  返回TRUE,  如果创建线程池失败, 返回FALSE

int CreateThreadPool(int (*pProcessFunc)(void *, StQueue *), int nSock_id)
{
	INFO("CreateThreadPool: func begin%s", "");

	char szSock_id[100] = {0};
	snprintf(szSock_id, sizeof(szSock_id) - 1, "%d", nSock_id);

	DEBUG("CreateThreadPool: [socket id]=%d", nSock_id);

	
	StThread_Resource *pThread_resource = GetThreadResourceFromHash(nSock_id);
	if (NULL == pThread_resource)
	{
		ERROR("CreateThreadPool: we can't find the thread resource in the thread resource hashtable"
			" so we will exit the CreateThreadPool function [socket id]=%d", nSock_id);
		return FALSE;
	}

	Thread_Pool *pThread_pool = &(pThread_resource->thread_pool); 
	int nRet = 0;
	int i = 0;

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	int nMin_thread_num = g_srv_conf_info.nMin_thread_num;
	int nMax_thread_num = g_srv_conf_info.nMax_thread_num;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

		//初始化线程池互斥变量
	nRet = pthread_mutex_init(&(pThread_pool->thread_pool_mutex), NULL);	
	if (nRet != 0)
	{
		ERROR("CreateThreadPool: Call pthread_mutex_init error error[%d]=%s", \
			errno, strerror(errno));	
		return FALSE;
	}

	//把线程池的函数变量与各个函数关联起来

	pthread_mutex_lock(&pThread_pool->thread_pool_mutex);
	pThread_pool->pInitThreadPool = InitThreadPool;	
	pThread_pool->pGetThreadSeqById = GetThreadSeqById;
	
	pThread_pool->nMin_thread_num = nMin_thread_num;		//最少线程
	pThread_pool->nCur_thread_num = nMin_thread_num;		//当前线程
	pThread_pool->nMax_thread_num = nMax_thread_num;//最大线程(最大线程的作用是为了以后能后动态扩充线程的数量)

	DEBUG("CreateThreadPool: [min thread num]=%d [current thread num]=%d [max thread num]=%d", \
		pThread_pool->nMin_thread_num, pThread_pool->nCur_thread_num, pThread_pool->nMax_thread_num);

	MM_FREE(pThread_pool->pThread_info);

	pThread_pool->pThread_info = (Thread_Info *)MM_MALLOC_WITH_DESC(nMax_thread_num * sizeof(Thread_Info), \
	"CreateThreadPool: Call func for thread info");
	if (NULL == pThread_pool->pThread_info)
	{
		FATAL("CreateThreadPool: Call MM_MALLOC_WITH_DESC error%s", "");
		pthread_mutex_unlock(&pThread_pool->thread_pool_mutex);
		return FALSE;;
	}
	memset(pThread_pool->pThread_info, 0, nMax_thread_num * sizeof(Thread_Info));

	//初始化线程信息
	if (NULL == pProcessFunc)
	{
		INFO("CreateThreadPool: we will use the system default thread func%s", "");
		for (i=0; i<nMax_thread_num; i++)
		{	
			pThread_pool->pThread_info[i].en_thread_usr = THREAD_NOT_USE;			//把线程指定为未使用状态
			pThread_pool->pThread_info[i].pProcessFunc = ProcessFunc;				//使用系统默认的执行函数
			pThread_pool->pThread_info[i].bIs_thread_exit = FALSE;
			pThread_pool->pThread_info[i].enThread_status = THREAD_NOT_EXIT;
			pThread_pool->pThread_info[i].thread_id = 0;
		}		
	}
	else
	{
		INFO("CreateThreadPool: we will use the self define thread func%s", "");
		for (i=0; i<nMax_thread_num; i++)
		{
			pThread_pool->pThread_info[i].en_thread_usr = THREAD_NOT_USE;				//把线程指定为未使用状态
			pThread_pool->pThread_info[i].pProcessFunc = pProcessFunc;				//使用用户指定的函数
			pThread_pool->pThread_info[i].bIs_thread_exit = FALSE;
			pThread_pool->pThread_info[i].enThread_status = THREAD_NOT_EXIT;
			pThread_pool->pThread_info[i].thread_id = 0;
		}	
	}

	//初始化线程池
	nRet = InitThreadPool(nSock_id, pThread_pool);
	if (FALSE == nRet)
	{
		ERROR("CreateThreadPool: Call InitThreadPool error%s", "");
		MM_FREE(pThread_pool->pThread_info);
		pthread_mutex_unlock(&pThread_pool->thread_pool_mutex);
		return FALSE;
	}

	pthread_mutex_unlock(&pThread_pool->thread_pool_mutex);
	INFO("CreateThreadPool: func end%s", "");	
	return TRUE;
}

