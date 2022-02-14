
#include "../util/access_global.h"
//#include "../include/ydt_log.h"
#include "access_sock_info.h"
#include "access_heartbeat_detectthread.h"
#include "access_epoll.h"

/*该文件主要是用来操作epoll的, 该模式主要处理客户端的大并发连接的
  *epoll 主要分ET和LE两种模式,   ET模式是高速工作模式,  只支持非阻塞模式
  *在这种模式下, 当描述符从未就绪变成就绪时,  内核通过epoll 告诉你,
  *然后它会假设你知道文件描述符已经就绪,  并且不会再为那个文件描述符发送更多的的就绪
  *通知, 直到你做了某些操作导致那个文件描述符不再为就绪状态
  */

//epoll结构
extern Epoll_Fd g_epoll_fd;

extern Server_Conf_Info g_srv_conf_info;

extern StClient_Sock_Queue g_clientsocket_queue;

#define MAX_ACCEPT_NUM 4

//函数用途:  创建epoll
//输入参数:  无
//输出参数:  无
//返回值	: 创建成功,  返回TRUE,  创建失败,  返回FALSE

int CreateEpoll(void)
{
	INFO("CreateEpoll: func begin%s", "");
	pthread_mutex_lock(&g_epoll_fd.epoll_mutex);
	int nEpoll_size = g_srv_conf_info.nEpoll_size;
	g_epoll_fd.nEp_fd = epoll_create(nEpoll_size);
	DEBUG("CreateEpoll; epoll size=%d [epoll fd]=%d", \
		nEpoll_size, g_epoll_fd.nEp_fd);
	if (g_epoll_fd.nEp_fd <= 0)
	{
		ERROR("CreateEpoll: Call epoll_create error error[%d]=%s", \
			errno, strerror(errno));
		pthread_mutex_unlock(&g_epoll_fd.epoll_mutex);
		return FALSE;
	}
	
	pthread_mutex_unlock(&g_epoll_fd.epoll_mutex);

	INFO("CreateEpoll: func end%s", "");
	return TRUE;
}


//函数用途:  把epoll加入(注册)到epoll中
//输入参数:  加入到epoll 中的socket  id
//输出参数:  无
//返回值	: 加入成功,  返回TRUE,   加入失败,  返回FALSE

int PutSockIntoEpoll(int nSock, uint32_t event_type)
{
	INFO("PutSockIntoEpoll: func begin%s", "");
	pthread_mutex_lock(&g_epoll_fd.epoll_mutex);
	int nRet = 0;
	struct epoll_event ev = {0, {0}};
	//把socket放入epoll中
	ev.events = event_type;
	ev.data.fd = nSock;
	nRet = epoll_ctl(g_epoll_fd.nEp_fd, EPOLL_CTL_ADD, nSock, &ev);	
	if (nRet < 0)
	{
		ERROR("PutSockIntoEpoll: Call epoll_ctl error [return value]=%d error[%d]=%s", nRet, errno, strerror(errno));
		pthread_mutex_unlock(&g_epoll_fd.epoll_mutex);
		return FALSE;
	}
	
	pthread_mutex_unlock(&g_epoll_fd.epoll_mutex);

	INFO("PutSockIntoEpoll: func end%s", "");
	return TRUE;
}

#if 0
//函数用途: 修改socket 的事件类型
//输入参数:  修改的socket 事件类型的 socket id
//输出参数:  无
//返回值	: 修改成功,  返回TRUE,   修改失败,  返回FALSE
int ModifySockEvent(int nSock, uint32_t event_type)
{
	pthread_mutex_lock(&g_epoll_fd.epoll_mutex);
	struct epoll_event ev = {0, {0}};
	ev.events = event_type;
	ev.data.fd = nSock;
	if (epoll_ctl(g_epoll_fd.nEp_fd, EPOLL_CTL_MOD, nSock, &ev) < 0)
	{
		ERROR("ModifySockEvent: Call epoll_ctl error errno=%d", errno);
		pthread_mutex_unlock(&g_epoll_fd.epoll_mutex);
		return FALSE;
	}
	
	pthread_mutex_unlock(&g_epoll_fd.epoll_mutex);
	return TRUE;
}
#endif

//把socket 句柄从epoll中删除
int DelSockEventFromepoll(int nSock)
{
	INFO("DelSockEventFromepoll: func begin%s", "");
	pthread_mutex_lock(&g_epoll_fd.epoll_mutex);
	struct epoll_event ev = {0, {0}};
	int nRet = 0;

	nRet = epoll_ctl(g_epoll_fd.nEp_fd, EPOLL_CTL_DEL, nSock, &ev);	
	if (nRet < 0)
	{
		pthread_mutex_unlock(&g_epoll_fd.epoll_mutex);
		ERROR("DelSockEventFromepoll: Call epoll_ctl error%s", "");
		INFO("DelSockEventFromepoll: func end%s", "");
		return FALSE;
	}
	else
	{
		pthread_mutex_unlock(&g_epoll_fd.epoll_mutex);
		INFO("DelSockEventFromepoll: delete socket from epoll succeed%s", "");
		INFO("DelSockEventFromepoll: func end%s", "");
		return TRUE;	
	}
}

