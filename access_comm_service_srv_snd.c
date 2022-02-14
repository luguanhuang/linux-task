
#include "../util/access_global.h"
//#include "../include/ydt_log.h"
#include "../interface/access_protocol.h"
#include "access_sock_info.h"
#include "access_comm_client_rcv.h"
#include "access_routing_maintain.h"
#include "../util/access_server_seq.h"
#include "../main/access_srv.h"
#include "access_comm_client_rcv.h"
#include "access_comm_service_srv_snd.h"

extern int g_iCliInfoId;
extern CSerRouteInfo g_serRouteInfo;


extern int g_iQid; 
extern int g_iShmId;
extern int g_iSemId;
extern map <string, Server_Info> g_mapSrvInfo;

/**********该文件主要是用来与业务服务器进行交互(发送消息给业务服务器)的************/

//函数用途: 发送请求消息到业务服务器
//输入参数: socket id,  存放消息的缓存
//输出参数: 无 
//返回值	:  发送成功,  返回TRUE ， 发送失败,  返回FALSE

extern StRouting_Table g_routing_table;				//业务路由表
extern Awake_Thread_Flg g_awake_thread_flg;			//唤醒标志

extern Msg_Counter g_msg_counter;
extern StClientInfo_Hashtable g_clientinfo_hash;

extern StMsg_Queue g_msg_queue;

extern Server_Conf_Info g_srv_conf_info;

//函数用途:  插入客户端消息包到哈希表里面
//输入参数: 客户端发送给服务器的请求包
//输出参数:  无
//返回值	: 插入成功,  返回TRUE， 插入失败,  返回FALSE；

int InsertClientInfoIntoHash(char *pMsg)
{
	INFO("InsertClientInfoIntoHash: func begin%s", "");
	if (NULL == pMsg)
	{
		ERROR("InsertClientInfoIntoHash: func Param error%s", "");
		return FUNC_PARAM_ERROR;	
	}

	int nRet = 0;
	Client_Info *pClient_info = (Client_Info *)MM_MALLOC_WITH_DESC(sizeof(Client_Info), \
	"InsertClientInfoIntoHash: Call func for client info");
	if (NULL == pClient_info)
	{	
		FATAL("InsertClientInfoIntoHash: Call malloc error%s", "");
		return 	FALSE;
	}

	memset(pClient_info, 0, sizeof(Client_Info));

	//modify by luguanhuang 20110917(reason: 在栈空间分配Client_Server_Msg)
	//而不是在堆空间分配Client_Server_Msg, 防止内存泄漏的情况产生
	Client_Server_Msg client_srv_msg;
	memset(&client_srv_msg, 0, sizeof(Client_Server_Msg));
	
	memcpy(&client_srv_msg, pMsg, sizeof(Client_Server_Msg) - 8);

	int nHeader_len = sizeof(Client_Server_Msg) - 8;
	int nData_len = ntohl(client_srv_msg.nData_len);
	pClient_info->bMsg_type = client_srv_msg.bMsg_type;
	pClient_info->bMain_service_code = client_srv_msg.bMain_service_code;
	pClient_info->bSub_service_code = client_srv_msg.bSub_service_code;
	pClient_info->wAccess_srv_seq = ntohs(client_srv_msg.wAccess_server_seq);
	pClient_info->nMsg_counter = ntohl(client_srv_msg.nMsg_counter);
	memcpy(pClient_info->arrAccess_check_code, pMsg + nHeader_len + nData_len,  4);
	pClient_info->bService_version = *(pMsg + nHeader_len);
	memcpy(pClient_info->arrService_check_code, pMsg + nHeader_len + nData_len - 4, 4);
	pClient_info->lSent_time = time((time_t *)NULL);
	pClient_info->nClient_socket = ntohl(client_srv_msg.nAccess_socket_id);

	int nSeq = *(int *)(pMsg + nHeader_len + 3);
	pClient_info->nSeq = ntohl(nSeq);

	//DEBUG("InsertClientInfoIntoHash: [sequence]=%d", pClient_info->nSeq);
	snprintf(pClient_info->arrSn, sizeof(pClient_info->arrSn) - 1, "%d%d", ntohs(client_srv_msg.wAccess_server_seq), ntohl(client_srv_msg.nMsg_counter));
	INFO("InsertClientInfoIntoHash: [sn]=%s", pClient_info->arrSn);

	nRet = HashInsert (&g_clientinfo_hash.pClient_info_hash, pClient_info->arrSn, pClient_info);
	if(FALSE == nRet)
	{
		ERROR("InsertClientInfoIntoHash: Call YDT_Hash_Insert error%s", "");
		return FALSE;
	}
	else if (HASH_SAMEKEY_EXIST == nRet)
	{
		StHash_Item *pItem = HashGetItem(g_clientinfo_hash.pClient_info_hash, pClient_info->arrSn);
		if (NULL != pItem)
		{
			pthread_mutex_lock(&g_clientinfo_hash.client_info_mutex);
			MM_FREE(pItem->pMatch_msg);
			pItem->pMatch_msg = (void *)pClient_info;
			pthread_mutex_unlock(&g_clientinfo_hash.client_info_mutex);
		}

		
		INFO("InsertClientInfoIntoHash: key exist yet in the client info hashtable"
						" then we will replace the previous value [sn]=%s", pClient_info->arrSn);
	}
	else
	{
		INFO("InsertClientInfoIntoHash: insert client info into client info hashtable succeed"
				" [sn]=%s", pClient_info->arrSn);	
	}
	INFO("InsertClientInfoIntoHash: func end%s", "");
	return TRUE;
}


