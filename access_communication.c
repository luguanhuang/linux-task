
#include "../util/access_global.h"
#include "../interface/access_protocol.h"
#include "access_routing_maintain.h"
#include "../util/access_init_var.h"
#include "access_thread_pool.h"
#include "access_heartbeat_detectthread.h"
#include "access_comm_service_srv_snd.h"
#include "access_comm_client_rcv.h"
#include "access_maintaininfo_thread.h"
#include "access_heartbeat_detectthread.h"
#include "access_comm_client_snd.h"
#include "access_epoll.h"
#include "access_sock_info.h"
#include "access_heartbeat_detectthread.h"
#include "access_check_timeoutmsg.h"
#include "access_communication.h"

/*���ļ���Ҫ�Ǹ�����������ͨѶʹ�õ�
  *���ļ��в���epollģʽ������ͻ��˵Ĵ󲢷�����
  *��ģʽ��Ҫ����ETģʽ,  Ҳ���Ǳ�Ե����,  Ҳ���ǵ���
  *��Ϣ����,  ֻ����һ��,  ��������ÿ�ν��տͻ��˵���Ϣ��ʱ��
  *Ҫ��ѭ�������տͻ��˵���Ϣ,  ֱ��������Ϊֹ,  �ڸ�ģʽ��, ��Ҫ����
  *��Ϣ�����ϣ�������տͻ��˵���Ϣ,  �ñ�����socket idΪ��,  �ͻ������ݰ�Ϊֵ��
  *�ñ�Ŀǰ���ڵ�������е�ʱ��ñ��޷���̬��ɾ��ɾ��һ��,  ʹ���´��ٲ����ϣ���ʱ��
  *�ᵼ��������Ϊ�����ݰ��Ѿ����ڶ��Ѹ����ݰ�����,  ��Ҳ��Ŀǰ��ϵͳ���ȶ������֮һ
  *���о�����Ϊepoll�е�socket �����˷�����ģʽ,   ���ڷ��ͻ��������������³��򲻶ϵط���EAGAIN, 
  *�������е�����������Ļ�,  �����Ī���Ĵ���,  ���Ҳ��Ŀǰϵͳ���ȶ������
  */

#define  HAVE_MSGHDR_MSG_CONTROL

extern int g_iShmId;

extern StRouting_Table g_routing_table;
Thread_Pool g_t_recv_client_msg;

/*�ͻ��˵�socket��ϣ��
  *���ǽ����������ź���Ϣ�����������
  *�˹�ϣ����Ҫ����������ͻ�����Ϣ������Ϣ��
  */



//��Ϣ����,  ��Ҫ�Ǵ�ſͻ��˴��͹�������Ϣ��


//����ά���̱߳�־,  ��Ҫ����������·��ά���߳�����ҵ����������������� 
Awake_Thread_Flg g_awake_thread_flg;

//epoll�ṹ,  ��Ҫ��������ͻ��˵Ĵ󲢷����ӵ�
//������Ҫ��ŵ���socket id
Epoll_Fd g_epoll_fd;


/*ѡ��ͬ��ҵ������������(���ҵ��������ж�̨,  
  *���ǿ���ѡ�����ѡ�����е�һ̨ҵ���������ת����Ϣ��)
  */
extern Service_Server_Seq g_login_srv_seq;
extern Service_Server_Seq g_register_srv_seq;
extern Service_Server_Seq g_simplechat_srv_seq;
extern Service_Server_Seq g_clusterchat_srv_seq;
extern Service_Server_Seq g_liantong_srv_seq;				//��ͨר�÷��������
extern Service_Server_Seq g_wanwei_loginsrv_seq;			//��ά��¼���������

//����ҵ�����������(ҵ������������ж�̨)
extern Service_Server_Num g_login_srv_num;
extern Service_Server_Num g_register_srv_num;;
extern Service_Server_Num g_simplechat_srv_num;
extern Service_Server_Num g_clusterchat_srv_num;
extern Service_Server_Num g_liantong_srv_num;				//��ͨר�÷���������
extern Service_Server_Num g_wanwei_loginsrv_num;			//��ά��¼����������

extern Server_Conf_Info g_srv_conf_info;


/*��ſͻ�����Ϣ�Ļ����(���ù��б�)
  *����socket id,  ֵ �ǿͻ��˵���Ϣ��
  *�˹�ϣ����Ҫ��Ϊ�˰ѿͻ��˵���Ϣ��������ȫ
  */
extern StMsgBuf_HashTable g_msgbuf_hashtable;


/*��Ϣ������(������������������Ϊ��ϣ��ļ�,
   *��ϣ���ֵΪ�ͻ�����Ϣ����Ϣ, ��Ҫ��Ϊ���ܹ�ͨ����ֵ����ȡ��
   *�ͻ�����Ϣ����Ϣ��)
   *
   */
  
