
#include "../util/access_global.h"
#include "../interface/access_protocol.h"
#include "access_comm_client_rcv.h"
#include "access_thread_pool.h"
#include "../include/link_queue.h"


extern StClient_Sock_Queue g_clientsocket_queue;

extern StTmp_clientSock_Hash g_tmp_clientsock_hash;

#define RECV_CLIENT_MESSAGE_ERROR -1

/*���ļ���Ҫ�����������̳߳ص�
  *�ڳ���������ʱ��ʹ����ù涨���߳�
  *���̳߳���Ҫ���������տͻ�����Ϣ��
  *���̳߳ش����ú�, �����߳̾ʹӴ��
  *�ͻ�����Ϣ������Ϣ��������ȡ��sockid
  *, ���ݵõ���sockid�����տͻ��˵���Ϣ
  *
  *
  */

extern Server_Conf_Info g_srv_conf_info;
int ReleaseThread(void)
{
	return TRUE;
}


//������;: �����߳�ID��ȡ�߳����
//�������: �߳�ID
//�������: ��
//����ֵ: �߳����

static int GetThreadSeqById(int nId, Thread_Pool *pThread_pool)
{
	INFO("GetThreadSeqById: func begin%s", "");
	pthread_mutex_lock(&(pThread_pool->thread_pool_mutex));

	int i = 0;
	for (i=0; i<pThread_pool->nCur_thread_num; i++)
	{
		if (nId == pThread_pool->pThread_info[i].thread_id)			//�Ƚ��߳�ID
		{
			pthread_mutex_unlock(&(pThread_pool->thread_pool_mutex));
			INFO("GetThreadSeqById: [thread id]=%u [thread sequence]=%d", (unsigned int)nId, i);
			INFO("GetThreadSeqById: func end%s", "");
			return i;	
		}
	}

	pthread_mutex_unlock(&(pThread_pool->thread_pool_mutex));

	INFO("GetThreadSeqById: we can't get the thread seq by thread id [thread id]=%u", (unsigned int)nId);
	INFO("GetThreadSeqById: func end%s", "");
	return -1;
}

