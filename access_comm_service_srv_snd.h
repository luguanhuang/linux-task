
#ifndef ACCESS_COMM_SERVICE_SRV_SND_H
#define ACCESS_COMM_SERVICE_SRV_SND_H

#include "../util/access_types.h"
#include "../interface/access_protocol.h"
#include "../include/hash.h"




//接入服务器返回给业务服务器的响应包消息类型
#define RESPONSE_CLIENT_SRV_MSG 0x04

#define YDT_SN_LENGTH 100			//哈希表的键值长度

//连接聊天业务服务器或万维登录服务器后发送的第一条消息类型
#define AFTER_ACCESS_SRV_CONN 0x50

#define CLIENT_SERVER_MSG 0x01      //客户端到服务器的消息类型
#define CLIENT_SERVER_BIG_MSG  0x05
#define PUSHTO_CLIENT_RESPONSE 0x0B 

//业务服务器的数量
typedef struct
{
	pthread_mutex_t num_mutex;					//业务服务器互斥变量
	unsigned short int service_server_num;		//业务服务器数量 
}Service_Server_Num;


//选择业务服务器序号
typedef struct
{
	pthread_mutex_t srv_seq_mutex;				//选择业务服务器的互斥变量
	unsigned int nService_server_seq;			//选择业务服务器的序号
}Service_Server_Seq;

//发送的消息类型
typedef enum
{
	SND_REQUEST_MSG, 				//发送的请求消息
	SND_RESPONSE_MSG				//发送的响应消息
}SND_MSG_TYPE;

//保存客户端的发送消息包的信息
typedef struct
{	
	BYTE bMsg_type;					//消息类型
	BYTE bMain_service_code; 		//主业务代码
	BYTE bSub_service_code; 		//子业务代码
	WORD wAccess_srv_seq;
	unsigned int nMsg_counter;		//消息计数器
	BYTE arrAccess_check_code[4];	//接入服务器的校验码
	BYTE bService_version;			//业务服务器的版本号
	BYTE arrService_check_code[4];	//业务服务器的校验码
	BYTE arrSn[YDT_SN_LENGTH + 1];	//哈希表的键值(包括主业务代码, 子业务代码和消息计数器)
	unsigned int nClient_socket;	//接入服务器和客户端连接的socket
	long lSent_time; 				//存放发送给业务服务器的消息包
	int nSeq;
}Client_Info;

//业务服务器路由信息
typedef struct
{
	pthread_mutex_t mutex; 
	BYTE bMain_code;								//主业务码
	BYTE bSub_code;									//子业务码	
	WORD wService_seq;//the ID of service server		//业务服务器序号
	char arrService_srv_ip[100];					//ip
	WORD wSrv_port;									//端口
	int nSock;										//socket id
}Server_Info;

typedef struct
{
	BYTE bMain_code;								//主业务码
	BYTE bSub_code;									//子业务码	
	WORD wService_seq;//the ID of service server		//业务服务器序号
	char arrService_srv_ip[100];					//ip
	WORD wSrv_port;									//端口
	int nSock;										//socket id
}StShmSrvInfo;


typedef struct
{
	pthread_mutex_t client_info_mutex;				//客户端信息互斥变量
	StHash_Table *pClient_info_hash;				//客户端信息哈希表
}StClientInfo_Hashtable;

struct StCliInfo
{	
	StCliInfo(int iAccessId, int iCnt, int iCliSock)
	{
		snprintf(arrSn, sizeof(arrSn) - 1, "%d%d", iAccessId, iCnt);
		iSock = iCliSock;
	}
	char arrSn[40];
	int iSock;
};


//发送消息到业务服务器
int SendSrvMsg(int nSock, void *pMsg, SND_MSG_TYPE snd_msg_type);

//发送消息给业务服务器
void *SndMsgToServiceServerThread(void *);

int S_SndMsgToServer(void);
int SndDisposeResultToServiceSrv(int nSock, Forward_Srv_Client_Msg forward_srv_client_msg, BYTE bStatus);

int SndAfterConnSrvMsg(int nSock, BYTE bMain_service_code, BYTE bSub_service_code, WORD wService_srv_seq);

int InsertClientInfoIntoHash(char *pMsg);

int S_InsertClientInfoIntoHash(Client_Server_Msg *pClient_srv_msg);

#endif
