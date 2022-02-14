
#include "access_global.h"
//#include "../include/ydt_log.h"
#include "../interface/access_protocol.h"
#include "../communication/access_heartbeat_detectthread.h"
#include "../communication/access_routing_maintain.h"
#include "../communication/access_comm_client_snd.h"
#include "access_init_var.h"

/******���ļ���Ҫ��������ʼ�����������õ���ȫ�ֱ�����************/

//���ֲ�ͬ���͵�ҵ���������������
extern Service_Server_Num g_login_srv_num;
extern Service_Server_Num g_register_srv_num;;
extern Service_Server_Num g_simplechat_srv_num;
extern Service_Server_Num g_clusterchat_srv_num;
extern Service_Server_Num g_liantong_srv_num;				//��ͨר�÷���������
extern Service_Server_Num g_wanwei_loginsrv_num;			//��ά��¼����������

extern int g_iShmId;

//Ŀǰ�����������ѡ���ҵ������������
//ѡ��ҵ������������

Service_Server_Seq g_login_srv_seq;					//��¼��������� 
Service_Server_Seq g_register_srv_seq;				//ע����������
Service_Server_Seq g_simplechat_srv_seq;			//���ķ��������
Service_Server_Seq g_clusterchat_srv_seq;			//Ⱥ�ķ��������
Service_Server_Seq g_liantong_srv_seq;				//��ͨר�÷��������
Service_Server_Seq g_wanwei_loginsrv_seq;			//��ά��¼���������
Service_Server_Seq g_wanwei_report_gps_srvseq;			
Service_Server_Seq g_wanwei_querypush_srvseq;			

//����ҵ�����������(ҵ������������ж�̨)
Service_Server_Num g_login_srv_num;					//��¼����������
Service_Server_Num g_register_srv_num;				//ע�����������
Service_Server_Num g_simplechat_srv_num;			//���ķ���������
Service_Server_Num g_clusterchat_srv_num;			//Ⱥ�ķ���������
Service_Server_Num g_liantong_srv_num;				//��ͨר�÷���������
Service_Server_Num g_wanwei_loginsrv_num;			//��ά��¼����������
Service_Server_Num g_wanwei_report_gps_srvnum;		
Service_Server_Num g_wanwei_querypush_srvnum;		

StRouting_Table g_routing_table;
StMsg_Queue g_msg_queue;
StClientInfo_Hashtable g_clientinfo_hash;
StHeartbeat_Detect_Table g_heartbeatdetect_table;
StSnd_Client_MsgQueue g_sndclient_msgqueue;
StTmp_clientSock_Hash g_tmp_clientsock_hash;
StTmp_clientSock_Hash g_sndclient_tmp_sockhash;
StClient_Sock_Queue g_clientsocket_queue;
extern Server_Conf_Info g_srv_conf_info;

//epoll�ṹ
extern Epoll_Fd g_epoll_fd;

//��Ϣ������
extern Msg_Counter g_msg_counter;

//����ά���̱߳�־
extern Awake_Thread_Flg g_awake_thread_flg;

//���տͻ�����Ϣ�Ļ����(��ϣ��)

StMsgBuf_HashTable g_msgbuf_hashtable;

int InitTmpClientSockHash(void)
{
	INFO("InitTmpClientSockHash: func begin%s", "");
	if (pthread_mutex_init(&g_tmp_clientsock_hash.hash_mutex, NULL) != 0)
	{
		ERROR("InitTmpClientSockHash: Call pthread_mutex_init error%s", "");
		return FALSE;
	}

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	int nTmp_clientsock_hash_size = g_srv_conf_info.nTmp_clientsock_hash_size;
	DEBUG("InitTmpClientSockHash: [temp clientsock hash size]=%d", nTmp_clientsock_hash_size);
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);
	if (FALSE == HashInit(&g_tmp_clientsock_hash.pTmp_clientsock_table, nTmp_clientsock_hash_size, NULL))
	{
		ERROR("InitTmpClientSockHash: Call YDT_Hash_Init error%s", "");
		return FALSE;
	}
	

	INFO("InitTmpClientSockHash: func end%s", "");
	return TRUE;
}


