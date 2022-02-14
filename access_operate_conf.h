
#ifndef ACCESS_OPERATOR_SERVICE_CONF
#define ACCESS_OPERATOR_SERVICE_CONF

/***************该文件主要是用来处理配置文件的***************/

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


//服务器配置文件信息
typedef struct
{
	pthread_mutex_t conf_info_mutex;		//配置文件互斥变量
	WORD wPort;								//端口号
	int nLiantong_srv_num;					//联通服务器数量
	int nMin_thread_num;					//最少线程数量
	int nMax_thread_num;					//最大线程数量
	int nPoll_heartbeat_interval;			//轮询心跳包的时间间隔
	int nHeartbeat_timeout_time;			//心跳包的超时时间
	int nPool_srvconfig_interval;			//轮询业务服务器配置文件的时间间隔
	int nPool_clientmsg_interval;			//轮询客户端消息包的时间间隔
	int nClientmsg_timeout_time;			//客户端消息包的超时消息
	int nMsg_queue_size;					//消息队列的大小
	int nRoutingtable_size;					//业务路由哈希表的大小
	int nHeartbeat_table_size;				//心跳检测表大小
	int nClient_msgbuf_tablesize;			//客户端消息缓存哈希表大小
	int nClient_msginfo_tablesize;			//客户端信息哈希表大小
	int nDec_reconnect_servicesrv_interval;	//检测重连业务服务器的时间间隔
	WORD wMaintain_port;					//侦听运维客户端的端口号
	int nEpoll_size;						//epoll大小	
	int nServer_mode;
	int nMax_clientmsg_thread_num;			//最大客户端消息线程数量
	BYTE bLog_print_level;
	int nTmp_clientsock_hash_size;
	int nSndmsgto_servicesrv_threadnum;
	int nSndmsgto_client_threadnum;
	WORD wAccess_srv_seq;
	int iMaxWorker;
}Server_Conf_Info;

struct StConnInfo
{
	BYTE bMainCode;								//主业务码
	BYTE bSubCode;									//子业务码	
	WORD wServiceSeq;//the ID of service server		//业务服务器序号
	char arrIP[100];					//ip
	WORD wPort;									//端口
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

//读取路由配置文件
int ReadConfFile(void);

//维护路由配置文件
int MaintenanceConfFile(void);

int ReadServerConfFile(void);

int MaintainConfFile(void);

#endif
