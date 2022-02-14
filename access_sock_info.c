
#include "../util/access_global.h"
#include "../interface/access_protocol.h"
//#include "../include/ydt_log.h"
#include "access_epoll.h"
#include "access_sock_info.h"

/*���ļ���������������socket��
  *
  *
  *
  */
  
//������;: ����socket
//�������: ��
//�������: ��Ŵ����õ�socket ����
//����ֵ: ��������ɹ�,  ����TRUE, �������ʧ��,  ����FALSE


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

//������;: ��socket ���ض��ĵ�ַ�ṹ��
//�������: ����socket id
//�������: ��
//����ֵ: ����󶨳ɹ�,  ����TRUE�� �����ʧ��,  ����FALSE

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


//������;: ����socket 
//�������:  ����socket id
//�������: ��
//����ֵ:  �����ɹ�,  ����TRUE,  ����ʧ��,  ����FALSE

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


//������;: ����TCP��������
//�������:  ����socket id
//�������: �������ӳɹ����صĿͻ��� socket  ����
//����ֵ	:	�������ӳɹ�,  ����TRUE,  ��������ʧ��,  ����FALSE 	

int Accept(int nListen_sock, int *pConn_sock)
{
	//����
	INFO("Accept: func begin%s", "");
	*pConn_sock = accept(nListen_sock, NULL, NULL);

	//����socket �Ƿ�����ģʽ,  ���Ե�*pConn_sock  С���㲢��errno����EAGAINʱ
	//��ʾ��������û�ж���
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

//������;:  �������� socket�˿�Ϊ�����ö˿�(��Ҫ�Ǳ���socket ����TIME_WAIT״̬ʱ, ���������ø�IP�Ͷ˿�)
//�������:  ����socket id
//�������:  ��
//����ֵ	: ���óɹ�,  ����TRUE�� ����ʧ�ܣ� ����FALSE

int SetSockReuseAddr(int nListen_sock)
{
	INFO("SetSockReuseAddr: func begin%s", "");
	int nVal = 1;
	//����socketΪ�����ö˿�
	if(setsockopt(nListen_sock, SOL_SOCKET,SO_REUSEADDR, &nVal, sizeof(nVal)) != 0)
	{
		ERROR("SetSockReuseAddr: Call setsockopt error error[%d]=%s", errno, strerror(errno));
		return FALSE;
	}

	INFO("SetSockReuseAddr: func end%s", "");
	return TRUE;
}

//������;:  ��������socket,  �ڳ����˳�ʱ, ���Ϲر�TCP����(��ֹwait-2 �ȴ�ʱ��)
//�������:  ������ socket id
//�������:  ��
//����ֵ	: ���óɹ�,  ����TRUE�� ����ʧ�ܣ� ����FALSE


int SetsockDonotLinger(int nListen_sock)
{
	INFO("SetsockDonotLinger: func begin%s", "");
	struct linger lin;
	lin.l_onoff = 1; // ��linegr����
	lin.l_linger = 0; // �����ӳ�ʱ��Ϊ 0 ��, ע�� TCPIP�����رգ������п��ܳ��ֻ���
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

//������;:  ����socketΪ������״̬
//�������: socket id
//�������:  ��
//����ֵ	: ���óɹ�,  ����TRUE�� ����ʧ�ܣ� ����FALSE

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

//���÷��ͽ��ջ���Ĵ�С
int SetSockBuf(int nSock)
{
	INFO("SetSockBuf: func begin%s", "");
	int nRet = 0;

	int nSndBuf = 40960;
	int nLen = sizeof(int);
	nRet = setsockopt(nSock, SOL_SOCKET, SO_SNDBUF, (void *)&nSndBuf, nLen);			//���÷��ͻ������Ĵ�С
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


//������;:  ����������socket 
//�������:  ��
//�������:  ��Ŵ������˵�socket ����
//����ֵ	: �����ɹ�,  ����TRUE������ʧ�ܣ� ����FALSE

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



//������;: ������Ϣ����
//�������:  socket id,  ������Ϣ�ĳ���
//�������:  �����Ϣ�Ļ���
//����ֵ	:  ���ճɹ�,  ����TRUE,  ����ʧ��,  ����FALSE

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
		
		//������ֵΪ-1ʱ˵����socket������copy���ͻ�������ʱ�����˴���
		//������ֵΪ��ʱ, ˵������Ͽ��˻����ǶԷ�������

		//������ͷ���ֵΪ-1,  ������Ϊ104,  ˵����ҵ��������Ͽ���
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

//������;: ������Ϣ����
//�������: socket id,  �����Ϣ�Ļ���, ������Ϣ�ĳ���, ����ID
//�������: �� 
//����ֵ	:  ���ͳɹ�,  ����TRUE �� ����ʧ��,  ����FALSE

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
	
	//������ֵΪ-1ʱ˵����socket������copy���ͻ�������ʱ�����˴���
	//������ֵΪ��ʱ, ˵������Ͽ��˻����ǶԷ�������
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