//������;: ��ʼ����������ϣ�����(����socket id,   ֵ��ʱ��,  �ñ���Ҫ��Ϊ�˷�ֹ�ͻ����������)
//�������: ��
//�������: �� 
//����ֵ	: ��ʼ���ɹ�,  ����TRUE�� ��ʼ��ʧ��,  ����FALSE;

int InitHeartbeatDetectHashtable(int nHashtable_size)
{
	INFO("InitHeartbeatDetectHashtable: func begin%s", "");
	if (pthread_mutex_init(&g_heartbeatdetect_table.heartbeat_detect_mutex, NULL) != 0)
	{
		ERROR("InitHeartbeatDetectHashtable: Call pthread_mutex_init error%s", "");
		return FALSE;
	}

	INFO("InitHeartbeatDetectHashtable: heart beat hashtable size=%d", nHashtable_size);
	if (TRUE != HashInit(&g_heartbeatdetect_table.pHeartbeat_detect_table, nHashtable_size, NULL))
	{
		ERROR("InitHeartbeatDetectHashtable: Call YDT_Hash_Init error%s", "");
		return FALSE;
	}

	INFO("InitHeartbeatDetectHashtable: func end%s", "");
	return TRUE;
}

int InitClientMsgQueue(void)
{
	INFO("InitClientMsgQueue: func begin%s", "");
	if (pthread_mutex_init(&g_sndclient_msgqueue.queue_mutex, NULL) != 0)
	{
		ERROR("InitClientMsgQueue: Call pthread_mutex_init error%s", "");
		return FALSE;
	}

	InitLinkQueue(&g_sndclient_msgqueue.stLink_queue);
	
	if (pthread_cond_init(&g_sndclient_msgqueue.queue_cond, NULL) != 0)
	{
		ERROR("InitClientMsgQueue: Call pthread_cond_init error%s", "");
		return FALSE;
	}


	INFO("InitClientMsgQueue: func end%s", "");
	return TRUE;
}

int InitSndClientTmpSockHash(void)
{
	INFO("InitSndClientTmpSockHash: func begin%s", "");
	if (pthread_mutex_init(&g_sndclient_tmp_sockhash.hash_mutex, NULL) != 0)
	{
		ERROR("InitSndClientTmpSockHash: Call pthread_mutex_init error%s", "");
		return FALSE;
	}

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	int nTmp_clientsock_hash_size = g_srv_conf_info.nTmp_clientsock_hash_size;
	DEBUG("InitSndClientTmpSockHash: [temp clientsock hash size]=%d", nTmp_clientsock_hash_size);
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	if (FALSE == HashInit(&g_sndclient_tmp_sockhash.pTmp_clientsock_table, nTmp_clientsock_hash_size, NULL))
	{
		ERROR("InitSndClientTmpSockHash: Call YDT_Hash_Init error%s", "");
		return FALSE;
	}

	
	INFO("InitSndClientTmpSockHash: func end%s", "");
	return TRUE;
}

//������;: ��ʼ����Ž����������ͻ���֮���socket����Ϣ����
//�������: ��
//�������: �� 
//����ֵ	: ��ʼ���ɹ�,  ����TRUE�� ��ʼ��ʧ��,  ����FALSE;

int InitClientSockQueueInfo(void)
{
	INFO("InitClientSockQueueInfo: func begin%s", "");
	if (pthread_mutex_init(&g_clientsocket_queue.queue_mutex, NULL) != 0)
	{
		ERROR("InitClientSockQueueInfo: Call pthread_mutex_init error%s", "");
		return FALSE;
	}
	
	InitLinkQueue(&g_clientsocket_queue.stLink_queue);
	if (pthread_cond_init(&g_clientsocket_queue.queue_cond, NULL) != 0)
	{
		ERROR("InitClientSockQueueInfo: Call pthread_cond_init error%s", "");
		return FALSE;
	}

	INFO("InitClientSockQueueInfo: func end%s", "");
	return TRUE;
}