int SendRequestMsg(int nSock, void *pMsg)
{
	INFO("SendRequestMsg: func begin%s", "");
	Client_Server_Msg *pClient_srv_msg = (Client_Server_Msg *)pMsg;
	int nSnd_len = 0;
	WORD wCmd_id = 0;

	int iTotalLen = sizeof(Client_Server_Msg) - 8 + ntohl(pClient_srv_msg->nData_len) + 4;
	INFO("SendRequestMsg: [data len]=%d [total len]=%d [socket]=%d", \
		ntohl(pClient_srv_msg->nData_len), iTotalLen, nSock);

	#if 0	
	//发送消息头
	nSnd_len = Send(nSock, (char *)pClient_srv_msg, sizeof(Client_Server_Msg) - 8, wCmd_id);
	
	if (-1 == nSnd_len)
	{
		ERROR("SendRequestMsg: Call Send error%s", "");
		return FALSE;
	}

	nData_len = ntohl(pClient_srv_msg->nData_len);

	//发送消息体
	nSnd_len = Send(nSock, (char *)pClient_srv_msg->pData, nData_len, wCmd_id);
	if (-1 == nSnd_len)
	{
		ERROR("SendRequestMsg: Call Send error%s", "");
		return FALSE;	
	}

	//发送校验码
	nSnd_len = Send(nSock, (char *)pClient_srv_msg->arrCheck_code, 4, wCmd_id);
	if (-1 == nSnd_len)
	{
		ERROR("SendRequestMsg: Call Send error%s", "");
		return FALSE;	
	}

	INFO("SendRequestMsg: func end%s", "");
	#else
	//发送消息头
	nSnd_len = Send(nSock, (char *)pMsg, iTotalLen, wCmd_id);
	if (-1 == nSnd_len)
	{
		ERROR("SendRequestMsg: Call Send error%s", "");
		return FALSE;
	}
	#endif
	return TRUE;
}

//函数用途: 发送响应消息给业务服务器(主要是针对聊天业务的,  把发送给客户端的状态信息发送给相对应的业务服务器)
//输入参数: socket id,  存放消息的缓存
//输出参数: 无 
//返回值	:  发送成功,  返回TRUE ， 发送失败,  返回FALSE

int SendResponseMsg(int nSock, void *pMsg)
{
	Response_Client_Srv_Msg *pRes_client_srv_msg = (Response_Client_Srv_Msg *)pMsg;

	BYTE bMsg_type = pRes_client_srv_msg->bMsg_type;
	DEBUG("SendResponseMsg: enter SendResponseMsg msg_type=%d", bMsg_type);
	
	int nSnd_len = Send(nSock, (char *)pRes_client_srv_msg, sizeof(Response_Client_Srv_Msg), 0);
	if (-1 == nSnd_len)
	{
		ERROR("SendResponseMsg: Call Send error%s", "");
		return FALSE;
	}

	DEBUG("SendResponseMsg: leave SendResponseMsg msg_type=%d", bMsg_type);
	
	return TRUE;
}


//函数用途: 发送消息到业务服务器
//输入参数: socket id,  存放消息的缓存,  发送消息的类型
//输出参数: 无 
//返回值	:  发送成功,  返回TRUE ， 发送失败,  返回FALSE

