
#ifndef SERVICE_SOCK_INFO_H	
#define SERVICE_SOCK_INFO_H


/***************���ļ�������ʼ��socket��Ϣ*********************/

//service  socket�ŶӵĶ��г���
#define LISTEN_NO 5
#define DISCONNWITH_ACCESS_SRV 0x08

#define MSG_TYPE_OFFSET 2

int Socket(int *pListen_sock);

int InitMaintainSockInfo(int *pListenSock, WORD wPort);
//��ʼ��������socket��Ϣ
int InitListenSockInfo(int *pListenSock, WORD wPort);

int T_Accept(int nListen_sock, int *pConn_sock, char *pIP, WORD *pPort);
int Accept(int nListen_sock, int *pConn_sock);

int Recv(int nSock, char *pMsg, int nLen);
int Send(int nSock, char *pMsg, int nLen);

int SendMsg(int nSock, void *pMsg);									//������Ϣ

//int RecvMsg(int nSock, void *pMsg);									//������Ϣ
int RecvAccessSrvMsg(int nSock, void *pMsg);

int SendDisconnWithClientMsg(void *pMsg);



#ifdef _WANWEI_QUERY_SERVICE_
int SndMsgToPushServer(int nSock, void *pMsg);
#endif

#ifdef _WANWEI_PUSH_SERVICE_
int SndPushServiceMsg(void *pMsg);
#endif


#endif
