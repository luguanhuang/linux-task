
#ifndef ACCESS_EPOLL_H_H	
#define ACCESS_EPOLL_H_H

#include "../include/link_queue.h"

//epoll关注的最大的socket数量
#define EPOLL_MAX_SOCKET_NUM 10000

//可关注的最大的事件的数量
#define MAX_EVENT_NUM 10000

//epoll结构
typedef struct
{
	int nEp_fd;								//epoll id
	pthread_mutex_t epoll_mutex;			//epoll 互斥变量
}Epoll_Fd;

typedef struct
{	
	pthread_mutex_t queue_mutex;					//队列互斥变量
	pthread_cond_t queue_cond;						//队列同步变量
	StLinkQueue stLink_queue;
}StClient_Sock_Queue;

int CreateEpoll(void);

int ProcessClientRequest(int nListen_sock);

int PutSockIntoEpoll(int nSock, uint32_t event_type);

int DelSockEventFromepoll(int nSock);

#endif
