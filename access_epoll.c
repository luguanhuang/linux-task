
#include "../util/access_global.h"
//#include "../include/ydt_log.h"
#include "access_sock_info.h"
#include "access_heartbeat_detectthread.h"
#include "access_epoll.h"

/*���ļ���Ҫ����������epoll��, ��ģʽ��Ҫ����ͻ��˵Ĵ󲢷����ӵ�
  *epoll ��Ҫ��ET��LE����ģʽ,   ETģʽ�Ǹ��ٹ���ģʽ,  ֻ֧�ַ�����ģʽ
  *������ģʽ��, ����������δ������ɾ���ʱ,  �ں�ͨ��epoll ������,
  *Ȼ�����������֪���ļ��������Ѿ�����,  ���Ҳ�����Ϊ�Ǹ��ļ����������͸���ĵľ���
  *֪ͨ, ֱ��������ĳЩ���������Ǹ��ļ�����������Ϊ����״̬
  */

//epoll�ṹ
extern Epoll_Fd g_epoll_fd;

extern Server_Conf_Info g_srv_conf_info;

extern StClient_Sock_Queue g_clientsocket_queue;

#define MAX_ACCEPT_NUM 4

//������;:  ����epoll
//�������:  ��
//�������:  ��
//����ֵ	: �����ɹ�,  ����TRUE,  ����ʧ��,  ����FALSE

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


//������;:  ��epoll����(ע��)��epoll��
//�������:  ���뵽epoll �е�socket  id
//�������:  ��
//����ֵ	: ����ɹ�,  ����TRUE,   ����ʧ��,  ����FALSE

int PutSockIntoEpoll(int nSock, uint32_t event_type)
{
	INFO("PutSockIntoEpoll: func begin%s", "");
	pthread_mutex_lock(&g_epoll_fd.epoll_mutex);
	int nRet = 0;
	struct epoll_event ev = {0, {0}};
	//��socket����epoll��
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
//������;: �޸�socket ���¼�����
//�������:  �޸ĵ�socket �¼����͵� socket id
//�������:  ��
//����ֵ	: �޸ĳɹ�,  ����TRUE,   �޸�ʧ��,  ����FALSE
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

//��socket �����epoll��ɾ��
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

//������;:  ����ͻ��˵�������������ݽ�������
//�������:  ����socket  id
//�������:  ��
//����ֵ	:  ����ɹ�,  ����TRUE,  ����ʧ��,   ����FALSE

int ProcessClientRequest(int nListen_sock)
{
	INFO("ProcessClientRequest: func begin%s", "");
	int nFds = 0;
	int i = 0;
	int nClient_sock = 0;
	int nRet = 0;
	int nSock_fd = 0;
	uint32_t event_type = 0;			//�¼�����
	int nConn_counter = 0;
	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	int nEpoll_size = g_srv_conf_info.nEpoll_size;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	INFO("ProcessClientRequest: [epoll size]=%d", nEpoll_size);
	
	//�¼��ṹ
	struct epoll_event events[nEpoll_size];
	memset(events, 0, sizeof(events));
	  /*����ǿͻ��˵�������Ϣ,	���������Ϣ����epoll��
	  *����ǽ��տͻ�����Ϣ��,	�Ͱ���Ӧ��socket id ����socket ��Ϣ������
	  *�������̳߳�������߳���ȡ����Ϣ���������socket  �����ݵõ���socket
	  *�����տͻ��˵���Ϣ
	  */
	while (1)
	{	
		//�ȴ���Ϣ�ĵ���
		nFds = epoll_wait(g_epoll_fd.nEp_fd, events, nEpoll_size, -1);
		for (i=0; i<nFds; i++)
		{
			//����ǽ���������Ϣ
			if ((events[i].data.fd == nListen_sock) && (events[i].events & EPOLLIN))
			{	  
				nConn_counter = 0;
				//ͬһ��������������ж�������ӵ���Ϣ������������Ŷ�(����epollֻ����һ��)
				//����Ҫ���ϵ�ѭ�������տͻ��˵���������
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
					//����socketΪ������״̬
					nRet = SetNonBlocking(nClient_sock);
					if (FALSE == nRet)			//���ʧ��,  �Ͳ���������epoll��
					{
						ERROR("ProcessClientRequest: Call SetNonBlocking error%s", "");	
						break;
					}

					//��socket���ע�ᵽ(����) epoll��
					event_type = EPOLLIN | EPOLLET;
					nRet = PutSockIntoEpoll(nClient_sock, event_type);
					if (FALSE == nRet)					//���ע��ʧ��,  �Ͳ���Ҫ����������
					{
						ERROR("ProcessClientRequest: Call PutSockIntoEpoll error%s", "");
						break;
					}

					//����������,  �������ʧ��,  �Ͱ�sock ��epoll��ɾ����
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
			else if (events[i].events & EPOLLIN)		//����ǽ��տͻ�����Ϣ
			{
				INFO("ProcessClientRequest: epoll event show we have recv client msg request%s", "");
				nSock_fd = events[i].data.fd;
				if (nSock_fd < 0)
				{
					INFO("ProcessClientRequest: socket=%d socket is not in the range and then discard this client request", nSock_fd);
					continue;	
				}	
				
				//�ѿͻ��˵�socket������Ϣ������
				nRet = InsertItemToQueue(&g_clientsocket_queue.stLink_queue, nSock_fd);
				if (TRUE != nRet)
				{
					ERROR("ProcessClientRequest: Call InsertItemToQueue error%s", "");
					continue;
				}
				
				pthread_cond_signal(&g_clientsocket_queue.queue_cond);  
				//���ڵ�nSock_fd����ڴ����¼�����,  ��������Ҫ��EPOLL_CTL_MOD��,  ���Ƕ�д�󽫸�
				//���modifyһ������������ȶ���,  �ر�����ETģʽ��
				//event_type = EPOLLIN | EPOLLET | EPOLLONESHOT;
				//ModifySockEvent(nSock_fd, event_type);

				//�����߳������տͻ��˵�������Ϣ
				//PrintContent("StartServer: EPOLLIN\n");
				//GetThread(nSock_fd);	
			}
		}
	}

	INFO("ProcessClientRequest: func end%s", "");
	return TRUE;
}

