
#ifndef SERVICE_COMMUNICATION_H
#define SERVICE_COMMUNICATION_H

//���Ĵ����߳�����
#define MAX_PROCESS_THREAD_NUM  400

/********�ṹ���忪ʼ********************/

//���ͺͽ����̵߳�����
typedef enum 
{
	SND_THREAD_TYPE,			//�����߳�����
	RECV_THREAD_TYPE			//�����߳�����
}Thread_Type;

#pragma pack(1)
//�����̵߳Ĳ����ṹ
typedef struct
{
	int nSeq;
	StSequence_Queue *pQueue;				//��Ϣ����
	StThreads_Exit_Info *pThreads_exit_info;	//�߳��˳���Ϣ
	Client_Server_Msg cli_srv_msg;		//��Ϣ�ṹ
	pthread_mutex_t *pProc_mutex;		//�������
	pthread_cond_t *pCond;				//ͬ������
}Process_Param;
#pragma pack()


/**********�ṹ�������*******************/

//����������
int StartServer(void);

#endif