int SendSrvMsg(int nSock, void *pMsg, SND_MSG_TYPE snd_msg_type)
{	
	INFO("SendSrvMsg: func begin%s", "");
	int nRet = 0;

	DEBUG("SendSrvMsg: [snd msg type]=%d", snd_msg_type);

	//根据不同的消息类型而发送不同的消息到业务服务器
	switch (snd_msg_type)
	{
		case SND_REQUEST_MSG:
			nRet = SendRequestMsg(nSock, pMsg);
			break;
		case SND_RESPONSE_MSG:
			nRet = SendResponseMsg(nSock, pMsg);
			break;
		default:
			break;	
	}

	INFO("SendSrvMsg: func end%s", "");
	return nRet;
}
int S_InsertClientInfoIntoHash(Client_Server_Msg *pClient_srv_msg)
{
	TRACE();
	if (NULL == pClient_srv_msg) 
	{
		ERROR("S_InsertClientInfoIntoHash: param error%s", "");
		return FALSE;	
	}

	int *pSize = (int *)shmat(g_iCliInfoId, NULL, 0);
	INFO("S_InsertClientInfoIntoHash: [size]=%d", *pSize);
	if (-1 == *pSize)
	{
		*pSize = 1;	
		int iAccSrvSeq = pClient_srv_msg->wAccess_server_seq;
		int iCnt = ntohl(pClient_srv_msg->nMsg_counter);
		int iCliSock = ntohl(pClient_srv_msg->nAccess_socket_id);
		INFO("S_InsertClientInfoIntoHash: [client socket]=%d", iCliSock);
		StCliInfo stCliInfo(iAccSrvSeq, iCnt, iCliSock);
		pSize++;
		memcpy(pSize, &stCliInfo, sizeof(stCliInfo));
	}
	else
	{
		*pSize += 1;
	}

	

	int nData_len = 0;
	Client_Info *pClient_info = (Client_Info *)MM_MALLOC_WITH_DESC(sizeof(Client_Info), \
	"S_InsertClientInfoIntoHash: Call func for client info");
	if (NULL == pClient_info)
	{	
		FATAL("S_InsertClientInfoIntoHash: Call malloc error%s", "");
		return 	FALSE;
	}

	memset(pClient_info, 0, sizeof(Client_Info));

	nData_len = ntohl(pClient_srv_msg->nData_len);
	pClient_info->bMsg_type = pClient_srv_msg->bMsg_type;
	pClient_info->bMain_service_code = pClient_srv_msg->bMain_service_code;
	pClient_info->bSub_service_code = pClient_srv_msg->bSub_service_code;
	pClient_info->nMsg_counter = ntohl(pClient_srv_msg->nMsg_counter);
	memcpy(pClient_info->arrAccess_check_code, pClient_srv_msg->arrCheck_code, 4);
	pClient_info->bService_version = *(pClient_srv_msg->pData);
	memcpy(pClient_info->arrService_check_code, pClient_srv_msg->pData + (nData_len - 4), 4);
	pClient_info->lSent_time = time((time_t *)NULL);
	pClient_info->nClient_socket = ntohl(pClient_srv_msg->nAccess_socket_id);

	int nSeq = *(WORD *)(pClient_srv_msg->pData + 3);
	pClient_info->nSeq = ntohl(nSeq);
			
	sprintf(pClient_info->arrSn, "%d%d", ntohs(pClient_srv_msg->wAccess_server_seq), ntohl(pClient_srv_msg->nMsg_counter));

	//char arrService[5] = {0};
	//char arrAccess[5] = {0};

	//memcpy(arrAccess, pClient_info->arrAccess_check_code, 4);
	//memcpy(arrService, pClient_info->arrService_check_code, 4);
	//PrintContent("\n\nInsertClientInfoIntoHash: pClient_info->arrSn=%s msg_type=%d main code=%d sub code=%d counter=%d access_code=%s service_code=%s version=%d client_sock=%d\n\n", pClient_info->arrSn, pClient_info->bMsg_type, pClient_info->bMain_service_code, pClient_info->bSub_service_code, pClient_info->nMsg_counter, arrAccess, arrService, pClient_info->bService_version, pClient_info->nClient_socket);
	
	if(FALSE == HashInsert (&g_clientinfo_hash.pClient_info_hash, pClient_info->arrSn, pClient_info))
	{
		ERROR("S_InsertClientInfoIntoHash: Call YDT_Hash_Insert error%s", "");
		return FALSE;
	}

	return TRUE;
}

void *GetMessageFromQueue(void)
{
	INFO("GetMessageFromQueue: func begin%s", "");
	int nRet = 0;
	void *pMsg = NULL;
	
	pthread_mutex_lock(&g_msg_queue.queue_mutex);
	nRet = IsSeqQueueEmpty(&g_msg_queue.msg_queue);
	while (TRUE == nRet)
	{
		INFO("GetMessageFromQueue: message queue is empty so we will wait to get message from queue%s", "");
		pthread_cond_wait(&g_msg_queue.queue_cond, &g_msg_queue.queue_mutex);
		INFO("GetMessageFromQueue: we recv a signal to recv message from message queue%s", "");
		nRet = IsSeqQueueEmpty(&g_msg_queue.msg_queue);
	}

	nRet = GetMsgFromSeqQueue(&g_msg_queue.msg_queue, &pMsg);
	pthread_mutex_unlock(&g_msg_queue.queue_mutex);	
	INFO("GetMessageFromQueue: func end%s", "");
	return pMsg;
}


