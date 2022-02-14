
#ifndef ACCESS_EPOLL_H_H	
#define ACCESS_EPOLL_H_H

#include "../include/link_queue.h"

//epoll��ע������socket����
#define EPOLL_MAX_SOCKET_NUM 10000

//�ɹ�ע�������¼�������
#define MAX_EVENT_NUM 10000

//epoll�ṹ
typedef struct
{
	int nEp_fd;								//epoll id
	pthread_mutex_t epoll_mutex;			//epoll �������
}Epoll_Fd;

typedef struct
{	
	pthread_mutex_t queue_mutex;					//���л������
	pthread_cond_t queue_cond;						//����ͬ������
	StLinkQueue stLink_queue;
}StClient_Sock_Queue;

int CreateEpoll(void);

int ProcessClientRequest(int nListen_sock);

int PutSockIntoEpoll(int nSock, uint32_t event_type);

int DelSockEventFromepoll(int nSock);

#endif