int InitMsgBufHashInfo(int nClient_msgbuf_tablesize)
{
	INFO("InitMsgBufHashInfo: func begin%s", "");
	if (pthread_mutex_init(&g_msgbuf_hashtable.msg_buf_mutex, NULL) != 0)
	{
		ERROR("InitMsgBufInfo: Call pthread_mutex_init error%s", "");
		return FALSE;
	}

	DEBUG("InitMsgBufHashInfo: client msg buffer hashtable size=%d", nClient_msgbuf_tablesize);

	if (FALSE == HashInit(&g_msgbuf_hashtable.pMsg_buf_hashtable, nClient_msgbuf_tablesize, NULL))
	{
		ERROR("InitMsgBufInfo: Call YDT_Hash_Init error%s", "");
		return FALSE;
	}

	INFO("InitMsgBufHashInfo: func end%s", "");
	return TRUE;
}


//������;: ��ʼ�������̱߳�־��Ϣ
//�������: ��
//�������: �� 
//����ֵ	: ��ʼ���ɹ�,  ����TRUE�� ��ʼ��ʧ��,  ����FALSE;

int InitAwakeThreadFlgInfo(void)
{
	INFO("InitAwakeThreadFlgInfo: func begin%s", "");
	int nRet = pthread_mutex_init(&g_awake_thread_flg.awake_flg_mutex, NULL);
	if (nRet != 0)
	{
		ERROR("InitAwakeThreadFlgInfo: Call pthread_mutex_init error%s", "");
		return FALSE;
	}
	else
	{
		g_awake_thread_flg.nAwake_thread_flg = TRUE;
	}

	INFO("InitAwakeThreadFlgInfo: func end%s", "");
	return TRUE;
}

//������;: ��ʼ���û���Ϣ��ϣ�����(��������ͻ��˷����������ݰ���Ϣ)
//�������: ��
//�������: �� 
//����ֵ	: ��ʼ���ɹ�,  ����TRUE�� ��ʼ��ʧ��,  ����FALSE

int InitClientInfoHash(int nHashtable_size)
{
	INFO("InitClientInfoHash: func begin%s", "");
	if (pthread_mutex_init(&g_clientinfo_hash.client_info_mutex, NULL) != 0)
	{
		ERROR("InitClientInfoHash: Call pthread_mutex_init error%s", "");
		return FALSE;
	}

	DEBUG("InitClientInfoHash: [client msg info hash table size]=%d", nHashtable_size);

	if(TRUE != HashInit(&g_clientinfo_hash.pClient_info_hash, nHashtable_size, NULL))
	{
		ERROR("InitClientInfoHash: Call YDT_Hash_Init error%s", "");
		return FALSE;
	}

	INFO("InitClientInfoHash: func end%s", "");
	return TRUE;
	
}

//������;:��ʼ����Ϣ������Ϣ(��ſͻ�����Ϣ����Ϣ����)
//�������: ��
//�������: �� 
//����ֵ	: ��ʼ���ɹ�,  ����TRUE�� ��ʼ��ʧ��,  ����FALSE

int InitQueueInfo(int nQueue_size)
{
	INFO("InitSQueueInfo: func begin%s", "");
	if (pthread_mutex_init(&g_msg_queue.queue_mutex, NULL) != 0)
	{
		ERROR("InitSQueueInfo: Call pthread_mutex_init error%s", "");
		return FALSE;
	}

	DEBUG("InitSQueueInfo: [clieng msg queue size]=%d", nQueue_size);
	if (FALSE == InitSeqQueue(&g_msg_queue.msg_queue, nQueue_size, FreeMsgBuffer))
	{
		ERROR("InitSQueueInfo: Call YDT_InitQueue error%s", "");
		return FALSE;
	}

	if (pthread_cond_init(&g_msg_queue.queue_cond, NULL) != 0)
	{
		ERROR("InitSQueueInfo: Call pthread_cond_init error%s", "");
		return FALSE;
	}

	INFO("InitSQueueInfo: func end%s", "");
	return TRUE;
}

