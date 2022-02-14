
#include "../util/service_global.h"
#include "../interface/service_protocol.h"
#include "../include/ydt_log.h"
#include "../service/common_service/service_record_accesssrv_info.h"
#include "service_sock_info.h"

/*该文件是用来创建侦听socket的
  *
  *
  *
  */

//每次最大的接收数据的大小
#define RECV_LEN 2048
//每次最大的发送数据的大小
#define SEND_LEN 2048


extern Server_Conf_Info g_srv_conf_info;

extern StAccessSrv_Seq_Hash g_accesssrv_seq_hash;

//函数用途: 创建socket
//输入参数: 无
//输出参数: 存放创建好的socket 缓存
//返回值: 如果创建成功,  返回TRUE, 如果创建失败,  返回FALSE

static int RecvPushToCliResMsg(int nSock, void *pMsg);
static int RecvMsg(int nSock, void *pMsg);

//函数用途: 创建socket
//输入参数: 无
//输出参数: socket指针
//返回值	: 如果创建成功,  返回TRUE， 如果创建成功,  返回FALSE

int Socket(int *pListen_sock)
{
	INFO("Socket: func begin%s", "");
	if (-1 == (*pListen_sock= socket(AF_INET, SOCK_STREAM, 0)))
	{
		ERROR("Socket: Call socket error error[%d]=%s", errno, strerror(errno));
		return FALSE;
	}

	INFO("Socket: func end%s", "");
	return TRUE;
}

//函数用途: 绑定socket 到特定的地址结构上
//输入参数: 侦听socket id
//输出参数: 无
//返回值	: 如果绑定成功,  返回TRUE， 如果绑定失败,  返回FALSE

int Bind(int nListen_sock, WORD wPort)
{
	INFO("Bind: func begin%s", "");
	struct sockaddr_in srv_addr;	
	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr =	htonl(INADDR_ANY);
	srv_addr.sin_port = htons(wPort);

	if (-1 == bind(nListen_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)))
	{
		ERROR("Bind: Call bind error error[%d]=%s", errno, strerror(errno));
		return FALSE;
	}

	INFO("Bind: func end%s", "");
	return TRUE;
}

//函数用途: 侦听socket 
//输入参数:  侦听socket id
//输出参数: 无
//返回值:  侦听成功,  返回TRUE,  侦听失败,  返回FALSE

int Listen(int nListen_sock)
{
	INFO("Listen: func begin%s", "");
	if (-1 == listen(nListen_sock, LISTEN_NO))
	{
		ERROR("Listen: Call listen error error[%d]=%s", errno, strerror(errno));
		return FALSE;
	}	

	INFO("Listen: func end%s", "");
	return TRUE;
}

//函数用途: 接收客户端连接
//输入参数: 侦听socket
//输出参数: 连接socket, ip, 端口
//返回值:   接收连接成功,  返回TRUE,  接收连接失败,  返回FALSE

