
#include "../util/access_global.h"
//#include "../include/ydt_log.h"
#include "../interface/access_protocol.h"
#include "access_sock_info.h"
#include "access_thread_pool.h"
#include "access_comm_service_srv_rcv.h"
#include "access_heartbeat_detectthread.h"
#include "access_comm_client_snd.h"

extern int g_iSndCliQid; 

/**********该文件主要是用来与客户端进行交互(发送消息给客户端*********/

//函数用途: 发送转发消息到客户端
//输入参数: socket id,  存放消息的缓存
//输出参数: 无 
//返回值	:  发送成功,  返回TRUE ， 发送失败,  返回FALSE
extern StClientInfo_Hashtable g_clientinfo_hash;
extern StTmp_clientSock_Hash g_sndclient_tmp_sockhash;

extern StSnd_Client_MsgQueue g_sndclient_msgqueue;

int SendForwardToClientMsg(int nSock, void *pMsg)
{
	Forward_Srv_Client_Msg *pForward_srv_client_msg = (Forward_Srv_Client_Msg *)pMsg;

	BYTE bMsg_type = pForward_srv_client_msg->bMsg_type;
	
	int nSnd_len = 0;
	int nData_len = 0;

	//PrintContent("SendForwardToClientMsg\n");
	WORD wCmd_id = *(WORD *)(pForward_srv_client_msg->pData + 1);
	wCmd_id = ntohs(wCmd_id);

	char szLog_info[1024] = {0};
	
	snprintf(szLog_info, sizeof(szLog_info) - 1, "SendForwardToClientMsg: enter SendForwardToClientMsg msg_type=%d cmd_id=%d", bMsg_type, wCmd_id);
	LOG_DEBUG(szLog_info, FILE_NAME, FILE_LINE);

	DEBUG("SendForwardToClientMsg: enter SendForwardToClientMsg msg_type=%d cmd_id=%d", bMsg_type, wCmd_id);

	//发送消息头
	nSnd_len = Send(nSock, (char *)pForward_srv_client_msg, sizeof(Forward_Srv_Client_Msg) - 8, wCmd_id);
	if (-1 == nSnd_len)
	{
		LOG_ERROR("SendForwardToClientMsg: Call Send error", FILE_NAME, FILE_LINE);
		return FALSE;
	}

	nData_len = ntohl(pForward_srv_client_msg->nData_len);

	//发送消息体
	nSnd_len = Send(nSock, (char *)pForward_srv_client_msg->pData, nData_len, wCmd_id);
	if (-1 == nSnd_len)
	{
		LOG_ERROR("SendForwardToClientMsg: Call Send error", FILE_NAME, FILE_LINE);
		return FALSE;
	}

	//发送校验码
	nSnd_len = Send(nSock, (char *)pForward_srv_client_msg->arrCheck_code, 4, wCmd_id);
	if (-1 == nSnd_len)
	{
		LOG_ERROR("SendForwardToClientMsg: Call Send error", FILE_NAME, FILE_LINE);
		return FALSE;
	}

	snprintf(szLog_info, sizeof(szLog_info) - 1, "SendForwardToClientMsg: leave SendForwardToClientMsg msg_type=%d cmd_id=%d", bMsg_type, wCmd_id);
	LOG_DEBUG(szLog_info, FILE_NAME, FILE_LINE);

	DEBUG("SendForwardToClientMsg: leave SendForwardToClientMsg msg_type=%d cmd_id=%d", bMsg_type, wCmd_id);
		
	return TRUE;
}

