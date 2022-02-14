
#include "../util/access_global.h"
//#include "../include/ydt_log.h"
#include "../interface/access_protocol.h"
#include "access_sock_info.h"
#include "access_heartbeat_detectthread.h"
#include "access_routing_maintain.h"
#include "access_comm_client_snd.h"
#include "access_maintaininfo_thread.h"

#define MODIFY_LOG_LEVEL_SUCCEED 0x01
#define MODIFY_LOG_LEVEL_FAILURE 0x00

extern StMsg_Queue g_msg_queue;
extern StHeartbeat_Detect_Table g_heartbeatdetect_table;
extern StSnd_Client_MsgQueue g_sndclient_msgqueue;
extern StTmp_clientSock_Hash g_tmp_clientsock_hash;

extern StMsgBuf_HashTable g_msgbuf_hashtable;

#define QUERY_SRV_INFO 0x00			//查询服务器信息
#define MODIFY_LOG_LEVEL 0x01		//修改日记级别

extern StMsg_Queue g_msg_queue;			//消息队列,  主要是存放客户端传送过来的消息包
extern StRouting_Table g_routing_table;

//业务服务器	路由表
//extern Routing_Table g_routing_table;
extern Server_Conf_Info g_srv_conf_info;
extern StClientInfo_Hashtable g_clientinfo_hash;
extern StClient_Sock_Queue g_clientsocket_queue;

extern StTmp_clientSock_Hash g_sndclient_tmp_sockhash;

/*该文件是用来处理运维客户端到来的消息的
  *
  *
  *
  */

static int ProcQuerySrvInfoMsg(int nSock, void *pData)
{
	INFO("ProcQuerySrvInfoMsg: func begin%s", "");
	int nMsg_queue_len = 0;
	int nClientsock_queue_len = 0;
	int nRoutingtable_len = 0;
	int nHashdet_table_len = 0;
	int nMsgbuf_table_len = 0;
	int nMsginfo_table_len = 0;
	int nSndclient_msgqueue_len = 0;
	int nSndclient_sockhash_len = 0;
	int nRcvclimsg_tmpsockhash_len = 0;

	//g_tmp_clientsock_hash 没加信息

	Req_Query_Srv_Info *pQuery_srv_info = (Req_Query_Srv_Info *)pData;

	nMsg_queue_len  = SeqQueueLength(g_msg_queue.msg_queue);
	nHashdet_table_len = HashtableLen(g_heartbeatdetect_table.pHeartbeat_detect_table);
	nSndclient_msgqueue_len = LinkQueueLen(&g_sndclient_msgqueue.stLink_queue);
	nSndclient_sockhash_len = HashtableLen(g_sndclient_tmp_sockhash.pTmp_clientsock_table);
	
	nClientsock_queue_len = LinkQueueLen(&g_clientsocket_queue.stLink_queue);
	nRoutingtable_len = HashtableLen(g_routing_table.pRouting_table);
	nMsgbuf_table_len = HashtableLen(g_msgbuf_hashtable.pMsg_buf_hashtable);
	nMsginfo_table_len = HashtableLen(g_clientinfo_hash.pClient_info_hash);
	nRcvclimsg_tmpsockhash_len = HashtableLen(g_tmp_clientsock_hash.pTmp_clientsock_table);
	
	Res_Query_Srv_Info res_query_srv_info;
	memset(&res_query_srv_info, 0, sizeof(res_query_srv_info));
	res_query_srv_info.bVersion = pQuery_srv_info->bVersion;
	res_query_srv_info.wCmd_id = pQuery_srv_info->wCmd_id;
	res_query_srv_info.nMsg_queue_len = htonl(nMsg_queue_len);
	res_query_srv_info.nClientsock_queue_len = htonl(nClientsock_queue_len);
	res_query_srv_info.nRoutingtable_len = htonl(nRoutingtable_len);
	res_query_srv_info.nHeartdet_table_len = htonl(nHashdet_table_len);
	res_query_srv_info.nClientmsgbuf_table_len = htonl(nMsgbuf_table_len);
	res_query_srv_info.nClientmsginfo_table_len = htonl(nMsginfo_table_len);
	res_query_srv_info.nSndclient_msgqueue_len = htonl(nSndclient_msgqueue_len);
	res_query_srv_info.nSndclient_sockhash_len = htonl(nSndclient_sockhash_len);
	res_query_srv_info.nRcv_climsg_tmpsockhash_len = htonl(nRcvclimsg_tmpsockhash_len);

	int nLen = sizeof(Res_Query_Srv_Info);
	nLen = htonl(nLen);
	send(nSock, (char *)&nLen, sizeof(int), 0);
	send(nSock, (char *)&res_query_srv_info, sizeof(res_query_srv_info), 0);

	DEBUG("ProcQuerySrvInfoMsg: send message to maintain client [version]=%d [command id]=%d [message queue length]=%d"
		" [client sock queue length]=%d [routingtable length]=%d [heartdetect hash length]=%d"
		" [client message buffer table length]=%d [client message info table length]=%d"
		" [snd client msgqueue length]=%d [snd client temp sockhash length]=%d"
		" [recv client msg tmp sockhash len]=%d", \
		res_query_srv_info.bVersion, ntohs(res_query_srv_info.wCmd_id), nMsg_queue_len, \
		nClientsock_queue_len, nRoutingtable_len, nHashdet_table_len, \
		nMsgbuf_table_len, nMsginfo_table_len, nSndclient_msgqueue_len, \
		nSndclient_sockhash_len, nRcvclimsg_tmpsockhash_len);

	INFO("ProcQuerySrvInfoMsg: func end%s", "");
	return TRUE;
}

