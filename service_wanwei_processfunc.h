
#ifndef SERVICE_WANWEI_PROCESSFUNC
#define SERVICE_WANWEI_PROCESSFUNC

#include "../util/service_global.h"
#include "../util/service_types.h"


//��άҵ��ĺ궨��
#define WANWEI_SERVICE_CODE 0x03
#define WANWEI_REPORTGPS_SUB_SERVICECODE 0x02
#define WANWEI_QUERYPUSH_SUB_SERVICECODE 0x03

//��άҵ������ID
#define WANWEI_PHONE_LOGIN			0x200
#define TASK_PUSH_QUERY				0X201
#define DEL_PUSH_TASK	 0x202			//ɾ����������	
#define WANWEI_ACTIVITY_QUERY		0x203
#define WANWEI_REPORT_GPS_INFO  			0x204
#define WANWEI_DISCONN_WITHCLIENT	0x205
#define WANWEI_INTEREST_ACTIVITY_QUERY		0x207
#define WANWEI_REPORT_INFO					0x208
#define WANWEI_REPORT_BUSI_INFO				0x209
#define WANWEI_PUSH_USR_INFO				0x20A
#define WANWEI_CONFIRM_SIGNIN				0x20B
#define WANWEI_PUSH_BUSI_INFO				0x20C

#define AFTER_ACEESS_SRV_CONN 0x50	//�������������ҵ�������ʱ�ϱ���ŵ���Ϣ����

int WanweiProcessFunc(WORD wCmd_id, Client_Server_Msg *pClient_srv_msg, StSequence_Queue *pQueue, pthread_mutex_t *pMutex, pthread_cond_t *pCond);

#endif