int HandleClientSock(int nSock, StTmp_clientSock_Hash *pTmp_clientsock_hash)
{
	INFO("HandleClientSock: func begin%s", "");
	if (NULL == pTmp_clientsock_hash)
	{
		ERROR("HandleClientSock: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	char szSock[100] = {0};
	StHash_Item *pItem = NULL;
	int nRet = 0;
	snprintf(szSock, sizeof(szSock) - 1, "%d", nSock);

	pItem = HashGetItem(pTmp_clientsock_hash->pTmp_clientsock_table, szSock);
	if (NULL != pItem)
	{
		INFO("HandleClientSock: client socket is exist in the tmp hashtable now [socket id]=%d", nSock);
		return CLIENT_SOCK_EXIST;
	}
	else
	{
		INFO("HandleClientSock: client socket is not exist in the tmp hashtable"
			" and then put the socket to this hashtable [socket id]=%d", nSock);
		int *pTmp_sock = (int *)MM_MALLOC_WITH_DESC(sizeof(int), \
		"HandleClientSock: Call func for client socket");
		if (NULL == pTmp_sock)
		{
			FATAL("HandleClientSock: Call malloc error [socket id]=%d", nSock);
			return OUT_OF_MEMORY_ERROR;
		}

		*pTmp_sock = nSock;
		
		nRet = HashInsert(&pTmp_clientsock_hash->pTmp_clientsock_table, szSock, (void *)pTmp_sock);
		if (TRUE != nRet && HASH_SAMEKEY_EXIST != nRet)
		{
			ERROR("HandleClientSock: Call HashInsert error%s", "");
			MM_FREE(pTmp_sock);
			return FALSE;
		}
		
		if (HASH_SAMEKEY_EXIST == nRet)
		{
			pItem = HashGetItem(pTmp_clientsock_hash->pTmp_clientsock_table, szSock);
			if (pItem != NULL)
			{
				pthread_mutex_lock(&pTmp_clientsock_hash->hash_mutex);
				if (NULL != pItem->pMatch_msg)
				{
					MM_FREE(pItem->pMatch_msg);
				}
				
				pItem->pMatch_msg = (void *)pTmp_sock;
				pthread_mutex_unlock(&pTmp_clientsock_hash->hash_mutex);
			}

			INFO("HandleClientSock: key exist yet in the tmp client socket hashtable"
				" then we will replace the previous value [socket]=%d", *pTmp_sock);
		}
		else
		{
			INFO("HandleClientSock: insert tmp socket into tmp client socket hashtable succeed"
				" [socket id]=%d", *pTmp_sock);
		}

		INFO("HandleClientSock: func end%s", "");
		return TRUE;
	}
}


//������;: �����̺߳���, �������Ϣ���������ȡsocket, ������socket id�����տͻ��˵���Ϣ��
//�������: ��
//�������: ��
//����ֵ:  ��

static void *WorkThread(void *pParam)
{
	INFO("WorkThread: func begin%s", "");
	pthread_t cur_id = pthread_self();

	//���������߳�
	pthread_detach(cur_id);
	
	int nRet = 0;
	int nSeq = 0;
	int nSock = 0;
	char arrTmp_buf[3] = {0};
	StNode *pTmp_node = NULL;
	Thread_Pool *pThread_pool = (Thread_Pool *)pParam;

	nSeq = pThread_pool->pGetThreadSeqById(cur_id, pThread_pool);
	if (nSeq < 0)
	{
		ERROR("WorkThread: we can't find the thread seq by thread id and then exit the thread [return value]=%d", nSeq);
		return NULL;
	}

	while (TRUE)
	{
		//modify by luguanhuang 20110913(replace sequence queue with link queue)
		pthread_mutex_lock(&g_clientsocket_queue.queue_mutex);
		nRet = IsLinkQueueEmpty(&g_clientsocket_queue.stLink_queue);
		while (TRUE == nRet)
		{
			INFO("WorkThread: link queue is empty so we will wait a signal to get item"
				" from link queue [thread id]=%u", cur_id);
			pthread_cond_wait(&g_clientsocket_queue.queue_cond, &g_clientsocket_queue.queue_mutex);
			INFO("WorkThread: we recv a signal to get item from  link queue [thread id]=%u", cur_id);
			nRet = IsLinkQueueEmpty(&g_clientsocket_queue.stLink_queue);
		}

		//����Ϣ���������ȡ��Ϣ
		pTmp_node = g_clientsocket_queue.stLink_queue.pFront;
		while (pTmp_node != NULL)
		{
			nSock = pTmp_node->nData;
			INFO("WorkThread: we get the socket from socket link queue to recv client message now [socket id]=%d", nSock);
			nRet = HandleClientSock(nSock, &g_tmp_clientsock_hash);
			if (TRUE == nRet)
			{
				INFO("WorkThread: the socket is not exist in the temp socket hashtable, so we will use this socket to"
					" recv client message [socket id]=%d", nSock);
				DelFromQueuetByValue(&g_clientsocket_queue.stLink_queue, nSock);
				break;	
			}
			else if (CLIENT_SOCK_EXIST == nRet)
			{
				INFO("WorkThread: the socket is exist yet in the temp socket hashtable, so we will get next"
					" socket from socket link queuet to recv client message%s", "");
				pTmp_node = pTmp_node->pNext;	
			}
			else
			{
				ERROR("WorkThread: Call HandleClientSock error%s", "");
				pTmp_node = NULL;
			}
		}
		pthread_mutex_unlock(&g_clientsocket_queue.queue_mutex);

		if (NULL == pTmp_node)
		{
			INFO("WorkThread: current thread can't find the socket from socket link queue"
				" to recv client message or insert socket to client temp socket hashtable error%s", "");
			pthread_mutex_lock(&g_clientsocket_queue.queue_mutex);
			INFO("WorkThread: we will wait a signal to continue search the queue to get the socket"
				" and recv client message%s", "");	
			pthread_cond_wait(&g_clientsocket_queue.queue_cond, &g_clientsocket_queue.queue_mutex);	
			INFO("WorkThread: we recv a signal to continue search the queue to get the socket"
				" and recv client message%s", "");	
			pthread_mutex_unlock(&g_clientsocket_queue.queue_mutex);
			continue;
		}

		nRet = pThread_pool->pThread_info[nSeq].pProcessFunc(nSock);
		DEBUG("WorkThread: Call RcvMsgFromClient [return value]=%d", nRet);

		//modify by luguanhuang 20110913(when we recv one message finish, if still has message to recv
		//, we will put this socket to the queue, and signal one thread to recv this socket's message
		//if error occur, we will close the socket)
		if (RECV_MSG_FINISH == nRet)		//һ����Ϣ���������			
		{	
			nRet = recv(nSock, arrTmp_buf, 2, MSG_PEEK);
			if (nRet > 0)
			{
				DEBUG("WorkThread: Call recv func(MSG_PEEK) [return value]=%d", nRet);
				INFO("WorkThread: now current socket is still has data to recv so"
					" we will push the socket into socket link queue and singal next thread"
					" to recv client message%s", "");

				DelTmpClientSockFromHashtable(nSock);
				nRet = InsertItemToQueue(&g_clientsocket_queue.stLink_queue, nSock);
				if (TRUE != nRet)
				{
					ERROR("WorkThread: Call InsertItemToQueue error%s", "");
					continue;
				}
				
				pthread_cond_signal(&g_clientsocket_queue.queue_cond);
			}
			else if (0 == nRet || (-1 == nRet && EAGAIN != errno))			//��ǰ���շ�������
			{
				if ((-1 == nRet && EAGAIN != errno))
				{
					ERROR("WorkThread: Call recv func [return value]=%d error[%d]=%s", \
					nRet, errno, strerror(errno));
				}
				else
				{
					INFO("WorkThread: client socket is close%s", "");		
				}
				
				RecvClientMsgErrorProcess(nSock);		
			}
			else			//�Ѿ�û����Ϣ������
			{
				INFO("WorkThread: Call recv func [return value]=%d error[%d]=%s", \
					nRet, errno, strerror(errno));
				INFO("WorkThread: we recv the client msg finish"
				" and then we will delete the socket from the tmp hash table"
				" [return value]=%d [socket id]=%d", \
				nRet, nSock);
				DelTmpClientSockFromHashtable(nSock);
			}		
		}
		else if (RECV_MSG_NOTFINISH == nRet)	//��Ҫ�����һ����Ϣ�������ε����
		{	
			INFO("WorkThread: recv client msg is not finish "
				" then we will get the socket from the link queue"
				" to continue to recv the client msg [return value]=%d [socket id]=%d", nRet, nSock);
			DelTmpClientSockFromHashtable(nSock);
		}
		else if (RECV_MSG_EMPTY == nRet)		//��Ҫ�����socket������������socket�����
		{
			INFO("WorkThread: current socket don't have message to recv so we will"
				" discard this recv process(return value is -1 and error is EAGAIN)%s", "");
			DelTmpClientSockFromHashtable(nSock);
		}
		else				//����RECV_MSG_ERROR, RECV_CLOSE_SOCKET, FALSE, OUT_OF_MEMORY �⼸��������
		{
			INFO("WorkThread: client socket is close or client exit in an unusual way"
				" or other error [return value]=%d [socket id]=%d", nRet, nSock);			
			RecvClientMsgErrorProcess(nSock);	
		}
		
	}
	INFO("WorkThread: func end%s", "");
	return NULL;
}


//������;: ��ʼ���̳߳غ���(��ʼ���̵߳�ͬ���ͻ������,   ���Ҵ�����Ӧ���߳�)
//�������: ��
//�������: ��
//����ֵ: �����ʼ���ɹ�, ����TRUE,  �����ʼ��ʧ��, ����FALSE

int InitThreadPool(Thread_Pool *pThread_pool)
{
	INFO("InitThreadPool: func begin%s", "");
	int nMin_thread_num = pThread_pool->nMin_thread_num;

	DEBUG("InitThreadPool: [initial thread num]=%d", nMin_thread_num);
	
	int i = 0;
	int nRet = 0;
	for (i=0; i<nMin_thread_num; i++)
	{
		nRet = pthread_cond_init(&(pThread_pool->pThread_info[i].thread_cond), NULL);
		if (nRet != 0)
		{
			ERROR("InitThreadPool: Call pthread_cond_init error%s", "");
			return FALSE;
		}

		//��ʼ���̻߳������
		nRet = pthread_mutex_init(&(pThread_pool->pThread_info[i].thread_mutex), NULL);
		if (nRet != 0)
		{
			ERROR("InitThreadPool: Call pthread_mutex_init error%s", "");	
			return FALSE;
		}

		//���������߳�
		nRet = pthread_create(&(pThread_pool->pThread_info[i].thread_id), NULL, WorkThread, pThread_pool);
		if (nRet != 0)
		{
			ERROR("InitThreadPool: Call pthread_create error%s", "");
			return FALSE;
		}
	}

	INFO("InitThreadPool: func end%s", "");
	return TRUE;
}

//������;: �̺߳�����Ĭ�ϴ�����
//�������: socket id
//�������: ��
//����ֵ: ����(Ŀǰ��û�õ�����ֵ)


static int ProcessFunc(int nSock)
{
	return TRUE;
}


//������;: �����̳߳�
//�������: �̺߳�����Ĭ�ϴ�����
//�������: ��
//����ֵ: ��������̳߳سɹ�,  ����TRUE,  ��������̳߳�ʧ��, ����FALSE

int CreateThreadPool(Thread_Pool *pThread_pool, int (*pProcessFunc)(int))
{
	INFO("CreateThreadPool: func begin%s", "");
	int nRet = 0;
	int i = 0;

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	int nMin_thread_num = g_srv_conf_info.nMin_thread_num;
	int nMax_thread_num = g_srv_conf_info.nMax_thread_num;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);
	
		//��ʼ���̳߳ػ������
	nRet = pthread_mutex_init(&(pThread_pool->thread_pool_mutex), NULL);	
	if (nRet != 0)
	{
		ERROR("CreateThreadPool: Call pthread_mutex_init error%s", "");	
		return FALSE;
	}

	//���̳߳صĺ������������������������

	pthread_mutex_lock(&pThread_pool->thread_pool_mutex);
	pThread_pool->pInitThreadPool = InitThreadPool;	
	pThread_pool->pGetThreadSeqById = GetThreadSeqById;
	
	pThread_pool->nMin_thread_num = nMin_thread_num;		//�����߳�
	pThread_pool->nCur_thread_num = nMin_thread_num;		//��ǰ�߳�
	pThread_pool->nMax_thread_num = nMax_thread_num;//����߳�(����̵߳�������Ϊ���Ժ��ܺ�̬�����̵߳�����)

	DEBUG("CreateThreadPool: [min thread num]=%d [current thread num]=%d [max thread num]=%d", \
		pThread_pool->nMin_thread_num, pThread_pool->nCur_thread_num, pThread_pool->nMax_thread_num);


	//��һ�����Ϊ�˱�֤pThread_pool->pThread_infoָ����ڴ治����
	MM_FREE(pThread_pool->pThread_info);
	pThread_pool->pThread_info = (Thread_Info *)MM_MALLOC_WITH_DESC(nMax_thread_num * sizeof(Thread_Info), 
	"CreateThreadPool: Call func for thread info");
	if (NULL == pThread_pool->pThread_info)
	{
		FATAL("CreateThreadPool: Call malloc error%s", "");
		pthread_mutex_unlock(&pThread_pool->thread_pool_mutex);
		return FALSE;;
	}
	memset(pThread_pool->pThread_info, 0, nMax_thread_num * sizeof(Thread_Info));

	//��ʼ���߳���Ϣ
	if (NULL == pProcessFunc)
	{
		INFO("CreateThreadPool: we will use the system default thread func%s", "");
		for (i=0; i<nMax_thread_num; i++)
		{	
			pThread_pool->pThread_info[i].en_thread_usr = THREAD_NOT_USE;			//���߳�ָ��Ϊδʹ��״̬
			pThread_pool->pThread_info[i].pProcessFunc = ProcessFunc;				//ʹ��ϵͳĬ�ϵ�ִ�к���
		}		
	}
	else
	{
		INFO("CreateThreadPool: we will use the self define thread func%s", "");
		for (i=0; i<nMax_thread_num; i++)
		{
			pThread_pool->pThread_info[i].en_thread_usr = THREAD_NOT_USE;				//���߳�ָ��Ϊδʹ��״̬
			pThread_pool->pThread_info[i].pProcessFunc = pProcessFunc;				//ʹ���û�ָ���ĺ���
		}	
	}

	//��ʼ���̳߳�
	nRet = InitThreadPool(pThread_pool);
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

