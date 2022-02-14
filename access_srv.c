

#include "../util/access_global.h"
#include "../interface/access_protocol.h"
#include "../util/access_operate_conf.h"
#include "../communication/access_heartbeat_detectthread.h"
#include "../communication/access_communication.h"
#include "../util/access_init_var.h"
#include "../communication/access_routing_maintain.h"
#include "access_srv.h"

/*该文件是程序的主函数,  主要是用来启动服务器的
 */


/*存放业务服务器连接信息的哈希表
  *键为(主业务码 , 子业务码 , 业务服务器的序号), 值为业务路由表信息, 通过找到业务服务器的信息
  *才可以把消息包发送给对应的业务服务器
  */

int g_iSndCliQid = 0; 
int g_iQid = 0; 
int g_iShmId = 0;
int g_iCliInfoId = 0;
int g_iSemId = 0;


extern StRouting_Table g_routing_table;

extern StHeartbeat_Detect_Table g_heartbeatdetect_table;
extern StTmp_clientSock_Hash g_tmp_clientsock_hash;

//存放客户端消息包的消息队列
extern StMsg_Queue g_msg_queue;

extern StClientInfo_Hashtable g_clientinfo_hash;

extern Server_Conf_Info g_srv_conf_info;

extern StMsgBuf_HashTable g_msgbuf_hashtable;

//函数用途: 释放程序启动时动态分配的内存函数
//输入参数:无
//输出参数: 无
//返回值	:  无
void ReleaseBuffer(void)
{
	INFO("ReleaseBuffer: func begin%s", "");
	HashFree(&g_tmp_clientsock_hash.pTmp_clientsock_table, FreeFunc);		//释放内存

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
	int iRet = InitSem(g_iSemId, 1);//初始值为1
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

//函数用途: 主函数(用来启动整个程序的)
//输入参数:无
//输出参数: 无
//返回值	:  启动成功,  返回TRUE,  启动失败,   返回FALSE;

int main(void)
{
	TRACE();
	signal(SIGCHLD, SIG_IGN);
	/*防止服务器意外崩溃
	*(目的是防止客户端连续发送两次的写信息而导致服务器收到
	*一个ASY信息而退出) 
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
	//产生随机数 
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
	//初始化整个工程用到的变量
	if (FALSE == InitVarInfo())
	{
		ERROR("main: Call InitVarInfo error%s", "");
		pthread_exit(NULL);
		return FALSE;
	}

	#if 1
	//读取配置文件，插入哈希表中
	if (FALSE == ReadConfFile())
	{
		ERROR("main: Call ReadConfFile error%s", "");
		ReleaseBuffer();
		pthread_exit(NULL);
		return FALSE;
	}
	#endif

	//启动接入服务器
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


