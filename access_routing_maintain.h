	
#ifndef ACCESS_ROUTING_MAINTAIN_H
#define ACCESS_ROUTING_MAINTAIN_H

#include "../util/access_global.h"

/*********该文件主要用来处理路由维护线程的*********/

//接收业务服务器消息的线程参数结构
typedef struct
{
	int nSock;							//接入服务器与业务服务器之间的连接socket id
	BYTE bMain_service_code;			//主业务码
	BYTE bSub_service_code;				//子业务码
	WORD wService_srv_seq;				//业务服务器的序号
}Recv_Thread_Param;

//此结构用来唤醒维护线程来进行业务服务器的重连
typedef struct
{
	pthread_mutex_t awake_flg_mutex;			//唤醒线程互斥变量
	unsigned int nAwake_thread_flg;				//唤醒线程标志
}Awake_Thread_Flg;

typedef struct
{
	pthread_mutex_t routing_mutex;				//路由维护表互斥变量
	StHash_Table *pRouting_table;				//路由维护哈希表	
}StRouting_Table;

//路由维护线程
void *RoutingMaintainThread(void *);

#endif