int SendResponseToClientMsg(int nSock, void *pMsg)
{
	TRACE();
	Server_Client_Msg *pSrv_client_msg = (Server_Client_Msg *)pMsg;
	char *pTmpMsg = (char *)pMsg;
	int nSnd_len = 0;
	int nData_len = 0;

	//WORD wCmd_id = *(WORD *)(pSrv_client_msg->pData + 1);
	//wCmd_id = ntohs(wCmd_id);
	//WORD wCmd_id = 0;
	//发送消息头
	WORD wCmd_id  = 0;
	nSnd_len = Send(nSock, (char *)pSrv_client_msg, sizeof(Server_Client_Msg) - 8, wCmd_id);
	if (-1 == nSnd_len)
	{
		ERROR("SendResponseToClientMsg: Call Send error%s", "");
		return FALSE;
	}

	nData_len = ntohl(pSrv_client_msg->nData_len);

	INFO("SendResponseToClientMsg: [data len]=%d", nData_len);
	//发送消息体
	pTmpMsg += sizeof(Server_Client_Msg) - 8;
	//nSnd_len = Send(nSock, (char *)pSrv_client_msg->pData, nData_len, wCmd_id);
	nSnd_len = Send(nSock, pTmpMsg, nData_len, wCmd_id);
	if (-1 == nSnd_len)
	{
		ERROR("SendResponseToClientMsg: Call Send error%s", "");
		return FALSE;
	}

	pTmpMsg += nData_len;
	//发送校验码
	//nSnd_len = Send(nSock, (char *)pSrv_client_msg->arrCheckout_code, 4, wCmd_id);
	nSnd_len = Send(nSock, pTmpMsg, 4, wCmd_id);
	if (-1 == nSnd_len)
	{
		ERROR("SendResponseToClientMsg: Call Send error%s", "");
		return FALSE;
	}

	char arrAcc_checkcode[5] = {0};
	memcpy(arrAcc_checkcode, pSrv_client_msg->arrCheckout_code, 4);

	DEBUG("SendResponseToClientMsg: [version]=%d [msg type]=%d [main service code]=%d [sub service code]=%d"
		" [access server seq]=%d [msg counter]=%d [access check code]=%s [data len]=%d [cmd_id]=%d", pSrv_client_msg->bVersion, pSrv_client_msg->bMsg_type, \
		pSrv_client_msg->bMain_service_code, pSrv_client_msg->bSub_service_code, ntohs(pSrv_client_msg->wAccess_server_seq), \
		ntohl(pSrv_client_msg->nMsg_counter), arrAcc_checkcode, ntohl(pSrv_client_msg->nData_len), wCmd_id);	
	
	INFO("SendResponseToClientMsg: func end%s", "");
	return TRUE;
}


//函数用途: 发送响应消息到客户端
//输入参数: socket id,  存放消息的缓存
//输出参数: 无 
//返回值	:  发送成功,  返回TRUE ， 发送失败,  返回FALSE

int SendPushToClientMsg(int nSock, void *pMsg)
{
	INFO("SendPushToClientMsg: func begin%s", "");

	PushTo_Client_Msg *pPushto_client_msg = (PushTo_Client_Msg *)pMsg;
	int nSnd_len = 0;
	int nData_len = 0;

	WORD wCmd_id = *(WORD *)(pPushto_client_msg->pData + 1);
	int nHeader_len = Offset(PushTo_Client_Msg, pData);
	wCmd_id = ntohs(wCmd_id);
	//发送消息头
	nSnd_len = Send(nSock, (char *)pPushto_client_msg, nHeader_len, wCmd_id);
	if (-1 == nSnd_len)
	{
		ERROR("SendPushToClientMsg: Call Send error%s", "");
		return FALSE;
	}

	nData_len = ntohl(pPushto_client_msg->nData_len);

	//发送消息体
	nSnd_len = Send(nSock, (char *)pPushto_client_msg->pData, nData_len, wCmd_id);
	if (-1 == nSnd_len)
	{
		ERROR("SendPushToClientMsg: Call Send error%s", "");
		return FALSE;
	}

	//发送校验码
	nSnd_len = Send(nSock, (char *)pPushto_client_msg->arrCheck_code, 4, wCmd_id);
	if (-1 == nSnd_len)
	{
		ERROR("SendPushToClientMsg: Call Send error%s", "");
		return FALSE;
	}

	char arrAcc_checkcode[5] = {0};
	memcpy(arrAcc_checkcode, pPushto_client_msg->arrCheck_code, 4);

	DEBUG("SendPushToClientMsg: [version]=%d [msg type]=%d [main service code]=%d [sub service code]=%d"
		" [access server seq]=%d [client socket]=%d [access check code]=%s [data len]=%d [cmd_id]=%d", \
		pPushto_client_msg->bVersion, pPushto_client_msg->bMsg_type, \
		pPushto_client_msg->bMain_service_code, pPushto_client_msg->bSub_service_code, \
		ntohs(pPushto_client_msg->wAccess_srv_seq), \
		ntohl(pPushto_client_msg->nClient_sock), arrAcc_checkcode, \
		ntohl(pPushto_client_msg->nData_len), wCmd_id);	
	
	INFO("SendPushToClientMsg: func end%s", "");
	return TRUE;
}