static int InitWanweiSrvSelectInfo(void)
{
	INFO("InitWanweiSrvSelectInfo: func begin%s", "");
	int nRet = 0;

	nRet = pthread_mutex_init(&g_wanwei_loginsrv_seq.srv_seq_mutex, NULL);
	if (nRet != 0)
	{
		ERROR("InitWanweiSrvSelectInfo: call pthread_mutex_init error%s", "");
		return FALSE;	
	}
	else
	{
		g_wanwei_loginsrv_seq.nService_server_seq = 0;
	}

	nRet = pthread_mutex_init(&g_wanwei_report_gps_srvseq.srv_seq_mutex, NULL);
	if (nRet != 0)
	{
		ERROR("InitWanweiSrvSelectInfo: call pthread_mutex_init error%s", "");
		return FALSE;	
	}
	else
	{
		g_wanwei_report_gps_srvseq.nService_server_seq = 0;
	}

	nRet = pthread_mutex_init(&g_wanwei_querypush_srvseq.srv_seq_mutex, NULL);
	if (nRet != 0)
	{
		ERROR("InitWanweiSrvSelectInfo: call pthread_mutex_init error%s", "");
		return FALSE;	
	}
	else
	{
		g_wanwei_querypush_srvseq.nService_server_seq = 0;
	}
	
	INFO("InitWanweiSrvSelectInfo: func end%s", "");
	return TRUE;
}

int InitImSrvSelectInfo(void)
{
	INFO("InitImSrvSelectInfo: func begin%s", "");
	int nRet = 0;

	nRet = pthread_mutex_init(&g_login_srv_seq.srv_seq_mutex, NULL);
	//��ʼ�� ��¼������ѡ����Ϣ
	if (0 != nRet)
	{
		ERROR("InitImSrvSelectInfo: call pthread_mutex_init error%s", "");
		return FALSE;
	}
	else
	{
		g_login_srv_seq.nService_server_seq = 0;
	}

	nRet = pthread_mutex_init(&g_register_srv_seq.srv_seq_mutex, NULL);
	//��ʼ��ע�������ѡ����Ϣ
	if (0 != nRet)
	{
		ERROR("InitImSrvSelectInfo: call pthread_mutex_init error%s", "");
		return FALSE;
	}
	else
	{
		g_register_srv_seq.nService_server_seq = 0;
	}

	nRet = pthread_mutex_init(&g_simplechat_srv_seq.srv_seq_mutex, NULL);
	//��ʼ�����ķ�����ѡ����Ϣ
	if (0 != nRet)
	{
		ERROR("InitImSrvSelectInfo: call pthread_mutex_init error%s", "");
		return FALSE;
	}
	else
	{
		g_simplechat_srv_seq.nService_server_seq = 0;
	}

	//��ʼ��Ⱥ�ķ�����ѡ����Ϣ
	nRet = pthread_mutex_init(&g_clusterchat_srv_seq.srv_seq_mutex, NULL);
	if (0 != nRet)
	{
		ERROR("InitImSrvSelectInfo: call pthread_mutex_init error%s", "");
		return FALSE;
	}
	else
	{
		g_clusterchat_srv_seq.nService_server_seq = 0;	
	}
	INFO("InitImSrvSelectInfo: func end%s", "");
	return TRUE;
}



//������;:��ʼ��ҵ�������ѡ����Ϣ(ѡ���ҵ����������)
//�������: ��
//�������: �� 
//����ֵ	: ��ʼ���ɹ�,  ����TRUE�� ��ʼ��ʧ��,  ����FALSE