static int ProcModifyLogLevel(int nSock, void *pData)
{
	INFO("ProcModifyLogLevel: func begin%s", "");
	
	Req__Modify_Log_Level *pModify_log_level = (Req__Modify_Log_Level *)pData;
	Res__Modify_Log_Level res_modify_log_level;
	memset(&res_modify_log_level, 0, sizeof(res_modify_log_level));
	SetLogLevel((EnLevel)pModify_log_level->bLog_level);
	res_modify_log_level.bVersion = pModify_log_level->bVersion;
	res_modify_log_level.wCmd_id = pModify_log_level->wCmd_id; 
	res_modify_log_level.bModify_status = MODIFY_LOG_LEVEL_SUCCEED;
	
	int nLen = sizeof(Res__Modify_Log_Level);
	nLen = htonl(nLen);
	send(nSock, (char *)&nLen, sizeof(int), 0);
	send(nSock, (char *)&res_modify_log_level, sizeof(res_modify_log_level), 0);

	DEBUG("ProcModifyLogLevel: send message to client [version]=%d [command id]=%d [log level]=%d", \
		pModify_log_level->bVersion, ntohs(pModify_log_level->wCmd_id), pModify_log_level->bLog_level);
	INFO("ProcModifyLogLevel: func end%s", "");
	return TRUE;
}

