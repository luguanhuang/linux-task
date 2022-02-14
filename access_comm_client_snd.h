
#ifndef ACCESS_COMM_CLIENT_SND_H
#define ACCESS_COMM_CLIENT_SND_H

typedef struct
{	
	pthread_mutex_t queue_mutex;					//队列互斥变量
	pthread_cond_t queue_cond;						//队列同步变量
	StLinkQueue stLink_queue;
}StSnd_Client_MsgQueue;

int SndMsgToCli(int iSock);

int HandleSndClientSock(int nSock);
void DelClientSockInfo(int nSock);
//发送消息到客户端
int SendClientMsg(int nSock, void *pMsg, WORD nMsg_type);
void *SndMsgToClientThread(void *);
void DelSndClientTmpSockFromHash(int nSock);
void DelClientInfoFromHash(int nSock);

#endif
