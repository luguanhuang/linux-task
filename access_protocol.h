
#ifndef ACCESS_PROTOCOL_H_H
#define ACCESS_PROTOCOL_H_H

//#include "../util/access_global.h"
#include "../util/access_types.h"

/*************该文件主要是用来处理通讯接口的***************************/

//客户端到服务器端的请求报

//modify by LiHuangYuan,2011/10/14 modify struct pack
#pragma pack(1)

#define ACCOUNT_LEN 30				//帐号长度

#define PASSWD_LEN 20				//密码长度

//客户端发送到服务器的消息包结构
typedef struct
{
 	BYTE bVersion; 					//版本号
	BYTE bMsg_type;					//消息类型
	BYTE bMain_service_code; 		//主业务码
	BYTE bSub_service_code; 		//子业务码
	WORD wAccess_server_seq;		//接入服务器序号
	unsigned int nMsg_counter; 		//消息计数器
	unsigned int nAccess_socket_id; //接入服务器和客户端之间的socket		
	WORD wService_server_seq;		//业务服务器序号
	unsigned int nData_len; 		//数据长度
	char *pData; 					//数据部分
	char  arrCheck_code[4]; 		//校验码
}Client_Server_Msg;

//服务器到客户端的响应包(一般的业务, 如登录业务)
typedef struct
{
 	BYTE bVersion; 					//版本号
 	BYTE bMsg_type;					//消息类型
 	BYTE bMain_service_code; 		//主业务代码
 	BYTE bSub_service_code; 		//子业务代码
 	WORD wAccess_server_seq; 		//接入服务器序号
	unsigned  int nMsg_counter; 	//消息计数器
	unsigned int nData_len; 		//数据长度
	char *pData; 					//数据
	char  arrCheckout_code[4]; 		//校验码
}Server_Client_Msg;

//该数据包主要是处理聊天业务(业务服务器转发给客户端的数据包)
typedef struct
{
	BYTE bVersion;							//版本号
	BYTE bMsg_type;							//消息类型
	BYTE bMain_service_code;				//主业务代码
	BYTE bSub_service_code;					//子业务代码
	WORD wSrc_access_srv_seq;				//原接入服务器序号
	unsigned int nAccess_Msg_counter;		//接入服务器的消息计数器
	WORD wDest_access_srv_seq;				//目的接入服务器序号
	unsigned int nAccess_socket_id; 		//接入服务器与客户端之间的socketid
	WORD wService_server_seq;				//业务服务器序号
	unsigned int nService_Msg_counter;		//业务服务器消息计数器
	unsigned int nData_len;					//数据部分长度
	char *pData;							//数据部分
	char arrCheck_code[4];					//校验码
}Forward_Srv_Client_Msg;

typedef struct
{
	BYTE bVersion;							//版本号
	BYTE bMsg_type;							//消息类型
	BYTE bMain_service_code;				//主业务代码
	BYTE bSub_service_code;					//子业务代码
	WORD wAccess_srv_seq;
	int nClient_sock;
	int nData_len;
	char *pData;
	char arrCheck_code[4];
}PushTo_Client_Msg;

typedef struct
{
	BYTE bVersion;							//版本号
	BYTE bMsg_type;							//消息类型
	BYTE bMain_service_code;				//主业务代码
	BYTE bSub_service_code;					//子业务代码
	WORD wAccess_srv_seq;
	int nClient_sock;
	WORD wService_srv_seq;
	int nData_len;
	char *pData;
	char arrCheck_code[4];
}PushToClient_Response_Msg;


typedef struct
{	
	BYTE bVersion;   				//版本号
	BYTE bMsg_type; 				//消息类型
	BYTE bMain_service_code;		//主业务码
	BYTE bSub_service_code; 		//子业务码
	WORD wAccess_server_seq;		//接入服务器序号
	int nClient_socket;
	unsigned int nData_len; 		//数据长度
	char *pData;    				//数据部分
	BYTE arrCheck_code[4]; 			//校验码
}StReq_DisconnWithClient_MSG;


//运行维护数据包消息开始
//查询服务器信息消息开始
typedef struct
{
	BYTE bVersion;
	WORD wCmd_id;
}Req_Query_Srv_Info;

typedef struct
{
	BYTE bVersion;
	WORD wCmd_id;
	int nMsg_queue_len;
	int nClientsock_queue_len;
	int nRoutingtable_len;
	int nHeartdet_table_len;
	int nClientmsgbuf_table_len;
	int nClientmsginfo_table_len;
	int nSndclient_msgqueue_len;
	int nSndclient_sockhash_len;
	int nRcv_climsg_tmpsockhash_len;
}Res_Query_Srv_Info;
//查询服务器信息消息结束


//修改日记级别消息开始
typedef struct
{
	BYTE bVersion;
	WORD wCmd_id;
	BYTE bLog_level;
}Req__Modify_Log_Level;

typedef struct
{
	BYTE bVersion;
	WORD wCmd_id;
	BYTE bModify_status;
}Res__Modify_Log_Level;


//修改日记级别消息开始

//运行维护数据包消息结束



//该数据包主要是把发送给
//客户端消息的结果转发给业务服务器
typedef struct
{
	BYTE bVersion;				//版本号
	BYTE bMsg_type;				//消息类型
	WORD wService_server_seq;	//业务服务器序号
	unsigned int nMsg_counter;	//消息计数器
	BYTE bTransfer_result;		//转发结果
	char arrCheck_code[4];
}Response_Client_Srv_Msg;

typedef struct
{
	BYTE bVersion;
	WORD wCmd_id;
	WORD wXML_len;
	BYTE *pXML_data;
	char arrCheck_code[4];
}Server_Error;


//接入服务器连接业务服务器成功后发送的第一个业务包
typedef struct
{
	BYTE bVersion;				//版本号
	WORD wCmd_id;				//命令ID
	BYTE arrCheck_code[4];		//校验码
}Req_AccessServer_Conn;

//modify by LiHuangYuan,2011/10/14 modify struct pack

int GetServerErrorPacket(Server_Client_Msg *pSrv_client_msg, void *pClient_info);
void FreeMsgBuffer(void *pMsg);

#pragma pack()
#endif