void *ListenSockThd(void *pParam)
{
	TRACE();
	int iListenSock = 0;
	int iCliSock = 0;
	WORD wPort = 5000;
	int nRet = 0;
	
	if (FALSE == S_InitListenSockInfo(&iListenSock, wPort))
	{
		ERROR("ListenSockThd: Call S_InitListenSockInfo error");
		return FALSE;	
	}

	#if 0
	int iNewFD = 0;
	char buffer[30];
	iovec *pOvec = new iovec;
	pOvec[0].iov_base = buffer;
	pOvec[0].iov_len = 30;
	bzero(pOvec[0].iov_base, 30);
	msghdr msg;
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_control = (caddr_t)&iNewFD;
	msg.msg_controllen = sizeof(int);			
	msg.msg_iov = pOvec;
	msg.msg_iovlen = 1;
	msg.msg_flags = 0;
	memcpy(pOvec[0].iov_base, "test luguanhuang", 20);
	int iSize = 0;
	int nread = 0;
	#else
	#define MAXLINE 20
	
	int newfd, nread, status;
	char *ptr, buf[MAXLINE];
	struct iovec iov[1];
	struct msghdr msg;
	status = -1; 
	#endif

	while (1)
	{			
		nRet = Accept(iListenSock, &iCliSock);
		if (FALSE == nRet)
		{	
			ERROR("ListenSockThd: Call accept error");
			return FALSE;

		}

		DEBUG("ListenSockThd: Call accept succeed");

		#if 0
		if((nread = recvmsg(iCliSock, &msg, 0)) < 0)
		{
			ERROR("ListenSockThd: recvmsg error");	
		}
		else if(0 == nread)            
		{                   
			ERROR("ListenSockThd: connection closed by server");   
			return NULL;            
		}  

		INFO("ListenSockThd: [read fd]=%d", nread);
		for(ptr = buffer; ptr<&buffer[nread]; ) 
		{                
			INFO("ListenSockThd: [read fd]=%d [content]=%d", \
				nread, (int)*ptr);	
			if(0 == *ptr++)                  
			{                       
				if(ptr != &buffer[nread-1])                            
					ERROR("ListenSockThd: message format error");           
					status = *ptr & 255;                 
					if(0 == status)                       
					{                              
						if(msg.msg_controllen != sizeof(int))  
						{
							ERROR("ListenSockThd: status = 0 but no fd"); 
						}                       
						else
						{
							newfd = -status;                       
						}
						nread -= 2;                  
					}          
			}     
		}
		#else
		#define CONTROLLEN (sizeof(struct cmsghdr) + sizeof(int))
		
		static struct cmsghdr *cmptr = NULL;
		iov[0].iov_base = buf;
		iov[0].iov_len = sizeof(buf);
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;
		msg.msg_name = NULL;
		msg.msg_namelen = 0;
		if (cmptr == NULL)
		{
			cmptr = (struct cmsghdr *)malloc(CONTROLLEN);
		}

		msg.msg_control = (caddr_t)cmptr;
		msg.msg_controllen = CONTROLLEN;
		if((nread = recvmsg(iCliSock, &msg, 0)) < 0)
			ERROR("recvmsg error");
		else if(0 == nread)
		{                   
			ERROR("connection closed by server");                   
			return NULL;            
		}            
		for(ptr = buf; ptr<&buf[nread]; )
		{          
			INFO("ListenSockThd: [read fd]=%d [content]=%d", \
				nread, (int)*ptr);	
			if(0 == *ptr++)                  
			{                       
				if(ptr != &buf[nread-1])                             
					ERROR("message format error");                       
				status = *ptr & 255;                       
				if(0 == status)
				{                              
					if(msg.msg_controllen != CONTROLLEN)
						ERROR("status = 0 but no fd");
					newfd = *(int *)CMSG_DATA(cmptr); 
					INFO("ListenSockThd: [fd]=%d", newfd);
				}                       
				else 
					newfd = -status; 
				nread -= 2;                 
			}   

			INFO("ListenSockThd: [status]=%d [fd]=%d", status, newfd);
		} 
		#endif
	}
	return NULL;
}



//函数用途: 从消息队列里面取出消息,  并发给对应的业务服务器  
//输入参数: 无
//输出参数:  无
//返回值	:  无

//备注:  不可以用pthread_exit函数 因为整个服务器只有一个发送函数