Msg_Counter g_msg_counter;

static volatile sig_atomic_t handle_sig_hup = 0;

enum FORK_TYPE
{
	FORK_UNKNOWN_TYPE,
	FORK_MAIN_SRV,
	FORK_SND_SERVICE_SRV,
	FORK_SND_CLIENT
};

int ProcessEpollMode(void)
{
	INFO("ProcessEpollMode: func begin%s", "");
	
	int nRet = 0;

	//����epoll
	nRet = CreateEpoll();
	if (FALSE == nRet)
	{
		ERROR("ProcessEpollMode: Call CreateEpoll error%s", "");
		return FALSE;
	}

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	WORD wPort = g_srv_conf_info.wPort;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	int nListen_sock = 0;

	//��ʼ������socket
	if (FALSE == InitListenSockInfo(&nListen_sock, wPort))
	{
		ERROR("ProcessEpollMode: Call InitListenSockInfo error%s", "");
		return FALSE;	
	}

	INFO("ProcessEpollMode: socket is listening now...%s", "");
	
	pthread_t thread_id = 0;
	INFO("ProcessEpollMode: [max worker]=%d", g_srv_conf_info.iMaxWorker);
	int numChilds = g_srv_conf_info.iMaxWorker;
	if (numChilds > 0)
	{
		int iChild = 0;
		while (!iChild)
		{
			INFO("ProcessEpollMode: [child cnt]=%d", numChilds);
			if (numChilds > 0)
			{
				switch (fork())
				{
					case -1:
						return -1;
					case 0:
						INFO("ProcessEpollMode: child%s", "");
						iChild = 1;
						break;
					default:
						INFO("ProcessEpollMode: parent create child%s", "");
						numChilds--;
						break;
				}
			}
			else
			{
				int iStatis = 0;
				if (-1 != wait(&iStatis))
				{
					INFO("ProcessEpollMode: parent create child finish after wait%s", "");
					numChilds++;
				}
				else
				{

					switch (errno)
					{
						case EINTR:
							break;
						default:
							break;
					}
				}
			}
		}
	}

	

	//����·��ά���߳�
	nRet= pthread_create(&thread_id, NULL, RoutingMaintainThread, NULL);
	if (nRet != 0)
	{
		ERROR("ProcessEpollMode: Call pthread_create error%s", "");
		return FALSE;
	}	

	int nSndmsgto_servicesrv_threadnum = 0;
	int nSndmsgto_client_threadnum = 0;

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	nSndmsgto_servicesrv_threadnum = g_srv_conf_info.nSndmsgto_servicesrv_threadnum;
	nSndmsgto_client_threadnum = g_srv_conf_info.nSndmsgto_client_threadnum;
	DEBUG("ProcessEpollMode: [snd msg to service server threadnum]=%d", nSndmsgto_servicesrv_threadnum);
	DEBUG("ProcessEpollMode: [snd msg to client threadnum]=%d", nSndmsgto_client_threadnum);
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	int nSnd_thread_num = 0;
	//����������Ϣ�߳�
	while (nSnd_thread_num < nSndmsgto_servicesrv_threadnum)
	{
		nRet= pthread_create(&thread_id, NULL, SndMsgToServiceServerThread, NULL);
		if (nRet != 0)
		{
			ERROR("ProcessEpollMode: Call pthread_create error sleep 2s and then create thread again%s", "");
			sleep(2);
			continue;
		}

		nSnd_thread_num++;
	}

	nSnd_thread_num = 0;
	while (nSnd_thread_num < nSndmsgto_client_threadnum)
	{
		nRet= pthread_create(&thread_id, NULL, SndMsgToClientThread, NULL);
		if (nRet != 0)
		{
			ERROR("ProcessEpollMode: Call pthread_create error sleep 2s and then create thread again%s", "");
			sleep(2);
			continue;
		}

		nSnd_thread_num++;
	}

	//��������ά���߳�
	nRet= pthread_create(&thread_id, NULL, RunMaintainThread, NULL);
	if (nRet!= 0)
	{
		ERROR("ProcessEpollMode: Call pthread_create error%s", "");
		return FALSE;
	}

	//������ʱ��⺯���߳�
	nRet= pthread_create(&thread_id, NULL, CheckTimeoutMsg, NULL);
	if (nRet!= 0)
	{
		ERROR("ProcessEpollMode: Call pthread_create error%s", "");
		return FALSE;
	}

	nRet= pthread_create(&thread_id, NULL, HeartBeatDetectThread, NULL);
	if (nRet!= 0)
	{
		ERROR("ProcessEpollMode: Call pthread_create error%s", "");
		return FALSE;
	}

		//�����̳߳�
	memset(&g_t_recv_client_msg, 0, sizeof(g_t_recv_client_msg));
	nRet = CreateThreadPool(&g_t_recv_client_msg, RcvMsgFromClient);
	if (FALSE == nRet)
	{
		ERROR("ProcessEpollMode: Call CreateThreadPool error%s", "");
		return FALSE;
	}

	//����ͻ��˵�����(����������������ݵ�����Ϣ)
	ProcessClientRequest(nListen_sock);

	INFO("ProcessEpollMode: func end%s", "");
	return TRUE;
}

