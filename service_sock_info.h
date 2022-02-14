
#ifndef SERVICE_SOCK_INFO_H	
#define SERVICE_SOCK_INFO_H


/***************该文件用来初始化socket信息*********************/

//service  socket排队的队列长度
#define LISTEN_NO 5
#define DISCONNWITH_ACCESS_SRV 0x08

#define MSG_TYPE_OFFSET 2

int Socket(int *pListen_sock);

int InitMaintainSockInfo(int *pListenSock, WORD wPort);
//初始化侦听的socket信息
int InitListenSockInfo(int *pListenSock, WORD wPort);

int T_Accept(int nListen_sock, int *pConn_sock, char *pIP, WORD *pPort);
int Accept(int nListen_sock, int *pConn_sock);

int Recv(int nSock, char *pMsg, int nLen);
int Send(int nSock, char *pMsg, int nLen);

int SendMsg(int nSock, void *pMsg);									//发送消息

//int RecvMsg(int nSock, void *pMsg);									//接收消息
int RecvAccessSrvMsg(int nSock, void *pMsg);

int SendDisconnWithClientMsg(void *pMsg);



#ifdef _WANWEI_QUERY_SERVICE_
int SndMsgToPushServer(int nSock, void *pMsg);
#endif

#ifdef _WANWEI_PUSH_SERVICE_
int SndPushServiceMsg(void *pMsg);
#endif


#endif
