
#ifndef ACCESS_THREAD_POOL_H
#define ACCESS_THREAD_POOL_H

#define CLIENT_SOCK_EXIST 0x02

//�����߳�����
#define MIN_THREAD_NUM 10

//����߳�����
#define MAX_THREAD_NUM 20

//�߳���������־
typedef enum
{
	THREAD_USE,					//�߳���ʹ��
	THREAD_NOT_USE				//�߳�û����ʹ��
}En_Thread_Usr;


//�߳���Ϣ
typedef struct
{
	En_Thread_Usr en_thread_usr;	//�߳�ʹ�ñ�־
	pthread_t thread_id;			//�߳�ID
	pthread_cond_t thread_cond;		//�߳�ͬ������
	pthread_mutex_t thread_mutex;	//�̻߳������

	int (*pProcessFunc)(int);		//�̺߳���
	int nSock;						//socket  id
}Thread_Info;


//�̳߳���Ϣ
typedef struct Thread_Pool_S
{
	int (*pInitThreadPool)(struct Thread_Pool_S *);			//��ʼ���̳߳�
	int (*pGetThreadSeqById)(int, struct Thread_Pool_S *);		//��ȡ�߳����

	int nMin_thread_num; 		//�̳߳������̵߳���С����
	int nCur_thread_num;		//�̳߳����浱ǰ���߳�����
	int nMax_thread_num;		//�̳߳����������߳�����

	pthread_mutex_t thread_pool_mutex;		//�̳߳ػ������
	Thread_Info *pThread_info;	//�߳���Ϣ
	
}Thread_Pool;


//�����̳߳�
int CreateThreadPool(Thread_Pool *pThread_pool, int (*pProcessFunc)(int));

//��ȡ�߳�
int GetThread(int nSock);

//�ͷ��߳�
int ReleaseThread(void);

int HandleClientSock(int nSock, StTmp_clientSock_Hash *pTmp_clientsock_hash);

//��ʼ���̳߳�
int InitThreadPool(Thread_Pool *pThread_pool);
#endif