int SendDisconnWithClientMsg(int nSock, void *pMsg)
{
	INFO("SendDisconnWithClientMsg: func begin%s", "");
	StReq_DisconnWithClient_MSG *pDisconn_withclient_msg = (StReq_DisconnWithClient_MSG *)pMsg;
	int nSnd_len = 0;
	int nData_len = 0;

	WORD wCmd_id = *(WORD *)(pDisconn_withclient_msg->pData + 1);
	wCmd_id = ntohs(wCmd_id);
	//发送消息头
	nSnd_len = Send(nSock, (char *)pDisconn_withclient_msg, sizeof(Server_Client_Msg) - 8, wCmd_id);
	if (-1 == nSnd_len)
	{
		ERROR("SendDisconnWithClientMsg: Call Send error%s", "");
		return FALSE;
	}

	nData_len = ntohl(pDisconn_withclient_msg->nData_len);

	//发送消息体
	nSnd_len = Send(nSock, (char *)pDisconn_withclient_msg->pData, nData_len, wCmd_id);
	if (-1 == nSnd_len)
	{
		ERROR("SendDisconnWithClientMsg: Call Send error%s", "");
		return FALSE;
	}

	//发送校验码
	nSnd_len = Send(nSock, (char *)pDisconn_withclient_msg->arrCheck_code, 4, wCmd_id);
	if (-1 == nSnd_len)
	{
		ERROR("SendDisconnWithClientMsg: Call Send error%s", "");
		return FALSE;
	}

	char arrAcc_checkcode[5] = {0};
	memcpy(arrAcc_checkcode, pDisconn_withclient_msg->arrCheck_code, 4);

	DEBUG("SendDisconnWithClientMsg: [version]=%d [msg type]=%d [main service code]=%d [sub service code]=%d"
		" [access server seq]=%d [client sock]=%d [access check code]=%s [data len]=%d [cmd_id]=%d", pDisconn_withclient_msg->bVersion, pDisconn_withclient_msg->bMsg_type, \
		pDisconn_withclient_msg->bMain_service_code, pDisconn_withclient_msg->bSub_service_code, ntohs(pDisconn_withclient_msg->wAccess_server_seq), \
		ntohl(pDisconn_withclient_msg->nClient_socket), arrAcc_checkcode, ntohl(pDisconn_withclient_msg->nData_len), wCmd_id);	
	
	INFO("SendDisconnWithClientMsg: func end%s", "");
	return TRUE;
}



//函数用途: 发送消息到客户端
//输入参数: socket id,  存放消息的缓存, 消息类型
//输出参数: 无 
//返回值	:  发送成功,  返回TRUE ， 发送失败,  返回FALSE

int SendClientMsg(int nSock, void *pMsg, WORD wMsg_type)
{
	int nRet = 0; 
	INFO("SendClientMsg: func begin%s", "");
	INFO("SendClientMsg: [msg type]=%d", wMsg_type);
	switch (wMsg_type)
	{
		case RESPONSE_CLIENT_MSG:
			nRet = SendResponseToClientMsg(nSock, pMsg);			//发送响应消息
			break;
		case FORWARD_CLIENT_MSG:
			nRet = SendForwardToClientMsg(nSock, pMsg);				//发送转发消息
			break;
		case NOTIFY_DISCONNECT_WITHCLIENT_MSG:
			nRet = SendDisconnWithClientMsg(nSock, pMsg);
			break;
		case PUSHTO_CLIENT_MSG:
			nRet = SendPushToClientMsg(nSock, pMsg);
			break;
		default:
			break;
	} 

	INFO("SendClientMsg: func end%s", "");
	return nRet;
}

void DelSndClientTmpSockFromHash(int nSock)
{
	INFO("DelSndClientTmpSockFromHashtable: func begin%s", "");
	void *pData = NULL;
	char szSock[100] = {0};
 
	snprintf(szSock, sizeof(szSock) - 1, "%d", nSock);
	pData = HashDel(&(g_sndclient_tmp_sockhash.pTmp_clientsock_table), szSock);
	if (NULL != pData)
	{
		INFO("DelSndClientTmpSockFromHashtable: delete temp client socket from temp client socket hashtable succeed%s", "");
		MM_FREE(pData);
	}
	else
	{
		INFO("DelSndClientTmpSockFromHashtable: delete temp client socket from temp client socket hashtable fail%s", "");		
	}

	INFO("DelSndClientTmpSockFromHashtable: func end%s", "");
}

