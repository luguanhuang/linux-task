
#include "../util/access_global.h"
#include "../interface/access_protocol.h"
//#include "../include/ydt_log.h"
#include "access_epoll.h"
#include "access_sock_info.h"

/*该文件是用来创建侦听socket的
  *
  *
  *
  */
  
//函数用途: 创建socket
//输入参数: 无
//输出参数: 存放创建好的socket 缓存
//返回值: 如果创建成功,  返回TRUE, 如果创建失败,  返回FALSE


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
//返回值: 如果绑定成功,  返回TRUE， 如果绑定失败,  返回FALSE

int Bind(int nListen_sock, WORD wPort)
{
	INFO("Bind: func begin%s", "");
	struct sockaddr_in srv_addr;		
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr =	htonl(INADDR_ANY);
	srv_addr.sin_port = htons(wPort);

	DEBUG("Bind: [port]=%d", wPort);
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
	DEBUG("Listen: [listen socket]=%d", nListen_sock);
	if (-1 == listen(nListen_sock, LISTEN_NO))
	{
		ERROR("Listen: Call listen error error[%d]=%s", errno, strerror(errno));
		return FALSE;
	}	

	INFO("Listen: func end%s", "");
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

	//侦听socket 是非阻塞模式,  所以当*pConn_sock  小于零并且errno等于EAGAIN时
	//表示还有数据没有读完
	if (*pConn_sock < 0)
	{
		if (-1 == *pConn_sock && EAGAIN == errno)
		{
			INFO("Accept: we finish to accept client connection request%s", "");
			return SERVER_ACCEPT_FINISH;		
		}
		else
		{
			ERROR("Accept: Call accept error error[%d]=%s", errno, strerror(errno));
			return FALSE;

		}
	}
	else
	{
		INFO("Accept: func end%s", "");
		return TRUE;	

	}
}

//函数用途:  设置侦听 socket端口为可重用端口(主要是避免socket 处于TIME_WAIT状态时, 你又想重用该IP和端口)
//输入参数:  侦听socket id
//输出参数:  无
//返回值	: 设置成功,  返回TRUE， 设置失败， 返回FALSE