static int ProcessMaintainMsg(WORD wCmd_id, int nSock, void *pData)
{
	INFO("ProcessMaintainMsg: func begin%s", "");
	if (NULL == pData)
	{
		ERROR("ProcessMaintainMsg: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = 0;

	DEBUG("ProcessMaintainMsg: [command id]=%d [socket id]=%d", wCmd_id, nSock);
	switch (wCmd_id)
	{
		case QUERY_SRV_INFO:
			nRet = ProcQuerySrvInfoMsg(nSock, pData);
			break;
		case MODIFY_LOG_LEVEL:
			nRet = ProcModifyLogLevel(nSock, pData);
			break;
		default:
			break;
	}

	INFO("ProcessMaintainMsg: func end%s", "");
	return nRet;
}

static void *RecvMaintainClientMsg(void *pSock)
{
	INFO("RecvMaintainClientMsg: func begin%s", "");
	if (NULL == pSock)
	{
		ERROR("RecvMaintainClientMsg: func param error%s", "");	
		return NULL;
	}
	
	int nRet = 0;
	WORD wCmd_id = 0;
	int nLen = 0;
	char *pData = NULL;
	int *pClient_sock = (int *)pSock;

	while (TRUE)
	{
		nRet = Recv(*pClient_sock, (char *)&nLen, sizeof(int));
		if (FALSE == nRet)
		{
			ERROR("RecvMaintainClientMsg: Call Recv error%s", "");
			close(*pClient_sock);
			MM_FREE(pClient_sock);
			pthread_exit(NULL);
			return NULL;
		}
		
		nLen = ntohl(nLen);
		pData = (char *)MM_MALLOC_WITH_DESC(nLen, \
		"RecvMaintainClientMsg: Call func for maintain cient message");
		if (NULL == pData)
		{
			FATAL("RecvMaintainClientMsg: Call malloc error%s", "");
			close(*pClient_sock);
			MM_FREE(pClient_sock);
			pthread_exit(NULL);
			return NULL;
		}
		
		nRet = Recv(*pClient_sock, pData, nLen);
		if (FALSE == nRet)
		{
			ERROR("RecvMaintainClientMsg: Call Recv error%s", "");
			close(*pClient_sock);
			MM_FREE(pClient_sock);
			pthread_exit(NULL);
			return NULL;
		}

		
		wCmd_id = *(WORD *)(pData + 1);
		wCmd_id = ntohs(wCmd_id);

		DEBUG("RecvMaintainClientMsg: recv client message [data len]=%d [command id]=%d", nLen, wCmd_id);
		ProcessMaintainMsg(wCmd_id, *pClient_sock, (void *)pData);


		MM_FREE(pData);	
	}

	INFO("RecvMaintainClientMsg: func end%s", "");
	return NULL;
}

//函数用途:  该函数主要用来让客户端了解服务器当前的运行情况的
//输入参数:  无
//输出参数:  无
//返回值	  : 无

void *RunMaintainThread(void *pParam)
{
	INFO("RunMaintainThread: func begin%s", "");
	pthread_detach(pthread_self());

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	WORD wMaintain_port = g_srv_conf_info.wMaintain_port;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	int nListen_sock = 0;
	int nClient_sock = 0;

	pthread_t thread_id;
	int *pClient_sock = NULL;

	DEBUG("RunMaintainThread: [run maintain port]=%d", wMaintain_port);
	int nRet = InitMaintainSockInfo(&nListen_sock, wMaintain_port);
	if (FALSE == nRet)
	{
	  ERROR("RunMaintainThread: Call InitMaintainSockInfo error%s", "");
	  pthread_exit(NULL);
	  return NULL;
	}

	while (TRUE)
	{
		nClient_sock = accept(nListen_sock, NULL, NULL);
		if (nClient_sock < 0)
		{
			ERROR("RunMaintainThread: Call accept error so sleep 3s"
				" and then accept client connection request again%s", "");

			sleep(3);
			continue;
		}


		INFO("RunMaintainThread: maintain thread accept client connection request succeed%s", "");

		pClient_sock = (int *)MM_MALLOC_WITH_DESC(sizeof(int), \
		"RunMaintainThread: Call func for maintain client socket");
		if (NULL == pClient_sock)
		{
			FATAL("StartServer: Call malloc error%s", "");
			continue;
			//return;

		}

		*pClient_sock = nClient_sock;

		nRet= pthread_create(&thread_id, NULL, RecvMaintainClientMsg, (void *)pClient_sock);
		//可能创建的线程数量太多, 等线程资源释放了,  再继续执行程序
		if (nRet != 0)
		{
			ERROR("RunMaintainThread: Call pthread_create error, wait 10s "
				" and then accept client connection request again%s", "");
			close(*pClient_sock);

			MM_FREE(pClient_sock);
			sleep(10);
			continue;
		}			
	}

	INFO("RunMaintainThread: func end%s", "");
	return NULL;
}