int InitServiceSrvSelectInfo(void)
{
	INFO("InitServiceSrvSelectInfo: func begin%s", "");
	int nRet = 0;

	nRet = InitImSrvSelectInfo(); 
	if (TRUE != nRet)
	{
		ERROR("InitServiceSrvSelectInfo: Call InitImSrvSelectInfo error%s", "");
		return FALSE;
	}
	
	nRet = pthread_mutex_init(&g_liantong_srv_seq.srv_seq_mutex, NULL);
	if (nRet != 0)
	{
		ERROR("InitServiceSrvSelectInfo: call pthread_mutex_init error%s", "");
		return FALSE;	
	}
	else
	{
		g_liantong_srv_seq.nService_server_seq = 0;
	}

	nRet = InitWanweiSrvSelectInfo();
	if (TRUE != nRet)
	{
		ERROR("InitServiceSrvSelectInfo: Call InitWanweiLoginSrvSelectInfo error%s", "");
		return FALSE;
	}

	INFO("InitServiceSrvSelectInfo: func end%s", "");
	return TRUE;
}

//��ʼ����ά��¼������������Ϣ
int InitWanweiSrvNumInfo(void)
{
	INFO("InitWanweiSrvNumInfo: func begin%s", "");
	int nRet = 0;
	nRet = pthread_mutex_init(&g_wanwei_loginsrv_num.num_mutex, NULL);
	if (nRet != 0)
	{
		ERROR("InitWanweiSrvNumInfo: call pthread_mutex_init error%s", "");
		return FALSE;	
	}
	else
	{
		g_wanwei_loginsrv_num.service_server_num = 0;
	}

	nRet = pthread_mutex_init(&g_wanwei_report_gps_srvnum.num_mutex, NULL);
	if (nRet != 0)
	{
		ERROR("InitWanweiSrvNumInfo: call pthread_mutex_init error%s", "");
		return FALSE;	
	}
	else
	{
		g_wanwei_report_gps_srvnum.service_server_num = 0;
	}

	nRet = pthread_mutex_init(&g_wanwei_querypush_srvnum.num_mutex, NULL);
	if (nRet != 0)
	{
		ERROR("InitWanweiSrvNumInfo: call pthread_mutex_init error%s", "");
		return FALSE;	
	}
	else
	{
		g_wanwei_querypush_srvnum.service_server_num = 0;
	}
	
	INFO("InitWanweiSrvNumInfo: func end%s", "");
	return TRUE;
}

int InitImSrvNumInfo(void)
{
	INFO("InitImSrvNumInfo: func begin%s", "");
	//��ʼ����¼����������������Ϣ
	if (pthread_mutex_init(&g_login_srv_num.num_mutex, NULL) != 0)
	{
		ERROR("InitImSrvNumInfo: call pthread_mutex_init error%s", "");
		return FALSE;
	}
	else
	{
		g_login_srv_num.service_server_num = 0;
	}

	//��ʼ��ע�����������������Ϣ
	if (pthread_mutex_init(&g_register_srv_num.num_mutex, NULL) != 0)
	{
		ERROR("InitImSrvNumInfo: call pthread_mutex_init error%s", "");
		return FALSE;
	}
	else
	{
		g_register_srv_num.service_server_num = 0;
	}

	//��ʼ�����ķ���������������Ϣ 
	if (pthread_mutex_init(&g_simplechat_srv_num.num_mutex, NULL) != 0)
	{
		ERROR("InitImSrvNumInfo: call pthread_mutex_init error%s", "");
		return FALSE;
	}
	else
	{
		g_simplechat_srv_num.service_server_num = 0;
	}

	//��ʼ��Ⱥ�ķ���������������Ϣ
	if (pthread_mutex_init(&g_clusterchat_srv_num.num_mutex, NULL) != 0)
	{
		ERROR("InitImSrvNumInfo: call pthread_mutex_init error%s", "");
		return FALSE;
	}
	else
	{
		g_clusterchat_srv_num.service_server_num = 0;
	}
		
	INFO("InitImSrvNumInfo: func end%s", "");
	return TRUE;
}

