
#include "../util/service_global.h"
#include "../include/ydt_log.h"
#include "../interface/service_protocol.h"
#include "service_communication.h"
#include "service_maintaininfo_thread.h"
#include "service_process_func.h"
#include "service_sock_info.h"
#include "service_simplemode_server.h"

extern Server_Conf_Info g_srv_conf_info;
extern Threads_Info g_threads_info;							//�߳�ID��Ϣ
extern Process_Thread_Counter g_process_thread_counter;

//������;: ��ʼ�����ͽ����̵߳Ĳ�����Ϣ
//�������: �̲߳���,   socket ids
//�������: �� 
//����ֵ	: ��ʼ���ɹ�,   ����TRUE,  ��ʼ��ʧ�ܣ�����FALSE

int InitSndRcvParamInfo(Snd_Recv_Param *pParam, int nSock, int nMsg_queue_size)
{
	INFO("InitSndRcvParamInfo: func begin%s", "");
	if (NULL == pParam)
	{
		ERROR("InitSndRcvParamInfo: func param error%s", "");
		return FALSE;
	}
	
	pParam->nSock = nSock;
	
	//��ʼ���������	
	if (pthread_mutex_init(&pParam->mutex, NULL) != 0)
	{
		ERROR("InitSndRcvParamInfo: call pthread_mutex_init error%s", "");
		return FALSE;
	}
	
	//��ʼ��ͬ������
	if (pthread_cond_init(&pParam->cond, NULL) != 0)
	{
		ERROR("InitSndRcvParamInfo: call pthread_cond_init error%s", "");
		return FALSE;
	}	
	
	//��ʼ����Ϣ����
	pthread_mutex_lock(&(pParam->mutex));
	memset(&(pParam->threads_exit_info), 0, sizeof(pParam->threads_exit_info));

	if (pthread_mutex_init(&(pParam->threads_exit_info.thread_mutex), NULL) != 0)
	{
		ERROR("InitSndRcvParamInfo: call pthread_mutex_init error%s", "");
		pthread_mutex_unlock(&(pParam->mutex));
		return FALSE;
	}

	DEBUG("InitSndRcvParamInfo: [msg queue size]=%d", nMsg_queue_size);
	
	//if (FALSE == YDT_InitQueue(&pParam->queue, nMsg_queue_size, FreeMsgBuffer))
	if (FALSE == InitSeqQueue(&pParam->queue, nMsg_queue_size, FreeMsgBuffer))
	{
		ERROR("InitSndRcvParamInfo: Call YDT_InitQueue error%s", "");
		pthread_mutex_unlock(&(pParam->mutex));
		return FALSE;
	}

	pthread_mutex_unlock(&(pParam->mutex));
	INFO("InitSndRcvParamInfo: func end%s", "");
	return TRUE;

}

//add by luguanhuang 2011-8-13
//�����߳�ID��ȡ�߳����
static int GetThreadSeqById(StThreads_Exit_Info *pThread_exit_info, pthread_t thread_id)
{
	INFO("GetThreadSeqById: func begin%s", "");
	pthread_mutex_lock(&(pThread_exit_info->thread_mutex));
	if (NULL == pThread_exit_info)
	{
		ERROR("GetThreadSeqById: func param error%s", "");
		pthread_mutex_unlock(&(pThread_exit_info->thread_mutex));
		return -1;
	}
	
	int nThread_num = pThread_exit_info->nThread_num;

	int i = 0;
	for (i=0; i<nThread_num; i++)
	{
		if (thread_id == pThread_exit_info->arrThread_exit[i].thread_id)
		{
			pthread_mutex_unlock(&(pThread_exit_info->thread_mutex));
			DEBUG("GetThreadSeqById: [thread id]=%u [thread sequence]=%d", (unsigned int)thread_id, i);
			INFO("GetThreadSeqById: func end%s", "");		
			return i;	
		}
	}

	pthread_mutex_unlock(&(pThread_exit_info->thread_mutex));

	INFO("GetThreadSeqById: Can't get thread sequence by id%s", "");
	INFO("GetThreadSeqById: func end%s", "");
	return -1;
}


