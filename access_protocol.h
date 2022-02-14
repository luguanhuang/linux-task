
#ifndef ACCESS_PROTOCOL_H_H
#define ACCESS_PROTOCOL_H_H

//#include "../util/access_global.h"
#include "../util/access_types.h"

/*************���ļ���Ҫ����������ͨѶ�ӿڵ�***************************/

//�ͻ��˵��������˵�����

//modify by LiHuangYuan,2011/10/14 modify struct pack
#pragma pack(1)

#define ACCOUNT_LEN 30				//�ʺų���

#define PASSWD_LEN 20				//���볤��

//�ͻ��˷��͵�����������Ϣ���ṹ
typedef struct
{
 	BYTE bVersion; 					//�汾��
	BYTE bMsg_type;					//��Ϣ����
	BYTE bMain_service_code; 		//��ҵ����
	BYTE bSub_service_code; 		//��ҵ����
	WORD wAccess_server_seq;		//������������
	unsigned int nMsg_counter; 		//��Ϣ������
	unsigned int nAccess_socket_id; //����������Ϳͻ���֮���socket		
	WORD wService_server_seq;		//ҵ����������
	unsigned int nData_len; 		//���ݳ���
	char *pData; 					//���ݲ���
	char  arrCheck_code[4]; 		//У����
}Client_Server_Msg;

//���������ͻ��˵���Ӧ��(һ���ҵ��, ���¼ҵ��)
typedef struct
{
 	BYTE bVersion; 					//�汾��
 	BYTE bMsg_type;					//��Ϣ����
 	BYTE bMain_service_code; 		//��ҵ�����
 	BYTE bSub_service_code; 		//��ҵ�����
 	WORD wAccess_server_seq; 		//������������
	unsigned  int nMsg_counter; 	//��Ϣ������
	unsigned int nData_len; 		//���ݳ���
	char *pData; 					//����
	char  arrCheckout_code[4]; 		//У����
}Server_Client_Msg;

//�����ݰ���Ҫ�Ǵ�������ҵ��(ҵ�������ת�����ͻ��˵����ݰ�)
typedef struct
{
	BYTE bVersion;							//�汾��
	BYTE bMsg_type;							//��Ϣ����
	BYTE bMain_service_code;				//��ҵ�����
	BYTE bSub_service_code;					//��ҵ�����
	WORD wSrc_access_srv_seq;				//ԭ������������
	unsigned int nAccess_Msg_counter;		//�������������Ϣ������
	WORD wDest_access_srv_seq;				//Ŀ�Ľ�����������
	unsigned int nAccess_socket_id; 		//�����������ͻ���֮���socketid
	WORD wService_server_seq;				//ҵ����������
	unsigned int nService_Msg_counter;		//ҵ���������Ϣ������
	unsigned int nData_len;					//���ݲ��ֳ���
	char *pData;							//���ݲ���
	char arrCheck_code[4];					//У����
}Forward_Srv_Client_Msg;

typedef struct
{
	BYTE bVersion;							//�汾��
	BYTE bMsg_type;							//��Ϣ����
	BYTE bMain_service_code;				//��ҵ�����
	BYTE bSub_service_code;					//��ҵ�����
	WORD wAccess_srv_seq;
	int nClient_sock;
	int nData_len;
	char *pData;
	char arrCheck_code[4];
}PushTo_Client_Msg;

typedef struct
{
	BYTE bVersion;							//�汾��
	BYTE bMsg_type;							//��Ϣ����
	BYTE bMain_service_code;				//��ҵ�����
	BYTE bSub_service_code;					//��ҵ�����
	WORD wAccess_srv_seq;
	int nClient_sock;
	WORD wService_srv_seq;
	int nData_len;
	char *pData;
	char arrCheck_code[4];
}PushToClient_Response_Msg;


typedef struct
{	
	BYTE bVersion;   				//�汾��
	BYTE bMsg_type; 				//��Ϣ����
	BYTE bMain_service_code;		//��ҵ����
	BYTE bSub_service_code; 		//��ҵ����
	WORD wAccess_server_seq;		//������������
	int nClient_socket;
	unsigned int nData_len; 		//���ݳ���
	char *pData;    				//���ݲ���
	BYTE arrCheck_code[4]; 			//У����
}StReq_DisconnWithClient_MSG;


//����ά�����ݰ���Ϣ��ʼ
//��ѯ��������Ϣ��Ϣ��ʼ
typedef struct
{
	BYTE bVersion;
	WORD wCmd_id;
}Req_Query_Srv_Info;

typedef struct
{
	BYTE bVersion;
	WORD wCmd_id;
	int nMsg_queue_len;
	int nClientsock_queue_len;
	int nRoutingtable_len;
	int nHeartdet_table_len;
	int nClientmsgbuf_table_len;
	int nClientmsginfo_table_len;
	int nSndclient_msgqueue_len;
	int nSndclient_sockhash_len;
	int nRcv_climsg_tmpsockhash_len;
}Res_Query_Srv_Info;
//��ѯ��������Ϣ��Ϣ����


//�޸��ռǼ�����Ϣ��ʼ
typedef struct
{
	BYTE bVersion;
	WORD wCmd_id;
	BYTE bLog_level;
}Req__Modify_Log_Level;

typedef struct
{
	BYTE bVersion;
	WORD wCmd_id;
	BYTE bModify_status;
}Res__Modify_Log_Level;


//�޸��ռǼ�����Ϣ��ʼ

//����ά�����ݰ���Ϣ����



//�����ݰ���Ҫ�ǰѷ��͸�
//�ͻ�����Ϣ�Ľ��ת����ҵ�������
typedef struct
{
	BYTE bVersion;				//�汾��
	BYTE bMsg_type;				//��Ϣ����
	WORD wService_server_seq;	//ҵ����������
	unsigned int nMsg_counter;	//��Ϣ������
	BYTE bTransfer_result;		//ת�����
	char arrCheck_code[4];
}Response_Client_Srv_Msg;

typedef struct
{
	BYTE bVersion;
	WORD wCmd_id;
	WORD wXML_len;
	BYTE *pXML_data;
	char arrCheck_code[4];
}Server_Error;


//�������������ҵ��������ɹ����͵ĵ�һ��ҵ���
typedef struct
{
	BYTE bVersion;				//�汾��
	WORD wCmd_id;				//����ID
	BYTE arrCheck_code[4];		//У����
}Req_AccessServer_Conn;

//modify by LiHuangYuan,2011/10/14 modify struct pack

int GetServerErrorPacket(Server_Client_Msg *pSrv_client_msg, void *pClient_info);
void FreeMsgBuffer(void *pMsg);

#pragma pack()
#endif
