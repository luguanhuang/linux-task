
#ifndef ACCESS_OPERATOR_SERVICE_CONF
#define ACCESS_OPERATOR_SERVICE_CONF

/***************���ļ���Ҫ���������������ļ���***************/

#define FILE_PATH_LEN 512
#define CONFIG_FILE_NAME "config.txt"

#define SERVER_CONFFILE_NAME "server_config.txt"

typedef struct
{
	char arrKey[100];
	char arrValue[100];
}StConfLine_Data;

typedef struct
{
	StConfLine_Data arrConf_line_data[100];
	int nIdx;
	int nCount;
}StConf_File_Data;


//�����������ļ���Ϣ
typedef struct
{
	pthread_mutex_t conf_info_mutex;		//�����ļ��������
	WORD wPort;								//�˿ں�
	int nLiantong_srv_num;					//��ͨ����������
	int nMin_thread_num;					//�����߳�����
	int nMax_thread_num;					//����߳�����
	int nPoll_heartbeat_interval;			//��ѯ��������ʱ����
	int nHeartbeat_timeout_time;			//�������ĳ�ʱʱ��
	int nPool_srvconfig_interval;			//��ѯҵ������������ļ���ʱ����
	int nPool_clientmsg_interval;			//��ѯ�ͻ�����Ϣ����ʱ����
	int nClientmsg_timeout_time;			//�ͻ�����Ϣ���ĳ�ʱ��Ϣ
	int nMsg_queue_size;					//��Ϣ���еĴ�С
	int nRoutingtable_size;					//ҵ��·�ɹ�ϣ��Ĵ�С
	int nHeartbeat_table_size;				//���������С
	int nClient_msgbuf_tablesize;			//�ͻ�����Ϣ�����ϣ���С
	int nClient_msginfo_tablesize;			//�ͻ�����Ϣ��ϣ���С
	int nDec_reconnect_servicesrv_interval;	//�������ҵ���������ʱ����
	WORD wMaintain_port;					//������ά�ͻ��˵Ķ˿ں�
	int nEpoll_size;						//epoll��С	
	int nServer_mode;
	int nMax_clientmsg_thread_num;			//���ͻ�����Ϣ�߳�����
	BYTE bLog_print_level;
	int nTmp_clientsock_hash_size;
	int nSndmsgto_servicesrv_threadnum;
	int nSndmsgto_client_threadnum;
	WORD wAccess_srv_seq;
	int iMaxWorker;
}Server_Conf_Info;

struct StConnInfo
{
	BYTE bMainCode;								//��ҵ����
	BYTE bSubCode;									//��ҵ����	
	WORD wServiceSeq;//the ID of service server		//ҵ����������
	char arrIP[100];					//ip
	WORD wPort;									//�˿�
	int nSock;										//socket id
};

class CSerRouteInfo
{
public:
	typedef map <string, vector <StConnInfo> >::iterator MapIter;
	typedef map <string, vector <StConnInfo> > MapType;
	
	CSerRouteInfo(void);
	MapType &GetMapConnInfo(void);
	int GetSrvSeq(BYTE bMainCode, BYTE bSubCode);
private:
	map <string, vector <StConnInfo> > m_mapSerConnInfo;
};

//��ȡ·�������ļ�
int ReadConfFile(void);

//ά��·�������ļ�
int MaintenanceConfFile(void);

int ReadServerConfFile(void);

int MaintainConfFile(void);

#endif
