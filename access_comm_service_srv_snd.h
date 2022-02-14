
#ifndef ACCESS_COMM_SERVICE_SRV_SND_H
#define ACCESS_COMM_SERVICE_SRV_SND_H

#include "../util/access_types.h"
#include "../interface/access_protocol.h"
#include "../include/hash.h"




//������������ظ�ҵ�����������Ӧ����Ϣ����
#define RESPONSE_CLIENT_SRV_MSG 0x04

#define YDT_SN_LENGTH 100			//��ϣ��ļ�ֵ����

//��������ҵ�����������ά��¼���������͵ĵ�һ����Ϣ����
#define AFTER_ACCESS_SRV_CONN 0x50

#define CLIENT_SERVER_MSG 0x01      //�ͻ��˵�����������Ϣ����
#define CLIENT_SERVER_BIG_MSG  0x05
#define PUSHTO_CLIENT_RESPONSE 0x0B 

//ҵ�������������
typedef struct
{
	pthread_mutex_t num_mutex;					//ҵ��������������
	unsigned short int service_server_num;		//ҵ����������� 
}Service_Server_Num;


//ѡ��ҵ����������
typedef struct
{
	pthread_mutex_t srv_seq_mutex;				//ѡ��ҵ��������Ļ������
	unsigned int nService_server_seq;			//ѡ��ҵ������������
}Service_Server_Seq;

//���͵���Ϣ����
typedef enum
{
	SND_REQUEST_MSG, 				//���͵�������Ϣ
	SND_RESPONSE_MSG				//���͵���Ӧ��Ϣ
}SND_MSG_TYPE;

//����ͻ��˵ķ�����Ϣ������Ϣ
typedef struct
{	
	BYTE bMsg_type;					//��Ϣ����
	BYTE bMain_service_code; 		//��ҵ�����
	BYTE bSub_service_code; 		//��ҵ�����
	WORD wAccess_srv_seq;
	unsigned int nMsg_counter;		//��Ϣ������
	BYTE arrAccess_check_code[4];	//�����������У����
	BYTE bService_version;			//ҵ��������İ汾��
	BYTE arrService_check_code[4];	//ҵ���������У����
	BYTE arrSn[YDT_SN_LENGTH + 1];	//��ϣ��ļ�ֵ(������ҵ�����, ��ҵ��������Ϣ������)
	unsigned int nClient_socket;	//����������Ϳͻ������ӵ�socket
	long lSent_time; 				//��ŷ��͸�ҵ�����������Ϣ��
	int nSeq;
}Client_Info;

//ҵ�������·����Ϣ
typedef struct
{
	pthread_mutex_t mutex; 
	BYTE bMain_code;								//��ҵ����
	BYTE bSub_code;									//��ҵ����	
	WORD wService_seq;//the ID of service server		//ҵ����������
	char arrService_srv_ip[100];					//ip
	WORD wSrv_port;									//�˿�
	int nSock;										//socket id
}Server_Info;

typedef struct
{
	BYTE bMain_code;								//��ҵ����
	BYTE bSub_code;									//��ҵ����	
	WORD wService_seq;//the ID of service server		//ҵ����������
	char arrService_srv_ip[100];					//ip
	WORD wSrv_port;									//�˿�
	int nSock;										//socket id
}StShmSrvInfo;


typedef struct
{
	pthread_mutex_t client_info_mutex;				//�ͻ�����Ϣ�������
	StHash_Table *pClient_info_hash;				//�ͻ�����Ϣ��ϣ��
}StClientInfo_Hashtable;

struct StCliInfo
{	
	StCliInfo(int iAccessId, int iCnt, int iCliSock)
	{
		snprintf(arrSn, sizeof(arrSn) - 1, "%d%d", iAccessId, iCnt);
		iSock = iCliSock;
	}
	char arrSn[40];
	int iSock;
};


//������Ϣ��ҵ�������
int SendSrvMsg(int nSock, void *pMsg, SND_MSG_TYPE snd_msg_type);

//������Ϣ��ҵ�������
void *SndMsgToServiceServerThread(void *);

int S_SndMsgToServer(void);
int SndDisposeResultToServiceSrv(int nSock, Forward_Srv_Client_Msg forward_srv_client_msg, BYTE bStatus);

int SndAfterConnSrvMsg(int nSock, BYTE bMain_service_code, BYTE bSub_service_code, WORD wService_srv_seq);

int InsertClientInfoIntoHash(char *pMsg);

int S_InsertClientInfoIntoHash(Client_Server_Msg *pClient_srv_msg);

#endif
