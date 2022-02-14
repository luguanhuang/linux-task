
#include "../util/access_global.h"
//#include "../include/ydt_log.h"
#include "../interface/access_protocol.h"
#include "../util/access_server_seq.h"
#include "../communication/access_routing_maintain.h"
#include "access_heartbeat_detectthread.h"
#include "access_epoll.h"
#include "access_comm_service_srv_snd.h"
#include "access_sock_info.h" 
#include "access_comm_client_snd.h"
#include "access_comm_client_rcv.h"

#define HAVE_MSGHDR_MSG_CONTROL

extern int g_iQid; 

/**********该文件主要是用来与客户端进行交互(接收客户端消息*********/

#define MSG_COUNT_BEGIN 0x01

extern StMsgBuf_HashTable g_msgbuf_hashtable;  //客户端消息缓存
extern StClient_Sock_Queue g_clientsocket_queue;;
extern StRouting_Table g_routing_table;
extern Msg_Counter g_msg_counter;	//消息计数器

extern Epoll_Fd g_epoll_fd;			//epoll结构
extern Awake_Thread_Flg g_awake_thread_flg;
extern StMsg_Queue g_msg_queue;

extern pthread_mutex_t g_recv_clientmsg_mutex;

extern StHeartbeat_Detect_Table g_heartbeatdetect_table;

extern StTmp_clientSock_Hash g_tmp_clientsock_hash;

extern Server_Conf_Info g_srv_conf_info;

#define HAVE_MSGTYPE_MSGLEN 0x02

//函数用途: 把客户端消息缓存插入到哈希表里面
//输入参数: socket id ,  存放客户端消息的缓存
//输出参数:  无
//返回值	: 无