//函数用途:  处理客户端的连接请求和数据接收请求
//输入参数:  侦听socket  id
//输出参数:  无
//返回值	:  处理成功,  返回TRUE,  处理失败,   返回FALSE

int ProcessClientRequest(int nListen_sock)
{
	INFO("ProcessClientRequest: func begin%s", "");
	int nFds = 0;
	int i = 0;
	int nClient_sock = 0;
	int nRet = 0;
	int nSock_fd = 0;
	uint32_t event_type = 0;			//事件类型
	int nConn_counter = 0;
	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	int nEpoll_size = g_srv_conf_info.nEpoll_size;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	INFO("ProcessClientRequest: [epoll size]=%d", nEpoll_size);
	
	//事件结构
	struct epoll_event events[nEpoll_size];
	memset(events, 0, sizeof(events));
	  /*如果是客户端的连接消息,	否把连接信息放入epoll中
	  *如果是接收客户端消息的,	就把相应的socket id 放入socket 消息队列中
	  *并触发线程池里面的线程来取出消息队列里面的socket  并根据得到的socket
	  *来接收客户端的消息
	  */
	while (1)
	{	
		//等待消息的到来
		nFds = epoll_wait(g_epoll_fd.nEp_fd, events, nEpoll_size, -1);
		for (i=0; i<nFds; i++)
		{
			//如果是接入连接消息
			if ((events[i].data.fd == nListen_sock) && (events[i].events & EPOLLIN))
			{	  
				nConn_counter = 0;
				//同一个连接请求可能有多个在连接的消息队列里面进行排队(但是epoll只触发一次)
				//所以要不断地循环来接收客户端的连接请求
				while (TRUE)
				{
					nRet = Accept(nListen_sock, &nClient_sock);
					if (SERVER_ACCEPT_FINISH == nRet)
					{
						INFO("ProcessClientRequest: now we finish to accept client connection request%s", "");
						break;

					}
					else if (FALSE == nRet)
					{
						ERROR("ProcessClientRequest: Call Accept error wait 2s and then accept client connect again%s", "");
						nConn_counter++;
						if (nConn_counter >= MAX_ACCEPT_NUM)
						{
							ERROR("ProcessClientRequest: accept client connect num exceed max connect num [conn_counter]=%d", \
								nConn_counter);
							break;	
						}
						
						sleep(2);
						continue;	
					}

					INFO("ProcessClientRequest: accept client connection succeed%s", "");
					//设置socket为非阻塞状态
					nRet = SetNonBlocking(nClient_sock);
					if (FALSE == nRet)			//如果失败,  就不把它放入epoll中
					{
						ERROR("ProcessClientRequest: Call SetNonBlocking error%s", "");	
						break;
					}

					//把socket句柄注册到(放入) epoll中
					event_type = EPOLLIN | EPOLLET;
					nRet = PutSockIntoEpoll(nClient_sock, event_type);
					if (FALSE == nRet)					//如果注册失败,  就不需要加入心跳包
					{
						ERROR("ProcessClientRequest: Call PutSockIntoEpoll error%s", "");
						break;
					}

					//插入心跳包,  如果插入失败,  就把sock 从epoll中删除掉
					nRet = InsertHeartbeatDetectItem(nClient_sock);
					if (TRUE != nRet)
					{
						ERROR("ProcessClientRequest: Call InsertHeartbeatDetectItem error%s", "");
						nRet = DelSockEventFromepoll(nClient_sock);
						if (FALSE == nRet)
						{
							ERROR("ProcessClientRequest: Call DelSockEventFromepoll error%s", "");		
						}
					}
				}
			}
			else if (events[i].events & EPOLLIN)		//如果是接收客户端消息
			{
				INFO("ProcessClientRequest: epoll event show we have recv client msg request%s", "");
				nSock_fd = events[i].data.fd;
				if (nSock_fd < 0)
				{
					INFO("ProcessClientRequest: socket=%d socket is not in the range and then discard this client request", nSock_fd);
					continue;	
				}	
				
				//把客户端的socket放入消息队列中
				nRet = InsertItemToQueue(&g_clientsocket_queue.stLink_queue, nSock_fd);
				if (TRUE != nRet)
				{
					ERROR("ProcessClientRequest: Call InsertItemToQueue error%s", "");
					continue;
				}
				
				pthread_cond_signal(&g_clientsocket_queue.queue_cond);  
				//现在的nSock_fd句柄期待的事件不变,  本来不需要用EPOLL_CTL_MOD的,  但是读写后将该
				//句柄modify一次有助于提高稳定性,  特别是在ET模式下
				//event_type = EPOLLIN | EPOLLET | EPOLLONESHOT;
				//ModifySockEvent(nSock_fd, event_type);

				//唤醒线程来接收客户端到来的消息
				//PrintContent("StartServer: EPOLLIN\n");
				//GetThread(nSock_fd);	
			}
		}
	}

	INFO("ProcessClientRequest: func end%s", "");
	return TRUE;
}