int S_SndMsgToServer(void)
{
	INFO("S_SndMsgToServer: func begin");
	pthread_detach(pthread_self());
	Client_Server_Msg *pClient_srv_msg = NULL;
	char szBuf[100] = {0};
	WORD wService_srv_seq = 0;
	int iLen = 0;
	pthread_t thread_id = 0;
	struct StMsg stMsg;
	//int iTmpId = shmget(ftok("/opt", 2), 1000, 0666 | IPC_CREAT);
	//INFO("S_SndMsgToServer: [iTmpId]=%d", iTmpId);

	#if 0
	int nRet= pthread_create(&thread_id, NULL, ListenSockThd, NULL);
	if (nRet!= 0)
	{
		ERROR("S_SndMsgToServer: Call ListenSockThd error");
		return FALSE;
	}	
	#endif
	
	for (;;)
	{
		memset(&stMsg, 0, sizeof(stMsg));
		iLen = msgrcv (g_iQid, &stMsg, 511, 0, 0); 
		INFO("S_SndMsgToServer: [return val]=%d", iLen);
		if (0 == iLen)
		{
			INFO("S_SndMsgToServer: msg queue don't have data");
			continue;
		}
		else if (iLen > 0)
		{
			INFO("S_SndMsgToServer: msg queue have data");	
			pClient_srv_msg = (Client_Server_Msg *)stMsg.msg_buf;
		}
		else
		{
			ERROR("S_SndMsgToServer: Call msgrcv error");
			continue;
		}

				
		//wService_srv_seq = (WORD)GetServiceSrvSeq(pClient_srv_msg->bMain_service_code, pClient_srv_msg->bSub_service_code);
		wService_srv_seq = g_serRouteInfo.GetSrvSeq(pClient_srv_msg->bMain_service_code, pClient_srv_msg->bSub_service_code);
		//wService_srv_seq = (WORD)GetServiceSrvSeq(pClient_srv_msg->bMain_service_code, pClient_srv_msg->bSub_service_code);
		if (0 == wService_srv_seq)
		{
			WARN("S_SndMsgToServer: we don't have service server now, then discard the msg%s", "");
			continue;	
		}

		//填充业务服务器的序号
		pClient_srv_msg->wService_server_seq = htons(wService_srv_seq);

		memset(szBuf, 0, sizeof(szBuf));
		sprintf(szBuf, "%d%d%d", pClient_srv_msg->bMain_service_code, pClient_srv_msg->bSub_service_code, wService_srv_seq);
		INFO("S_SndMsgToServer: [key]=%s", szBuf);
		SemP(g_iSemId);
		int *pSize = (int *)shmat(g_iShmId, NULL, 0);
		int iSize = *pSize;
		INFO("S_SndMsgToServer: [size]=%d", iSize);
		pSize++;
		StShmSrvInfo *pShmSrvInfo = (StShmSrvInfo *)pSize;
		stringstream strData;
		bool bFind = FALSE;
		
		for (int i=0; i<iSize; i++)
		{	
			strData.str("");
			strData << (int)pShmSrvInfo->bMain_code << (int)pShmSrvInfo->bSub_code << pShmSrvInfo->wService_seq;
			INFO("S_SndMsgToServer: [main code]=%d [sub code]=%d [service seq]=%d [socket]=%d [key]=%s", \
				pShmSrvInfo->bMain_code, pShmSrvInfo->bSub_code, \
				pShmSrvInfo->wService_seq, pShmSrvInfo->nSock, strData.str().c_str());		
			if (0 == strcmp(szBuf, strData.str().c_str()))
			{
				INFO("S_SndMsgToServer: we can find the info [val]=%s", szBuf);
				bFind = TRUE;
				break;	
			}
			
			pShmSrvInfo++;
		}

		if (FALSE == bFind)
		{
			WARN("S_SndMsgToServer: we can't find the value from hash table, then discard the msg%s", "");
			continue;
		}
		SemV(g_iSemId);
		
		if (0 == pShmSrvInfo->nSock)
		{
			WARN("S_SndMsgToServer: the socket is disconnect, then connect the socket%s", "");
			pthread_mutex_lock(&g_awake_thread_flg.awake_flg_mutex);
			g_awake_thread_flg.nAwake_thread_flg = 1;
			pthread_mutex_unlock(&g_awake_thread_flg.awake_flg_mutex);
			continue;	
		}

		INFO("S_SndMsgToServer: [before send [socket]=%d", \
			pShmSrvInfo->nSock);
		if (FALSE == SendSrvMsg(pShmSrvInfo->nSock, pClient_srv_msg, SND_REQUEST_MSG))
		{
			ERROR("SndMsgToServer: Call SendSrvMsg error%s", "");
			pthread_mutex_lock(&g_routing_table.routing_mutex);
			g_mapSrvInfo[szBuf].nSock = 0;
			pthread_mutex_unlock(&g_routing_table.routing_mutex);
			WARN("SndMsgToServer: disconnect with the service server, notify the maintain thread reconnect%s", "");
			pthread_mutex_lock(&g_awake_thread_flg.awake_flg_mutex);
			g_awake_thread_flg.nAwake_thread_flg = 1;
			pthread_mutex_unlock(&g_awake_thread_flg.awake_flg_mutex);
		}
		else
		{
			//PrintContent("SndMsgToServer: send msg to service server succeed\n");
			//如果消息发送成功,  就把从客户端发来的信息包存放入客户哈希表里面
			DEBUG("SndMsgToServer: send msg succeed%s", "");
			BYTE bMain_code = pClient_srv_msg->bMain_service_code;
			BYTE bSub_code = pClient_srv_msg->bSub_service_code;

			if ((1 == bMain_code && bSub_code != 4) || 
				(2 == bMain_code && 5 == bSub_code))
			{
				if (FALSE == S_InsertClientInfoIntoHash(pClient_srv_msg))
				{
					ERROR("SndMsgToServer: Call S_InsertClientInfoIntoHash error%s", "");
				}
				else
				{
					//PrintContent("SndMsgToServer: InsertClientInfoIntoHash succeed\n");
				}		
			}
		}
	}

	return 1;
}

static void DisplayClientSrvMsg(Client_Server_Msg *pClient_srv_msg, Client_Msg_Buf *pClient_msg_buf, WORD wCmd_id)
{
	INFO("DisplayClientSrvMsg: func begin%s", "");
	if (NULL == pClient_srv_msg || NULL == pClient_msg_buf)
	{
		ERROR("DisplayClientSrvMsg: func param error%s", "");
		return;
	}

	DEBUG("DisplayClientSrvMsg: [version]=%d [msg type]=%d [main service code]=%d [sub service code]=%d"
			" [access server seq]=%d [msg counter]=%d [client socket]=%d [dest service server seq]=%d"
			" [msg header len]=%d [msg body len]=%d [msg cmd_id]=%d", \
			pClient_srv_msg->bVersion, pClient_srv_msg->bMsg_type, \
			pClient_srv_msg->bMain_service_code, pClient_srv_msg->bSub_service_code, \
			ntohs(pClient_srv_msg->wAccess_server_seq), ntohl(pClient_srv_msg->nMsg_counter), \
			ntohl(pClient_srv_msg->nAccess_socket_id), \
			ntohs(pClient_srv_msg->wService_server_seq), \
			pClient_msg_buf->nRecv_header_len, \
			pClient_msg_buf->nMsg_body_len, wCmd_id);
	
	INFO("DisplayClientSrvMsg: func end%s", "");
}