//该函数做大并发的时候有问题
void DelClientInfoFromHash(int nSock)
{	
	INFO("DelClientInfoFromHash: func begin%s", "");
	int i = 0;
	int nSize = 0;
	StHash_Item *pItem = NULL;
	StHash_Item *pLast_item = NULL;
	Client_Info *pClient_info = NULL;
	
	pthread_mutex_lock(&g_clientinfo_hash.client_info_mutex);
	StHash_Table *pHashtable = g_clientinfo_hash.pClient_info_hash;
	nSize = pHashtable->nBucket_size;
	for (i=0; i<nSize; i++)
	{
		pItem = pHashtable->ppItem[i];
		pLast_item = pItem;
		while (NULL != pItem)
		{
			pClient_info = (Client_Info *)pItem->pMatch_msg;	
			if (nSock == pClient_info->nClient_socket)
			{
				INFO("DelClientInfoFromHash: client is disconnect with access server"
					" so we will delete the client info item from client info hashtable [client socket]=%d [hashtable length]=%d", \
					nSock, pHashtable->nHashtable_len);
				if (pLast_item == pItem)
				{
					pHashtable->ppItem[i] = pItem->pNext;
					pLast_item = pItem->pNext;
					MM_FREE(pItem->pKey);
					MM_FREE(pItem->pMatch_msg);
					MM_FREE(pItem);	
					pItem = pLast_item;
				}
				else
				{
					pLast_item->pNext = pItem->pNext;
					MM_FREE(pItem->pKey);
					MM_FREE(pItem->pMatch_msg);
					MM_FREE(pItem);
					pItem = pLast_item->pNext;
				}

				if (pHashtable->nHashtable_len > 0)
				{
					pHashtable->nHashtable_len--;
				}
			}
			else
			{
				pLast_item = pItem;
				pItem = pItem->pNext;
			}
		}
	}
	pthread_mutex_unlock(&g_clientinfo_hash.client_info_mutex);
	INFO("DelClientInfoFromHash: func end%s", "");
}


//还没从客户端消息队列中删除掉与客户端断连的消息包
void DelClientSockInfo(int nSock)
{
	INFO("DelCloseClientSockInfo: func begin%s", "");
	int nRet = 0;
	
	nRet = DelSockEventFromepoll(nSock);
	if (FALSE == nRet)
	{
		INFO("DelCloseClientSockInfo: Call DelSockEventFromepoll error%s", "");
	}
	else
	{
		INFO("DelCloseClientSockInfo: Call DelSockEventFromepoll succeed%s", "");
	}
	
	DelClientInfoFromHash(nSock);
	
	DelClientSockFromQueue(nSock);
	DelTmpClientSockFromHashtable(nSock);
	DelClientMsgBufFromHash(nSock);
	DeleteHeartbeatDetectItem(nSock);			//删除心跳包里面的一项
	DelSndClientTmpSockFromHash(nSock);
	INFO("DelCloseClientSockInfo: func end%s", "");
}

static void DelMsgMemory(void *pMsg, BYTE bMsg_type)
{
	INFO("DelMsgMemory: func begin%s", "");
	if (SERVER_CLIENT_MSG == bMsg_type)
	{
		Server_Client_Msg *pTmp_msg = (Server_Client_Msg *)pMsg;
		MM_FREE(pTmp_msg->pData);
		MM_FREE(pMsg);
	}
	else if (NOTIFY_DISCONNECT_WITHCLIENT_MSG == bMsg_type)
	{
		INFO("DelMsgMemory: we will del the disconn with client msg memory%s", "");
		StReq_DisconnWithClient_MSG *pDisconn_withclient_msg = (StReq_DisconnWithClient_MSG *)pMsg;
		MM_FREE(pDisconn_withclient_msg->pData);
		MM_FREE(pMsg);
	}
	
	INFO("DelMsgMemory: func end%s", "");
}

