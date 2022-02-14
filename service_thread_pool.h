
#ifndef SERVICE_THREAD_POOL_H
#define SERVICE_THREAD_POOL_H

#include "../util/service_global.h"
#include "../interface/service_protocol.h"
#include "../include/sequence_queue.h"

typedef struct
{
	StSequence_Queue queue;
	pthread_mutex_t mutex;						//互斥变量
	pthread_cond_t cond;						//同步变量
}StQueue;

//线程在用与否标志
typedef enum
{
	THREAD_USE,					//线程在使用
	THREAD_NOT_USE				//线程没有在使用
}En_Thread_Usr;

typedef enum
{
	THREAD_NOT_EXIST,
	THREAD_NOT_EXIT,
	THREAD_EXIT
}EnThread_Status;

//线程信息
typedef struct
{
	En_Thread_Usr en_thread_usr;	//线程使用标志
	pthread_t thread_id;			//线程ID
	pthread_cond_t thread_cond;		//线程同步变量
	pthread_mutex_t thread_mutex;	//线程互斥变量
	BYTE bIs_thread_exit;
	EnThread_Status enThread_status;
	int (*pProcessFunc)(void *, StQueue *);		//线程函数
	int nSock;						//socket  id
}Thread_Info;


//线程池信息
typedef struct Thread_Pool_S
{
	int (*pInitThreadPool)(int, struct Thread_Pool_S *);			//初始化线程池
	int (*pGetThreadSeqById)(int nId, struct Thread_Pool_S *);		//获取线程序号

	int nMin_thread_num; 		//线程池里面线程的最小数量
	int nCur_thread_num;		//线程池里面当前的线程数量
	int nMax_thread_num;		//线程池里面最大的线程数量

	pthread_mutex_t thread_pool_mutex;		//线程池互斥变量
	Thread_Info *pThread_info;	//线程信息
	
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


//创建线程池
int CreateThreadPool(int (*pProcessFunc)(void *, StQueue *), int);

//初始化线程池
int InitThreadPool(int nSock_id, Thread_Pool *pThread_pool);

StThread_Resource *GetThreadResourceFromHash(int nSock_id);

#endif
