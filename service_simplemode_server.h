
#ifndef SERVICE_SIMPLEMODE_SERVER
#define SERVICE_SIMPLEMODE_SERVER

#define MAX_ACCESS_SRV_NUM 400

#define MAX_THREAD_SIZE 2048					//最大的线程数(包括发送和接收线程)


//发送和接收线程的ID结构
typedef struct
{
	pthread_t snd_id;					//发送线程的ID
	pthread_t recv_id;					//接收线程的ID
}Threads_Id;

typedef struct
{
	Threads_Id arrThreads_id[MAX_ACCESS_SRV_NUM];
	int nID_counter;
	pthread_mutex_t id_mutex; 
}StThread_Ids_Info;

typedef struct
{
	pthread_t thread_id; 
	BYTE bIsproc_thread_exit;
}StThread_Exit;

typedef struct
{
	pthread_mutex_t thread_mutex;
	int nThread_num;
	StThread_Exit arrThread_exit[1000];
}StThreads_Exit_Info;

//发送和接收线程的参数结构
typedef struct
{
	int nSock;									//socket id
	StThreads_Exit_Info threads_exit_info;
	StSequence_Queue queue;							//消息队列
	pthread_mutex_t mutex;						//互斥变量
	pthread_cond_t cond;						//同步变量
}Snd_Recv_Param;

//用来保存整个线程信息的结构
typedef struct
{
	Threads_Id arrThreads_id[MAX_THREAD_SIZE / 2];  	 //存放线程ID的数组
	Snd_Recv_Param *pKeep_param[MAX_THREAD_SIZE/2];		//发送接收线程参数
	unsigned int nID_counter;			  				//线程计数器
	pthread_mutex_t thread_mutex;					 	//对线程信息加锁
}Threads_Info;

//对当前处理线程总数进行计数
typedef struct
{
	pthread_mutex_t counter_mutex;			//计数器互斥变量
	unsigned int nThread_counter;			//线程计数器
}Process_Thread_Counter;

int RunSimpleModeServer(void);

void SetCancelThreadFlg(void);

#endif
