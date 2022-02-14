
#include "../util/access_global.h"
//#include "../include/ydt_log.h"
#include "../interface/access_protocol.h"
#include "access_comm_service_srv_rcv.h"
#include "access_comm_client_snd.h"
#include "access_sock_info.h"
#include "access_comm_service_srv_snd.h"
#include "../util/access_operate_conf.h"
#include "../main/access_srv.h"
#include "access_routing_maintain.h"

extern int g_iSemId;

extern CSerRouteInfo g_serRouteInfo;

extern map <string, Server_Info> g_mapSrvInfo;

#define CONTROLLEN (sizeof(struct cmsghdr) + sizeof(int))


/*���ļ���Ҫ����������·��ά���̵߳�
  *��Ҫ�Ĳ���������ҵ�������, ��������������ļ����µ�
  *ҵ�����������,  ������ҵ�������
  */

//����ά���̱߳�־
extern Awake_Thread_Flg g_awake_thread_flg;

extern int g_iShmId;

//ҵ�������	·�ɱ�
extern StRouting_Table g_routing_table;
extern Server_Conf_Info g_srv_conf_info;

//��Ϣ������
extern Msg_Counter g_msg_counter;

//������;:  ����ҵ�������
//�������:  ҵ���������Ϣ(ҵ�������IP�� �˿ڵ���Ϣ)
//�������:  ��
//����ֵ	: ���ӳɹ�,  ����TRUE������ʧ�ܣ� ����FALSE

int ConnectSrv(StConnInfo *pSrv_info)
{
	INFO("ConnectSrv: func begin%s", "");
	int nClient_sock = 0;
	struct sockaddr_in srv_addr = {0};

	if (FALSE == Socket(&nClient_sock))
	{
		ERROR("ConnectSrv: Call Socket() error%s", "");
		return FALSE;
	}

	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(pSrv_info->wPort);
	srv_addr.sin_addr.s_addr = inet_addr(pSrv_info->arrIP);

	while (-1 == connect(nClient_sock, (struct sockaddr *)&srv_addr, sizeof(struct sockaddr)))
	{
		WARN("ConnectSrv: connect service server error, sleep 2s and then reconnect service server again%s", "");
		sleep(2);
		continue;
	}

	DEBUG("ConnectSrv: connect service server successful...[socket id]=%d", nClient_sock);
	INFO("ConnectSrv: func end%s", "");
	return nClient_sock;
}

int ConnectSockThd(void)
{
	TRACE();
	int nClient_sock = 0;
	struct sockaddr_in srv_addr = {0};

	if (FALSE == Socket(&nClient_sock))
	{
		ERROR("ConnectSockThd: Call Socket() error%s", "");
		return FALSE;
	}

	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(5000);
	srv_addr.sin_addr.s_addr = inet_addr("192.168.0.149");

	while (-1 == connect(nClient_sock, (struct sockaddr *)&srv_addr, sizeof(struct sockaddr)))
	{
		WARN("ConnectSockThd: connect socket thd error, sleep 2s and then reconnect socket thd again");
		sleep(2);
		continue;
	}

	DEBUG("ConnectSockThd: connect socket thd successful...[socket id]=%d", nClient_sock);
	return nClient_sock;
}

//#define HAVE_MSGHDR_MSG_CONTROL


//������;: �������е�ҵ�������(socket�ֶ�Ϊ0��Ҫ����)
//�������: ��
//�������: ��
//����ֵ	: ���ӳɹ�, ����TRUE,  ����ʧ��,   ����FALSE;

