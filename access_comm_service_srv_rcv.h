
#ifndef ACCESS_COMM_SERVICE_SRV_RCV_H
#define ACCESS_COMM_SERVICE_SRV_RCV_H

//Client_info ��ϣ��Ϊ�յĺ��־
#define GET_CLIENTINFO_ERROR 0x02

//��ͻ��˶�����־
#define DISCONNECT_WITH_CLIENT 0x03

//��ҵ������������ı�־
#define DISCONNECT_WITH_SERVER 0x04

//������Ϣ���ͻ��˳ɹ����
#define SND_CLIENT_MSG_SUCCEED 0x00					//���������ת����Ϣ�ɹ�
#define SND_CLIENT_MSG_FAILURE	   0x01				//���������ת����Ϣʧ��


//����ҵ�����������Ϣ��
void *RecvMsgFromServiceSrv(void *pClient);
void DelCloseClientSockInfo(int nSock);

struct StCliMsg
{
	int iCliSock;
	char arrSn[50];
	Server_Client_Msg serverCliMsg;
};

typedef struct
{
	void *pMsg;
	int nClient_sock;
	char arrSn[100];
}Srv_Client_Msg_Info;

#endif