static void DisplayPushToCliResMsg(PushToClient_Response_Msg *pPustocli_res_msg, Client_Msg_Buf *pClient_msg_buf, WORD wCmd_id)
{
	INFO("DisplayPushToCliResMsg: func begin%s", "");
	if (NULL == pPustocli_res_msg || NULL == pClient_msg_buf)
	{
		ERROR("DisplayPushToCliResMsg: func param error%s", "");
		return;
	}

	DEBUG("DisplayPushToCliResMsg: [version]=%d [msg type]=%d [main service code]=%d [sub service code]=%d"
			" [access server seq]=%d [client socket]=%d [dest service server seq]=%d"
			" [msg header len]=%d [msg body len]=%d [msg cmd_id]=%d", \
			pPustocli_res_msg->bVersion, pPustocli_res_msg->bMsg_type, \
			pPustocli_res_msg->bMain_service_code, pPustocli_res_msg->bSub_service_code, \
			ntohs(pPustocli_res_msg->wAccess_srv_seq), \
			ntohl(pPustocli_res_msg->nClient_sock), \
			ntohs(pPustocli_res_msg->wService_srv_seq), \
			pClient_msg_buf->nRecv_header_len, \
			pClient_msg_buf->nMsg_body_len, wCmd_id);
	
	INFO("DisplayPushToCliResMsg: func end%s", "");
}


//函数用途: 从消息队列里面取出消息,  并发给对应的业务服务器  
//输入参数: 无
//输出参数:  无
//返回值	:  无

//备注:  不可以用pthread_exit函数 因为整个服务器只有一个发送函数

void *SndMsgToServiceServerThread(void *pParam)
{
	INFO("SndMsgToServiceServerThread: func begin%s", "");
	pthread_detach(pthread_self());
	
	char *pMsg = NULL;
	void *pClient_msg = NULL;
	Client_Msg_Buf *pClient_msg_buf = NULL;
	Client_Server_Msg *pTmp_clientsrv_msg = NULL;
	PushToClient_Response_Msg *pTmp_pushtocli_res_msg = NULL;
	char szBuf[100] = {0};
	StHash_Item *pItem = NULL;
	Server_Info *pSrv_info = NULL;
	BYTE bMain_code = 0;
	BYTE bSub_code = 0;
	WORD wCmd_id = 0;
	int nSum_len = 0;
	WORD wService_srv_seq = 0;
	int nData_offset = 0;
	
	while (TRUE)
	{
		pClient_msg = GetMessageFromQueue();
		pClient_msg_buf = (Client_Msg_Buf *)pClient_msg;
		pMsg = pClient_msg_buf->arrBuf;
		DEBUG("SndMsgToServiceServerThread: [msg type]=%d", pClient_msg_buf->bMsg_type);
		if (CLIENT_SERVER_MSG == pClient_msg_buf->bMsg_type)
		{
			pTmp_clientsrv_msg = (Client_Server_Msg *)(pMsg);		
			bMain_code = pTmp_clientsrv_msg->bMain_service_code;
			bSub_code = pTmp_clientsrv_msg->bSub_service_code;
		}
		else if (PUSHTO_CLIENT_RESPONSE == pClient_msg_buf->bMsg_type)
		{
			pTmp_pushtocli_res_msg = (PushToClient_Response_Msg *)pMsg;
			bMain_code = pTmp_pushtocli_res_msg->bMain_service_code;
			bSub_code = pTmp_pushtocli_res_msg->bSub_service_code;
		}

		//modify by luguanhuang 20111014(reason: 为了在所有可能的情况下)
		//把客户端信息插入到哈希表中
		if ((1 == bMain_code && bSub_code != 4) || 
			(2 == bMain_code && 5 == bSub_code) ||
			(WANWEI_SERVICE_TYPE == bMain_code && WANWEI_LOGIN_SUB_SERVICETYPE == bSub_code) ||
			(WANWEI_SERVICE_TYPE == bMain_code && WANWEI_REPORT_GPS_SERVICETYPE == bSub_code) ||
			(WANWEI_SERVICE_TYPE == bMain_code && WANWEI_QUERYPUSH__SERVICETYPE == bSub_code))
		{
			InsertClientInfoIntoHash(pMsg);						//插入客户端信息到哈希表
		}

	
		wService_srv_seq = (WORD)GetServiceSrvSeq(bMain_code, bSub_code);
		DEBUG("SndMsgToServiceServerThread: [main service code]=%d [sub service code]=%d"
			" [server sequence]=%d", bMain_code, bSub_code, wService_srv_seq);
		if (0 == wService_srv_seq)
		{
			//打印调试信息
			WARN("SndMsgToServiceServerThread: we don't have service server now, then discard the msg%s", "");
			MM_FREE(pClient_msg);
			continue;
		}

		snprintf(szBuf, sizeof(szBuf) - 1, "%d%d%d", bMain_code, bSub_code, wService_srv_seq);

		//获取接入服务器和业务服务器的连接信息
		pItem = HashGetItem(g_routing_table.pRouting_table, szBuf);
		if (NULL == pItem || NULL == pItem->pMatch_msg)		//只是简单地把包丢弃
		{
			WARN("SndMsgToServiceServerThread: we can't find the service server info from routing table, then discard the msg%s", "");	
			MM_FREE(pClient_msg);		
			continue;	
			
		}

		pSrv_info = (Server_Info *)pItem->pMatch_msg;
		if (0 == pSrv_info->nSock)		//把包丢弃,  并通知维护线程重连业务服务器
		{
			INFO("SndMsgToServiceServerThread: the socket is disconnect, then notify the maintain thread to reconnect the socket%s", "");
			MM_FREE(pClient_msg);		
			pthread_mutex_lock(&g_awake_thread_flg.awake_flg_mutex);
			g_awake_thread_flg.nAwake_thread_flg = TRUE;
			pthread_mutex_unlock(&g_awake_thread_flg.awake_flg_mutex);
			continue;	
		}

		if (CLIENT_SERVER_MSG == pClient_msg_buf->bMsg_type)
		{
			pTmp_clientsrv_msg->wService_server_seq = htons(wService_srv_seq);		
			nData_offset = Offset(Client_Server_Msg, pData);
			wCmd_id = *(WORD *)(pMsg + nData_offset + 1);
			wCmd_id = ntohs(wCmd_id);
		}
		else if (PUSHTO_CLIENT_RESPONSE == pClient_msg_buf->bMsg_type)
		{
			pTmp_pushtocli_res_msg->wService_srv_seq = htons(wService_srv_seq);		
			nData_offset = Offset(PushToClient_Response_Msg, pData);
			wCmd_id = *(WORD *)(pMsg + nData_offset + 1);
			wCmd_id = ntohs(wCmd_id);	
		}
		
		nSum_len = pClient_msg_buf->nMsg_header_len + pClient_msg_buf->nMsg_body_len;
		
		pthread_mutex_lock(&pSrv_info->mutex);
		int nSnd_len = Send(pSrv_info->nSock, pMsg, nSum_len, wCmd_id);			//发送消息到业务服务器
		pthread_mutex_unlock(&pSrv_info->mutex);
		if (-1 == nSnd_len)
		{
			ERROR("SndMsgToServiceServerThread: Call Send error send msg to service server error"
				" then notify the maintain thread to reconnect the socket%s", "");
			
			MM_FREE(pClient_msg);
			pSrv_info->nSock = 0;
			pthread_mutex_lock(&g_awake_thread_flg.awake_flg_mutex);
			g_awake_thread_flg.nAwake_thread_flg = TRUE;
			pthread_mutex_unlock(&g_awake_thread_flg.awake_flg_mutex);
		}
		else
		{
			INFO("SndMsgToServiceServerThread: send msg to service server succeed%s", "");
			#if 0
			DEBUG("SndMsgToServiceServerThread: [version]=%d [msg type]=%d [main service code]=%d [sub service code]=%d"
			" [access server seq]=%d [msg counter]=%d [client socket]=%d [dest service server seq]=%d"
			" [msg header len]=%d [msg body len]=%d [msg cmd_id]=%d", \
			pTmp_clientsrv_msg->bVersion, pTmp_clientsrv_msg->bMsg_type, \
			pTmp_clientsrv_msg->bMain_service_code, pTmp_clientsrv_msg->bSub_service_code, \
			ntohs(pTmp_clientsrv_msg->wAccess_server_seq), ntohl(pTmp_clientsrv_msg->nMsg_counter), \
			ntohl(pTmp_clientsrv_msg->nAccess_socket_id), \
			ntohs(pTmp_clientsrv_msg->wService_server_seq), \
			pClient_msg_buf->nRecv_header_len, \
			pClient_msg_buf->nMsg_body_len, wCmd_id);
			#endif
			if (CLIENT_SERVER_MSG == pClient_msg_buf->bMsg_type)
			{
				DisplayClientSrvMsg(pTmp_clientsrv_msg, pClient_msg_buf, wCmd_id);		
			}
			else if (PUSHTO_CLIENT_RESPONSE == pClient_msg_buf->bMsg_type)
			{
				DisplayPushToCliResMsg(pTmp_pushtocli_res_msg, pClient_msg_buf, wCmd_id);		
			}
			
			MM_FREE(pClient_msg);
		}				
	}

	INFO("SndMsgToServiceServerThread: func end%s", "");
	return NULL;
}


