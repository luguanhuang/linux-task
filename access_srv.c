

#include "../util/access_global.h"
#include "../interface/access_protocol.h"
#include "../util/access_operate_conf.h"
#include "../communication/access_heartbeat_detectthread.h"
#include "../communication/access_communication.h"
#include "../util/access_init_var.h"
#include "../communication/access_routing_maintain.h"
#include "access_srv.h"

/*���ļ��ǳ����������,  ��Ҫ������������������
 */


/*���ҵ�������������Ϣ�Ĺ�ϣ��
  *��Ϊ(��ҵ���� , ��ҵ���� , ҵ������������), ֵΪҵ��·�ɱ���Ϣ, ͨ���ҵ�ҵ�����������Ϣ
  *�ſ��԰���Ϣ�����͸���Ӧ��ҵ�������
  */

int g_iSndCliQid = 0; 
int g_iQid = 0; 
int g_iShmId = 0;
int g_iCliInfoId = 0;
int g_iSemId = 0;


extern StRouting_Table g_routing_table;

extern StHeartbeat_Detect_Table g_heartbeatdetect_table;
extern StTmp_clientSock_Hash g_tmp_clientsock_hash;

//��ſͻ�����Ϣ������Ϣ����
extern StMsg_Queue g_msg_queue;

extern StClientInfo_Hashtable g_clientinfo_hash;

extern Server_Conf_Info g_srv_conf_info;

extern StMsgBuf_HashTable g_msgbuf_hashtable;

//������;: �ͷų�������ʱ��̬������ڴ溯��
//�������:��
//�������: ��
//����ֵ	:  ��
void ReleaseBuffer(void)
{
	INFO("ReleaseBuffer: func begin%s", "");
	HashFree(&g_tmp_clientsock_hash.pTmp_clientsock_table, FreeFunc);		//�ͷ��ڴ�

	HashFree(&g_clientinfo_hash.pClient_info_hash, FreeFunc);
	HashFree(&g_routing_table.pRouting_table, FreeFunc);

	HashFree(&g_msgbuf_hashtable.pMsg_buf_hashtable, FreeFunc);

	HashFree(&g_heartbeatdetect_table.pHeartbeat_detect_table, FreeFunc);
	DestroySeqQueue(&g_msg_queue.msg_queue);

	INFO("ReleaseBuffer: func end%s", "");
}

union UnSemNum
{
	int iVal;
};


int InitSem(int iSemId, int iVal)
{
	INFO("InitSem: func begin");
	union UnSemNum unSemNum;
	unSemNum.iVal = iVal;
	if (-1 == semctl(iSemId, 0, SETVAL,unSemNum))
	{
		ERROR("InitSem: Call semctl error");
		return -1;
	}
	
	INFO("InitSem: func end");
	return 0;
}

int SemP(int iSemId)
{
	struct sembuf semBuf;
	semBuf.sem_num = 0;
	semBuf.sem_op = -1;
	semBuf.sem_flg = SEM_UNDO;
	if (-1 == semop(iSemId, &semBuf, 1))
	{
		ERROR("SemP: Call semop error");
		return -1;
	}

	return 0;
}

int SemV(int iSemId)
{
	struct sembuf semBuf;
	semBuf.sem_num = 0;
	semBuf.sem_op = 1;
	semBuf.sem_flg = SEM_UNDO;
	if (-1 == semop(iSemId, &semBuf, 1))
	{
		ERROR("SemV: Call semop error%s", "");
		return -1;
	}

	return 0;
}

void InitShm(void)
{
	INFO("InitShm: func begin");
	g_iSemId = semget(ftok("/home", 2), 1, 0666 | IPC_CREAT);
	INFO("InitShm: [sem id]=%d", g_iSemId);
	int iRet = InitSem(g_iSemId, 1);//��ʼֵΪ1
	if (iRet < 0)
	{
		ERROR("InitShm: Call InitSem error");
		return;
	}

	g_iShmId = shmget(ftok("/opt", 2), 1000, 0666 | IPC_CREAT);
	if (g_iShmId < 0)
	{
		ERROR("InitShm: Call******* shmget error error[%d]=%s", \
			errno, strerror(errno));
		return;
	}
	else
	{
		INFO("InitShm: [shm id]=%d", g_iShmId);	
	}
	
	g_iCliInfoId = shmget(ftok("/home", 2), 1000, 0666 | IPC_CREAT);
	if (g_iCliInfoId < 0)
	{
		ERROR("InitShm: Call shmget error error[%d]=%s", \
			errno, strerror(errno));
		return;
	}
	else
	{
		INFO("InitShm: [client info shmid]=%d", g_iCliInfoId);	
	}

	int *pSize = (int *)shmat(g_iCliInfoId, NULL, 0);
	*pSize = -1;

	INFO("InitShm: func end");
}


int InitMsgQueue(void)
{
	TRACE();
	key_t key = 113;
	g_iQid = msgget(key, IPC_CREAT | 0666);
	DEBUG("InitMsgQueue: [qid]=%d", g_iQid);
	if (g_iQid < 0)
	{
		ERROR("InitMsgQueue: Call msgget error[%d]=%s", errno, strerror(errno));
		return 0;
	} 

	key = 110;
	g_iSndCliQid = msgget(key, IPC_CREAT | 0666);
	DEBUG("InitMsgQueue: [qid]=%d", g_iQid);
	if (g_iSndCliQid < 0)
	{
		ERROR("InitMsgQueue: Call msgget error[%d]=%s", errno, strerror(errno));
		return 0;
	} 
	
	return 1;
}

//������;: ������(�����������������)
//�������:��
//�������: ��
//����ֵ	:  �����ɹ�,  ����TRUE,  ����ʧ��,   ����FALSE;

int main(void)
{
	TRACE();
	signal(SIGCHLD, SIG_IGN);
	/*��ֹ�������������
	*(Ŀ���Ƿ�ֹ�ͻ��������������ε�д��Ϣ�����·������յ�
	*һ��ASY��Ϣ���˳�) 
	*/

	#if 1
	SetLogLevel(FIVE_LEVEL_LOG);
	#endif

	
	  /* Seed the random-number generator with GetTickCount so that
		 the numbers will be different every time we run.
		  */
	srand(time(NULL));
	InitShm();
	InitMsgQueue();
	
	INFO("main: func begin%s", "");
	signal(SIGPIPE,SIG_IGN);

	MY_INIT();
	int nRet  = 0;
	//��������� 
	srand((unsigned int)time(0));
	nRet = ReadServerConfFile();
	if (FALSE == nRet)
	{
		ERROR("main: Call ReadServerConfFile error%s", "");
		return FALSE;
	}

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	BYTE bLog_print_level = g_srv_conf_info.bLog_print_level;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	DEBUG("main: log_print_level=%d", bLog_print_level);
	SetLogLevel((EnLevel)bLog_print_level);
	//��ʼ�����������õ��ı���
	if (FALSE == InitVarInfo())
	{
		ERROR("main: Call InitVarInfo error%s", "");
		pthread_exit(NULL);
		return FALSE;
	}

	#if 1
	//��ȡ�����ļ��������ϣ����
	if (FALSE == ReadConfFile())
	{
		ERROR("main: Call ReadConfFile error%s", "");
		ReleaseBuffer();
		pthread_exit(NULL);
		return FALSE;
	}
	#endif

	//�������������
	if (FALSE == StartServer())
	{
		ERROR("main: Call StartServer error%s", "");
		ReleaseBuffer();
		pthread_exit(NULL);
		return FALSE;
	}

	ReleaseBuffer();

	INFO("main: func end%s", "");
	return TRUE;
}


