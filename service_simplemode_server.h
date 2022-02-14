
#ifndef SERVICE_SIMPLEMODE_SERVER
#define SERVICE_SIMPLEMODE_SERVER

#define MAX_ACCESS_SRV_NUM 400

#define MAX_THREAD_SIZE 2048					//�����߳���(�������ͺͽ����߳�)


//���ͺͽ����̵߳�ID�ṹ
typedef struct
{
	pthread_t snd_id;					//�����̵߳�ID
	pthread_t recv_id;					//�����̵߳�ID
}Threads_Id;

typedef struct
{
	Threads_Id arrThreads_id[MAX_ACCESS_SRV_NUM];
	int nID_counter;
	pthread_mutex_t id_mutex; 
}StThread_Ids_Info;

typedef struct
{
	pthread_t thread_id; 
	BYTE bIsproc_thread_exit;
}StThread_Exit;

typedef struct
{
	pthread_mutex_t thread_mutex;
	int nThread_num;
	StThread_Exit arrThread_exit[1000];
}StThreads_Exit_Info;

//���ͺͽ����̵߳Ĳ����ṹ
typedef struct
{
	int nSock;									//socket id
	StThreads_Exit_Info threads_exit_info;
	StSequence_Queue queue;							//��Ϣ����
	pthread_mutex_t mutex;						//�������
	pthread_cond_t cond;						//ͬ������
}Snd_Recv_Param;

//�������������߳���Ϣ�Ľṹ
typedef struct
{
	Threads_Id arrThreads_id[MAX_THREAD_SIZE / 2];  	 //����߳�ID������
	Snd_Recv_Param *pKeep_param[MAX_THREAD_SIZE/2];		//���ͽ����̲߳���
	unsigned int nID_counter;			  				//�̼߳�����
	pthread_mutex_t thread_mutex;					 	//���߳���Ϣ����
}Threads_Info;

//�Ե�ǰ�����߳��������м���
typedef struct
{
	pthread_mutex_t counter_mutex;			//�������������
	unsigned int nThread_counter;			//�̼߳�����
}Process_Thread_Counter;

int RunSimpleModeServer(void);

void SetCancelThreadFlg(void);

#endif