int ConnectServiceServers(int iSock)
{	
	INFO("ConnectServiceServers: func begin%s", "");
	int nClient_sock = 0;
	int nRet = 0;

	CSerRouteInfo::MapType &mapRouteConnInfo = g_serRouteInfo.GetMapConnInfo();
	CSerRouteInfo::MapIter begin = mapRouteConnInfo.begin();
	while (begin != mapRouteConnInfo.end())
	{
		vector <StConnInfo> &vecConnInfo = begin->second;	
		for (int i=0; i<vecConnInfo.size(); i++)
		{
			INFO("ReadConfFile: [MainCode]=%d [SubCode]=%d [wServiceSeq]=%d"
				" [arrIP]=%s [port]=%d [socket]=%d", vecConnInfo[i].bMainCode, \
				vecConnInfo[i].bSubCode, vecConnInfo[i].wServiceSeq, vecConnInfo[i].arrIP, \
				vecConnInfo[i].wPort, vecConnInfo[i].nSock);
			if (0 == vecConnInfo[i].nSock)
			{
				nClient_sock = ConnectSrv(&vecConnInfo[i]);
				if (FALSE == nClient_sock)
				{
					ERROR("ConnectServiceServers: Call ConnectSrv error%s", "");
					return FALSE;
				}
				#if 0
				if ((IM_SERVICE_TYPE == pSrv_info->bMain_code && SIMPLECHAT_SERVICE_TYPE == pSrv_info->bSub_code) || \
					(IM_SERVICE_TYPE == pSrv_info->bMain_code && CLUSTERCHAT_SERVICE_TYPE == pSrv_info->bSub_code) || \
					(WANWEI_SERVICE_TYPE == pSrv_info->bMain_code && WANWEI_LOGIN_SUB_SERVICETYPE == pSrv_info->bSub_code) || \
					(WANWEI_SERVICE_TYPE == pSrv_info->bMain_code && WANWEI_QUERYPUSH__SERVICETYPE == pSrv_info->bSub_code))
				#endif
				{
					//�ú�����Ҫ�Ƕ�����ҵ���õ�,
					//��Ҫ��ʹ��ҵ����������Ա�������������ҵ�������֮���socket id
					if (TRUE != SndAfterConnSrvMsg(nClient_sock, vecConnInfo[i].bMainCode, vecConnInfo[i].bSubCode, vecConnInfo[i].wServiceSeq))
					{
						ERROR("ConnectServiceServers: Call SndAfterConnSrvMsg error%s", "");				
						return FALSE;
					}		
				}

				vecConnInfo[i].nSock = nClient_sock;

				//����ҵ���������Ϣ���̲߳���
				Recv_Thread_Param *pThread_param = (Recv_Thread_Param *)MM_MALLOC_WITH_DESC(sizeof(Recv_Thread_Param), 
				"ConnectServicesServer: Call func for recv thread param");
				if (NULL == pThread_param)
				{
					FATAL("ConnectServicesServer: Call malloc error%s", "");
					close(nClient_sock);
					return FALSE;
				}

				memset(pThread_param, 0, sizeof(Recv_Thread_Param));
				pThread_param->nSock = nClient_sock;
				pThread_param->bMain_service_code = vecConnInfo[i].bMainCode;
				pThread_param->bSub_service_code = vecConnInfo[i].bSubCode;
				pThread_param->wService_srv_seq = vecConnInfo[i].wServiceSeq;

				
				nRet = fork();
				if (0 == nRet)
				{
					RecvMsgFromServiceSrv((void *)pThread_param);
					//SndMsgToCli();
					//SndMsgToClientThread(NULL);
				}

				#if 0
				//���������߳�
				nRet= pthread_create(&thread_id, NULL, RecvMsgFromServiceSrv, (void *)pThread_param);
				if (nRet != 0)
				{
					ERROR("ConnectServiceServers: Call pthread_create error wait 2s and then connect service server again%s", "");
					close(nClient_sock);
					MM_FREE(pThread_param);
					iter->second.nSock = 0;
					sleep(2);
					continue;
				}	
				#endif
			}
		}
		begin++;	
	}

	#if 1
	int *pSize = (int *)shmat(g_iShmId, NULL, 0);
	*pSize = g_mapSrvInfo.size();
	pSize++;
	StShmSrvInfo *pShmSrvInfo = (StShmSrvInfo *)pSize;
	
	SemP(g_iSemId);
	begin = mapRouteConnInfo.begin();
	while (begin != mapRouteConnInfo.end())
	{
		vector <StConnInfo> &vecConnInfo = begin->second;	
		for (int i=0; i<vecConnInfo.size(); i++)
		{
			memcpy(pShmSrvInfo, &vecConnInfo[i].bMainCode, sizeof(*pShmSrvInfo));
			INFO("ConnectServiceServers: [main code]=%d [sub code]=%d [service seq]=%d [socket]=%d", pShmSrvInfo->bMain_code, \
				pShmSrvInfo->bSub_code, pShmSrvInfo->wService_seq, pShmSrvInfo->nSock);		
			pShmSrvInfo++;
		}
		
		begin++;
	}
	
	pSize = (int *)shmat(g_iShmId, NULL, 0);
	int iSize = *pSize;
	INFO("ConnectServiceServers: [size]=%d", iSize);
	pSize++;
	pShmSrvInfo = (StShmSrvInfo *)pSize;
	for (int i=0; i<iSize; i++)
	{
		//hmSrvInfo = (StShmSrvInfo *)pSize;
		INFO("ConnectServiceServers: share memory [main code]=%d [sub code]=%d [service seq]=%d [socket]=%d", \
			pShmSrvInfo->bMain_code, \
			pShmSrvInfo->bSub_code, pShmSrvInfo->wService_seq, pShmSrvInfo->nSock);		
		//(char *)pSize += sizeof(StShmSrvInfo);
		pShmSrvInfo++;
	}

	SemV(g_iSemId);
	#endif
	
	INFO("ConnectServiceServers: func end%s", "");
	return TRUE;
}