int SetSockReuseAddr(int nListen_sock)
{
	INFO("SetSockReuseAddr: func begin%s", "");
	int nVal = 1;
	//设置socket为可重用端口
	if(setsockopt(nListen_sock, SOL_SOCKET,SO_REUSEADDR, &nVal, sizeof(nVal)) != 0)
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

//函数用途:  设置socket为非阻塞状态
//输入参数: socket id
//输出参数:  无
//返回值	: 设置成功,  返回TRUE， 设置失败， 返回FALSE

int SetNonBlocking(int nSock)
{
	INFO("SetNonBlocking: func begin%s", "");
	int nOpts = fcntl(nSock, F_GETFL);
	if (nOpts < 0)
	{
		ERROR("SetNonBlocking: Call fcntl error [return value]=%d error[%d]=%s", \
			nOpts, errno, strerror(errno));
		return FALSE;
	}

	nOpts |= O_NONBLOCK;
	int nRet = fcntl(nSock, F_SETFL, nOpts);
	if (nRet < 0)
	{
		ERROR("SetNonBlocking: Call fcntl error [return value]=%d error[%d]=%s", \
			nRet, errno, strerror(errno));
		return FALSE;
	}

	INFO("SetNonBlocking: func end%s", "");
	return TRUE;
}

//设置发送接收缓存的大小
int SetSockBuf(int nSock)
{
	INFO("SetSockBuf: func begin%s", "");
	int nRet = 0;

	int nSndBuf = 40960;
	int nLen = sizeof(int);
	nRet = setsockopt(nSock, SOL_SOCKET, SO_SNDBUF, (void *)&nSndBuf, nLen);			//设置发送缓冲区的大小
	if (-1 == nRet)
	{
		ERROR("SetSockBuf: Call setsockopt error [return value]=%d error[%d]=%s", \
			nRet, errno, strerror(errno));
		return FALSE;
	}

	int nRecvBuf = 40960;
	nRet = setsockopt(nSock, SOL_SOCKET, SO_RCVBUF, (void *)&nRecvBuf, nLen);
	if (-1 == nRet)
	{
		ERROR("SetSockBuf: Call setsockopt error [return value]=%d error[%d]=%s", \
			nRet, errno, strerror(errno));
		return FALSE;
	}

	INFO("SetSockBuf: func end%s", "");
	return TRUE;
}

int InitMaintainSockInfo(int *pListenSock, WORD wPort)
{	
	INFO("InitMaintainSockInfo: func begin%s", "");
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
	int nRet = 0;
	if (FALSE == Socket(pListenSock))
	{
		ERROR("InitListenSockInfo: Call Socket error%s", "");
		return FALSE;
	}

	if (FALSE == SetNonBlocking(*pListenSock))
	{
		ERROR("InitListenSockInfo: Call SetNonBlocking error%s", "");	
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

	

	if (FALSE == SetSockBuf(*pListenSock))
	{
		ERROR("InitListenSockInfo: Call SetSockBuf error%s", "");
		return FALSE;
	}

	uint32_t event_type = EPOLLIN | EPOLLET;
	
	nRet = PutSockIntoEpoll(*pListenSock, event_type);
	if (FALSE == nRet)
	{
		ERROR("InitListenSockInfo: Call PutSockIntoEpoll error%s", "");
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



int S_InitListenSockInfo(int *pListenSock, WORD wPort)
{
	if (FALSE == Socket(pListenSock))
	{
		ERROR("InitSimModeListenSockInfo: Call Socket error%s", "");
		return FALSE;
	}

	if (FALSE == SetSockReuseAddr(*pListenSock))
	{
		ERROR("InitSimModeListenSockInfo: Call SetSockReuseAddr error%s", "");	
		return FALSE;	
	}

	if (FALSE == SetsockDonotLinger(*pListenSock))
	{
		ERROR("InitSimModeListenSockInfo: Call SetsockDonotLinger error%s", "");	
		return FALSE;	
	}

	if (FALSE == Bind(*pListenSock, wPort))
	{
		ERROR("InitSimModeListenSockInfo: Call Bind error%s", "");	
		return FALSE;
	}

	
	if (FALSE == Listen(*pListenSock))
	{
		ERROR("InitSimModeListenSockInfo: Call Listen error%s", "");	
		return FALSE;
	}
	
	return TRUE;
}



//函数用途: 接收消息函数
//输入参数:  socket id,  接收消息的长度
//输出参数:  存放消息的缓存
//返回值	:  接收成功,  返回TRUE,  接收失败,  返回FALSE

int Recv(int nSock, char *pMsg, int nLen)
{
	INFO("Recv: func begin%s", "");
	if (NULL == pMsg || nLen <= 0)
	{
		ERROR("Recv: func param error%s", "");
		return FALSE;
	}
	
	int nTotal_len = nLen;
	int nRecv_len = 0;
	int nRet = 0;
	int nRemain_len = nTotal_len - nRecv_len;
	
	while (nRemain_len > 0)
	{
		nRemain_len = nRemain_len > RECV_LEN ? RECV_LEN : nRemain_len;
		nRet = recv(nSock, pMsg + nRecv_len, nRemain_len, 0);
		
		//当返回值为-1时说明从socket缓冲区copy到客户缓存区时发生了错误
		//当返回值为零时, 说明网络断开了或者是对方断连了

		//如果发送返回值为-1,  错误码为104,  说明是业务服务器断开了
		if (-1 == nRet || 0 == nRet)
		{
			ERROR("Recv: Call recv error, [return value]=%d error[%d]=%s", \
				nRet, errno, strerror(errno));
			INFO("Recv: func end%s", "");
			return FALSE;	
		}
		
		nRecv_len += nRet;
		nRemain_len = nTotal_len - nRecv_len;
	}

	DEBUG("Recv: recv data len=%d", nRecv_len);
	INFO("Recv: func end%s", "");
	return nRecv_len;
}

//函数用途: 发送消息函数
//输入参数: socket id,  存放消息的缓存, 发送消息的长度, 命令ID
//输出参数: 无 
//返回值	:  发送成功,  返回TRUE ， 发送失败,  返回FALSE

int Send(int nSock, char *pMsg, int nLen, WORD wCmd_id)
{
	INFO("Send: func begin%s", "");
	int nTotal_len = nLen;
	int nSnd_len = 0;
	int nRet = 0;
	int nRemain_len = nTotal_len - nSnd_len;
	
	while (nRemain_len > 0)
	{
		nRemain_len = nRemain_len > SEND_LEN ? SEND_LEN : nRemain_len;
		nRet = send(nSock, pMsg + nSnd_len, nRemain_len, 0);
		if (-1 == nRet)
		{
			ERROR("Send: [return]=%d [command id]=%d error[%d]=%s [socket]=%d", \
				nRet, wCmd_id, errno, strerror(errno), nSock);
			if (EAGAIN == errno)
			{
				WARN("Send: system send buf is full so wait 10ms and then send msg again%s", "");
				usleep(10000);
				continue;	
			}
			else
			{
				ERROR("Send: Call send error [return value]=%d [command id]=%d", nRet, wCmd_id);
				INFO("Send: func end%s", "");
				return nRet;			
			}
		}
		
		nSnd_len += nRet;
		nRemain_len = nTotal_len - nSnd_len;
	}

	DEBUG("Send: send data [data Len]=%d [socket id]=%d [command id]=%d", nLen, nSock, wCmd_id);
	INFO("Send: func end%s", "");
	return nSnd_len;
}



int RecvClientMsg(int nSock, void *pClient_server_msg)
{	
	TRACE();
	Client_Server_Msg *pClient_srv_msg = (Client_Server_Msg *)pClient_server_msg;
	int nRcv_len = 0;
	int nData_len = 0;
	nRcv_len = Recv(nSock, (char *)pClient_srv_msg, sizeof(Client_Server_Msg) - 8);
	INFO("RecvClientMsg: [socket]=%d", nSock);
	
	//当返回值为-1时说明从socket缓冲区copy到客户缓存区时发生了错误
	//当返回值为零时, 说明网络断开了或者是对方断连了
	if (0 == nRcv_len || -1 == nRcv_len)
	{
		ERROR("RecvClientMsg: client close or recv error recv Rcv_len=%d", nRcv_len);		
		close(nSock);
		return FALSE;
	}
	
	nData_len = ntohl(pClient_srv_msg->nData_len);
	pClient_srv_msg->pData = (char *)MM_MALLOC_WITH_DESC(nData_len, \
	"RecvClientMsg: Call func for service layer data");
	if (NULL == pClient_srv_msg->pData)
	{	
		FATAL("RecvClientMsg: Call malloc error%s", "");
		close(nSock);
		return FALSE;
	}
	
	nRcv_len = Recv(nSock, (char *)pClient_srv_msg->pData, nData_len);
	if (0 == nRcv_len || -1 == nRcv_len)
	{
		ERROR("RecvClientMsg: client close or recv error recv len=%d", nRcv_len);		
		MM_FREE(pClient_srv_msg->pData);
		close(nSock);
		return FALSE;
	}

	nRcv_len = Recv(nSock, (char *)pClient_srv_msg->arrCheck_code, 4);
	if (0 == nRcv_len || -1 == nRcv_len)
	{
		ERROR("RecvClientMsg: client close or recv error recv len=%d", nRcv_len);		
		MM_FREE(pClient_srv_msg->pData);
		close(nSock);
		return FALSE;
	}

	

	return TRUE;
}

