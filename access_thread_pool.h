
#ifndef ACCESS_THREAD_POOL_H
#define ACCESS_THREAD_POOL_H

#define CLIENT_SOCK_EXIST 0x02

//最少线程数量
#define MIN_THREAD_NUM 10

//最大线程数量
#define MAX_THREAD_NUM 20

//线程在用与否标志
typedef enum
{
	THREAD_USE,					//线程在使用
	THREAD_NOT_USE				//线程没有在使用
}En_Thread_Usr;


//线程信息
typedef struct
{
	En_Thread_Usr en_thread_usr;	//线程使用标志
	pthread_t thread_id;			//线程ID
	pthread_cond_t thread_cond;		//线程同步变量
	pthread_mutex_t thread_mutex;	//线程互斥变量

	int (*pProcessFunc)(int);		//线程函数
	int nSock;						//socket  id
}Thread_Info;


//线程池信息
typedef struct Thread_Pool_S
{
	int (*pInitThreadPool)(struct Thread_Pool_S *);			//初始化线程池
	int (*pGetThreadSeqById)(int, struct Thread_Pool_S *);		//获取线程序号

	int nMin_thread_num; 		//线程池里面线程的最小数量
	int nCur_thread_num;		//线程池里面当前的线程数量
	int nMax_thread_num;		//线程池里面最大的线程数量

	pthread_mutex_t thread_pool_mutex;		//线程池互斥变量
	Thread_Info *pThread_info;	//线程信息
	
}Thread_Pool;


//创建线程池
int CreateThreadPool(Thread_Pool *pThread_pool, int (*pProcessFunc)(int));

//获取线程
int GetThread(int nSock);

//释放线程
int ReleaseThread(void);

int HandleClientSock(int nSock, StTmp_clientSock_Hash *pTmp_clientsock_hash);

//初始化线程池
int InitThreadPool(Thread_Pool *pThread_pool);
#endif
