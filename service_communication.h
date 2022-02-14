
#ifndef SERVICE_COMMUNICATION_H
#define SERVICE_COMMUNICATION_H

//最大的处理线程数量
#define MAX_PROCESS_THREAD_NUM  400

/********结构定义开始********************/

//发送和接收线程的类型
typedef enum 
{
	SND_THREAD_TYPE,			//发送线程类型
	RECV_THREAD_TYPE			//接收线程类型
}Thread_Type;

#pragma pack(1)
//处理线程的参数结构
typedef struct
{
	int nSeq;
	StSequence_Queue *pQueue;				//消息队列
	StThreads_Exit_Info *pThreads_exit_info;	//线程退出信息
	Client_Server_Msg cli_srv_msg;		//消息结构
	pthread_mutex_t *pProc_mutex;		//互斥变量
	pthread_cond_t *pCond;				//同步变量
}Process_Param;
#pragma pack()


/**********结构定义结束*******************/

//开启服务器
int StartServer(void);

#endif
