
#ifndef ACCESS_COMM_CLIENT_RCV_H
#define ACCESS_COMM_CLIENT_RCV_H


#include "../include/sequence_queue.h"	
#include "../include/hash.h"



struct StMsg
{                     /* ������Ϣ�ṹ�� */ 
    long msg_types;             /* ��Ϣ���ͳ�Ա */    
    char msg_buf[511];          /* ��Ϣ */ 
}; 


//�ͻ��˵�����������Ϣ����ͷ������
#define CLIENT_MSG_HEADER_LEN (sizeof(Client_Server_Msg) - 8)
#define PUSHTO_CLIENTRES_HEADER_LEN (sizeof(PushToClient_Response_Msg) - 8)


#define RECV_MSG_FINISH 0X04
#define RECV_MSG_EMPTY 0x05
#define RECV_MSG_NOTFINISH 0x06
#define RECV_MSG_BUFFER_EMPTY 0x07
#define RECV_MSG_ERROR 0x08
#define RECV_CLOSE_SOCKET 0x30

//��Ϣ����Ĵ�С
#define MSG_BUF_LEN (1024 * 10)

#pragma pack(1)
//�ͻ�����Ϣ����
typedef struct
{
	BYTE bHas_msg_type;
	BYTE bMsg_type;
	int nMsg_header_len;
	int nRecv_header_len;			//���յ���ͷ������
	BYTE bHas_data_len;				//�Ƿ��Ѿ����յ����ݵĳ�����
	int nCur_recv_pos;				//ָ��ǰ�������ݵ�λ��
	int nRecv_body_len;				//Ŀǰ�ѽ��յ������ݵĳ���
	int nMsg_body_len;				//���������ܳ���
	char arrBuf[MSG_BUF_LEN]; 		//�����Ϣ�����buf
	//char *pMsg_body;
}Client_Msg_Buf;

#pragma pack()

//�˽ṹ����������Ϣ������
//��Ϣ������
typedef struct
{
	pthread_mutex_t msg_counter_mutex;				//��Ϣ�������������
	unsigned int nMsg_counter;						//��Ϣ������
}Msg_Counter;	

//��ʱ�����Ϣ�Ĺ�ϣ��
typedef struct
{
	pthread_mutex_t msg_buf_mutex;				//�������
	StHash_Table *pMsg_buf_hashtable;			//��ʱ�����Ϣ�Ĺ�ϣ��	
}StMsgBuf_HashTable;

typedef enum
{
	RECV_HEADER_ERROR,
	RECV_BODY_ERROR,
	RECV_MSG_PEEK_ERROR
}RecvClientMsg_Error_Type;

//���տͻ��˵���Ϣ
typedef struct
{
	pthread_mutex_t queue_mutex;				//���л������
	pthread_cond_t queue_cond;					//����ͬ������
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