int T_Accept(int nListen_sock, int *pConn_sock, char *pIP, WORD *pPort)
{
	//接收
	INFO("T_Accept: func begin%s", "");
	if (NULL == pConn_sock || NULL == pIP || NULL == pPort)
	{
		ERROR("T_Accept: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	struct sockaddr_in sock_addr;
	memset(&sock_addr, 0, sizeof(sock_addr));
	unsigned int nSize = sizeof(struct sockaddr);	
	*pConn_sock = accept(nListen_sock, (struct sockaddr *)&sock_addr, &nSize);
	if (-1 == *pConn_sock)
	{
		ERROR("T_Accept: Call accept error error[%d]=%s", errno, strerror(errno));
		return FALSE;
	}

	DEBUG("T_Accept: access server connect [ip]=%s [port]=%d", inet_ntoa(sock_addr.sin_addr), ntohs(sock_addr.sin_port));
	memcpy(pIP, inet_ntoa(sock_addr.sin_addr), 50);
	*pPort = ntohs(sock_addr.sin_port);
	INFO("T_Accept: func end%s", "");
	return TRUE;
}


//函数用途: 接受TCP连接请求
//输入参数:  侦听socket id
//输出参数: 保存连接成功返回的客户端 socket  缓存
//返回值	:	接受连接成功,  返回TRUE,  接收连接失败,  返回FALSE 	

int Accept(int nListen_sock, int *pConn_sock)
{
	//接收	
	INFO("Accept: func begin%s", "");
	*pConn_sock = accept(nListen_sock, NULL, NULL);
	if (-1 == *pConn_sock)
	{
		ERROR("Accept: Call accept error error[%d]=%s", errno, strerror(errno));
		return FALSE;
	}

	INFO("Accept: func end%s", "");
	return TRUE;
}

//函数用途:  设置侦听 socket端口为可重用端口
//输入参数:  侦听socket id
//输出参数:  无
//返回值	: 设置成功,  返回TRUE， 设置失败， 返回FALSE

int SetSockReuseAddr(int nListen_sock)
{
	INFO("SetSockReuseAddr: func begin%s", "");
	int nVal = 1;

	//设置socket为可重用端口
	int nRet = setsockopt(nListen_sock, SOL_SOCKET,SO_REUSEADDR, &nVal, sizeof(nVal));
	
	if(nRet != 0)
	{
		ERROR("SetSockReuseAddr: Call setsockopt error error[%d]=%s", errno, strerror(errno));
		return FALSE;
	}

	INFO("SetSockReuseAddr: func end%s", "");
	return TRUE;
}


//函数用途:  设置侦听socket,  在程序退出时, 马上关闭TCP连接(防止wait-2 等待时间)
//输入参数:  侦听的 socket id
//输出参数:  无
//返回值	: 设置成功,  返回TRUE， 设置失败， 返回FALSE


int SetsockDonotLinger(int nListen_sock)
{
	INFO("SetsockDonotLinger: func begin%s", "");
	struct linger lin;
	lin.l_onoff = 1; // 打开linegr开关
	lin.l_linger = 0; // 设置延迟时间为 0 秒, 注意 TCPIP立即关闭，但是有可能出现化身
	int nRet = setsockopt(nListen_sock, SOL_SOCKET, SO_LINGER, (char *)&lin, sizeof(lin));
	if(nRet != 0)
	{
		ERROR("SetsockDonotLinger: Call setsockopt error [return value]=%d error[%d]=%s", \
			nRet, errno, strerror(errno));
		return FALSE;
	}

	INFO("SetsockDonotLinger: func end%s", "");
	return TRUE;
}

//函数用途:  初始化维护侦听socket信息
//输入参数:  侦听的 端口
//输出参数:  侦听socket指针
//返回值	: 设置成功,  返回TRUE， 设置失败， 返回FALSE

int InitMaintainSockInfo(int *pListenSock, WORD wPort)
{	
	INFO("InitMaintainSockInfo: func begin%s", "");
	
	DEBUG("InitMaintainSockInfo: [maintain thread  listen port]=%d", wPort);
	if (FALSE == Socket(pListenSock))
	{
		ERROR("InitMaintainSockInfo: Call Socket error%s", "");
		return FALSE;
	}

	if (FALSE == SetSockReuseAddr(*pListenSock))
	{
		ERROR("InitMaintainSockInfo: Call SetSockReuseAddr error%s", "");	
		return FALSE;	
	}

	if (FALSE == SetsockDonotLinger(*pListenSock))
	{
		ERROR("InitMaintainSockInfo: Call SetsockDonotLinger error%s", "");	
		return FALSE;	
	}

	if (FALSE == Bind(*pListenSock, wPort))
	{	
		ERROR("InitMaintainSockInfo: Call Bind error%s", "");	
		return FALSE;
	}

	
	if (FALSE == Listen(*pListenSock))
	{
		ERROR("InitMaintainSockInfo: Call Listen error%s", "");	
		return FALSE;
	}

	INFO("InitMaintainSockInfo: func end%s", "");
	return TRUE;
}




//函数用途:  创建侦听的socket 
//输入参数:  无
//输出参数:  存放创建好了的socket 缓存
//返回值	: 创建成功,  返回TRUE，创建失败， 返回FALSE

int InitListenSockInfo(int *pListenSock, WORD wPort)
{
	INFO("InitListenSockInfo: func begin%s", "");

	DEBUG("InitListenSockInfo: [server listen port]=%d", wPort);
	
	if (FALSE == Socket(pListenSock))
	{
		ERROR("InitListenSockInfo: Call Socket error%s", "");
		return FALSE;
	}

	if (FALSE == SetSockReuseAddr(*pListenSock))
	{
		ERROR("InitListenSockInfo: Call SetSockReuseAddr error%s", "");
		return FALSE;	
	}

	if (FALSE == SetsockDonotLinger(*pListenSock))
	{
		ERROR("InitListenSockInfo: Call SetsockDonotLinger error%s", "");
		return FALSE;	
	}

	if (FALSE == Bind(*pListenSock, wPort))
	{
		ERROR("InitListenSockInfo: Call Bind error%s", "");
		return FALSE;
	}

	
	if (FALSE == Listen(*pListenSock))
	{
		ERROR("InitListenSockInfo: Call Listen error%s", "");
		return FALSE;
	}

	INFO("InitListenSockInfo: func end%s", "");
	return TRUE;
}


//函数用途: 接收消息函数
//输入参数:  socket id,  接收消息的长度
//输出参数:  存放消息的缓存
//返回值	:  接收成功,  返回TRUE,  接收失败,  返回FALSE

int Recv(int nSock, char *pMsg, int nLen)
{
	INFO("Recv: func begin%s", "");
	int nTotal_len = nLen;
	int nRecv_len = 0;
	int nRet = 0;
	int nRemain_len = nTotal_len - nRecv_len;

	//不断地循环接收消息, 直到接收消息完成为止
	while (nRemain_len > 0)
	{
		nRemain_len = nRemain_len > RECV_LEN ? RECV_LEN : nRemain_len;
		nRet = recv(nSock, pMsg + nRecv_len, nRemain_len, 0);
		if (-1 == nRet || 0 == nRet)
		{
			ERROR("Recv: Call recv error [return value]=%d error[%d]=%s", \
				nRet, errno, strerror(errno));
			return nRet;	
		}
		
		nRecv_len += nRet;
		nRemain_len = nTotal_len - nRecv_len;
	}

	DEBUG("Recv: [socket id]=%d [recv data len]=%d", nSock, nRecv_len);
	INFO("Recv: func end%s", "");
	return nRecv_len;
}

//函数用途: 发送消息函数
//输入参数: socket id,  存放消息的缓存, 发送消息的长度, 命令ID
//输出参数: 无 
//返回值	:  发送成功,  返回TRUE ， 发送失败,  返回FALSE

int Send(int nSock, char *pMsg, int nLen)
{
	INFO("Send: func begin%s", "");
	int nTotal_len = nLen;
	int nSnd_len = 0;
	int nRet = 0;
	int nRemain_len = nTotal_len - nSnd_len;

	//不断地循环发送消息, 直到消息发送完为止
	while (nRemain_len > 0)
	{
		nRemain_len = nRemain_len > SEND_LEN ? SEND_LEN : nRemain_len;			//每次发送的数据不可以超过SEND_LEN长度
		nRet = send(nSock, pMsg + nSnd_len, nRemain_len, 0);
		if (-1 == nRet)
		{
			ERROR("Send: Call send error, [return value]=%d error[%d]=%s", \
				nRet, errno, strerror(errno));
			return nRet;	
		}
		
		nSnd_len += nRet;
		nRemain_len = nTotal_len - nSnd_len;
	}

	DEBUG("Send: [socket id]=%d [send data len]=%d", nSock, nSnd_len);
	INFO("Send: func end%s", "");
	return nSnd_len;
}

//函数用途: 获取接入服务器socketid
//输入参数: 接入服务器序号
//输出参数: socket信息
//返回值  : 获取成功,  返回TRUE ，获取失败,  返回FALSE

#if defined(_WANWEI_LOGIN_SERVICE_) || defined(_WANWEI_PUSH_SERVICE_)
static int GetAccessSrvSockId(WORD wAccess_srv_seq, StSock_Info *pSock_info)
{
	INFO("GetAccessSrvSockId: func begin%s", "");
	if (NULL == pSock_info)
	{
		ERROR("GetAccessSrvSockId: func param error%s", "");	
		return FUNC_PARAM_ERROR;
	}
	
	char arrAccess_srv_seq[50] = {0};
		
	snprintf(arrAccess_srv_seq, sizeof(arrAccess_srv_seq) - 1, "%d", wAccess_srv_seq);

	//获取socket信息
	StSock_Info *pTmp_sock_info = (StSock_Info *)HashData(g_accesssrv_seq_hash.pAccesssrv_seq_hash, arrAccess_srv_seq);
	if (NULL != pTmp_sock_info)
	{
		pSock_info->nSock = pTmp_sock_info->nSock;
		pSock_info->pSock_mutex = pTmp_sock_info->pSock_mutex;
	}
	else
	{
		WARN("GetAccessSrvSockId: we can't find the access srv socket id%s", "");
		return FALSE;
	}

	INFO("GetAccessSrvSockId: func end%s", "");
	return TRUE;
}

//函数用途: 从哈希表删除接入服务器信息
//输入参数: 接入服务器序号
//输出参数: 无
//返回值  : 无

static void DelAccessSrvInfoFromHash(WORD wAccess_srv_seq)
{
	char arrAccess_srv_seq[50] = {0};

	snprintf(arrAccess_srv_seq, sizeof(arrAccess_srv_seq) - 1, "%d", wAccess_srv_seq);
	void *pData = HashDel(&g_accesssrv_seq_hash.pAccesssrv_seq_hash, arrAccess_srv_seq);
	if (NULL != pData)
	{
		MM_FREE(pData);	
	}
}
#endif

#ifdef _WANWEI_PUSH_SERVICE_
int SndPushServiceMsg(void *pMsg)
{
	INFO("SndPushServiceMsg: func begin%s", "");
	if (NULL == pMsg)
	{
		ERROR("SndPushServiceMsg: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	WORD wAccess_srv_seq = 0;
	int nAccess_srv_sock = 0;
	int nRet = 0;
	
	Server_Client_Msg *pSrv_client_msg = (Server_Client_Msg *)pMsg;
	wAccess_srv_seq = ntohs(pSrv_client_msg->wAccess_server_seq);

	StSock_Info stSock_info;
	memset(&stSock_info, 0, sizeof(stSock_info));
	
	nRet = GetAccessSrvSockId(wAccess_srv_seq, &stSock_info);
	if (TRUE != nRet)
	{
		WARN("SndPushServiceMsg: Call GetAccessSrvSockId error%s", "");
		return TRUE;
	}
	
	nAccess_srv_sock = stSock_info.nSock;
	nRet = Send(nAccess_srv_sock, (char *)pMsg, sizeof(Server_Client_Msg) - 8);	
	if (-1 == nRet)
	{
		ERROR("SndPushServiceMsg: Call Send error ret=%d", nRet);
		close(nAccess_srv_sock);
		DelAccessSrvInfoFromHash(wAccess_srv_seq);
		return FALSE;
	}

	int nData_len = ntohl(pSrv_client_msg->nData_len);
		
	nRet = Send(nAccess_srv_sock, (char *)pSrv_client_msg->pData, nData_len); 
	if (-1 == nRet)
	{
		ERROR("SndPushServiceMsg: Call Send error ret=%d", nRet);
		close(nAccess_srv_sock);
		DelAccessSrvInfoFromHash(wAccess_srv_seq);
		return FALSE;
	}

	nRet= Send(nAccess_srv_sock, (char *)pSrv_client_msg->arrCheck_code, 4);	
	if (-1 == nRet)
	{
		ERROR("SndPushServiceMsg: Call Send error ret=%d", nRet);
		close(nAccess_srv_sock);
		DelAccessSrvInfoFromHash(wAccess_srv_seq);
		return FALSE;
	}
	
	INFO("SndPushServiceMsg: func end%s", "");
	return TRUE;
}
#endif


#ifdef _WANWEI_LOGIN_SERVICE_
int SendDisconnWithClientMsg(void *pMsg)
{
	INFO("SendDisconnWithClientMsg: func begin%s", "");
	int nRet = 0;
	WORD wAccess_srv_seq = 0;
	int nAccess_srv_sock = 0;
	
	StReq_DisconnWithClient_MSG *pDisconn_withclient_msg = (StReq_DisconnWithClient_MSG *)pMsg;
	wAccess_srv_seq = ntohs(pDisconn_withclient_msg->wAccess_server_seq);
	nRet = GetAccessSrvSockId(wAccess_srv_seq, &nAccess_srv_sock);
	if (TRUE != nRet)
	{
		ERROR("SendDisconnWithClientMsg: Call GetAccessSrvSockId error%s", "");
		return nRet;
	}
	
	nRet = Send(nAccess_srv_sock, (char *)pDisconn_withclient_msg, sizeof(StReq_DisconnWithClient_MSG) - 8);	
	if (-1 == nRet)
	{
		ERROR("SendDisconnWithClientMsg: Call Send error ret=%d", nRet);
		close(nAccess_srv_sock);
		DelAccessSrvInfoFromHash(wAccess_srv_seq);
		return DISCONNWITH_ACCESS_SRV;
	}

	int nData_len = ntohl(pDisconn_withclient_msg->nData_len);
		
	nRet = Send(nAccess_srv_sock, (char *)pDisconn_withclient_msg->pData, nData_len); 
	if (-1 == nRet)
	{
		ERROR("SendDisconnWithClientMsg: Call Send error ret=%d", nRet);
		close(nAccess_srv_sock);
		DelAccessSrvInfoFromHash(wAccess_srv_seq);
		return DISCONNWITH_ACCESS_SRV;
	}

	nRet= Send(nAccess_srv_sock, (char *)pDisconn_withclient_msg->arrCheck_code, 4);	
	if (-1 == nRet)
	{
		ERROR("SendDisconnWithClientMsg: Call Send error ret=%d", nRet);
		close(nAccess_srv_sock);
		DelAccessSrvInfoFromHash(wAccess_srv_seq);
		return DISCONNWITH_ACCESS_SRV;
	}
	
	INFO("SendDisconnWithClientMsg: func end%s", "");
	return TRUE;
}
#endif

//函数用途: 发送消息到接入服务器
//输入参数: socket id,  存放消息的缓存
//输出参数: 无 
//返回值	:  发送成功,  返回TRUE ， 发送失败,  返回FALSE


int SendMsg(int nSock, void *pMsg)
{
	INFO("SendMsg: func begin%s", "");
	if (NULL == pMsg)
	{
		ERROR("SendMsg: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	int nRet = 0;
	Server_Client_Msg *pSer_cli_msg = (Server_Client_Msg *)pMsg;

	nRet = Send(nSock, (char *)pSer_cli_msg, sizeof(Server_Client_Msg) - 8);	
	if (-1 == nRet)
	{
		ERROR("SendMsg: Call Send error ret=%d", nRet);
		return FALSE;
	}

	int nData_len = ntohl(pSer_cli_msg->nData_len);
		
	nRet = Send(nSock, (char *)pSer_cli_msg->pData, nData_len);	
	if (-1 == nRet)
	{
		ERROR("SendMsg: Call Send error ret=%d", nRet);
		return FALSE;
	}

	nRet= Send(nSock, (char *)pSer_cli_msg->arrCheck_code, 4);	
	if (-1 == nRet)
	{
		ERROR("SendMsg: Call Send error ret=%d", nRet);
		return FALSE;
	}

	INFO("SendMsg: func end%s", "");
	return TRUE;

}

int RecvAccessSrvMsg(int nSock, void *pMsg)
{
	INFO("RecvAccessSrvMsg: func begin%s", "");
	char szBuf[10] = {0};
	int nRet = 0;

	nRet = Recv(nSock, szBuf, MSG_TYPE_OFFSET);
	if (-1 == nRet || 0 == nRet)
	{
		ERROR("RecvAccessSrvMsg: Call Recv error [return value]=%d", nRet);
		return FALSE;
	}

	BYTE bMsg_type = szBuf[1];
	char *pTmp_msg = (char *)pMsg;
	*pTmp_msg = szBuf[0];
	pTmp_msg++;
	*pTmp_msg = szBuf[1];
	
	switch (bMsg_type)
	{
		case CLIENT_SERVER_MSG:
			nRet = RecvMsg(nSock, pMsg);
			break;
		case PUSHTO_CLIENT_RESPONSE:
			nRet = RecvPushToCliResMsg(nSock, pMsg);
			break;
		default:
			WARN("RecvAccessSrvMsg: msg type error [msg type]=%d", \
				bMsg_type);
	}
	
	INFO("RecvAccessSrvMsg: func end%s", "");
	return nRet;
}

int RecvPushToCliResMsg(int nSock, void *pMsg)
{
	INFO("RecvPushToCliResMsg: func begin%s", "");
	int nRet = 0;
	PushToClient_Response_Msg *pPushtocli_res_msg = (PushToClient_Response_Msg *)pMsg;
	char *pTmp_msg = (char *)pMsg;

	nRet = Recv(nSock, pTmp_msg + 2, sizeof(PushToClient_Response_Msg) - 10);
	if (-1 == nRet || 0 == nRet)
	{
		ERROR("RecvPushToCliResMsg: Call Recv error [return value]=%d", nRet);
		return FALSE;
	}

	int nData_len = ntohl(pPushtocli_res_msg->nData_len);
	pPushtocli_res_msg->pData= (char *)MM_MALLOC_WITH_DESC(nData_len, \
	"RecvPushToCliResMsg: Call func for service layer data");
	if (NULL == pPushtocli_res_msg->pData)
	{
		FATAL("RecvPushToCliResMsg: Call malloc() error%s", "");
		return FALSE;
	}

	memset(pPushtocli_res_msg->pData, 0, nData_len);

	//接收消息数据
	nRet = Recv(nSock, (char *)pPushtocli_res_msg->pData, nData_len);
	if (-1 == nRet || 0 == nRet)
	{	
		ERROR("RecvPushToCliResMsg: Call Recv error [return value]=%d", nRet);	
		MM_FREE(pPushtocli_res_msg->pData);
		return FALSE;
	}

	nRet = Recv(nSock, (char *)pPushtocli_res_msg->arrCheck_code, 4);
	if (-1 == nRet || 0 == nRet)
	{
		ERROR("RecvPushToCliResMsg: Call Recv error [return value]=%d", nRet);
		MM_FREE(pPushtocli_res_msg->pData);
		return FALSE;
	}
	
	INFO("RecvPushToCliResMsg: func end%s", "");
	return TRUE;
}



//函数用途: 接收接入服务器发送过来的整体的数据包
//输入参数: socket id,  存放消息的缓存
//输出参数: 无 
//返回值	:  接收成功,  返回TRUE ， 接收失败,  返回FALSE

int RecvMsg(int nSock, void *pMsg)
{
	INFO("RecvMsg: func begin%s", "");
	int nRet = 0;
	Client_Server_Msg *pClient_srv_msg = (Client_Server_Msg *)pMsg;
	char *pTmp_msg = (char *)pMsg;

	//接收消息包头部
	//nRet = Recv(nSock, pTmp_msg, sizeof(Client_Server_Msg) - 8);
	nRet = Recv(nSock, pTmp_msg + 2, sizeof(Client_Server_Msg) - 10);
	if (-1 == nRet || 0 == nRet)
	{
		ERROR("RecvMsg: Call Recv error [return value]=%d", nRet);
		return FALSE;
	}

	if (nRet != sizeof(Client_Server_Msg) - 10)
	{
		ERROR("RecvMsg: Call Recv error [return value]=%d", nRet);
		return FALSE;
	}
	
	int nData_len = ntohl(pClient_srv_msg->nData_len);
	pClient_srv_msg->pData= (char *)MM_MALLOC_WITH_DESC(nData_len, \
	"RecvMsg: Call func for service layer data");
	if (NULL == pClient_srv_msg->pData)
	{
		FATAL("RecvMsg: Call malloc() error%s", "");
		return FALSE;
	}

	memset(pClient_srv_msg->pData, 0, nData_len);

	//接收消息数据
	nRet = Recv(nSock, (char *)pClient_srv_msg->pData, nData_len);
	if (-1 == nRet || 0 == nRet)
	{	
		ERROR("RecvMsg: Call Recv error [return value]=%d", nRet);	
		MM_FREE(pClient_srv_msg->pData);
		return FALSE;
	}

	if (nRet != nData_len)
	{
		ERROR("RecvMsg: Call Recv error [return value]=%d", nRet);
		MM_FREE(pClient_srv_msg->pData);
		return FALSE;
	}

	//接收校验码
	nRet = Recv(nSock, (char *)pClient_srv_msg->arrCheck_code, 4);
	if (-1 == nRet || 0 == nRet)
	{
		ERROR("RecvMsg: Call Recv error [return value]=%d", nRet);
		MM_FREE(pClient_srv_msg->pData);
		return FALSE;
	}

	if (nRet != 4)
	{
		ERROR("RecvMsg: Call Recv error [return value]=%d", nRet);
		MM_FREE(pClient_srv_msg->pData);
		return FALSE;
	}

	INFO("RecvMsg: func end%s", "");
	return TRUE;
}

#ifdef _WANWEI_QUERY_SERVICE_
int SndMsgToPushServer(int nSock, void *pMsg)
{
	INFO("SndMsgToPushServer: func begin%s", "");
	if (NULL == pMsg)
	{
		ERROR("SndMsgToPushServer: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	Client_Server_Msg *pClient_srv_msg = (Client_Server_Msg *)pMsg;
	int nRet = 0;
	
	nRet = Send(nSock, (char *)pClient_srv_msg, sizeof(Client_Server_Msg) - 8);	
	if (-1 == nRet)
	{
		ERROR("SndMsgToPushServer: Call Send error ret=%d", nRet);
		return FALSE;
	}

	int nData_len = ntohl(pClient_srv_msg->nData_len);
	nRet = Send(nSock, (char *)pClient_srv_msg->pData, nData_len);	
	if (-1 == nRet)
	{
		ERROR("SndMsgToPushServer: Call Send error ret=%d", nRet);
		return FALSE;
	}

	nRet= Send(nSock, (char *)pClient_srv_msg->arrCheck_code, 4);	
	if (-1 == nRet)
	{
		ERROR("SndMsgToPushServer: Call Send error ret=%d", nRet);
		return FALSE;
	}
	
	INFO("SndMsgToPushServer: func end%s", "");
	return TRUE;
}
#endif

