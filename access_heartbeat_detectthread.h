#ifndef ACCESS_HEARTBEAT_DETECTTHREAD_H
#define ACCESS_HEARTBEAT_DETECTTHREAD_H

#if 0
//心跳检测哈希表
typedef struct
{
	pthread_mutex_t heartbeat_detect_mutex;
	YDT_HashTable *pHeartbeat_detect_table;
}Heartbeat_Detect_Table;
#endif

typedef struct
{
	pthread_mutex_t heartbeat_detect_mutex;
	StHash_Table *pHeartbeat_detect_table;
}StHeartbeat_Detect_Table;

typedef struct
{
	pthread_t thread_id;
	long lTime;
}StClient_Conn_Info;

void *HeartBeatDetectThread(void *);

int DeleteHeartbeatDetectItem(int nSock);

int DeleteHeartbeatDetectItem(int nSock);

void *S_HeartBeatDetectThread(void *);

int S_InsertHeartbeatDetectItem(int nSock, pthread_t thread_id);

//插入心跳检测包的键值 

int InsertHeartbeatDetectItem(int nSock);

int ChangeHeartBeatPacketTime(int nSock);


#endif
