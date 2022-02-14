
#ifndef ACCESS_SOCK_INFO_H	
#define ACCESS_SOCK_INFO_H	

/************该文件主要用来处理socket信息的************/

//在socket队列中排队的socket的最大数量
#define LISTEN_NO 5

//初始化侦听socket的信息

//发送和接收的数据的大小
#define RECV_LEN 2048
#define SEND_LEN 2048

int Socket(int *pListen_sock);

int Accept(int nListen_sock, int *pConn_sock);

int InitListenSockInfo(int *pListenSock, WORD wPort);

int S_InitListenSockInfo(int *pListenSock, WORD wPort);
	
int InitMaintainSockInfo(int *pListenSock, WORD wPort);

int SetNonBlocking(int nSock);

int RecvClientMsg(int nSock, void *pClient_server_msg);
int Recv(int nSock, char *pMsg, int nLen);
int Send(int nSock, char *pMsg, int nLen, WORD wCmd_id);

#endif