//������;: ά��·�ɱ��߳�(�����������ҵ��,  ��֪ͨ·��ά���߳�����ҵ�������)
//�������: ��
//�������: ��
//����ֵ	: ��

void *MaintainRoutingTable(void *pParam)
{
	INFO("MaintainRoutingTable: func begin%s", "");
	pthread_detach(pthread_self());

	int nAwake_flg = FALSE;
	int nPool_srvconf_interval = 0;

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	nPool_srvconf_interval = g_srv_conf_info.nPool_srvconfig_interval;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);
	
	DEBUG("MaintainRoutingTable: [poll server config time interval]=%d", nPool_srvconf_interval);
	while (TRUE)
	{
		sleep(nPool_srvconf_interval);				//��ѯ�̵߳�ʱ����
		nAwake_flg = MaintainConfFile();

		if (TRUE == nAwake_flg)
		{
			pthread_mutex_lock(&g_awake_thread_flg.awake_flg_mutex);
			g_awake_thread_flg.nAwake_thread_flg = TRUE;			//��������־����ΪTURE,  ֪ͨά���߳̽����������ݿ�
			pthread_mutex_unlock(&g_awake_thread_flg.awake_flg_mutex);
			nAwake_flg = FALSE;
		}
		else if (MAINTAIN_CONF_FILE_ERROR == nAwake_flg)
		{
			ERROR("MaintainRoutingTable: Call MaintainConfFile error%s", "");
			pthread_exit(NULL);
			return NULL;
		}
	}

	INFO("MaintainRoutingTable: func end%s", "");
	return NULL;
}


//������;:·��ά���߳�,   ��������ҵ�������
//�������: ��
//�������: ��
//����ֵ	: ��

void *RoutingMaintainThread(void *pParam)
{
	INFO("RoutingMaintainThread: func begin%s", "");
	//�����߳�Ϊ�����߳�
	pthread_detach(pthread_self());

	pthread_t thread_id = 0;
	int nRet = 0;

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	int nDec_reconnect_interval = g_srv_conf_info.nDec_reconnect_servicesrv_interval;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	DEBUG("RoutingMaintainThread: [Detect reconnect service server time interval]=%d", nDec_reconnect_interval);

	//����ά�������ļ��߳�
	nRet= pthread_create(&thread_id, NULL, MaintainRoutingTable, NULL);
	if (nRet != 0)
	{
		ERROR("RoutingMaintainThread: Call pthread_create error%s", "");
		//modify by luguanhuang 20111014 (reason: ����ʧ�ܺ�����˳��߳�)
		//��ô�ͽ����Ӳ���ҵ�������
		//pthread_exit(NULL);	
		//return;
	}	

	//int iSock = ConnectSockThd();	
	int iSock = 0;	
	
	while (TRUE)
	{
		if (TRUE == g_awake_thread_flg.nAwake_thread_flg)
		{
			//�������е�ҵ�������
			nRet = ConnectServiceServers(iSock);
			if (FALSE == nRet)
			{
				ERROR("RoutingMaintainThread: Call ConnectServiceServers error wait %ds and then reconnect service server again", nDec_reconnect_interval);
				sleep(nDec_reconnect_interval);
				continue;
			}

			//�ѻ��ѱ�־����FALSE
			pthread_mutex_lock(&g_awake_thread_flg.awake_flg_mutex);
			g_awake_thread_flg.nAwake_thread_flg = FALSE;
			pthread_mutex_unlock(&g_awake_thread_flg.awake_flg_mutex);
		}

		sleep(nDec_reconnect_interval);					//��ѯ��ʱ����
	}

	INFO("RoutingMaintainThread: func end%s", "");
	return NULL;
}

