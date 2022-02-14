
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

/*该文件主要是负责接入服务器通讯使用的
  *该文件中采用epoll模式来处理客户端的大并发连接
  *该模式主要采用ET模式,  也就是边缘触发,  也就是当有
  *消息到来,  只触发一次,  所以我们每次接收客户端的消息的时候
  *要用循环来接收客户端的消息,  直到接收完为止,  在该模式中, 主要采用
  *消息缓存哈希表来接收客户端的消息,  该表是以socket id为键,  客户端数据包为值的
  *该表目前存在的情况是有的时候该表无法动态地删除删除一项,  使得下次再插入哈希表的时候
  *会导致我们认为该数据包已经存在而把该数据包丢弃,  这也是目前本系统不稳定的情况之一
  *还有就是因为epoll中的socket 采用了非阻塞模式,   对于发送缓冲区已满而导致程序不断地返回EAGAIN, 
  *当程序中的数据量过大的话,  会出现莫名的错误,  这个也是目前系统不稳定的情况
  */

#define  HAVE_MSGHDR_MSG_CONTROL

extern int g_iShmId;

extern StRouting_Table g_routing_table;
Thread_Pool g_t_recv_client_msg;

/*客户端的socket哈希表
  *键是接入服务器序号和消息计数器的组合
  *此哈希表主要是用来保存客户端消息包的信息的
  */



//消息队列,  主要是存放客户端传送过来的消息包


//唤醒维护线程标志,  主要是用来唤醒路由维护线程来对业务服务器进行重连的 
Awake_Thread_Flg g_awake_thread_flg;

//epoll结构,  主要用来处理客户端的大并发连接的
//里面主要存放的是socket id
Epoll_Fd g_epoll_fd;


/*选择不同的业务服务器的序号(如果业务服务器有多台,  
  *我们可以选择随机选择其中的一台业务服务器来转发消息包)
  */
extern Service_Server_Seq g_login_srv_seq;
extern Service_Server_Seq g_register_srv_seq;
extern Service_Server_Seq g_simplechat_srv_seq;
extern Service_Server_Seq g_clusterchat_srv_seq;
extern Service_Server_Seq g_liantong_srv_seq;				//联通专用服务器序号
extern Service_Server_Seq g_wanwei_loginsrv_seq;			//万维登录服务器序号

//各种业务服务器数量(业务服务器可能有多台)
extern Service_Server_Num g_login_srv_num;
extern Service_Server_Num g_register_srv_num;;
extern Service_Server_Num g_simplechat_srv_num;
extern Service_Server_Num g_clusterchat_srv_num;
extern Service_Server_Num g_liantong_srv_num;				//联通专用服务器数量
extern Service_Server_Num g_wanwei_loginsrv_num;			//万维登录服务器数量

extern Server_Conf_Info g_srv_conf_info;


/*存放客户端消息的缓冲池(采用哈市表)
  *键是socket id,  值 是客户端的消息包
  *此哈希表主要是为了把客户端的消息包接收完全
  */
extern StMsgBuf_HashTable g_msgbuf_hashtable;


/*消息计数器(与接入服务器合用来作为哈希表的键,
   *哈希表的值为客户端消息包信息, 主要是为了能够通过键值来获取到
   *客户端消息包信息的)
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

	//创建epoll
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

	//初始化侦听socket
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

	

	//创建路由维护线程
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
	//创建发送消息线程
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

	//创建运行维护线程
	nRet= pthread_create(&thread_id, NULL, RunMaintainThread, NULL);
	if (nRet!= 0)
	{
		ERROR("ProcessEpollMode: Call pthread_create error%s", "");
		return FALSE;
	}

	//创建超时检测函数线程
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

		//创建线程池
	memset(&g_t_recv_client_msg, 0, sizeof(g_t_recv_client_msg));
	nRet = CreateThreadPool(&g_t_recv_client_msg, RcvMsgFromClient);
	if (FALSE == nRet)
	{
		ERROR("ProcessEpollMode: Call CreateThreadPool error%s", "");
		return FALSE;
	}

	//处理客户端的请求(如连接请求和有数据到来消息)
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

	//创建运行维护线程
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

		//可能创建的线程数量太多, 等线程资源释放了,  再继续执行程序
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

	//创建运行维护线程
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

//函数用途:  该函数主要用来启动整个项目需要做的事务
//输入参数:  无
//输出参数:  无
//返回值	: 启动成功,  返回TRUE,  启动失败,  返回FALSE

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

