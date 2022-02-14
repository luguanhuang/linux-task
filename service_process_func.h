
#ifndef SERVICE_PROCESS_FUNC
#define SERVICE_PROCESS_FUNC

#include "../util/service_global.h"
#include "../util/service_types.h"

typedef int (*PProcessFunc)(void *, StSequence_Queue *, pthread_mutex_t *, pthread_cond_t *);

typedef struct
{
	WORD wCmd_id;
	PProcessFunc pProcessFunc;
}StProcFunc_MapTable;

#include "service_wanwei_processfunc.h"


#define UUID_ISNOT_EXIST_ERROR		0x07			//UUID�����ڴ���

#define CLIENT_SERVER_MSG 0x01			//�ͻ��˵�����������Ϣ����
#define SERVER_CLIENT_MSG 0x02 			//���������ͻ��˵���Ϣ����
#define FORWARD_SRV_CLIENT_MSG 0x03		//��ת��Ϣ
#define PUSHTO_CLIENT_RESPONSE 0x0B 

#define NOTIFY_DISCONNECT_WITHCLIENT_MSG 0x08
#define PUSHTO_CLIENT_MSG				 0x0A

//�������Ϣ���ͺ�
#define APPLY_TATTEDCODE_MSG 0x01		//�����������Ϣ����
#define LOGIN_MSG 0x02					//��¼��Ϣ����
#define FRI_GROUP_MSG 0x03				//����������б���Ϣ����
#define FRI_LIST_MSG 0x04				//��������б���Ϣ����
#define USRID_REGISTER_MSG 0x05	   //�û�IDע����Ϣ����
#define CHAT_MSG 0x07					//������Ϣ����
#define ACC_REGISTER_MSG 0x08		//ע��(�ʺ�ע��)��Ϣ����
#define ADD_FRIEND_GROUP_MSG 0x09	//��Ӻ�������Ϣ����
#define DEL_FRIEND_GROUP_MSG 0x0A	//ɾ����������Ϣ����
#define ADD_FRINED_MSG		 0x0B	//��Ӻ�����Ϣ����
#define DEL_FRIEND_MSG		 0x0C  //ɾ��������Ϣ����
#define CREATE_CLUSTER		0x10	//����Ⱥ
#define CLUSTER_LIST		0x11    //����Ⱥ�б�
#define CLUSTER_MEMBER_LIST 0x12	//����Ⱥ��Ա�б�����
#define INVITE_JOIN_CLUSTER 0x13	//������Ѽ���Ⱥ��Ϣ����
#define CHANGE_IMAGE 		0x14	//�ı�ͷ����Ϣ����
#define USRID_LOGIN_MSG		0x19	//�û�ID��¼��Ϣ
#define ADD_CLUSTER_MEMBER		0x21	//���Ⱥ��Ա��Ϣ����
#define DELETE_CLUSTER_MEMBER   0x22	//ɾ��Ⱥ��Ա
#define DISMISS_CLUSTER			0x23	//��ɢȺ
#define EXIT_CLUSTER			  0x25	//Ⱥ��Ա��Ⱥ
#define HEARTBEAT_DETECT		  0x28	//���������
#define APPLICATION_EXIT 0x2A //�����˳�

//��ͨ����Ϣ���ͺ궨��
#define ANDROID_REGISTER_MSG 		0x100			//androidע����Ϣ����
#define IPHONE_REGISTER_MSG 		0x101			//�û�ע����Ϣ����
#define REPORT_ANDROID_PHONEINFO	0x102			//����android�ֻ���Ϣ
#define REPORT_IPHONE_PHONEINFO		0x103			//����IPHONE�ֻ���Ϣ
#define PHONE_LOGIN_MSG				0x104			//�ֻ��û���¼��Ϣ����
#define CHECK_ANDROID_VERSION    	0x105			//(android)�ͻ��˰汾�������
#define REPORT_VERSION_UPDATE		0x106			//�����������
#define TASKS_SEND_QUERY			0x107			//�������Ͳ�ѯ
#define DEL_TASK_RECORD				0x108			//ɾ�������¼
#define CLIENT_FEEDBACK				0x109			//�ͻ���Ϣ����
#define TASK_TOUCH_RECORD			0x10A			//��������¼
#define SERVER_ERROR				0x10B			// ����������
#define GET_CHARGE_TYPE 			0x10C			//��ȡͶ������
#define RECORD_CHARGE_INFO			0x10D			//��¼Ͷ����Ϣ		
#define HEARTBEAT_DEC_PACKET		0x10E			//��������
#define GET_IMAGE					0x1FF
#define REPORTIPHONE_MOREINFO		0x110			//����iphone�ֻ��ȶ���Ϣ
#define BIND_UUID_SOCKETID			0x111			//��socketID
#define NEW_TASK_PUSH				0x112
#define TAG_CLICK_RECORD			0x113
#define PUSHMSG_TYPE				0x114
#define USRSHARE_CLICKRECORD		0x115
#define RECORD_MALFUNC_REPORT 		0x116
#define REQ_HALL_INFO		 		0x117


int YDTProcessFunc(WORD wCmd_id, Client_Server_Msg *pClient_srv_msg, StSequence_Queue *pQueue, pthread_mutex_t *pMutex, pthread_cond_t *pCond);
int LTProcessFunc(WORD wCmd_id, Client_Server_Msg *pClient_srv_msg, StSequence_Queue *pQueue, pthread_mutex_t *pMutex, pthread_cond_t *pCond);
int DefProcessFunc(WORD wCmd_id, Client_Server_Msg *pClient_srv_msg, StSequence_Queue *pQueue, pthread_mutex_t *pMutex, pthread_cond_t *pCond);

#endif