//函数用途: 该函数主要用来处理聊天业务的(发送给客户端的消息成功与否,  把发送状态 转发给对应的业务服务器)
//输入参数: socket id, Forward_Srv_Client_Msg 结构体, 发送消息给客户端的状态值
//输出参数:  无
//返回值	: 发送成功, 返回TRUE,  发送失败,  返回FALSE

int SndDisposeResultToServiceSrv(int nSock, Forward_Srv_Client_Msg forward_srv_client_msg, BYTE bStatus)
{
	INFO("SndDisposeResultToServiceSrv: func begin%s", "");

	//DEBUG("SndDisposeResultToServiceSrv: enter SndDisposeResultToServiceSrv\n");

	int nRet = FALSE;

	Response_Client_Srv_Msg res_client_srv_msg;
	memset(&res_client_srv_msg, 0, sizeof(res_client_srv_msg));
	res_client_srv_msg.bVersion = forward_srv_client_msg.bVersion;
	res_client_srv_msg.bMsg_type = RESPONSE_CLIENT_SRV_MSG;
	res_client_srv_msg.wService_server_seq = forward_srv_client_msg.wService_server_seq;
	res_client_srv_msg.nMsg_counter = forward_srv_client_msg.nService_Msg_counter;
	res_client_srv_msg.bTransfer_result = bStatus;
	memcpy(res_client_srv_msg.arrCheck_code, forward_srv_client_msg.arrCheck_code, 4);

	nRet = SendSrvMsg(nSock, (void *)&res_client_srv_msg, SND_RESPONSE_MSG);

	LOG_DEBUG("SndDisposeResultToServiceSrv: leave SndDisposeResultToServiceSrv", FILE_NAME, FILE_LINE);

	INFO("SndDisposeResultToServiceSrv: func end%s", "");

	return nRet;
}