#if 0
int ProcessSimpleMode(void)
{
	int nListen_sock = 0;
	int *pClient_sock = NULL;
	int nRet = 0;
	pthread_t thread_id = 0;
	pthread_t s_thread_id = 0;
	int nMax_thread_num = 0;

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	nMax_thread_num = g_srv_conf_info.nMax_clientmsg_thread_num;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	WORD wPort = g_srv_conf_info.wPort;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	if (FALSE == S_InitListenSockInfo(&nListen_sock, wPort))
	{
		ERROR("StartServer: Call InitListenSockInfo error%s", "");
		return FALSE;	
	}

	DEBUG("StartServer: listening...%s", "");


	nRet= pthread_create(&thread_id, NULL, (void *)RoutingMaintainThread, NULL);
	if (nRet!= 0)
	{
		ERROR("StartServer: create RoutingMaintainThread thread error%s", "");
		return FALSE;
	}	

	//��������ά���߳�
	nRet= pthread_create(&thread_id, NULL, (void *)RunMaintainThread, NULL);
	if (nRet!= 0)
	{
		ERROR("StartServer: Call RunMaintainThread error%s", "");
		return FALSE;
	}

	nRet= pthread_create(&thread_id, NULL, (void *)S_SndMsgToServer, NULL);
	if (nRet!= 0)
	{
		ERROR("StartServer: Call pthread_create error%s", "");
		return FALSE;
	}

	nRet = pthread_create(&s_thread_id, NULL, (void *)S_HeartBeatDetectThread, NULL);
	if (nRet != 0)
	{
		ERROR("StartServer: Call pthread_create error%s", "");
		return FALSE;
	}

	nRet= pthread_create(&thread_id, NULL, (void *)CheckTimeoutMsg, NULL);
	if (nRet!= 0)
	{
		ERROR("StartServer: Call pthread_create error%s", "");
		return FALSE;
	}	
	
	while (1)
	{
		pClient_sock = (int *)MM_MALLOC_WITH_DESC(sizeof(int), \
		"StartServer: Call func for accept socket");
		if (NULL == pClient_sock)
		{
			FATAL("StartServer: Call malloc error%s", "");
			return FALSE;

		}
		
		nRet = Accept(nListen_sock, pClient_sock);
		if (FALSE == nRet)
		{	
			ERROR("StartServer: accept error%s", "");
			return FALSE;

		}

		DEBUG("main: accept succeed%s", "");
		
		nRet= pthread_create(&thread_id, NULL, (void *)S_RcvMsgFromClient, (void *)pClient_sock);

		//���ܴ������߳�����̫��, ���߳���Դ�ͷ���,  �ټ���ִ�г���
		if (nRet!= 0)
		{
			ERROR("StartServer: Call pthread_create error, wait 10s, then Call pthread_create again%s", "");
			close(*pClient_sock);
			MM_FREE(pClient_sock);
			
			sleep(10);
			continue;
		}	

		S_InsertHeartbeatDetectItem(*pClient_sock, thread_id);
	}
}
#else

int WriteFd(int iFd, int iSndFd)
{
	TRACE();
	struct msghdr   msg;   
    struct iovec    iov[1];   
  
#ifdef  HAVE_MSGHDR_MSG_CONTROL   
    union {   
      struct cmsghdr    cm;   
      char              control[CMSG_SPACE(sizeof(int))];   
    } control_un;   
    struct cmsghdr  *cmptr;   
  
    msg.msg_control = control_un.control;   
    msg.msg_controllen = sizeof(control_un.control);   

	char c = '\0';   
    cmptr = CMSG_FIRSTHDR(&msg);   
    cmptr->cmsg_len = CMSG_LEN(sizeof(int));   
    cmptr->cmsg_level = SOL_SOCKET;   
    cmptr->cmsg_type = SCM_RIGHTS; 
	INFO("WriteFd: [snd fd]=%d", iSndFd);
    *((int *) CMSG_DATA(cmptr)) = iSndFd;   
#else   
    msg.msg_accrights = (caddr_t) &iSndFd;   
    msg.msg_accrightslen = sizeof(int);   
#endif   
  
    msg.msg_name = NULL;   
    msg.msg_namelen = 0;   
  
    iov[0].iov_base = &c;   
    iov[0].iov_len = 1;   
    msg.msg_iov = iov;   
    msg.msg_iovlen = 1;   
  
    return(sendmsg(iFd, &msg, 0));
};