//������;:������һ���̵߳ĺ���
//�������: ��Ҫ�������߳�ID,  �߳�����
//�������: �� 
//����ֵ	:�����̳߳ɹ�,  ����TRUE,  �����߳�ʧ��,   ����FALSE

int CancelOperatorProc(pthread_t cur_thread_id, Thread_Type thread_type, StThreads_Exit_Info *pThreads_exit_info)
{
	INFO("CancelOperatorProc: begin func%s", "");
	if (NULL == pThreads_exit_info)
	{
		ERROR("CancelOperatorProc: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	pthread_t tmp_id = cur_thread_id;
	pthread_t cancel_thread_id = 0;
	int i = 0;
	int nTmp_cnt = 0;
	Threads_Id tmp_ids = {0, 0};

	//��ΪҪ����ȫ�ֱ���,  ����Ҫ����
	pthread_mutex_lock(&g_threads_info.thread_mutex);
	nTmp_cnt = g_threads_info.nID_counter;
	//�����߳�����(���ͻ��ǽ���)����ȡ���ջ����̵߳�ID
	if (RECV_THREAD_TYPE== thread_type)
	{
		INFO("CancelOperatorProc: cancel send thread operator%s", "");
		for (i=0; i<nTmp_cnt; i++)
		{
			tmp_ids = g_threads_info.arrThreads_id[i];
			if (tmp_id == tmp_ids.recv_id)
			{
				cancel_thread_id = tmp_ids.snd_id;
				break;
			}
		}
	}
	else if (SND_THREAD_TYPE == thread_type)
	{
		INFO("CancelOperatorProc: cancel recv thread operator%s", "");
		for (i=0; i<nTmp_cnt; i++)
		{
			tmp_ids = g_threads_info.arrThreads_id[i];
			if (tmp_id == tmp_ids.snd_id)
			{
				cancel_thread_id = tmp_ids.recv_id;
				break;
			}
		}
	}

	//ȡ������
	if (pthread_cancel(cancel_thread_id) != 0)
	{
		ERROR("CancelOperatorProc: Call pthread_cancel error cancel thread fail%s", "");
		pthread_mutex_unlock(&g_threads_info.thread_mutex);
		return FALSE;
	}

	g_threads_info.nID_counter--;
	pthread_mutex_unlock(&g_threads_info.thread_mutex);

	DEBUG("CancelOperatorProc: cancel thread succeed [current thread counter]=%d", g_threads_info.nID_counter);
	sleep(1);

	DEBUG("CancelOperatorProc: [cancel thread id]=%d", (int)cancel_thread_id);
	//add by luguanhuang 2011-08-13
	//��ȡ�߳����
	int nSeq = GetThreadSeqById(pThreads_exit_info, cancel_thread_id);
	if (nSeq != -1)
	{
		DEBUG("CancelOperatorProc: [cancel thread sequence]=%d", nSeq);
		pThreads_exit_info->arrThread_exit[nSeq].bIsproc_thread_exit = TRUE;
	}

	INFO("CancelOperatorProc: func end%s", "");
	return TRUE;
}

//add by luguanhuang 2011-08-13
//�ж������߳��Ƿ��Ѿ��˳�
static int IsAllThreadExit(StThreads_Exit_Info *pThreads_exit_info, pthread_t thread_id)
{
	INFO("IsAllThreadExit: func begin%s", "");
	if (NULL == pThreads_exit_info)
	{
		ERROR("IsAllThreadExit: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int i = 0;
	int nThread_num = pThreads_exit_info->nThread_num;

	for (i=0; i<nThread_num; i++)
	{
		if (thread_id != pThreads_exit_info->arrThread_exit[i].thread_id)
		{
			if (FALSE == pThreads_exit_info->arrThread_exit[i].bIsproc_thread_exit)
			{
				INFO("IsAllThreadExit: not all the thread exit%s", "");
				INFO("IsAllThreadExit: func end%s", "");
				return FALSE;	
			}	
		}
	}

	INFO("IsAllThreadExit: all the thread exit now%s", "");
	INFO("IsAllThreadExit: func end%s", "");
	return TRUE;
}

//������;: ����ȡ���̵߳ı�־
//�������:  ��
//�������: �� 
//����ֵ	: ��

void SetCancelThreadFlg(void)
{
	INFO("SetCancelThreadFlg: func begin%s", "");
	int nState = 0;
	int nOld_state = 0;

	int nType = 0;
	int nOld_type = 0;

	//����ȡ��״̬Ϊenable״״̬
	nState = PTHREAD_CANCEL_ENABLE;
	pthread_setcancelstate(nState, &nOld_state);

	//���õ����յ�ȡ���ź�ʱӦ����ִ��ȡ������
	nType = PTHREAD_CANCEL_ASYNCHRONOUS;
	pthread_setcanceltype(nType, &nOld_type);
	INFO("SetCancelThreadFlg: func end%s", "");
}

//add by luguanhuang 2011-08-13
//���ٶ���
void DestroyQueue(StThreads_Exit_Info *pThreads_exit_info, StSequence_Queue *pQueue, pthread_t thread_id)
{
	INFO("DestroyQueue: func begin%s", "");
	int nRet = IsAllThreadExit(pThreads_exit_info, thread_id);
	while (TRUE)
	{
		if (TRUE == nRet)			//������е��̶߳��˳�, �Ͱ���Ϣ��������
		{
			INFO("DestroyQueue: all the thread exit and then destroy the queue%s", "");
			DestroySeqQueue(pQueue);
			break;
		}
		else if (FALSE == nRet)
		{
			INFO("DestroyQueue: not all the thread exit sleep 1s and then destroy the queue again%s", "");
			sleep(1);
			nRet = IsAllThreadExit(pThreads_exit_info, thread_id);
			continue;
		}
	}

	INFO("DestroyQueue: func end%s", "");
}


//������;: ����ͻ�����Ϣ�ĺ���
//�������: ��Ŵ�������Ļ���
//�������: �� 
//����ֵ	: ��

void *ProcessMsg(void *pParam)
{
	INFO("ProcessMsg: func begin%s", "");
	pthread_detach(pthread_self());
	Process_Param *pProcess_param = (Process_Param *)pParam;
	Client_Server_Msg *pTmp_msg = &pProcess_param->cli_srv_msg;
	StSequence_Queue *pQueue = pProcess_param->pQueue;
	pthread_mutex_t *pProc_mutex = pProcess_param->pProc_mutex;
	pthread_cond_t *pCond = pProcess_param->pCond;
	StThreads_Exit_Info *pThread_exit_info = pProcess_param->pThreads_exit_info;
	int nSeq = pProcess_param->nSeq;			//��ȡ�߳����	
	
	char *pData = pTmp_msg->pData;
	pData++;
	
	WORD uMsg_type = ntohs(*(WORD *)pData);	

	DEBUG("ProcessMsg: [service msg type]=%d", uMsg_type);

	//���ݲ�ͬ����Ϣ���͵��ò�ͬ�ĵĴ�����
	if(uMsg_type >= 0x01 && uMsg_type <= 0x28)
	{
		YDTProcessFunc(uMsg_type, pTmp_msg, pQueue, pProc_mutex, pCond);
	}
	else if (uMsg_type >= 0x100 && uMsg_type <= 0x10A)
	{
		LTProcessFunc(uMsg_type, pTmp_msg, pQueue, pProc_mutex, pCond);	
	}

	//pthread_mutex_lock(pProc_mutex);

	MM_FREE(pProcess_param->cli_srv_msg.pData);
	MM_FREE(pProcess_param);

	//pthread_mutex_unlock(pProc_mutex);
	
	pthread_mutex_lock(&g_process_thread_counter.counter_mutex);
	g_process_thread_counter.nThread_counter--;					//�����̵߳���Ϣ��������1
	pthread_mutex_unlock(&g_process_thread_counter.counter_mutex);

	//add by luguanhuang 2011-8-13
	pthread_mutex_lock(pProc_mutex);
	//add by luguanhuang 2011-08-13
	pThread_exit_info->arrThread_exit[nSeq].bIsproc_thread_exit = TRUE;				//���߳��˳���־����ΪTRUE
	pThread_exit_info->nThread_num--;												//���߳�������һ
	pthread_mutex_unlock(pProc_mutex);
	INFO("ProcessMsg: func end%s", "");
	return NULL;
	
}


//������;:���ս�����������͹�������Ϣ(������ճɹ�,  �߿���һ�������߳�������ͻ��˵���Ϣ)
//�������: ���ͽ����̵߳Ĳ���
//�������: �� 
//����ֵ	:������Ϣ�ɹ��������̳߳ɹ�,   ����TRUE,   ���߷���FALSE

void *RcvFunc(void* pThread_param)
{
	INFO("RcvFunc: func begin%s", "");
	//�����߳�Ϊ�����߳�
	pthread_detach(pthread_self());
	Snd_Recv_Param *pParam = (Snd_Recv_Param *)pThread_param;

	StThreads_Exit_Info *pThreads_exit_info = &(pParam->threads_exit_info);
	Process_Param *pProcess_param = NULL;
	int nRcv_sock = pParam->nSock;
	pthread_t cur_thread_id = 0;
	int nRet = 0;
	cur_thread_id = pthread_self();					//��ȡ�߳�ID

	SetCancelThreadFlg();
	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	int nMax_procthread_num =g_srv_conf_info.nMax_process_thread_num;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	DEBUG("RcvFunc: [max process thread num]=%d", nMax_procthread_num);
	while (TRUE)
	{	
		pProcess_param = (Process_Param *)MM_MALLOC_WITH_DESC(sizeof(Process_Param), \
		"RcvFunc: Call func for process thread param");
		if (NULL == pProcess_param)
		{
			FATAL("RcvFunc: Call malloc error%s", "");
			CancelOperatorProc(cur_thread_id, RECV_THREAD_TYPE, pThreads_exit_info);
			//hebin 20110813 YDT_DestroyQueue(&pParam->queue);
			//add by luguanhuang 2011-08-13
			//���ٶ���
			DestroyQueue(pThreads_exit_info, &pParam->queue, cur_thread_id);
			MM_FREE(pParam);
			close(nRcv_sock);
			pthread_exit(NULL);
			return NULL;
		}

		memset(pProcess_param, 0, sizeof(Process_Param));

		//������Ϣ
		if (FALSE == RecvAccessSrvMsg(nRcv_sock, (void *)&pProcess_param->cli_srv_msg))
		{
			ERROR("RcvFunc: Call RecvMsg error [return value]=%d", FALSE);
			CancelOperatorProc(cur_thread_id, RECV_THREAD_TYPE, pThreads_exit_info);				//ȡ���̲߳���
			//hebin 20110813 YDT_DestroyQueue(&pParam->queue);				//������Ϣ����
			//add by luguanhuang 2011-08-13
			//���ٶ���
			DestroyQueue(pThreads_exit_info, &pParam->queue, cur_thread_id);

			//�ͷ��ڴ�

			MM_FREE(pProcess_param);
			MM_FREE(pParam);
			close(nRcv_sock);
			pthread_exit(NULL);				//�Ƴ��߳�
			return NULL;
		}
		
		pProcess_param->pQueue = &pParam->queue;
		pProcess_param->pProc_mutex = &pParam->mutex;
		pProcess_param->pCond = &pParam->cond;
		pProcess_param->pThreads_exit_info = &(pParam->threads_exit_info);
		pThreads_exit_info = &(pParam->threads_exit_info);						//�����߳��˳���Ϣ

		//����
		pthread_mutex_lock(&g_process_thread_counter.counter_mutex);
		g_process_thread_counter.nThread_counter++;

		//����������̳߳��������������̵߳��������
		if (nMax_procthread_num == g_process_thread_counter.nThread_counter)
		{
			DEBUG("RcvFunc: max process thread num=%d", g_process_thread_counter.nThread_counter);
			WARN("RcvFunc: current process thread num exceed the max num of process thread please wait 2s"
				" and then recv client message again%s", "");
			sleep(2);
			g_process_thread_counter.nThread_counter--;
			MM_FREE(pProcess_param->cli_srv_msg.pData);
			MM_FREE(pProcess_param);
			pthread_mutex_unlock(&g_process_thread_counter.counter_mutex);
			continue;
		}

		int nThread_num = pThreads_exit_info->nThread_num;
		pProcess_param->nSeq = nThread_num;
		pThreads_exit_info->nThread_num++;
		pThreads_exit_info->arrThread_exit[nThread_num].bIsproc_thread_exit = FALSE;
		
		//���������߳�
		nRet = pthread_create(&pThreads_exit_info->arrThread_exit[nThread_num].thread_id, NULL, ProcessMsg, (void *)pProcess_param);
		if(nRet != 0)
		{
			DEBUG("RcvFunc: Call pthread_create error Create process thread error current thread num=%d", g_process_thread_counter.nThread_counter);
			//CancelOperatorProc(cur_thread_id, RECV_THREAD_TYPE, pThreads_exit_info);		//�߳�ȡ������
			//DestroyQueue(pThreads_exit_info, &pParam->queue, cur_thread_id);

			g_process_thread_counter.nThread_counter--;

			MM_FREE(pProcess_param->cli_srv_msg.pData);
			MM_FREE(pProcess_param);
			pthread_mutex_unlock(&g_process_thread_counter.counter_mutex);
			continue;
		}	

		pthread_mutex_unlock(&g_process_thread_counter.counter_mutex);			//����
	}
	
	INFO("RcvFunc: func end%s", "");
	return NULL;
}

//������;:������Ϣ���ض��Ľ��������
//�������: ���ͽ����̵߳Ĳ���
//�������: �� 
//����ֵ	:������Ϣ�ɹ�,   ����TRUE,  ������Ϣʧ��,   ����FALSE 

void *SendFunc(void *pThread_param)
{	
	INFO("SendFunc: func begin%s", "");
	pthread_detach(pthread_self());	
	Snd_Recv_Param *pParam = (Snd_Recv_Param *)pThread_param;
	
	int nSnd_sock = pParam->nSock;
	StSequence_Queue *pQueue = &pParam->queue;
	Server_Client_Msg *pSer_cli_msg = NULL;
	StThreads_Exit_Info *pThreads_exit_info = &(pParam->threads_exit_info);
	int nRet = 0;
	WORD wCmd_id = 0;
	char *pData = NULL;
	
	pthread_t cur_thread_id = pthread_self();
	struct timespec tm = {0};
	SetCancelThreadFlg();
	
	for(;;)
	{
		pthread_mutex_lock(&pParam->mutex);

		//�ж���Ϣ�����Ƿ�Ϊ��
		nRet = IsSeqQueueEmpty(pQueue);
		while (TRUE == nRet)
		{
			INFO("SendFunc: msg queue is empty, please wait to get msg from queue%s", "");
			tm.tv_sec = time(NULL) + 4;
			tm.tv_nsec = 0;
			pthread_cond_wait(&pParam->cond, &pParam->mutex);
			//pthread_cond_timedwait(&pParam->cond, &pParam->mutex, &tm);
			nRet = IsSeqQueueEmpty(pQueue);
		}
		
		//����Ϣ������ȡ����Ϣ
		nRet = GetMsgFromSeqQueue(pQueue, (void **)&pSer_cli_msg);
		pthread_mutex_unlock(&pParam->mutex);

		//������Ϣ
		if (FALSE == SendMsg(nSnd_sock, (void *)pSer_cli_msg))
		{
			INFO("SendFunc: Call SendMsg error [return value]=%d", FALSE);
			CancelOperatorProc(cur_thread_id, SND_THREAD_TYPE, pThreads_exit_info);				//ȡ���߳������
			//hebin 20110813 YDT_DestroyQueue(&pParam->queue);								//���ٶ���
			//add by luguanhuang 2011-08-13
			//���ٶ���
			DestroyQueue(pThreads_exit_info, &pParam->queue, cur_thread_id);					
			//�ͷ��ڴ�
			MM_FREE(pParam);
			MM_FREE(pSer_cli_msg->pData);
			MM_FREE(pSer_cli_msg);
			close(nSnd_sock);											//�ر�socket
			pthread_exit(NULL);											//�˳��߳�
			return NULL;
		}

		pData = pSer_cli_msg->pData;
		wCmd_id = *(WORD *)(pData + 1);
		wCmd_id = ntohs(wCmd_id);

		char arrAcc_checkcode[5] = {0};
		memcpy(arrAcc_checkcode, pSer_cli_msg->arrCheck_code, 4);
		
		DEBUG("SendFunc: [version]=%d [msg type]=%d [main service code]=%d [sub service code]=%d"
		" [access server seq]=%d [msg counter]=%d [access check code]=%s [data len]=%d [cmd_id]=%d", pSer_cli_msg->bVersion, pSer_cli_msg->bMsg_type, \
		pSer_cli_msg->bMain_service_code, pSer_cli_msg->bSub_service_code, ntohs(pSer_cli_msg->wAccess_server_seq), \
		ntohl(pSer_cli_msg->nMsg_counter), arrAcc_checkcode, ntohl(pSer_cli_msg->nData_len), wCmd_id);	
		INFO("SendFunc: send msg to access server succeed%s", "");
		MM_FREE(pSer_cli_msg->pData);
		MM_FREE(pSer_cli_msg);
	}

	INFO("SendFunc: func end%s", "");
	return NULL;
}


int RunSimpleModeServer(void)
{
	INFO("RunSimpleModeServer: func begin%s", "");
	int nListen_sock = 0;
	int nConn_sock = 0;
	pthread_t snd_id = 0;
	pthread_t recv_id = 0;
	pthread_t thread_id = 0;
	
	int nRet = 0;
	//��ʼ��������socket
	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	int nMsg_queue_size = g_srv_conf_info.nMsg_queue_size;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	DEBUG("RunSimpleModeServer: [message queue size]=%d", nMsg_queue_size);

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	WORD wPort = g_srv_conf_info.wPort;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	if (FALSE == InitListenSockInfo(&nListen_sock, wPort))
	{
		ERROR("RunSimpleModeServer: Call InitListenSockInfo error%s", "");
		return FALSE;	
	}

	INFO("RunSimpleModeServer: service server listening......%s", "");

	//��������ά���߳�
	nRet= pthread_create(&thread_id, NULL, RunMaintainThread, NULL);
	if (nRet!= 0)
	{
		ERROR("RunSimpleModeServer: Call pthread_create error%s", "");
		return FALSE;
	}
	
	while (TRUE)
	{
		//����
		nRet = Accept(nListen_sock, &nConn_sock);
		if (FALSE == nRet)
		{	
			ERROR("RunSimpleModeServer: Call Accept error sleep 1s and then accept client connection request again%s", "");
			sleep(1);
			continue;  //hebin del 20110813 return FALSE;
		}

		DEBUG("RunSimpleModeServer: accept client connection request succeed%s", "");

		Snd_Recv_Param *pThread_param = (Snd_Recv_Param *)MM_MALLOC_WITH_DESC(sizeof(Snd_Recv_Param), \
		"RunSimpleModeServer: Call func for send recv thread param");
		if (NULL == pThread_param)
		{
			FATAL("RunSimpleModeServer: Call malloc error%s", "");
			continue;  //hebin del 20110813 return FALSE;
		}

		//��ʼ�����ͽ����̲߳�����Ϣ
		if (FALSE == InitSndRcvParamInfo(pThread_param, nConn_sock, nMsg_queue_size))
		{
			ERROR("RunSimpleModeServer: Call InitSndRcvParamInfo error%s", "");
			MM_FREE(pThread_param);
			continue;  //hebin del 20110813 return FALSE;	
		}

		//add by luguanhuang 2011-08-13
		pthread_mutex_lock(&pThread_param->mutex);
		int nThread_num = pThread_param->threads_exit_info.nThread_num;				//��ȡ��ǰ���߳�����
		pThread_param->threads_exit_info.nThread_num++;								//�߳�������һ
		pThread_param->threads_exit_info.arrThread_exit[nThread_num].bIsproc_thread_exit = FALSE;		//���߳��˳���־����ΪFALSE

		//���������߳�
		nRet= pthread_create(&pThread_param->threads_exit_info.arrThread_exit[nThread_num].thread_id, NULL, RcvFunc, (void *)pThread_param);	
		if(nRet!= 0)
		{
			ERROR("RunSimpleModeServer: Call pthread_create error%s", "");
			continue;  //hebin del 20110813 return FALSE;
		}

		recv_id = pThread_param->threads_exit_info.arrThread_exit[nThread_num].thread_id;			//��������̵߳�ID


		//add by luguanhuang 2011-8-13
		nThread_num = pThread_param->threads_exit_info.nThread_num;					//��ȡ�߳�����
		pThread_param->threads_exit_info.nThread_num++;								//�߳�������һ
		pThread_param->threads_exit_info.arrThread_exit[nThread_num].bIsproc_thread_exit = FALSE;			//���߳��˳���־����ΪFALSE
		//���������߳�
		nRet= pthread_create(&pThread_param->threads_exit_info.arrThread_exit[nThread_num].thread_id, NULL, SendFunc, (void *)pThread_param);
		if(nRet!= 0)
		{
			ERROR("RunSimpleModeServer: Call pthread_create error%s", "");
			if (pthread_cancel(recv_id) != 0)
			{
				ERROR("RunSimpleModeServer: Call pthread_cancel error%s", "");
			}
		
			continue;  //hebin del 20110813 return FALSE;
		}

		//add by luguanhuang 2011-08-13
		snd_id = pThread_param->threads_exit_info.arrThread_exit[nThread_num].thread_id;			//���淢���̵߳�ID
	
		nThread_num = pThread_param->threads_exit_info.nThread_num;
		pthread_mutex_unlock(&pThread_param->mutex);

		//������������߳�
		/*nRet= pthread_create(&snd_id, NULL, (void *)HeartBeatDetectThread, (void *)pThread_param);
		if(nRet!= 0)
		{
			PrintContent("StartServer: Call pthread_create error! \n");
			return FALSE;
		}*/

		//���淢�ͽ����̵߳�ID
		pthread_mutex_lock(&g_threads_info.thread_mutex);
		g_threads_info.arrThreads_id[g_threads_info.nID_counter].recv_id = recv_id;					//���淢���̵߳�ID
		g_threads_info.arrThreads_id[g_threads_info.nID_counter].snd_id = snd_id;					//��������̵߳�ID
		g_threads_info.pKeep_param[g_threads_info.nID_counter] = pThread_param;						//���淢�ͽ����̵߳Ĳ���
		g_threads_info.nID_counter++;																//��������һ����
		pthread_mutex_unlock(&g_threads_info.thread_mutex);
	}

	INFO("RunSimpleModeServer: func end%s", "");
	return TRUE;
}