int InsertClientMsgBufToHash(int nSock, Client_Msg_Buf *pClient_msg_buf)
{
	INFO("InsertClientMsgBufToHash: func begin%s", "");
	if (NULL == pClient_msg_buf)
	{
		ERROR("InsertClientMsgBufToHash: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = 0;
	char szSock[100] = {0};
	StHash_Item *pItem = NULL;
	snprintf(szSock, sizeof(szSock) - 1, "%d", nSock);
	
	nRet = HashInsert(&g_msgbuf_hashtable.pMsg_buf_hashtable, szSock, (void *)pClient_msg_buf);
	if (TRUE != nRet && HASH_SAMEKEY_EXIST != nRet)
	{
		ERROR("InsertClientMsgBufToHash: Call YDT_Hash_Insert error%s", "");
		return FALSE;	
	}
	
	if (HASH_SAMEKEY_EXIST == nRet)
	{
		pItem = HashGetItem(g_msgbuf_hashtable.pMsg_buf_hashtable, szSock);
		if (NULL != pItem)
		{
			pthread_mutex_lock(&g_msgbuf_hashtable.msg_buf_mutex);
			MM_FREE(pItem->pMatch_msg);		
			pItem->pMatch_msg = (void *)pClient_msg_buf;
			pthread_mutex_unlock(&g_msgbuf_hashtable.msg_buf_mutex);
		}

		
		INFO("InsertClientMsgBufToHash: key exist yet in the client message buffer hashtable"
						" then we will replace the previous value [socket]=%d", nSock);
	}
	else
	{
		INFO("InsertClientMsgBufToHash: insert client message into client message buffer hashtable succeed"
						" [socket id]=%d", nSock);
	}
		
	INFO("InsertClientMsgBufToHash: func end%s", "");
	return TRUE;
}


//函数用途:从客户端消息缓存哈希表里面删除客户端消息的一项
//输入参数:  socket id
//输出参数:  无
//返回值	: 无

void DelClientMsgBufFromHash(int nSock)
{
	INFO("DelClientMsgBufFromHash: func begin%s", "");
	void *pData = NULL;
	char szSock[100] = {0};

	//mydify by luguanhuang 20110916(reason: 改变了以前采用pthread_mutex_trylock的现象)
	snprintf(szSock, sizeof(szSock) - 1, "%d", nSock);
	pData = HashDel(&(g_msgbuf_hashtable.pMsg_buf_hashtable), szSock);
	if (NULL != pData)
	{
		INFO("DelClientMsgBufFromHash: delete client msg buffer succeed%s", "");
		MM_FREE(pData);
	}
	else
	{
		INFO("DelClientMsgBufFromHash: delete client msg buffer fail%s", "");		
	}
	
	INFO("DelClientMsgBufFromHash: func end%s", "");
}

void DelTmpClientSockFromHashtable(int nSock)
{
	INFO("DelTmpClientSockFromHashtable: func begin%s", "");
	void *pData = NULL;
	char szSock[100] = {0};

	snprintf(szSock, sizeof(szSock) - 1, "%d", nSock);
	pData = HashDel(&(g_tmp_clientsock_hash.pTmp_clientsock_table), szSock);
	if (NULL != pData)
	{
		INFO("DelTmpClientSockFromHashtable: delete temp client socket from temp client socket hashtable succeed%s", "");
		MM_FREE(pData);
	}
	else
	{
		INFO("DelTmpClientSockFromHashtable: delete temp client socket from temp client socket hashtable fail%s", "");		
	}
	
	INFO("DelTmpClientSockFromHashtable: func end%s", "");
}

void DelClientSockFromQueue(int nSock)
{
	INFO("DelClientSockFromQueue: func begin%s", "");
	int nRet = 0;
	nRet = DelFromQueuetByValue(&g_clientsocket_queue.stLink_queue, nSock);
	while (TRUE == nRet)
	{
		nRet = DelFromQueuetByValue(&g_clientsocket_queue.stLink_queue, nSock);		
	}	
	
	INFO("DelClientSockFromQueue: func end%s", "");
	return;
}

//函数用途: 处理接收客户端消息错误的函数
//输入参数:  socket id,  接收客户端消息错误的类型
//输出参数:  无
//返回值	: 无

void RecvClientMsgErrorProcess(int nSock)
{
	INFO("RecvClientMsgErrorProcess: func  begin%s", "");
	int nRet = 0;
	nRet = DelSockEventFromepoll(nSock);
	if (FALSE == nRet)
	{
		INFO("RecvClientMsgErrorProcess: Call DelSockEventFromepoll error%s", "");
	}
	else
	{
		INFO("RecvClientMsgErrorProcess: Call DelSockEventFromepoll succeed%s", "");
	}

	DelSndClientTmpSockFromHash(nSock);

	//接收失败不删除客户端信息, 因为这个太频繁了
	//只有发送消息给客户端失败的时候才会调用这个函数
	//DelClientInfoFromHash(nSock);			
	DelClientSockFromQueue(nSock);
	DelTmpClientSockFromHashtable(nSock);	
	DelClientMsgBufFromHash(nSock);
	DeleteHeartbeatDetectItem(nSock);			//删除心跳包里面的一项
	close(nSock);								//关闭socket
	INFO("RecvClientMsgErrorProcess: func end%s", "");
}

int RecvClientMsgType(int nSock, Client_Msg_Buf *pClient_msg_buf)
{
	INFO("RecvClientMsgType: func begin%s", "");
	int nLeft_len = 2;		
	int nRecv_len = 0;
	int nRet = 0;
	
	while (nLeft_len > 0)
	{	
		nRecv_len = nLeft_len > RECV_LEN ? RECV_LEN : nLeft_len;
		nRet = recv(nSock, pClient_msg_buf->arrBuf + pClient_msg_buf->nCur_recv_pos, nRecv_len, 0);
		DEBUG("RecvClientMsgType: Call recv func [return value]=%d", nRet);
		if (-1 == nRet)
		{
			if (EAGAIN == errno)				//当前接收缓冲区已经没有数据接收了
			{
				DEBUG("RecvClientMsgType: server recv buffer is empty now [return value]=%d error[%d]=%s", \
				nRet, errno, strerror(errno));
				INFO("RecvClientMsgType: func end%s", "");
				return RECV_MSG_BUFFER_EMPTY;
			}
			else								//接收错误
			{
				ERROR("RecvClientMsgType: Call recv error [return value]=%d error[%d]=%s", \
					nRet, errno, strerror(errno));
				INFO("RecvClientMsgType: func end%s", "");
				return RECV_MSG_ERROR;
			}			
		}
		else if (0 == nRet)				//与服务器断连
		{
			INFO("RecvClientMsgType: client close the socket [return value]=%d", nRet);
			INFO("RecvClientMsgType: func end%s", "");
			return RECV_CLOSE_SOCKET;
		}

		nLeft_len -= nRet;								//接收头部的剩余字节
		pClient_msg_buf->nCur_recv_pos += nRet;
		pClient_msg_buf->nRecv_header_len += nRet;		//接收到的消息头部的长度
	}
	
	INFO("RecvClientMsgType: func end%s", "");
	return TRUE;
}

int RecvHeaderMoreMsg(int nSock, Client_Msg_Buf *pClient_msg_buf)
{
	INFO("RecvHeaderMoreMsg: func begin%s", "");
	int nLeft_len = 0;
	int nRet = 0;
	int nRecv_len = 0;

	nLeft_len = pClient_msg_buf->nMsg_header_len - pClient_msg_buf->nRecv_header_len;		
	while (nLeft_len > 0)
	{	
		nRecv_len = nLeft_len > RECV_LEN ? RECV_LEN : nLeft_len;
		nRet = recv(nSock, pClient_msg_buf->arrBuf + pClient_msg_buf->nCur_recv_pos, nRecv_len, 0);
		DEBUG("RecvHeaderMoreMsg: Call recv func [return value]=%d", nRet);
		if (-1 == nRet)
		{
			if (EAGAIN == errno)				//当前接收缓冲区已经没有数据接收了
			{
				DEBUG("RecvHeaderMoreMsg: server recv buffer is empty now [return value]=%d error[%d]=%s", \
				nRet, errno, strerror(errno));
				INFO("RecvHeaderMoreMsg: func end%s", "");
				return RECV_MSG_BUFFER_EMPTY;
			}
			else								//接收错误
			{
				ERROR("RecvHeaderMoreMsg: Call recv error [return value]=%d error[%d]=%s", \
					nRet, errno, strerror(errno));
				INFO("RecvHeaderMoreMsg: func end%s", "");
				return RECV_MSG_ERROR;
			}			
		}
		else if (0 == nRet)				//与服务器断连
		{
			INFO("RecvHeaderMoreMsg: client close the socket [return value]=%d", nRet);
			INFO("RecvHeaderMoreMsg: func end%s", "");
			return RECV_CLOSE_SOCKET;
		}

		nLeft_len -= nRet;								//接收头部的剩余字节
		pClient_msg_buf->nCur_recv_pos += nRet;
		pClient_msg_buf->nRecv_header_len += nRet;		//接收到的消息头部的长度
	}			
	
	INFO("RecvHeaderMoreMsg: func end%s", "");
	return TRUE;
}

//函数用途:  接收客户端消息的消息头
//输入参数:  socket id
//输出参数:  存放客户端消息的缓存
//返回值	接收成功,  返回TRUE,  接收失败,  返回FALSE

int RecvClientMsgHeader(int nSock, Client_Msg_Buf *pClient_msg_buf)
{
	INFO("RecvClientMsgHeader: func begin%s", "");
	if (NULL == pClient_msg_buf)
	{
		ERROR("RecvClientMsgHeader: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = 0;
	if (FALSE == pClient_msg_buf->bHas_msg_type)
	{
		nRet = RecvClientMsgType(nSock, pClient_msg_buf);
		if (TRUE != nRet)
		{
			if (RECV_MSG_ERROR == nRet)
			{
				ERROR("RecvClientMsgHeader: Call RecvClientMsgType error%s", "");			
			}
			
			return nRet;
		}
		
		pClient_msg_buf->bHas_msg_type = TRUE;
		pClient_msg_buf->bMsg_type = pClient_msg_buf->arrBuf[1];
	}

	if (TRUE == pClient_msg_buf->bHas_msg_type)
	{
		if (CLIENT_SERVER_MSG == pClient_msg_buf->bMsg_type)
		{
			pClient_msg_buf->nMsg_header_len = CLIENT_MSG_HEADER_LEN;
		}	
		else if (PUSHTO_CLIENT_RESPONSE == pClient_msg_buf->bMsg_type)
		{
			pClient_msg_buf->nMsg_header_len = PUSHTO_CLIENTRES_HEADER_LEN;
		}

		nRet = RecvHeaderMoreMsg(nSock, pClient_msg_buf);
		if (TRUE != nRet)
		{
			if (RECV_MSG_ERROR == nRet)
			{
				ERROR("RecvClientMsgHeader: Call RecvClientSrvMsgHeader error%s", "");	
			}
			
			return nRet;
		}	
	}

	INFO("RecvClientMsgHeader: func end%s", "");
	return TRUE;
}

//函数用途:  接收客户端消息的消息体
//输入参数:  socket id
//输出参数:  存放客户端消息的缓存
//返回值	接收成功,  返回TRUE,  接收失败,  返回FALSE

int RecvClientMsgBody(int nSock, Client_Msg_Buf *pClient_msg_buf)
{
	INFO("RecvClientMsgBody: func begin%s", "");
	if (NULL == pClient_msg_buf)
	{
		ERROR("RecvClientMsgBody: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nLeft_len = pClient_msg_buf->nMsg_body_len - pClient_msg_buf->nRecv_body_len;
	int nRet = 0;
	int nRecv_len = 0;
	
	while (nLeft_len > 0)
	{
		nRecv_len = nLeft_len > RECV_LEN ? RECV_LEN : nLeft_len;
		nRet = recv(nSock, pClient_msg_buf->arrBuf + pClient_msg_buf->nCur_recv_pos, nRecv_len, 0);		
		DEBUG("RecvClientMsgBody: Call recv func [return value]=%d", nRet);
		if (-1 == nRet)
		{
			if (EAGAIN == errno)			//当前接收缓冲区已经没有数据接收了
			{
				DEBUG("RecvClientMsgBody: server recv buffer is empty now [return value]=%d error[%d]=%s", \
				nRet, errno, strerror(errno));
				INFO("RecvClientMsgBody: func end%s", "");
				return RECV_MSG_BUFFER_EMPTY;
			}
			else
			{
				ERROR("RecvClientMsgBody: Call recv error [return value]=%d error[%d]=%s", \
					nRet, errno, strerror(errno));
				INFO("RecvClientMsgBody: func end%s", "");
				return RECV_MSG_ERROR;
			}
		}
		else if (0 == nRet)
		{
			INFO("RecvClientMsgBody: client close the socket [return value]=%d", nRet);
			INFO("RecvClientMsgBody: func end%s", "");
			return RECV_CLOSE_SOCKET;
		}

		nLeft_len -= nRet;
		pClient_msg_buf->nCur_recv_pos += nRet;
		pClient_msg_buf->nRecv_body_len += nRet;
	}

	INFO("RecvClientMsgBody: func end%s", "");
	return TRUE;
}

//函数用途:  接收客户端消息
//输入参数:  socket id
//输出参数:  存放客户端消息的缓存
//返回值	接收成功,  返回TRUE,  接收失败,  返回FALSE

int RecvAsyncMsg(int nSock, Client_Msg_Buf *pMsg_buf)
{
	INFO("RecvAsyncMsg: func begin%s", "");
	if (NULL == pMsg_buf)
	{
		ERROR("RecvAsyncMsg: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = 0;
	int nData_len = 0;
	/*如果消息头还没接收完, 就继续接收消息头,  如果消息头接收完成
	  *就继续接收消息体(因为只有消息头接收完成了,  才可以获取消息长度等信息)
	  */
	//消息头还没接收到,   就接收消息头
	if (FALSE == pMsg_buf->bHas_data_len)
	{
		INFO("RecvAsyncMsg: we will recv client message header%s", "");
		nRet = RecvClientMsgHeader(nSock, pMsg_buf);
		if (RECV_MSG_ERROR == nRet)
		{
			ERROR("RecvAsyncMsg: Call RecvClientMsgHeader error%s", "");
			return nRet;	
		}
		else if (RECV_MSG_BUFFER_EMPTY == nRet)
		{
			INFO("RecvAsyncMsg: when we recv message header, we occur recv client message buffer"
				" is empty phenomenon%s", "");
			return nRet;
		}
		else if (RECV_CLOSE_SOCKET == nRet)
		{
			INFO("RecvAsyncMsg: when we recv message header, client close the socket%s", "");
			return nRet;
		}

		if (pMsg_buf->nMsg_header_len == pMsg_buf->nRecv_header_len)
		{
			INFO("RecvAsyncMsg: we recv the message header finish [header len]=%d", pMsg_buf->nRecv_header_len);
			pMsg_buf->bHas_data_len = TRUE;
			nData_len = *(int *)(pMsg_buf->arrBuf + pMsg_buf->nMsg_header_len - 4);
			pMsg_buf->nMsg_body_len = ntohl(nData_len);
			pMsg_buf->nMsg_body_len += 4;		//之所以要加四, 是因为接收的数据包括接入服务器的校验码
			/*DEBUG("RecvAsyncMsg: [sum data length]=%d [message type]=%d", \
				pMsg_buf->nSum_data_len, pMsg_buf->bMsg_type);*/

			DEBUG("RecvAsyncMsg: [body data length]=%d", pMsg_buf->nMsg_body_len);
		}
		//....
		//read len data
	}

	//继续接收消息体
	if (TRUE == pMsg_buf->bHas_data_len && pMsg_buf->nMsg_header_len == pMsg_buf->nRecv_header_len)		//接收消息体
	{	
		INFO("RecvAsyncMsg: we will recv client message body%s", "");
		nRet = RecvClientMsgBody(nSock, pMsg_buf);
		if (RECV_MSG_ERROR == nRet)
		{
			ERROR("RecvAsyncMsg: Call RecvClientMsgBody error%s", "");
			INFO("RecvAsyncMsg: func end%s", "");
			return nRet;	
		}
		else if (RECV_MSG_BUFFER_EMPTY == nRet)
		{
			INFO("RecvAsyncMsg: when we recv message body, we occur recv client message buffer"
				" is empty phenomenon%s", "");
			INFO("RecvAsyncMsg: func end%s", "");
			return nRet;
		}
		else if (RECV_CLOSE_SOCKET == nRet)
		{
			INFO("RecvAsyncMsg: when we recv message body, client close the socket%s", "");
			INFO("RecvAsyncMsg: func end%s", "");	
			return nRet;
		}
	}
	INFO("RecvAsyncMsg: func end%s", "");	
	return TRUE;
}

int PutMessageToQueue(void *pClient_msg_buffer, StSequence_Queue *pQueue, pthread_cond_t *pQueue_cond)
{
	INFO("PutMessageToQueue: func begin%s", "");
	int nRet = 0;
	
	nRet = PutMsgToSeqQueue(pQueue, pClient_msg_buffer); 
	if (FALSE == nRet)								//队列为空的情况
	{
		WARN("PutMessageToQueue: queue is full now so we will discard the message%s", "");
		MM_FREE(pClient_msg_buffer);
		INFO("PutMessageToQueue: func end%s", "");
		return nRet;
	}
	else if (FUNC_PARAM_ERROR == nRet)
	{
		ERROR("PutMessageToQueue: Call PutMsgToSeqQueue func param error%s", "");							
		MM_FREE(pClient_msg_buffer);
		INFO("PutMessageToQueue: func end%s", "");
		return nRet;
	}
	
	pthread_cond_signal(pQueue_cond);
	
	INFO("PutMessageToQueue: func end%s", "");
	return TRUE;
}

int HandleClientSrvMsg(void *pMsg_buf, int nSock)
{
	INFO("HandleClientSrvMsg: func  begin%s", "");
	if (NULL == pMsg_buf)
	{
		ERROR("HandleClientSrvMsg: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	Client_Msg_Buf *pTmp_msg_buf = (Client_Msg_Buf *)pMsg_buf;
	Client_Server_Msg *pClient_srv_msg = (Client_Server_Msg *)pTmp_msg_buf->arrBuf;
	char *pTmp_Client_srv_msg = (char *)pTmp_msg_buf->arrBuf;
	WORD wCmd_id = *(WORD *)(pTmp_Client_srv_msg + pTmp_msg_buf->nMsg_header_len + 1);
	wCmd_id = ntohs(wCmd_id);

	WORD wAccess_srv_seq = pClient_srv_msg->wAccess_server_seq;
	wAccess_srv_seq = ntohs(wAccess_srv_seq);
	int nMsg_counter = pClient_srv_msg->nMsg_counter;
	nMsg_counter = ntohl(nMsg_counter);
	
	int nClient_socket = pClient_srv_msg->nAccess_socket_id;
	nClient_socket = ntohl(nClient_socket);
	WORD wDest_srv_seq = pClient_srv_msg->wService_server_seq;
	wDest_srv_seq = ntohs(wDest_srv_seq);
	
	DEBUG("RcvMsgFromClient: [version]=%d [msg type]=%d [main service code]=%d [sub service code]=%d"
		" [access server seq]=%d [msg counter]=%d [client socket]=%d [dest service server seq]=%d"
		" [msg header len]=%d [msg body len]=%d [msg cmd_id]=%d", pClient_srv_msg->bVersion, \
		pClient_srv_msg->bMsg_type, pClient_srv_msg->bMain_service_code, \
		pClient_srv_msg->bSub_service_code, wAccess_srv_seq, nMsg_counter, \
		nClient_socket, wDest_srv_seq, pTmp_msg_buf->nRecv_header_len, \
		pTmp_msg_buf->nMsg_body_len, wCmd_id);	
	pClient_srv_msg->wAccess_server_seq = htons(g_srv_conf_info.wAccess_srv_seq);

	pthread_mutex_lock(&g_msg_counter.msg_counter_mutex);
	g_msg_counter.nMsg_counter++;

	if (g_msg_counter.nMsg_counter >= (pow(2, 32) - 1))
	{
		g_msg_counter.nMsg_counter = MSG_COUNT_BEGIN;
	}

	pClient_srv_msg->nMsg_counter = htonl(g_msg_counter.nMsg_counter);
	pthread_mutex_unlock(&g_msg_counter.msg_counter_mutex);
			
	pClient_srv_msg->nAccess_socket_id = htonl(nSock);		
	DEBUG("RcvMsgFromClient: [access srv seq]=%d", \
	ntohs(pClient_srv_msg->wAccess_server_seq));
	DEBUG("RcvMsgFromClient: [msg counter]=%d", \
		ntohl(pClient_srv_msg->nMsg_counter));
	DEBUG("RcvMsgFromClient: [client socket]=%d", \
		ntohl(pClient_srv_msg->nAccess_socket_id));
	
	INFO("HandleClientSrvMsg: func end%s", "");
	return TRUE;
}

int HandlePushToCliResMsg(void *pMsg_buf, int nSock)
{
	INFO("HandlePushToCliResMsg: func  begin%s", "");
	if (NULL == pMsg_buf)
	{
		ERROR("HandlePushToCliResMsg: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	Client_Msg_Buf *pTmp_msg_buf = (Client_Msg_Buf *)pMsg_buf;
	PushToClient_Response_Msg *pPushto_client_res_msg = (PushToClient_Response_Msg *)pTmp_msg_buf->arrBuf;
	char *pTmp_Client_srv_msg = (char *)pTmp_msg_buf->arrBuf;
	WORD wCmd_id = *(WORD *)(pTmp_Client_srv_msg + pTmp_msg_buf->nMsg_header_len + 1);
	wCmd_id = ntohs(wCmd_id);

	WORD wAccess_srv_seq = pPushto_client_res_msg->wAccess_srv_seq;
	wAccess_srv_seq = ntohs(wAccess_srv_seq);
	
	int nClient_socket = pPushto_client_res_msg->nClient_sock;
	nClient_socket = ntohl(nClient_socket);
	WORD wDest_srv_seq = pPushto_client_res_msg->wService_srv_seq;
	wDest_srv_seq = ntohs(wDest_srv_seq);
	
	DEBUG("HandlePushToCliResMsg: [version]=%d [msg type]=%d [main service code]=%d [sub service code]=%d"
		" [access server seq]=%d [client socket]=%d [dest service server seq]=%d"
		" [msg header len]=%d [msg body len]=%d [msg cmd_id]=%d", pPushto_client_res_msg->bVersion, \
		pPushto_client_res_msg->bMsg_type, pPushto_client_res_msg->bMain_service_code, \
		pPushto_client_res_msg->bSub_service_code, wAccess_srv_seq, \
		nClient_socket, wDest_srv_seq, pTmp_msg_buf->nRecv_header_len, \
		pTmp_msg_buf->nMsg_body_len, wCmd_id);	
	pPushto_client_res_msg->wAccess_srv_seq = htons(g_srv_conf_info.wAccess_srv_seq);
			
	pPushto_client_res_msg->nClient_sock = htonl(nSock);		
	DEBUG("HandlePushToCliResMsg: [access srv seq]=%d", \
	ntohs(pPushto_client_res_msg->wAccess_srv_seq));
	DEBUG("HandlePushToCliResMsg: [client socket]=%d", \
		ntohl(pPushto_client_res_msg->nClient_sock));
	
	INFO("HandlePushToCliResMsg: func end%s", "");
	return TRUE;
}


//函数用途:  接收从客户端发送过来的消息
//输入参数: socket id
//输出参数:  无
//返回值	: 接收成功,  返回TRUE， 接收失败,  返回FALSE；

int RcvMsgFromClient(int nSock)
{
	INFO("RcvMsgFromClient: func begin%s", "");
	int nRet = 0;
	void *pValue = NULL;
	Client_Msg_Buf *pMsg_buf = NULL;
	void *pDel_value = NULL;
	char szSock[100] = {0};
	int nRecv_msg_state = TRUE;
	
	snprintf(szSock, sizeof(szSock) - 1, "%d", nSock);
	StHash_Table *pMsg_buf_hash = g_msgbuf_hashtable.pMsg_buf_hashtable;
	pValue = HashData(pMsg_buf_hash, szSock);
	
	//如果消息缓存不存在的情况下
	if (NULL == pValue)
	{
		INFO("RcvMsgFromClient: client message is not exist in the client message buffer hashtable%s", "");
		pMsg_buf = (Client_Msg_Buf *)MM_MALLOC_WITH_DESC(sizeof(Client_Msg_Buf), \
		"RcvMsgFromClient: Call func for client message buffer");
		if (NULL == pMsg_buf)
		{
			FATAL("RcvMsgFromClient: Call MM_MALLOC_WITH_DESC error%s", "");
			return OUT_OF_MEMORY_ERROR;
		}

		memset(pMsg_buf, 0, sizeof(Client_Msg_Buf));
	}
	else		//如果消息缓存存在的情况下
	{
		INFO("RcvMsgFromClient: client message exist in the client message buffer hashtable%s", "");
		pMsg_buf = (Client_Msg_Buf *)pValue;		//应该在这里把分配的缓存放入消息缓存哈希表
	}

	nRet = RecvAsyncMsg(nSock, pMsg_buf);
	if (RECV_MSG_ERROR == nRet || RECV_CLOSE_SOCKET == nRet)
	{
		if (RECV_MSG_ERROR == nRet)
		{
			ERROR("RcvMsgFromClient: Call RecvAsyncMsg error%s", "");		
		}
		
		//如果为空, 表示客户端消息缓存还没插入哈希表中
		//如果不为空, 就不用删除了, 因为RecvClientMsgErrorProcess(nSock)中的
		//DelClientMsgBufFromHash 函数已经把客户端消息缓存释放掉了)
		if (NULL == pValue)				
		{
			MM_FREE(pMsg_buf);		
		}
		
		return nRet;		
	}
	else if(RECV_MSG_BUFFER_EMPTY == nRet && 0 == pMsg_buf->nRecv_header_len)		//处理socket 队列里面多余的socket
	{
		INFO("RcvMsgFromClient: current socket don't have message to recv so we will exit the"
			" function%s", "");	

		//该语句主要是为了避免多余的socket导致的内存泄漏的产生
		MM_FREE(pMsg_buf);		
		return RECV_MSG_EMPTY;	
	}

	//接收消息,  并把消息放入客户端消息缓存哈希表里面(主要是用来接收完整的一条消息)
	if (NULL == pValue)
	{
		nRet = InsertClientMsgBufToHash(nSock, pMsg_buf);
		if (TRUE != nRet)
		{
			ERROR("RcvMsgFromClient: Call InsertClientMsgBufToHash error%s", "");
			//RecvClientMsgErrorProcess(nSock);			//把错误处理放在调用函数中
			return FALSE;
		}
	}

	//如果消息已经全部接收
	if (TRUE == pMsg_buf->bHas_data_len && pMsg_buf->nMsg_header_len == pMsg_buf->nRecv_header_len &&
		pMsg_buf->nRecv_body_len == pMsg_buf->nMsg_body_len)
	{	
		INFO("RcvMsgFromClient: we recv the client message finish%s", "");
		if (CLIENT_SERVER_MSG == pMsg_buf->bMsg_type)
		{
			HandleClientSrvMsg(pMsg_buf, nSock);	
		}
		else if (PUSHTO_CLIENT_RESPONSE == pMsg_buf->bMsg_type)
		{
			HandlePushToCliResMsg(pMsg_buf, nSock);
		}
	
		//把数据包放入消息队列中
		//modify by luguanhuang 20111009 reason: 添加了队列为满的情况
		//如果队列为满, 就把这条消息指向的堆空间内存释放

		nRet = PutMessageToQueue(pMsg_buf, &g_msg_queue.msg_queue, &g_msg_queue.queue_cond);
		if (TRUE != nRet)
		{
			ERROR("RcvMsgFromClient: Call PutMessageToQueue error%s", "");	
		}

		//把客户端哈希缓存的一项删除,   但是不把返回的值内存删除,  因为
		//还需要把该数值放入消息队列里面,  等待发送线程从消息队列里面取出消息
		pDel_value = HashDel(&pMsg_buf_hash, szSock);
		if (NULL == pDel_value)
		{
			INFO("RcvMsgFromClient: delete client message buffer from client message buffer hashtable error%s", "");
		}
		else
		{
			INFO("RcvMsgFromClient: delete client message buffer from client message buffer hashtable succeed%s", "");	
		}

		//改变心跳包时间
		nRet = ChangeHeartBeatPacketTime(nSock);
		if (TRUE != nRet)
		{
			ERROR("RcvMsgFromClient: Call ChangeHeartBeatPacketTime error%s", "");	
		}

		nRecv_msg_state = RECV_MSG_FINISH;
	}
	else
	{
		INFO("RcvMsgFromClient: we are not finish to recv one message now%s", "");
		nRecv_msg_state = RECV_MSG_NOTFINISH;
	}

	INFO("RcvMsgFromClient: func end%s", "");
	return nRecv_msg_state;
}


int S_ChangeHeartbeatTime(int nSock, pthread_t thread_id)
{
	char szSock[100] = {0};
	long lCur_time = 0;
	
	snprintf(szSock, sizeof(nSock), "%d", nSock);

	StHash_Item *pItem = NULL;
	StClient_Socket_Info *pClient_sock_info = NULL;
	
	pItem = HashGetItem(g_heartbeatdetect_table.pHeartbeat_detect_table, szSock);
	lCur_time = time((time_t *)NULL);
	if (pItem != NULL)
	{
		pthread_mutex_lock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
		pClient_sock_info = (StClient_Socket_Info *)pItem->pMatch_msg;
		pClient_sock_info->lTime = lCur_time;
		pthread_mutex_unlock(&g_heartbeatdetect_table.heartbeat_detect_mutex);
	}
	else
	{
		pClient_sock_info = (StClient_Socket_Info *)MM_MALLOC_WITH_DESC(sizeof(StClient_Socket_Info), \
		"ChangeHeartbeatTime: Call func for client socket info");
		if (NULL == pClient_sock_info)
		{
			FATAL("ChangeHeartbeatTime: Call malloc error%s", "");
			return FALSE;
		}

		memset(pClient_sock_info, 0, sizeof(StClient_Socket_Info));
		pClient_sock_info->thread_id = thread_id;
		pClient_sock_info->lTime = lCur_time;

		HashInsert(&g_heartbeatdetect_table.pHeartbeat_detect_table, szSock, pClient_sock_info);
	}
	
	return TRUE;
}

int DelHeartBeaTabletItem(int nSock)
{
	char szSock[100] = {0};
	snprintf(szSock, sizeof(szSock) - 1, "%d", nSock);
	void *pData = HashDel(&g_heartbeatdetect_table.pHeartbeat_detect_table, szSock);
	if (NULL != pData)
	{
		MM_FREE(pData);		
		INFO("DelHeartBeaTabletItem: delete heartbeat detect table item succeed%s", "");
	}
	else
	{
		INFO("DelHeartBeaTabletItem: delete heartbeat detect table item fail%s", "");	
	}
	
	return TRUE;
}

#if 0
//接收从客户端发送来的数据包
void S_RcvMsgFromClient(void *pSock)
{
	INFO("S_RcvMsgFromClient: func begin%s", "");
	pthread_detach(pthread_self());
	int *pClient_sock = (int *)pSock;
	Client_Server_Msg *pClient_srv_msg = NULL;
	int nRet = 0;
	pthread_t thread_id = pthread_self(); 
		
	while (1)
	{
		pClient_srv_msg = (Client_Server_Msg *)MM_MALLOC_WITH_DESC(sizeof(Client_Server_Msg), \
		"S_RcvMsgFromClient: Call func for client server message");
		if (NULL == pClient_srv_msg)
		{
			FATAL("S_RcvMsgFromClient: Call malloc error%s", "");
			DelHeartBeaTabletItem(*pClient_sock);
			close(*pClient_sock);
			MM_FREE(pClient_sock);
			pthread_exit(NULL);
			return;
		}

		if (FALSE == RecvClientMsg(*pClient_sock, pClient_srv_msg))
		{
			ERROR("S_RcvMsgFromClient: Call RecvClientMsg error%s", "");
			DelHeartBeaTabletItem(*pClient_sock);
			MM_FREE(pClient_srv_msg);
			close(*pClient_sock);
			MM_FREE(pClient_sock);
			pthread_exit(NULL);
			return;
		}

		S_ChangeHeartbeatTime(*pClient_sock, thread_id);
		
		char arrCheck_code[5] = {0};
		memcpy(arrCheck_code, pClient_srv_msg->arrCheck_code, 4); 
		DEBUG("S_RcvMsgFromClient: version=%d msg_type=%d main service code=%d"
		" sub service code=%d access_server_seq=%d msg_counter=%d client_id=%d"
		" service_seq=%d len=%d check_code=%s", pClient_srv_msg->bVersion, pClient_srv_msg->bMsg_type, \
		pClient_srv_msg->bMain_service_code, pClient_srv_msg->bSub_service_code, \
		ntohs(pClient_srv_msg->wAccess_server_seq), ntohl(pClient_srv_msg->nMsg_counter), \
		ntohl(pClient_srv_msg->nAccess_socket_id), ntohs(pClient_srv_msg->wService_server_seq), \
		ntohl(pClient_srv_msg->nData_len), arrCheck_code);

		if (ACCESS_SRV_VERSION == pClient_srv_msg->bVersion)
		{
			pClient_srv_msg->wAccess_server_seq = htons(ACCESS_SRV_SEQ);
			pthread_mutex_lock(&g_msg_counter.msg_counter_mutex);
			g_msg_counter.nMsg_counter++;
			if ((pow(2, 32) - 1) == g_msg_counter.nMsg_counter)
			{
				g_msg_counter.nMsg_counter = 1;
			}
		
			pClient_srv_msg->nMsg_counter = htonl(g_msg_counter.nMsg_counter);
			pthread_mutex_unlock(&g_msg_counter.msg_counter_mutex);
			
			pClient_srv_msg->nAccess_socket_id = htonl(*pClient_sock);

			nRet = PutMessageToQueue(pClient_srv_msg, &g_msg_queue.msg_queue, &g_msg_queue.queue_cond);
		}
		else
		{
			MM_FREE(pClient_srv_msg->pData);
			MM_FREE(pClient_srv_msg);
			//close(*pClient_sock);
			//版本号不对, 直接把数据包丢失, 不需要结束整个线程
			//pthread_exit(NULL);
		}
	}

	INFO("S_RcvMsgFromClient: func end%s", "");
}
#else

static void err_quit(const char *msg)   
{   
    printf("%s\n", msg);   
    exit(0);   
}   


ssize_t read_fd(int fd, int *recvfd)   
{   
	TRACE();
    struct msghdr   msg;   
    struct iovec    iov[1];   
    ssize_t         n;   
  
#ifdef  HAVE_MSGHDR_MSG_CONTROL   
    union {   
      struct cmsghdr    cm;   
      char              control[CMSG_SPACE(sizeof(int))];   
    } control_un;   
    struct cmsghdr  *cmptr;   
  
    msg.msg_control = control_un.control;   
    msg.msg_controllen = sizeof(control_un.control);   
#else   
    int             newfd;   
  
    msg.msg_accrights = (caddr_t) &newfd;   
    msg.msg_accrightslen = sizeof(int);   
#endif   
  
    msg.msg_name = NULL;   
    msg.msg_namelen = 0;   

  	char c = 0;
    iov[0].iov_base = &c;   
    iov[0].iov_len = 1;   
    msg.msg_iov = iov;   
    msg.msg_iovlen = 1;   
  
    if ( (n = recvmsg(fd, &msg, 0)) <= 0)   
        return(n);   
  
#ifdef  HAVE_MSGHDR_MSG_CONTROL   
    if ( (cmptr = CMSG_FIRSTHDR(&msg)) != NULL &&   
        cmptr->cmsg_len == CMSG_LEN(sizeof(int))) {   
        if (cmptr->cmsg_level != SOL_SOCKET)   
            err_quit("control level != SOL_SOCKET");   
        if (cmptr->cmsg_type != SCM_RIGHTS)   
            err_quit("control type != SCM_RIGHTS");   
        *recvfd = *((int *) CMSG_DATA(cmptr));   
    } else   
        *recvfd = -1;       /* descriptor was not passed */   
#else   
/* *INDENT-OFF* */   
    if (msg.msg_accrightslen == sizeof(int))   
        *recvfd = newfd;   
    else   
        *recvfd = -1;       /* descriptor was not passed */   
/* *INDENT-ON* */   
#endif   
  
    return(n);   
}   
  

//接收从客户端发送来的数据包
void S_RcvMsgFromClient(void *pSock, int iSockFd)
{
	INFO("S_RcvMsgFromClient: func begin%s", "");
	pthread_detach(pthread_self());
	int *pClient_sock = (int *)pSock;
	int nRet = 0;
	pthread_t thread_id = pthread_self(); 
	Client_Server_Msg cliSrvMsg;
	struct StMsg stMsg;
	int iLen = 0;

	int iCliSock = 0;
		
	while (1)
	{	
		#if 0
		read_fd(iSockFd, &iCliSock);
		INFO("S_RcvMsgFromClient: [sock]=%d  [conpare socket]=%d", \
			iCliSock, *pClient_sock);
		*pClient_sock = iCliSock;
		#endif
		memset(&cliSrvMsg, 0, sizeof(cliSrvMsg));
		if (FALSE == RecvClientMsg(*pClient_sock, &cliSrvMsg))
		{
			ERROR("S_RcvMsgFromClient: Call RecvClientMsg error%s", "");
			DelHeartBeaTabletItem(*pClient_sock);
			close(*pClient_sock);
			exit(1);
		}

		S_ChangeHeartbeatTime(*pClient_sock, thread_id);
		char arrCheck_code[5] = {0};
		memcpy(arrCheck_code, cliSrvMsg.arrCheck_code, 4); 
		DEBUG("S_RcvMsgFromClient: version=%d msg_type=%d main service code=%d"
		" sub service code=%d access_server_seq=%d msg_counter=%d client_id=%d"
		" service_seq=%d len=%d check_code=%s", cliSrvMsg.bVersion, cliSrvMsg.bMsg_type, \
		cliSrvMsg.bMain_service_code, cliSrvMsg.bSub_service_code, \
		ntohs(cliSrvMsg.wAccess_server_seq), ntohl(cliSrvMsg.nMsg_counter), \
		ntohl(cliSrvMsg.nAccess_socket_id), ntohs(cliSrvMsg.wService_server_seq), \
		ntohl(cliSrvMsg.nData_len), arrCheck_code);

		if (ACCESS_SRV_VERSION == cliSrvMsg.bVersion)
		{
			cliSrvMsg.wAccess_server_seq = htons(ACCESS_SRV_SEQ);
			pthread_mutex_lock(&g_msg_counter.msg_counter_mutex);
			g_msg_counter.nMsg_counter++;
			if ((pow(2, 32) - 1) == g_msg_counter.nMsg_counter)
			{
				g_msg_counter.nMsg_counter = 1;
			}
		
			cliSrvMsg.nMsg_counter = htonl(g_msg_counter.nMsg_counter);
			pthread_mutex_unlock(&g_msg_counter.msg_counter_mutex);
			
			cliSrvMsg.nAccess_socket_id = htonl(*pClient_sock);
			memset(&stMsg, 0, sizeof(stMsg));
			stMsg.msg_types = 1;
			iLen = sizeof(cliSrvMsg) - 8;
			memcpy(stMsg.msg_buf, &cliSrvMsg, iLen);
			memcpy(stMsg.msg_buf + iLen, cliSrvMsg.pData, \
				ntohl(cliSrvMsg.nData_len));
			nRet = msgsnd(g_iQid, &stMsg, 511, 0);
			INFO("S_RcvMsgFromClient: msgsnd [return val]=%d", nRet);
			if (nRet)
			{
				ERROR("S_RcvMsgFromClient: Call msgsnd error error[%d]=%s", \
					errno, strerror(errno));
			}
			//nRet = PutMessageToQueue(cliSrvMsg, &g_msg_queue.msg_queue, &g_msg_queue.queue_cond);
		}
		else
		{
			MM_FREE(cliSrvMsg.pData);
			//close(*pClient_sock);
			//版本号不对, 直接把数据包丢失, 不需要结束整个线程
			//pthread_exit(NULL);
		}
	}

	INFO("S_RcvMsgFromClient: func end%s", "");
}

#endif
