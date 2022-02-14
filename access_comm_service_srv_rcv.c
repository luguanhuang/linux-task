
#include "../util/access_global.h"
//#include "../include/ydt_log.h"
#include "../interface/access_protocol.h"
#include "access_comm_client_snd.h"
#include "access_comm_service_srv_snd.h"
#include "access_comm_client_rcv.h"
#include "access_epoll.h"
#include "access_sock_info.h" 
#include "access_routing_maintain.h"
#include "access_comm_service_srv_rcv.h"

/**********该文件主要是用来与业务服务器进行交互(接收业务服务器消息)的************/

extern int g_iCliInfoId;
extern int g_iSndCliQid;

extern StClientInfo_Hashtable g_clientinfo_hash;

extern StRouting_Table g_routing_table;

extern StSnd_Client_MsgQueue g_sndclient_msgqueue;

int RecvResponseToClientMsg(int nSock, char *pContent)
{
	INFO("RecvResponseToClientMsg: func begin%s", "");
	if (NULL == pContent)
	{
		ERROR("RecvResponseToClientMsg: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = 0;
	int nData_len = 0;
	char szBuf[50] = {0};
	Client_Info *pClient_info = NULL;
	Server_Client_Msg *pSrv_client_msg = (Server_Client_Msg *)MM_MALLOC_WITH_DESC(sizeof(Server_Client_Msg), \
		"RecvResponseToClientMsg: Call func for server client msg");
	if (NULL == pSrv_client_msg)
	{
		FATAL("RecvResponseToClientMsg: Call MM_MALLOC_WITH_DESC error%s", "");
		return OUT_OF_MEMORY_ERROR;
	}
	memset(pSrv_client_msg, 0, sizeof(Server_Client_Msg));

	
	pSrv_client_msg->bVersion = pContent[0];
	pSrv_client_msg->bMsg_type = pContent[1];
	
	char *pTmp_msg = (char *)pSrv_client_msg;
	nRet = Recv(nSock, pTmp_msg + 2, sizeof(Server_Client_Msg) - 10);
	if (FALSE == nRet)
	{
		ERROR("RecvResponseToClientMsg: Call Recv error%s", "");
		close(nSock);
		MM_FREE(pSrv_client_msg);
		return DISCONNECT_WITH_SERVER;
	}

	nData_len = ntohl(pSrv_client_msg->nData_len);
	
	pSrv_client_msg->pData = (char *)MM_MALLOC_WITH_DESC(nData_len, \
	"RecvResponseToClientMsg: Call func for service layer data");
	if (NULL == pSrv_client_msg->pData)
	{
		FATAL("RecvResponseToClientMsg: Call MM_MALLOC_WITH_DESC error%s", "");
		close(nSock);
		MM_FREE(pSrv_client_msg);
		return OUT_OF_MEMORY_ERROR;
	}

	nRet = Recv(nSock, (char *)pSrv_client_msg->pData, nData_len);
	if (FALSE == nRet)
	{
		ERROR("RecvResponseToClientMsg: Call Recv error%s", "");
		close(nSock);
		MM_FREE(pSrv_client_msg->pData);
		MM_FREE(pSrv_client_msg);
		return DISCONNECT_WITH_SERVER;
	}

	nRet = Recv(nSock, (char *)pSrv_client_msg->arrCheckout_code, 4);
	if (FALSE == nRet)
	{
		ERROR("RecvResponseToClientMsg: Call Recv error%s", "");
		close(nSock);
		MM_FREE(pSrv_client_msg->pData);
		MM_FREE(pSrv_client_msg);	
		return DISCONNECT_WITH_SERVER;
	}

	char *pTmp_Data = pSrv_client_msg->pData;
	WORD wCmd_id = *(WORD *)(pTmp_Data + 1);
	wCmd_id = ntohs(wCmd_id);

	char arrAcc_checkcode[5] = {0};
	memcpy(arrAcc_checkcode, pSrv_client_msg->arrCheckout_code, 4);
	
	DEBUG("RecvResponseToClientMsg: [version]=%d [msg type]=%d [main service code]=%d [sub service code]=%d"
		" [access server seq]=%d [msg counter]=%d [access check code]=%s [data len]=%d [cmd_id]=%d", pSrv_client_msg->bVersion, pSrv_client_msg->bMsg_type, \
		pSrv_client_msg->bMain_service_code, pSrv_client_msg->bSub_service_code, ntohs(pSrv_client_msg->wAccess_server_seq), \
		ntohl(pSrv_client_msg->nMsg_counter), arrAcc_checkcode, ntohl(pSrv_client_msg->nData_len), wCmd_id);	

	snprintf(szBuf, sizeof(szBuf) - 1, "%d%d", ntohs(pSrv_client_msg->wAccess_server_seq), ntohl(pSrv_client_msg->nMsg_counter));	
	INFO("RecvResponseToClientMsg: [sn]=%s", szBuf);

	int *pSize = (int *)shmat(g_iCliInfoId, NULL, 0);
	INFO("RecvResponseToClientMsg: [size]=%d", *pSize);
	pSize++;
	
	StCliInfo *pCliInfo = (StCliInfo *)pSize;
	INFO("RecvResponseToClientMsg: [cli socket]=%d", pCliInfo->iSock);

	pClient_info = (Client_Info *)HashData(g_clientinfo_hash.pClient_info_hash, szBuf);	
	if (NULL == pClient_info)
	{
		WARN("RecvResponseToClientMsg: we can't find the client info from the client"
			" info hashtable then we will discard the msg which recv from service server%s", "");
		//MM_FREE(pSrv_client_msg->pData);	
		//MM_FREE(pSrv_client_msg);
		//return FALSE;		//不算错误, 只是找不到客户端socket 信息, 只是简单地丢弃数据包就可以了
	}

	Srv_Client_Msg_Info *pSrv_client_msg_info = (Srv_Client_Msg_Info *)MM_MALLOC_WITH_DESC(sizeof(Srv_Client_Msg_Info), \
		"RecvResponseToClientMsg: Call func for srv msg client info");
	if (NULL == pSrv_client_msg_info)
	{
		FATAL("RecvResponseToClientMsg: Call MM_MALLOC_WITH_DESC error%s", "");
		//MM_FREE(pSrv_client_msg->pData);
		//MM_FREE(pSrv_client_msg);	
		return OUT_OF_MEMORY_ERROR;
	}

	memset(pSrv_client_msg_info, 0, sizeof(Srv_Client_Msg_Info));

	pSrv_client_msg_info->pMsg = (void *)pSrv_client_msg;
	pSrv_client_msg_info->nClient_sock = pCliInfo->iSock;
	memcpy(pSrv_client_msg_info->arrSn, szBuf, sizeof(pSrv_client_msg_info->arrSn));

	struct StMsg stMsg;
	memset(&stMsg, 0, sizeof(stMsg));
	stMsg.msg_types = 1;
	int iTmpLen = 0;
	memcpy(stMsg.msg_buf + iTmpLen, &(pCliInfo->iSock), sizeof(int));
	iTmpLen += 4;
	memcpy(stMsg.msg_buf + iTmpLen, szBuf, sizeof(szBuf) - 1);
	iTmpLen += sizeof(szBuf) - 1;
	memcpy(stMsg.msg_buf + iTmpLen, (char *)pSrv_client_msg, sizeof(Server_Client_Msg) - 8);
	iTmpLen += sizeof(Server_Client_Msg) - 8;
	memcpy(stMsg.msg_buf + iTmpLen, pSrv_client_msg->pData, nData_len);
	iTmpLen += nData_len;
	memcpy(stMsg.msg_buf + iTmpLen, pSrv_client_msg->arrCheckout_code, 4);
	
	nRet = msgsnd(g_iSndCliQid, &stMsg, 511, 0);
	INFO("RecvResponseToClientMsg: msgsnd [return val]=%d", nRet);
	if (nRet)
	{
		ERROR("RecvResponseToClientMsg: Call msgsnd error error[%d]=%s", \
			errno, strerror(errno));
	}

	char aCode[5] = {0};
	strncpy(aCode, pSrv_client_msg->arrCheckout_code, 4);
	INFO("RecvResponseToClientMsg: [check code]=%s", aCode);
	
	INFO("RecvResponseToClientMsg: func end%s", "");
	return TRUE;
}

int RecvPushToClientMsg(int nSock, char *pContent)
{
	INFO("RecvPushToClientMsg: func begin%s", "");
	if (NULL == pContent)
	{
		ERROR("RecvPushToClientMsg: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = 0;
	int nData_len = 0;
	PushTo_Client_Msg *pPushto_client_msg = (PushTo_Client_Msg *)MM_MALLOC_WITH_DESC(sizeof(PushTo_Client_Msg), \
		"RecvPushToClientMsg: Call func for pushto client msg");
	if (NULL == pPushto_client_msg)
	{
		FATAL("RecvPushToClientMsg: Call MM_MALLOC_WITH_DESC error%s", "");
		return OUT_OF_MEMORY_ERROR;
	}
	
	memset(pPushto_client_msg, 0, sizeof(PushTo_Client_Msg));
	
	pPushto_client_msg->bVersion = pContent[0];
	pPushto_client_msg->bMsg_type = pContent[1];

	int nHeader_offset = Offset(PushTo_Client_Msg, pData);
	DEBUG("RecvPushToClientMsg: [header len]=%d", nHeader_offset);
	char *pTmp_msg = (char *)pPushto_client_msg;
	nRet = Recv(nSock, pTmp_msg + 2, nHeader_offset - 2);
	if (FALSE == nRet)
	{
		ERROR("RecvPushToClientMsg: Call Recv error%s", "");
		close(nSock);
		MM_FREE(pPushto_client_msg);
		return DISCONNECT_WITH_SERVER;
	}

	nData_len = ntohl(pPushto_client_msg->nData_len);
	DEBUG("RecvPushToClientMsg: [data len]=%d", nData_len);
	
	pPushto_client_msg->pData = (char *)MM_MALLOC_WITH_DESC(nData_len, \
	"RecvPushToClientMsg: Call func for service layer data");
	if (NULL == pPushto_client_msg->pData)
	{
		FATAL("RecvPushToClientMsg: Call MM_MALLOC_WITH_DESC error%s", "");
		close(nSock);
		MM_FREE(pPushto_client_msg);
		return OUT_OF_MEMORY_ERROR;
	}

	nRet = Recv(nSock, (char *)pPushto_client_msg->pData, nData_len);
	if (FALSE == nRet)
	{
		ERROR("RecvPushToClientMsg: Call Recv error%s", "");
		close(nSock);
		MM_FREE(pPushto_client_msg->pData);
		MM_FREE(pPushto_client_msg);
		return DISCONNECT_WITH_SERVER;
	}

	nRet = Recv(nSock, (char *)pPushto_client_msg->arrCheck_code, 4);
	if (FALSE == nRet)
	{
		ERROR("RecvPushToClientMsg: Call Recv error%s", "");
		close(nSock);
		MM_FREE(pPushto_client_msg->pData);
		MM_FREE(pPushto_client_msg);	
		return DISCONNECT_WITH_SERVER;
	}

	char *pTmp_Data = pPushto_client_msg->pData;
	WORD wCmd_id = *(WORD *)(pTmp_Data + 1);
	wCmd_id = ntohs(wCmd_id);

	char arrAcc_checkcode[5] = {0};
	memcpy(arrAcc_checkcode, pPushto_client_msg->arrCheck_code, 4);
	
	DEBUG("RecvPushToClientMsg: [version]=%d [msg type]=%d [main service code]=%d [sub service code]=%d"
		" [access server seq]=%d [client socket]=%d [access check code]=%s [data len]=%d [cmd_id]=%d", pPushto_client_msg->bVersion, pPushto_client_msg->bMsg_type, \
		pPushto_client_msg->bMain_service_code, pPushto_client_msg->bSub_service_code, ntohs(pPushto_client_msg->wAccess_srv_seq), \
		ntohl(pPushto_client_msg->nClient_sock), arrAcc_checkcode, ntohl(pPushto_client_msg->nData_len), wCmd_id);	

	Srv_Client_Msg_Info *pSrv_client_msg_info = (Srv_Client_Msg_Info *)MM_MALLOC_WITH_DESC(sizeof(Srv_Client_Msg_Info), \
		"RecvPushToClientMsg: Call func for srv client msg info");
	if (NULL == pSrv_client_msg_info)
	{
		FATAL("RecvPushToClientMsg: Call MM_MALLOC_WITH_DESC error%s", "");
		MM_FREE(pPushto_client_msg->pData);
		MM_FREE(pPushto_client_msg);	
		return OUT_OF_MEMORY_ERROR;
	}

	memset(pSrv_client_msg_info, 0, sizeof(Srv_Client_Msg_Info));

	pSrv_client_msg_info->pMsg = (void *)pPushto_client_msg;
	pSrv_client_msg_info->nClient_sock = ntohl(pPushto_client_msg->nClient_sock);
	//memcpy(pSrv_client_msg_info->arrSn, szBuf, sizeof(pSrv_client_msg_info->arrSn));

	nRet = InsertItemToQueue(&g_sndclient_msgqueue.stLink_queue, (int)pSrv_client_msg_info);
	if (TRUE != nRet)
	{
		ERROR("RecvPushToClientMsg: Call InsertItemToQueue error%s", "");
		MM_FREE(pPushto_client_msg->pData);	
		MM_FREE(pPushto_client_msg);
		MM_FREE(pSrv_client_msg_info);
		return FALSE;		//插入失败, 不必管他, 只是简单地丢弃数据包就可以了
	}
	
	pthread_cond_signal(&g_sndclient_msgqueue.queue_cond);  
	INFO("RecvPushToClientMsg: func end%s", "");
	return TRUE;
}

int RecvDisconWithClientMsg(int nSock, char *pContent)
{
	INFO("RecvDisconWithClientMsg: func begin%s", "");
	if (NULL == pContent)
	{
		ERROR("RecvDisconWithClientMsg: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	int nRet = 0;
	int nData_len = 0;
	StReq_DisconnWithClient_MSG *pDisconn_withclient_msg = (StReq_DisconnWithClient_MSG *)MM_MALLOC_WITH_DESC(sizeof(StReq_DisconnWithClient_MSG), \
		"RecvDisconWithClientMsg: Call func for disconn with client msg");
	if (NULL == pDisconn_withclient_msg)
	{
		FATAL("RecvDisconWithClientMsg: Call MM_MALLOC_WITH_DESC error%s", "");
		return OUT_OF_MEMORY_ERROR;
	}
	memset(pDisconn_withclient_msg, 0, sizeof(StReq_DisconnWithClient_MSG));

	
	pDisconn_withclient_msg->bVersion = pContent[0];
	pDisconn_withclient_msg->bMsg_type = pContent[1];
	
	char *pTmp_msg = (char *)pDisconn_withclient_msg;
	nRet = Recv(nSock, pTmp_msg + 2, sizeof(Server_Client_Msg) - 10);
	if (FALSE == nRet)
	{
		ERROR("RecvDisconWithClientMsg: Call Recv error%s", "");
		close(nSock);
		MM_FREE(pDisconn_withclient_msg);
		return DISCONNECT_WITH_SERVER;
	}

	nData_len = ntohl(pDisconn_withclient_msg->nData_len);
	
	pDisconn_withclient_msg->pData = (char *)MM_MALLOC_WITH_DESC(nData_len, \
	"RecvDisconWithClientMsg: Call func for service layer data");
	if (NULL == pDisconn_withclient_msg->pData)
	{
		FATAL("RecvDisconWithClientMsg: Call MM_MALLOC_WITH_DESC error%s", "");
		close(nSock);
		MM_FREE(pDisconn_withclient_msg);
		return OUT_OF_MEMORY_ERROR;
	}

	nRet = Recv(nSock, (char *)pDisconn_withclient_msg->pData, nData_len);
	if (FALSE == nRet)
	{
		ERROR("RecvDisconWithClientMsg: Call Recv error%s", "");
		close(nSock);
		MM_FREE(pDisconn_withclient_msg->pData);
		MM_FREE(pDisconn_withclient_msg);
		return DISCONNECT_WITH_SERVER;
	}

	nRet = Recv(nSock, (char *)pDisconn_withclient_msg->arrCheck_code, 4);
	if (FALSE == nRet)
	{
		ERROR("RecvDisconWithClientMsg: Call Recv error%s", "");
		close(nSock);
		MM_FREE(pDisconn_withclient_msg->pData);
		MM_FREE(pDisconn_withclient_msg);	
		return DISCONNECT_WITH_SERVER;
	}

	char *pTmp_Data = pDisconn_withclient_msg->pData;
	WORD wCmd_id = *(WORD *)(pTmp_Data + 1);
	wCmd_id = ntohs(wCmd_id);

	char arrAcc_checkcode[5] = {0};
		memcpy(arrAcc_checkcode, pDisconn_withclient_msg->arrCheck_code, 4);
		
		DEBUG("RecvDisconWithClientMsg: [version]=%d [msg type]=%d [main service code]=%d [sub service code]=%d"
			" [access server seq]=%d [client sock]=%d  [access check code]=%s [data len]=%d [cmd_id]=%d", pDisconn_withclient_msg->bVersion, pDisconn_withclient_msg->bMsg_type, \
		pDisconn_withclient_msg->bMain_service_code, pDisconn_withclient_msg->bSub_service_code, ntohs(pDisconn_withclient_msg->wAccess_server_seq), \
		ntohl(pDisconn_withclient_msg->nClient_socket), arrAcc_checkcode, ntohl(pDisconn_withclient_msg->nData_len), wCmd_id);	


	Srv_Client_Msg_Info *pSrv_client_msg_info = (Srv_Client_Msg_Info *)MM_MALLOC_WITH_DESC(sizeof(Srv_Client_Msg_Info), \
		"RecvDisconWithClientMsg: Call func for srv msg client info");
	if (NULL == pSrv_client_msg_info)
	{
		FATAL("RecvDisconWithClientMsg: Call MM_MALLOC_WITH_DESC error%s", "");
		MM_FREE(pDisconn_withclient_msg->pData);
		MM_FREE(pDisconn_withclient_msg);	
		return OUT_OF_MEMORY_ERROR;
	}

	memset(pSrv_client_msg_info, 0, sizeof(Srv_Client_Msg_Info));

	pSrv_client_msg_info->pMsg = (void *)pDisconn_withclient_msg;
	pSrv_client_msg_info->nClient_sock = ntohl(pDisconn_withclient_msg->nClient_socket);

	nRet = InsertItemToQueue(&g_sndclient_msgqueue.stLink_queue, (int)pSrv_client_msg_info);
	if (TRUE != nRet)
	{
		ERROR("RecvDisconWithClientMsg: Call InsertItemToQueue error%s", "");
		MM_FREE(pDisconn_withclient_msg->pData);	
		MM_FREE(pDisconn_withclient_msg);
		MM_FREE(pSrv_client_msg_info);
		return FALSE;		//插入失败, 不必管他, 只是简单地丢弃数据包就可以了
	}
	
	pthread_cond_signal(&g_sndclient_msgqueue.queue_cond);  
	INFO("RecvDisconWithClientMsg: func end%s", "");
	return TRUE;
};

//函数用途: 接收业务服务器发送过来的forward类型消息包,  并且把消息包装法给响应的客户端
//输入参数: socket id, 主业务码,  子业务码
//输出参数:  无
//返回值	: 接收成功,  返回TRUE,  接收失败,   返回FALSE


int RecvForwardToClientMsg(int nSock, char *pContent)
{
	LOG_DEBUG("RecvForwardToClientMsg: enter RecvForwardToClientMsg", FILE_NAME, FILE_LINE);

	Forward_Srv_Client_Msg forward_srv_client_msg;
	char *pForward_srv_client_msg = (char *)&forward_srv_client_msg;
	char szBuf[1024] = {0};
	int nRet = 0;
	int nData_len = 0;
	int nClient_sock = 0;
	BYTE bStatus = SND_CLIENT_MSG_FAILURE;
		
	forward_srv_client_msg.bVersion = pContent[0];
	forward_srv_client_msg.bMsg_type = pContent[1];

	//接收消息头
	nRet = Recv(nSock, pForward_srv_client_msg + 2, sizeof(Forward_Srv_Client_Msg) - 10);
	if (FALSE == nRet)
	{	
		LOG_ERROR("RecvForwardToClientMsg: Call Recv error", FILE_NAME, FILE_LINE);
		return FALSE;
	}


	nData_len = ntohl(forward_srv_client_msg.nData_len);
	
	forward_srv_client_msg.pData = (char *)MM_MALLOC_WITH_DESC(nData_len, \
	"RecvForwardToClientMsg: Call func for service layer data");
	if (NULL == forward_srv_client_msg.pData)
	{
		LOG_ERROR("RecvForwardToClientMsg: out of memory", FILE_NAME, FILE_LINE);
		return FALSE;
	}

	//接收消息体
	nRet = Recv(nSock, (char *)forward_srv_client_msg.pData, nData_len);
	if (FALSE == nRet)
	{
		LOG_ERROR("RecvForwardToClientMsg: Call Recv error", FILE_NAME, FILE_LINE);
		MM_FREE(forward_srv_client_msg.pData);
		return FALSE;
	}
	
	nRet = Recv(nSock, (char *)forward_srv_client_msg.arrCheck_code, 4);
	if (FALSE == nRet)
	{
		LOG_ERROR("RecvForwardToClientMsg: Call Recv error", FILE_NAME, FILE_LINE);
		MM_FREE(forward_srv_client_msg.pData);
		return FALSE;
	}
	
	char arrCheck_code[5] = {0};
	memcpy(arrCheck_code, forward_srv_client_msg.arrCheck_code, 4);

	
	/*PrintContent("RecvForwardToClientMsg: version=%d msg_type=%d main code=%d sub code=%d access_seq=%d \
			client_sock=%d service_srv_seq=%d counter=%d data_len=%d chk_code=%s\n", forward_srv_client_msg.bVersion, \
			forward_srv_client_msg.bMsg_type, forward_srv_client_msg.bMain_service_code, \
			forward_srv_client_msg.bSub_service_code, ntohs(forward_srv_client_msg.wDest_Access_srv_seq), \
			ntohl(forward_srv_client_msg.nAccess_socket_id), ntohs(forward_srv_client_msg.wService_server_seq), \
			ntohl(forward_srv_client_msg.nMsg_counter), ntohl(forward_srv_client_msg.nData_len), arrCheck_code);*/

	/*snprintf(szBuf, sizeof(szBuf) - 1, "RecvForwardToClientMsg: version=%d msg_type=%d main code=%d sub code=%d \
		src_access_seq=%d access_msg_counter=%d dest_access_seq=%d \
		client_sock=%d service_srv_seq=%d counter=%d data_len=%d chk_code=%s", forward_srv_client_msg.bVersion, \
		forward_srv_client_msg.bMsg_type, forward_srv_client_msg.bMain_service_code, \
		forward_srv_client_msg.bSub_service_code, ntohs(forward_srv_client_msg.wSrc_access_srv_seq), \
		ntohs(forward_srv_client_msg.nAccess_Msg_counter), ntohs(forward_srv_client_msg.wDest_access_srv_seq), \
		ntohl(forward_srv_client_msg.nAccess_socket_id), ntohs(forward_srv_client_msg.wService_server_seq), \
		ntohl(forward_srv_client_msg.nService_Msg_counter), ntohl(forward_srv_client_msg.nData_len), arrCheck_code);

	LOG_DEBUG(szBuf, FILE_NAME, FILE_LINE);*/

	nClient_sock = ntohl(forward_srv_client_msg.nAccess_socket_id);

	//PrintContent("RecvForwardToClientMsg: nClient_sock=%d\n", nClient_sock);

	//发送消息给客户端
	nRet = SendClientMsg(nClient_sock, &forward_srv_client_msg, FORWARD_CLIENT_MSG);

	//void *pData = NULL;
	
	if (FALSE == nRet)
	{
		LOG_ERROR("RecvForwardToClientMsg: Call SendClientMsg error", FILE_NAME, FILE_LINE);

		close(nClient_sock);
		MM_FREE(forward_srv_client_msg.pData);

		//删除客户端哈希表信息
		if (IM_SERVICE_TYPE == forward_srv_client_msg.bMain_service_code && SIMPLECHAT_SERVICE_TYPE == forward_srv_client_msg.bSub_service_code)
		{
			void *pData = HashDel(&g_clientinfo_hash.pClient_info_hash, szBuf);
			MM_FREE(pData);
			bStatus = SND_CLIENT_MSG_FAILURE;

			//这个是例外,   不是交给调用函数进行处理
			//发送给对应的业务服务器,  报告发送数据包给客户端的状态
			nRet = SndDisposeResultToServiceSrv(nSock, forward_srv_client_msg, bStatus);
			if (FALSE == nRet)
			{
				LOG_ERROR("RecvForwardToClientMsg: Calll SndDisposeResultToServiceSrv error", FILE_NAME, FILE_LINE);
				return DISCONNECT_WITH_SERVER;
			}
		}

		return DISCONNECT_WITH_CLIENT;	
	}
	else if (TRUE == nRet)
	{
		if (IM_SERVICE_TYPE == forward_srv_client_msg.bMain_service_code && SIMPLECHAT_SERVICE_TYPE == forward_srv_client_msg.bSub_service_code)
		{
			//从哈希表中删除客户端信息
			void *pData = HashDel(&g_clientinfo_hash.pClient_info_hash, szBuf);
			MM_FREE(pData);
		
			bStatus = SND_CLIENT_MSG_SUCCEED;

			//发送转发结果给对应的业务服务器
			nRet = SndDisposeResultToServiceSrv(nSock, forward_srv_client_msg, bStatus);
			if (FALSE == nRet)
			{
				LOG_ERROR("RecvForwardToClientMsg: Calll SndDisposeResultToServiceSrv error", FILE_NAME, FILE_LINE);
				MM_FREE(forward_srv_client_msg.pData);
				return DISCONNECT_WITH_SERVER;
			}
		}
	}

	//PrintContent("send succeed\n");

	MM_FREE(forward_srv_client_msg.pData);
	LOG_DEBUG("RecvForwardToClientMsg: leave RecvForwardToClientMsg", FILE_NAME, FILE_LINE);
	
	return TRUE;
}

static void ResetSocketValue(char *pKey)
{
	INFO("ResetSocketValue: func begin%s", "");
	if (NULL == pKey)
	{
		ERROR("ResetSocketValue: func param error%s", "");
		return;
	}

	StHash_Item *pItem = NULL;
	Server_Info *pServer_info = NULL;

	pItem = HashGetItem(g_routing_table.pRouting_table, pKey);
	if (pItem != NULL)
	{
		pthread_mutex_lock(&g_routing_table.routing_mutex);
		pServer_info = (Server_Info *)pItem->pMatch_msg;
		pServer_info->nSock = 0;
		pthread_mutex_unlock(&g_routing_table.routing_mutex);
	}
	
	INFO("ResetSocketValue: func end%s", "");
}

void RecvSrvMsgErrorProc(void)
{
	INFO("RecvSrvMsgErrorProc: func begin%s", "");
	
	INFO("RecvSrvMsgErrorProc: func end%s", "");
}

void *RcvServiceSrvMsg(void *pParam)
{
	TRACE();
	return NULL;
}

//函数用途: 接收从业务服务器发送过来的消息
//输入参数: 存放socket id 的缓存
//输出参数:  无
//返回值	: 无

void *RecvMsgFromServiceSrv(void *pThread_param)
{
	INFO("RecvMsgFromServiceSrv: func begin%s", "");
	//把线程设置为分离线程
	//pthread_detach(pthread_self());
	
	char szBuf[100] = {0};
	Recv_Thread_Param *pRecv_thread_param = (Recv_Thread_Param *)pThread_param;
	int nClient_sock = pRecv_thread_param->nSock;

	BYTE bMain_service_code = pRecv_thread_param->bMain_service_code;
	BYTE bSub_service_code = pRecv_thread_param->bSub_service_code;
	WORD wService_srv_seq = pRecv_thread_param->wService_srv_seq;
	char szKey[100] = {0};

	INFO("RecvMsgFromServiceSrv: [sock]=%d", pRecv_thread_param->nSock);

	snprintf(szKey, sizeof(szKey) - 1, "%d%d%d", bMain_service_code, bSub_service_code, wService_srv_seq);	
	int nRet = TRUE;

	while (TRUE)
	{
		//先接收消息类型
		nRet = Recv(nClient_sock, (char *)szBuf, 2);
		if (FALSE == nRet)
		{
			ERROR("RecvMsgFromServiceSrv: Call Recv error%s", "");
			ResetSocketValue(szKey);
			close(nClient_sock);
			MM_FREE(pRecv_thread_param);
			pthread_exit(NULL);
			INFO("RecvMsgFromServiceSrv: func end%s", "");
			return NULL;
		}


		//现在如果与业务服务器断开连接了,  暂时不用理会,  等下次客户端发送消息的时候
		//发送线程会进行业务服务器的重连操作,  因为如果这里进行重连的话,  找不到哈希表的键值

		DEBUG("RecvMsgFromServiceSrv: [msg type]=%d", szBuf[1]);
		switch (szBuf[1])
		{
			case RESPONSE_CLIENT_MSG:					//如果是响应客户端消息
				nRet = RecvResponseToClientMsg(nClient_sock, szBuf);
				if (DISCONNECT_WITH_SERVER == nRet || OUT_OF_MEMORY_ERROR == nRet)
				{
					ERROR("RecvMsgFromServiceSrv: Call RecvResponseToClientMsg error%s", "");
					if (DISCONNECT_WITH_SERVER == nRet)
					{
						ResetSocketValue(szKey);
					}
					
					MM_FREE(pRecv_thread_param);				
					pthread_exit(NULL);
					return NULL;
				}
				break;
			case PUSHTO_CLIENT_MSG:
				nRet = RecvPushToClientMsg(nClient_sock, szBuf);
				if (DISCONNECT_WITH_SERVER == nRet || OUT_OF_MEMORY_ERROR == nRet)
				{
					ERROR("RecvMsgFromServiceSrv: Call RecvPushToClientMsg error%s", "");
					if (DISCONNECT_WITH_SERVER == nRet)
					{
						ResetSocketValue(szKey);
					}
					
					MM_FREE(pRecv_thread_param);				
					pthread_exit(NULL);
					return NULL; 
				}
				break;
			case NOTIFY_DISCONNECT_WITHCLIENT_MSG:
				nRet = RecvDisconWithClientMsg(nClient_sock, szBuf);
				if (DISCONNECT_WITH_SERVER == nRet || OUT_OF_MEMORY_ERROR == nRet)
				{
					ERROR("RecvMsgFromServiceSrv: Call RecvDisconWithClientMsg error%s", "");
					if (DISCONNECT_WITH_SERVER == nRet)
					{
						ResetSocketValue(szKey);
					}
					
					MM_FREE(pRecv_thread_param);				
					pthread_exit(NULL);
					return NULL;
				}
				break;
			case FORWARD_CLIENT_MSG:					//如果是转发客户端
				nRet = RecvForwardToClientMsg(nClient_sock, szBuf);			//与业务服务器断连(发生在接收数据的时候)
				if (FALSE == nRet)
				{
					ERROR("RecvMsgFromServiceSrv: Call RecvForwardToClientMsg error%s", "");
					close(nClient_sock);
					MM_FREE(pRecv_thread_param);
					pthread_exit(NULL);
					return NULL;
				}
				else if (DISCONNECT_WITH_SERVER == nRet)			//与业务服务器断连(发生在发送数据的时候)
				{
					ERROR("RecvMsgFromServiceSrv: Call RecvForwardToClientMsg error%s", "");
					close(nClient_sock);
					MM_FREE(pRecv_thread_param);
					pthread_exit(NULL);			
					return NULL;
				}
				break;
			default:
				WARN("RecvMsgFromServiceSrv: msg type is not correct%s", "");
				break;
		}
	}

	INFO("RecvMsgFromServiceSrv: func end%s", "");
	return NULL;
}