int SndMsgToCli(int iRcvSock)
{
	TRACE();
	struct StMsg stMsg;
	memset(&stMsg, 0, sizeof(stMsg));
	int iLen = 0;
	int iSock = 0;
	char aSn[50] = {0};
	char *pTmpMsg = NULL;
	char aHead[40] = {0};
	Server_Client_Msg *pSrv_client_msg = NULL;
	int iCliSock = 0;
		
	while (1)
	{
		read_fd(iRcvSock, &iCliSock);
		INFO("SndMsgToCli: [sock]=%d", \
			iCliSock);
	
		iLen = msgrcv (g_iSndCliQid, &stMsg, 511, 0, 0); 
		INFO("SndMsgToCli: [return val]=%d", iLen);
		if (0 == iLen)
		{
			INFO("SndMsgToCli: msg queue don't have data");
			continue;
		}
		else if (iLen > 0)
		{
			INFO("SndMsgToCli: msg queue have data");	
		}
		else
		{
			ERROR("SndMsgToCli: Call msgrcv error");
			continue;
		}	

		pTmpMsg = stMsg.msg_buf;
		iSock = *(int *)pTmpMsg;
		pTmpMsg += sizeof(int);
		memset(aSn, 0, sizeof(aSn));
		memcpy(aSn, pTmpMsg, sizeof(aSn) - 1);
		pTmpMsg += sizeof(aSn) - 1;
		memset(aHead, 0, sizeof(aHead));
		char *pMsgBegin = pTmpMsg;
		memcpy(aHead, pTmpMsg, sizeof(Server_Client_Msg) - 8);
		pSrv_client_msg = (Server_Client_Msg *)aHead;
		pTmpMsg += sizeof(Server_Client_Msg) - 8;
		iLen = ntohl(pSrv_client_msg->nData_len);
		char aBuf[1024] = {0};
		memcpy(aBuf, pTmpMsg, iLen);
		char *pTmp = aBuf;
		char cVer = *pTmp;
		pTmpMsg++;
		WORD wCmdId = ntohs(*(WORD *)pTmpMsg);
		pTmpMsg += sizeof(WORD);
		WORD wLen = ntohs(*(WORD *)pTmpMsg);
		pTmpMsg += sizeof(WORD);
		char aContent[1024] = {0};
		memcpy(aContent, pTmpMsg, wLen);
		pTmpMsg += wLen;
		char aCheckCode[5] = {0};
		memcpy(aCheckCode, pTmpMsg, 4);
		
		DEBUG("SndMsgToCli: [version]=%d [msg type]=%d [main service code]=%d [sub service code]=%d"
		" [access server seq]=%d [msg counter]=%d  [data len]=%d [version]=%d [command id]=%d [len]=%d [content]=%s [check code]=%d", \
		pSrv_client_msg->bVersion, pSrv_client_msg->bMsg_type, \
		pSrv_client_msg->bMain_service_code, pSrv_client_msg->bSub_service_code, ntohs(pSrv_client_msg->wAccess_server_seq), \
		ntohl(pSrv_client_msg->nMsg_counter), ntohl(pSrv_client_msg->nData_len), cVer, wCmdId, wLen, aContent, \
		aCheckCode);	

		INFO("SndMsgToCli: [socket]=%d [sn]=%s", \
			iCliSock, aSn);

		int nRet = SendClientMsg(iCliSock, pMsgBegin, pSrv_client_msg->bMsg_type);
		if (FALSE == nRet)
		{
			ERROR("SndMsgToCli: Call SendClientMsg error send msg to client error");	
		}
		
	}

	
	
	return 1;
}