//������;:��ʼ��ҵ�������������Ϣ(�������͵�ҵ�����������)
//�������: ��
//�������: �� 
//����ֵ	: ��ʼ���ɹ�,  ����TRUE�� ��ʼ��ʧ��,  ����FALSE

int InitServiceSrvNumInfo(void)
{
	INFO("InitServiceSrvNumInfo: func  begin%s", "");
	int nRet = 0;

	nRet = InitImSrvNumInfo();
	if (TRUE != nRet)
	{
		ERROR("InitServiceSrvNumInfo: Call InitImSrvNumInfo error%s", "");
		return FALSE;
	}

	nRet = pthread_mutex_init(&g_liantong_srv_num.num_mutex, NULL);
	if (nRet != 0)
	{
		ERROR("InitServiceSrvNumInfo: call pthread_mutex_init error%s", "");
		return FALSE;	
	}
	else
	{
		g_liantong_srv_num.service_server_num = 0;
	}

	nRet = InitWanweiSrvNumInfo();
	if (TRUE != nRet)
	{
		ERROR("InitServiceSrvNumInfo: Call InitWanweiLoginSrvNumInfo error%s", "");
		return FALSE;
	}

	INFO("InitServiceSrvNumInfo: func end%s", "");
	return TRUE;
}

//������;: ��ʼ��ҵ��·�ɱ���Ϣ(���ҵ���������������Ϣ)
//�������: ��
//�������: �� 
//����ֵ	: ��ʼ���ɹ�,  ����TRUE�� ��ʼ��ʧ��,  ����FALSE

int InitRoutingTableInfo(int nHashtable_size)
{
	INFO("InitRoutingTableInfo; func begin%s", "");
	if(pthread_mutex_init(&g_routing_table.routing_mutex, NULL) != 0)
	{
		ERROR("InitRoutingTableInfo: Call pthread_mutex_init error%s", "");
		return FALSE;
	}

	DEBUG("InitRoutingTableInfo: service routing table=%d", nHashtable_size);

	if(TRUE != HashInit(&g_routing_table.pRouting_table, nHashtable_size, NULL))
	{
		ERROR("InitRoutingTableInfo: Call YDT_Hash_Init error%s", "");
		return FALSE;
	}

	StRouting_Table *pRouting_table = (StRouting_Table *)shmat(g_iShmId, NULL, 0);

	
	if(pthread_mutex_init(&pRouting_table->routing_mutex, NULL) != 0)
	{
		ERROR("InitRoutingTableInfo: Call pthread_mutex_init error%s", "");
		return FALSE;
	}

	DEBUG("InitRoutingTableInfo: service routing table=%d", nHashtable_size);

	if(TRUE != HashInit(&pRouting_table->pRouting_table, nHashtable_size, NULL))
	{
		ERROR("InitRoutingTableInfo: Call HashInit error%s", "");
		return FALSE;
	}

	INFO("InitRoutingTableInfo: func end%s", "");
	return TRUE;
}


//������;: ��ʼ����Ϣ������(��Ϣ��������Ҫ���������������������Ϊ��ֵ������ͻ��˵����ݰ���Ϣ��)
//�������: ��
//�������: �� 
//����ֵ	: ��ʼ���ɹ�,  ����TRUE�� ��ʼ��ʧ��,  ����FALSE

int InitMsgCounter(void)
{
	INFO("InitMsgCounter: func begin%s", "");
	int nRet = pthread_mutex_init(&g_msg_counter.msg_counter_mutex, NULL);
	if (nRet != 0)
	{
		ERROR("InitMsgCounter: Call pthread_mutex_init error%s", "");
		return FALSE;
	}
	else
	{
		g_msg_counter.nMsg_counter = 0;
	}

	INFO("InitMsgCounter: func end%s", "");
	return TRUE;
}

