
#ifndef ACCESS_COMM_CLIENT_RCV_H
#define ACCESS_COMM_CLIENT_RCV_H


#include "../include/sequence_queue.h"	
#include "../include/hash.h"



struct StMsg
{                     /* 声明消息结构体 */ 
    long msg_types;             /* 消息类型成员 */    
    char msg_buf[511];          /* 消息 */ 
}; 


//客户端到服务器的消息包的头部长度
#define CLIENT_MSG_HEADER_LEN (sizeof(Client_Server_Msg) - 8)
#define PUSHTO_CLIENTRES_HEADER_LEN (sizeof(PushToClient_Response_Msg) - 8)


#define RECV_MSG_FINISH 0X04
#define RECV_MSG_EMPTY 0x05
#define RECV_MSG_NOTFINISH 0x06
#define RECV_MSG_BUFFER_EMPTY 0x07
#define RECV_MSG_ERROR 0x08
#define RECV_CLOSE_SOCKET 0x30

//消息缓冲的大小
#define MSG_BUF_LEN (1024 * 10)

#pragma pack(1)
//客户端消息缓存
typedef struct
{
	BYTE bHas_msg_type;
	BYTE bMsg_type;
	int nMsg_header_len;
	int nRecv_header_len;			//接收到的头部长度
	BYTE bHas_data_len;				//是否已经接收到数据的长度了
	int nCur_recv_pos;				//指向当前接收数据的位置
	int nRecv_body_len;				//目前已接收到的数据的长度
	int nMsg_body_len;				//接收数据总长度
	char arrBuf[MSG_BUF_LEN]; 		//存放消息缓冲的buf
	//char *pMsg_body;
}Client_Msg_Buf;

#pragma pack()

//此结构用来计数消息的数量
//消息计数器
typedef struct
{
	pthread_mutex_t msg_counter_mutex;				//消息计数器互斥变量
	unsigned int nMsg_counter;						//消息计数器
}Msg_Counter;	

//临时存放消息的哈希表
typedef struct
{
	pthread_mutex_t msg_buf_mutex;				//互斥变量
	StHash_Table *pMsg_buf_hashtable;			//临时存放消息的哈希表	
}StMsgBuf_HashTable;

typedef enum
{
	RECV_HEADER_ERROR,
	RECV_BODY_ERROR,
	RECV_MSG_PEEK_ERROR
}RecvClientMsg_Error_Type;

//接收客户端的消息
typedef struct
{
	pthread_mutex_t queue_mutex;				//队列互斥变量
	pthread_cond_t queue_cond;					//队列同步变量
	StSequence_Queue msg_queue;
}StMsg_Queue;

ssize_t read_fd(int fd, int *recvfd);


void DelTmpClientSockFromHashtable(int nSock);

int RcvMsgFromClient(int nSock);

void S_RcvMsgFromClient(void *pSock, int iSockFd);

void RecvClientMsgErrorProcess(int nSock);

void DelClientSockFromQueue(int nSock);

void DelClientMsgBufFromHash(int nSock);
#endif