int ProcessSimpleMode(void)
{
	int nListen_sock = 0;
	int nRet = 0;
	pthread_t thread_id = 0;
	pthread_t s_thread_id = 0;
	int nMax_thread_num = 0;
	int iCliSock = 0;
	enum FORK_TYPE forkType;

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	nMax_thread_num = g_srv_conf_info.nMax_clientmsg_thread_num;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	WORD wPort = g_srv_conf_info.wPort;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	if (FALSE == S_InitListenSockInfo(&nListen_sock, wPort))
	{
		ERROR("StartServer: Call InitListenSockInfo error%s", "");
		return FALSE;	
	}

	DEBUG("StartServer: listening...%s", "");


	nRet= pthread_create(&thread_id, NULL, RoutingMaintainThread, NULL);
	if (nRet!= 0)
	{
		ERROR("StartServer: create RoutingMaintainThread thread error%s", "");
		return FALSE;
	}	

	//��������ά���߳�
	nRet= pthread_create(&thread_id, NULL, RunMaintainThread, NULL);
	if (nRet!= 0)
	{
		ERROR("StartServer: Call RunMaintainThread error%s", "");
		return FALSE;
	}

	#if 0
	nRet= pthread_create(&thread_id, NULL, (void *)S_SndMsgToServer, NULL);
	if (nRet!= 0)
	{
		ERROR("StartServer: Call pthread_create error%s", "");
		return FALSE;
	}
	#endif

	nRet = pthread_create(&s_thread_id, NULL, S_HeartBeatDetectThread, NULL);
	if (nRet != 0)
	{
		ERROR("StartServer: Call pthread_create error%s", "");
		return FALSE;
	}

	nRet= pthread_create(&thread_id, NULL, CheckTimeoutMsg, NULL);
	if (nRet!= 0)
	{
		ERROR("StartServer: Call pthread_create error%s", "");
		return FALSE;
	}	

	int sockfd[2] = {0};   
	socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd);   

	sleep(2);
	int j = 0;
	for (j=0; j<1; j++)
	{
		nRet = fork();
		if (0 == nRet)
		{
			forkType = FORK_SND_SERVICE_SRV;
			goto EXECUTE_CHILD_FORK;
		}
	}

	#if 1
	for (j=0; j<1; j++)
	{
		nRet = fork();
		if (0 == nRet)
		{
			forkType = FORK_SND_CLIENT;
			goto EXECUTE_CHILD_FORK;
		}
	}
	#endif

	forkType = FORK_UNKNOWN_TYPE;
	
	while (1)
	{			
		nRet = Accept(nListen_sock, &iCliSock);
		INFO("StartServer: [client socket]=%d", iCliSock);
		
		if (FALSE == nRet)
		{	
			ERROR("StartServer: accept error%s", "");
			return FALSE;

		}

		DEBUG("StartServer: accept succeed%s", "");
		nRet = fork();
		if (0 == nRet)
		{
			close(nListen_sock);
			S_RcvMsgFromClient((void *)&iCliSock, sockfd[1]);
			exit(1);
		}
		else if (nRet < 0)
		{
			ERROR("ProcessSimpleMode: Call fork error");
			close(iCliSock);
		}

		WriteFd(sockfd[0], iCliSock);

		//close(iCliSock);
	}

	EXECUTE_CHILD_FORK:
	if (FORK_SND_SERVICE_SRV == forkType)
	{
		S_SndMsgToServer();
	}
	else if (FORK_SND_CLIENT == forkType)
	{
		INFO("StartServer: before SndMsgToCli");
		SndMsgToCli(sockfd[1]);
		INFO("StartServer: after SndMsgToCli");
	}
	
	return 1;
}

#endif

//������;:  �ú�����Ҫ��������������Ŀ��Ҫ��������
//�������:  ��
//�������:  ��
//����ֵ	: �����ɹ�,  ����TRUE,  ����ʧ��,  ����FALSE

int StartServer(void)
{	
	INFO("StartServer: func begin%s", "");
	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	int nServer_mode = g_srv_conf_info.nServer_mode;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	DEBUG("StartServer: server mode=%d", nServer_mode);

	if (0 == nServer_mode)
	{
		INFO("StartServer: server will use the simple mode%s", "");
		ProcessSimpleMode();
	}
	else if (1 == nServer_mode)
	{
		INFO("StartServer: server will use the epoll mode%s", "");
		ProcessEpollMode();				
	}

	INFO("StartServer: func end%s", "");
	return TRUE;
}

