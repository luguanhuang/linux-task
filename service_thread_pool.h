
#ifndef SERVICE_THREAD_POOL_H
#define SERVICE_THREAD_POOL_H

#include "../util/service_global.h"
#include "../interface/service_protocol.h"
#include "../include/sequence_queue.h"

typedef struct
{
	StSequence_Queue queue;
	pthread_mutex_t mutex;						//�������
	pthread_cond_t cond;						//ͬ������
}StQueue;

//�߳���������־
typedef enum
{
	THREAD_USE,					//�߳���ʹ��
	THREAD_NOT_USE				//�߳�û����ʹ��
}En_Thread_Usr;

typedef enum
{
	THREAD_NOT_EXIST,
	THREAD_NOT_EXIT,
	THREAD_EXIT
}EnThread_Status;

//�߳���Ϣ
typedef struct
{
	En_Thread_Usr en_thread_usr;	//�߳�ʹ�ñ�־
	pthread_t thread_id;			//�߳�ID
	pthread_cond_t thread_cond;		//�߳�ͬ������
	pthread_mutex_t thread_mutex;	//�̻߳������
	BYTE bIs_thread_exit;
	EnThread_Status enThread_status;
	int (*pProcessFunc)(void *, StQueue *);		//�̺߳���
	int nSock;						//socket  id
}Thread_Info;


//�̳߳���Ϣ
typedef struct Thread_Pool_S
{
	int (*pInitThreadPool)(int, struct Thread_Pool_S *);			//��ʼ���̳߳�
	int (*pGetThreadSeqById)(int nId, struct Thread_Pool_S *);		//��ȡ�߳����

	int nMin_thread_num; 		//�̳߳������̵߳���С����
	int nCur_thread_num;		//�̳߳����浱ǰ���߳�����
	int nMax_thread_num;		//�̳߳����������߳�����

	pthread_mutex_t thread_pool_mutex;		//�̳߳ػ������
	Thread_Info *pThread_info;	//�߳���Ϣ
	
}Thread_Pool;

typedef struct
{
	pthread_t thread_id;
	BYTE bIs_thread_exit;
	EnThread_Status enThread_status;
}StThread_Info;

typedef struct
{
	BYTE arrAcccee_srv_ip[50];
	WORD wAccess_srv_port;
	int nSock_id;	
	pthread_mutex_t sock_mutex;
	StQueue stQueue_in;	 		
	StQueue stQueue_out;	
	pthread_mutex_t res_mutex;
	struct Thread_Pool_S thread_pool;
	StThread_Info snd_thread_info;
	StThread_Info recv_thread_info;
}StThread_Resource;


//�����̳߳�
int CreateThreadPool(int (*pProcessFunc)(void *, StQueue *), int);

//��ʼ���̳߳�
int InitThreadPool(int nSock_id, Thread_Pool *pThread_pool);

StThread_Resource *GetThreadResourceFromHash(int nSock_id);

#endif