void *SndMsgToClientThread(void *pParam)
{
	INFO("SndMsgToClientThread: func begin%s", "");
	pthread_detach(pthread_self());
	int nRet = 0;
	pthread_t thread_id = pthread_self();
	StNode *pTmp_node = NULL;
	int nData = 0;
	Srv_Client_Msg_Info *pSrv_client_msg_info = NULL;
	int nClient_sock = 0;
	BYTE bMsg_type = 0;
	char *pTmp_msg = NULL;
	

	while (TRUE)
	{
		pthread_mutex_lock(&g_sndclient_msgqueue.queue_mutex);
		nRet = IsLinkQueueEmpty(&g_sndclient_msgqueue.stLink_queue);
		while (TRUE == nRet)
		{
			INFO("SndMsgToClientThread: link queue is empty so we will wait a signal to get item"
						" from link queue [thread id]=%u", thread_id);
			pthread_cond_wait(&g_sndclient_msgqueue.queue_cond, &g_sndclient_msgqueue.queue_mutex);
			INFO("SndMsgToClientThread: we recv a signal to get item from  link queue [thread id]=%u", thread_id);
			nRet = IsLinkQueueEmpty(&g_sndclient_msgqueue.stLink_queue); 				
		}

		pTmp_node = g_sndclient_msgqueue.stLink_queue.pFront;
		while (NULL != pTmp_node)
		{
			nData =pTmp_node->nData;
			pSrv_client_msg_info = (Srv_Client_Msg_Info *)nData;
			nClient_sock = pSrv_client_msg_info->nClient_sock;
			nRet = HandleClientSock(nClient_sock, &g_sndclient_tmp_sockhash);
			if (TRUE == nRet)
			{
				INFO("SndMsgToClientThread: the socket is not exist in the temp socket hashtable, so we will use this socket to"
					" snd client message [socket id]=%d", nClient_sock);
				DelFromQueuetByValue(&g_sndclient_msgqueue.stLink_queue, nData);
				break;	
			}
			else if (CLIENT_SOCK_EXIST == nRet)
			{
				INFO("SndMsgToClientThread: the socket is exist yet in the temp socket hashtable, so we will get next"
									" msg from msg link queuet to snd client message%s", "");
				pTmp_node = pTmp_node->pNext;	
				continue;
					
			}
			else if (FALSE == nRet)
			{
				ERROR("SndMsgToClientThread: Call HandleClientSock error%s", "");
				pTmp_node = NULL;
			}
		}
		pthread_mutex_unlock(&g_sndclient_msgqueue.queue_mutex);

		if (NULL == pTmp_node)
		{
			INFO("SndMsgToClientThread: current thread can't find the item from msg queue"
				" to snd to client or insert socket to client temp socket hashtable error%s", "");
			pthread_mutex_lock(&g_sndclient_msgqueue.queue_mutex);
			INFO("SndMsgToClientThread: we will wait a signal to continue search the queue to get the msg"
				" and snd to client%s", "");	
			pthread_cond_wait(&g_sndclient_msgqueue.queue_cond, &g_sndclient_msgqueue.queue_mutex);	
			INFO("SndMsgToClientThread: we recv a signal to continue search the queue to get the msg"
				" and snd to client%s", "");	
			pthread_mutex_unlock(&g_sndclient_msgqueue.queue_mutex);
			continue;
		}

		pTmp_msg = (char *)pSrv_client_msg_info->pMsg;
		pTmp_msg++;
		bMsg_type = *pTmp_msg;

		nRet = SendClientMsg(nClient_sock, pSrv_client_msg_info->pMsg, bMsg_type);
		if (FALSE == nRet)
		{
			ERROR("SndMsgToClientThread: Call SendClientMsg error send msg to client error%s", "");	
			WARN("SndMsgToClientThread: as client is disconnect with access server so we will delete some info relate with client%s", "");
			DelClientSockInfo(nClient_sock);
			close(nClient_sock);

			DelMsgMemory(pSrv_client_msg_info->pMsg, bMsg_type);
			MM_FREE(pSrv_client_msg_info);
			continue;
		}

		INFO("SndMsgToClientThread: Send msg to client successful%s", "");

		if (NOTIFY_DISCONNECT_WITHCLIENT_MSG == bMsg_type)
		{
			INFO("SndMsgToClientThread: we will close the socket with client%s", "");
			DelClientSockInfo(nClient_sock);
			close(nClient_sock);	
			DelMsgMemory(pSrv_client_msg_info->pMsg, bMsg_type);
			MM_FREE(pSrv_client_msg_info);
			continue;
		}

				
		void *pData = HashDel(&g_clientinfo_hash.pClient_info_hash, pSrv_client_msg_info->arrSn);
		if (pData != NULL)
		{		
			MM_FREE(pData);
			INFO("SndMsgToClientThread: delete client info from client info hashtable succeed%s", "");
		}
		else
		{
			INFO("SndMsgToClientThread: delete client info from client info hashtable fail%s", "");	
		}
		

		DelMsgMemory(pSrv_client_msg_info->pMsg, bMsg_type);
		MM_FREE(pSrv_client_msg_info);
		DelSndClientTmpSockFromHash(nClient_sock);
	}
	
	INFO("SndMsgToClientThread: func end%s", "");
}