//������;: ��ʼ��epoll�ṹ��Ϣ(��Ҫ����������ͻ��˵Ĵ󲢷����ӵ�)
//�������: ��
//�������: �� 
//����ֵ	: ��ʼ���ɹ�,  ����TRUE�� ��ʼ��ʧ��,  ����FALSE

int IniEpollFdInfo(void)
{
	INFO("IniEpollFdInfo: func begin%s", "");
	int nRet = pthread_mutex_init(&g_epoll_fd.epoll_mutex, NULL);
	if (nRet != 0)
	{
		ERROR("IniEpollFdInfo: Call pthread_mutex_init error%s", "");
		return FALSE;
	}
	else
	{
		g_epoll_fd.nEp_fd = 0;
	}

	INFO("IniEpollFdInfo: func end%s", "");
	return TRUE;
}

//������;:��ʼ�����������õ������е�ȫ�ֱ�����Ϣ
//�������: ��
//�������: �� 
//����ֵ	: ��ʼ���ɹ�,  ����TRUE�� ��ʼ��ʧ��,  ����FALSE


int InitVarInfo(void)
{
	INFO("InitVarInfo: func begin%s", "");
	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	int nMsg_queue_size = g_srv_conf_info.nMsg_queue_size;
	int nRoutingtable_size = g_srv_conf_info.nRoutingtable_size;
	int nHeartbeat_table_size = g_srv_conf_info.nHeartbeat_table_size;
	int nClient_msgbuf_tablesize = g_srv_conf_info.nClient_msgbuf_tablesize;
	int nClient_msginfo_tablezie = g_srv_conf_info.nClient_msginfo_tablesize;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	if (FALSE == IniEpollFdInfo())
	{
		ERROR("InitVarInfo: Call IniEpollFdInfo error%s", "");	
		return FALSE;
	}

	if (FALSE == InitMsgCounter())
	{
		ERROR("InitVarInfo: Call InitMsgCounter error%s", "");	
		return FALSE;
	}

	
	if (FALSE == InitAwakeThreadFlgInfo())
	{
		ERROR("InitVarInfo: Call InitAwakeThreadFlgInfo error%s", "");	
		return FALSE;
	}


	if (FALSE == InitServiceSrvSelectInfo())
	{
		ERROR("InitVarInfo: Call InitServiceSrvSelectInfo error%s", "");	
		return FALSE;
	}
	
	if (FALSE == InitServiceSrvNumInfo())
	{
		ERROR("InitVarInfo: Call InitServiceSrvNumInfo error%s", "");	
		return FALSE;
	}

	if (FALSE == InitTmpClientSockHash())
	{
		ERROR("InitVarInfo: Call InitTmpClientSockHash error%s", "");	
		return FALSE;	
	}
 
	if (FALSE == InitClientMsgQueue())
	{
		ERROR("InitVarInfo: Call InitClientMsgQueue error%s", "");	
		HashFree(&g_tmp_clientsock_hash.pTmp_clientsock_table, FreeFunc);		//�ͷ��ڴ�
		return FALSE;	
	}

	if (FALSE == InitSndClientTmpSockHash())
	{
		ERROR("InitVarInfo: Call InitSndClientTmpSockHash error%s", "");	
		HashFree(&g_tmp_clientsock_hash.pTmp_clientsock_table, FreeFunc);		//�ͷ��ڴ�
		return FALSE;	
	}
	
	if (FALSE == InitClientInfoHash(nClient_msginfo_tablezie))
	{
		ERROR("InitVarInfo: Call InitClientInfoHash error%s", "");
		HashFree(&g_sndclient_tmp_sockhash.pTmp_clientsock_table, FreeFunc);		//�ͷ��ڴ�
		HashFree(&g_tmp_clientsock_hash.pTmp_clientsock_table, FreeFunc); 	//�ͷ��ڴ�
		return FALSE;	
	}

	//��ʼ��������Ϣ	
	if (FALSE == InitQueueInfo(nMsg_queue_size))
	{
		ERROR("InitSQueueInfo: Call InitQueueInfo error%s", "");
		HashFree(&g_tmp_clientsock_hash.pTmp_clientsock_table, FreeFunc); 	//�ͷ��ڴ�
		HashFree(&g_clientinfo_hash.pClient_info_hash, FreeFunc);		//�ͷ��ڴ�
		HashFree(&g_sndclient_tmp_sockhash.pTmp_clientsock_table, FreeFunc);		//�ͷ��ڴ�
		return FALSE;
	}

	//��ʼ��ҵ��·�ɱ���Ϣ
	if (FALSE == InitRoutingTableInfo(nRoutingtable_size))
	{
		ERROR("InitVarInfo: Call InitRoutingTableInfo error%s", "");	
		//�ͷ��ڴ�
		HashFree(&g_tmp_clientsock_hash.pTmp_clientsock_table, FreeFunc);		//�ͷ��ڴ�
		HashFree(&g_clientinfo_hash.pClient_info_hash, FreeFunc);
		HashFree(&g_sndclient_tmp_sockhash.pTmp_clientsock_table, FreeFunc);		//�ͷ��ڴ�
		DestroySeqQueue(&g_msg_queue.msg_queue);
		return FALSE;	
	}

	if (FALSE == InitMsgBufHashInfo(nClient_msgbuf_tablesize))
	{
		ERROR("InitVarInfo: Call InitMsgBufInfo error%s", "");

		//�ͷ��ڴ�
		HashFree(&g_tmp_clientsock_hash.pTmp_clientsock_table, FreeFunc);		//�ͷ��ڴ�
		HashFree(&g_clientinfo_hash.pClient_info_hash, FreeFunc);
		HashFree(&g_routing_table.pRouting_table, FreeFunc);
		HashFree(&g_sndclient_tmp_sockhash.pTmp_clientsock_table, FreeFunc);		//�ͷ��ڴ�

		DestroySeqQueue(&g_msg_queue.msg_queue);
		return FALSE;
	}

	if (FALSE == InitClientSockQueueInfo())
	{
		ERROR("InitVarInfo: Call InitClientSockQueueInfo error%s", "");

		//�ͷ��ڴ�
		HashFree(&g_tmp_clientsock_hash.pTmp_clientsock_table, FreeFunc);		//�ͷ��ڴ�
		HashFree(&g_clientinfo_hash.pClient_info_hash, FreeFunc);
		HashFree(&g_routing_table.pRouting_table, FreeFunc);
		HashFree(&g_msgbuf_hashtable.pMsg_buf_hashtable, FreeFunc);
		HashFree(&g_sndclient_tmp_sockhash.pTmp_clientsock_table, FreeFunc);		//�ͷ��ڴ�
		DestroySeqQueue(&g_msg_queue.msg_queue);
		return FALSE;	
	}

	if (FALSE == InitHeartbeatDetectHashtable(nHeartbeat_table_size))
	{
		ERROR("InitVarInfo: Call InitHeartbeatDetectHashtable error%s", "");

		//�ͷ��ڴ�
		HashFree(&g_tmp_clientsock_hash.pTmp_clientsock_table, FreeFunc);		//�ͷ��ڴ�
		HashFree(&g_clientinfo_hash.pClient_info_hash, FreeFunc);
		HashFree(&g_routing_table.pRouting_table, FreeFunc);
		HashFree(&g_msgbuf_hashtable.pMsg_buf_hashtable, FreeFunc);
		HashFree(&g_sndclient_tmp_sockhash.pTmp_clientsock_table, FreeFunc);		//�ͷ��ڴ�
		DestroySeqQueue(&g_msg_queue.msg_queue);
		return FALSE;
	}
	
	INFO("InitVarInfo: func end%s", "");
	return TRUE;
}