//函数用途: 连接聊天业务服务器成功后发送的第一个数据包, 目的是让业务服务器保存接入服务器和业务服务器连接的socket id
//输入参数: socket id,  主业务码,  子业务码,  业务服务器序号
//输出参数: 无
//返回值	: 发送成功,  返回TRUE,  发送失败,   返回FALSE

int SndAfterConnSrvMsg(int nSock, BYTE bMain_service_code, BYTE bSub_service_code, WORD wService_srv_seq)
{
	INFO("SndAfterConnSrvMsg: func begin%s", "");
	
	Req_AccessServer_Conn access_srv_conn;
	memset(&access_srv_conn, 0, sizeof(access_srv_conn));
	access_srv_conn.bVersion = 1;
	access_srv_conn.wCmd_id = htons(AFTER_ACCESS_SRV_CONN);
	memcpy(access_srv_conn.arrCheck_code, "conn", 4);

	Client_Server_Msg client_srv_msg;
	memset(&client_srv_msg, 0, sizeof(client_srv_msg));
	client_srv_msg.bVersion = ACCESS_SRV_VERSION;
	client_srv_msg.bMsg_type = CLIENT_SERVER_MSG;
	client_srv_msg.bMain_service_code = bMain_service_code;
	client_srv_msg.bSub_service_code = bSub_service_code;
	client_srv_msg.wAccess_server_seq = htons(g_srv_conf_info.wAccess_srv_seq);

	int nData_len = sizeof(Req_AccessServer_Conn);
	
	pthread_mutex_lock(&g_msg_counter.msg_counter_mutex);
	g_msg_counter.nMsg_counter++;
	if ((pow(2, 32) - 1) == g_msg_counter.nMsg_counter)
	{
		g_msg_counter.nMsg_counter = 1;
	}

	client_srv_msg.nMsg_counter = htonl(g_msg_counter.nMsg_counter);
	pthread_mutex_unlock(&g_msg_counter.msg_counter_mutex);

	client_srv_msg.nAccess_socket_id = 0;
	client_srv_msg.wService_server_seq = htons(wService_srv_seq);
	client_srv_msg.nData_len = htonl(nData_len);

	client_srv_msg.pData = (char *)MM_MALLOC_WITH_DESC(nData_len, \
	"SndAfterConnSrvMsg: Call func for service layer data");
	if (NULL == client_srv_msg.pData)
	{
		FATAL("SndAfterConnSrvMsg: Call MM_MALLOC_WITH_DESC error%s", "");
		return OUT_OF_MEMORY_ERROR;
	}
	
	memcpy(client_srv_msg.pData, &access_srv_conn, nData_len);
	memcpy(client_srv_msg.arrCheck_code, "afte", 4);

	char aBuf[1024] = {0};
	int iLen = sizeof(Client_Server_Msg) - 8;
	memcpy(aBuf, (char *)&client_srv_msg, iLen);
	int iDataLen = ntohl(client_srv_msg.nData_len);
	INFO("SndAfterConnSrvMsg: [data len]=%d", iDataLen);
	memcpy(aBuf + iLen, &access_srv_conn, iDataLen);
	strncpy(aBuf + iDataLen + iLen, "afte", 4);

	//发送消息
	//if (TRUE != SendSrvMsg(nSock, &client_srv_msg, SND_REQUEST_MSG))
	if (TRUE != SendSrvMsg(nSock, aBuf, SND_REQUEST_MSG))
	{
		ERROR("SndAfterConnSrvMsg: Call SendSrvMsg error%s", "");
		MM_FREE(client_srv_msg.pData);
		return FALSE;
	}

	char arrAccess_checkcode[5] = {0};
	char arrService_checkcode[5] = {0};

	memcpy(arrAccess_checkcode, client_srv_msg.arrCheck_code, 4);
	memcpy(arrService_checkcode, access_srv_conn.arrCheck_code, 4);

	DEBUG("SndAfterConnSrvMsg: [version]=%d [msg type]=%d [main service code]=%d [sub service code]=%d"
			" [access server seq]=%d [msg counter]=%d [client socket]=%d [dest service server seq]=%d"
			" [data len]=%d [cmd id]=%d [access check code]=%s [service check code]=%s", \
			client_srv_msg.bVersion, client_srv_msg.bMsg_type, \
			client_srv_msg.bMain_service_code, client_srv_msg.bSub_service_code, \
			ntohs(client_srv_msg.wAccess_server_seq), ntohl(client_srv_msg.nMsg_counter), \
			ntohl(client_srv_msg.nAccess_socket_id), ntohs(client_srv_msg.wService_server_seq), \
			ntohl(client_srv_msg.nData_len), ntohs(access_srv_conn.wCmd_id), arrAccess_checkcode, \
			arrService_checkcode);

	MM_FREE(client_srv_msg.pData);
	INFO("SndAfterConnSrvMsg: func end%s", "");
	return TRUE;
}

