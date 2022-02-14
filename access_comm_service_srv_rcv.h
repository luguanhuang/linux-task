
#ifndef ACCESS_COMM_SERVICE_SRV_RCV_H
#define ACCESS_COMM_SERVICE_SRV_RCV_H

//Client_info 哈希表为空的宏标志
#define GET_CLIENTINFO_ERROR 0x02

//与客户端断连标志
#define DISCONNECT_WITH_CLIENT 0x03

//与业务服务器断连的标志
#define DISCONNECT_WITH_SERVER 0x04

//发送消息给客户端成功与否
#define SND_CLIENT_MSG_SUCCEED 0x00					//接入服务器转发消息成功
#define SND_CLIENT_MSG_FAILURE	   0x01				//接入服务器转发消息失败


//接收业务服务器的消息包
void *RecvMsgFromServiceSrv(void *pClient);
void DelCloseClientSockInfo(int nSock);

struct StCliMsg
{
	int iCliSock;
	char arrSn[50];
	Server_Client_Msg serverCliMsg;
};

typedef struct
{
	void *pMsg;
	int nClient_sock;
	char arrSn[100];
}Srv_Client_Msg_Info;

#endif
