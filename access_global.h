#ifndef ACCESS_GLOBAL_H
#define ACCESS_GLOBAL_H

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <malloc.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <math.h>
#include <sys/msg.h> 
#include <sys/types.h> 
#include <signal.h>
#include <sys/epoll.h>
#include <errno.h> 
#include <stdarg.h>
#include <mcheck.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <vector>
#include <map>
#include <list>
#include <sstream>
#include <string>

using std::vector;
using std::map;
using std::list;
using std::string;


using std::stringstream;

#include "access_types.h"
#include "../communication/access_comm_client_rcv.h"
#include "access_operate_conf.h"
#include "../communication/access_epoll.h"
#include "../communication/access_comm_service_srv_snd.h"
#include "../communication/access_comm_service_srv_snd.h"

#include "mm_memorymonitor.h"

#if 1

//extern "C"

#include "../include/ydt_log.h"
#include "../include/sequence_queue.h"
#include "../include/hash.h"
#include "../include/link_queue.h"


#else
#include "ydt_log.h"
#include "sequence_queue.h"
#include "hash.h"
#include "link_queue.h"
#endif

/********���ļ���Ҫ���������ȫ�ֱ�����ȫ�ֺ���********/

//�ֽڶ���
//modify by LiHuangYuan,2011/10/14 modify struct pack
#pragma pack()

#ifndef HASH_SAMEKEY_EXIST
#define HASH_SAMEKEY_EXIST 0x02			//��ϣ��ļ�ֵ�ظ� 
#endif

#define SERVER_CLIENT_MSG 0x02 //���������ͻ��˵���Ϣ
#define FORWARD_SRV_CLIENT_MSG 0x03		//��ת��Ϣ

//����������˴�����
#define TIMEOUT_ERROR 0x01 		//��ʱ��Ϣ������
#define SRV_BUSY_ERROR 0x02		//���������æ������

//����������İ汾��
#define ACCESS_SRV_VERSION 0x01

//ҵ����������������������Ϣ����
#define RESPONSE_CLIENT_MSG 0x02					//��Ӧ�ͻ�����Ϣ���� 
#define FORWARD_CLIENT_MSG 0x03						//ת���ͻ�����Ϣ����
#define	HEARTBEAT_DETECT_MSG 0x05
#define NOTIFY_DISCONNECT_WITHCLIENT_MSG 0x08
#define PUSHTO_CLIENT_MSG 0x0A

//ά�������ļ��̴߳���
#define MAINTAIN_CONF_FILE_ERROR 0x02

#define FUNC_PARAM_ERROR 0x18
#define OUT_OF_MEMORY_ERROR 0x19


//IMҵ������
#define IM_SERVICE_TYPE 0x01
#define LIANTONG_SERVICE_TYPE 0x02
#define WANWEI_SERVICE_TYPE   0x03

//IMҵ���������ҵ��
#define LOGIN_SERVICE_TYPE 0x01
#define REGISTER_SERVICE_TYPE 0x02
#define SIMPLECHAT_SERVICE_TYPE 0x03
#define CLUSTERCHAT_SERVICE_TYPE 0x04
//��ͨҵ���������ҵ��
#define LIANTONG_SPECIAL_SERVICE_SUBTYPE 0x05
#define QUERY_HALL_INFO 0x06

//��άҵ����ҵ����
#define WANWEI_LOGIN_SUB_SERVICETYPE 	 0x01
#define WANWEI_REPORT_GPS_SERVICETYPE 	 0x02
#define WANWEI_QUERYPUSH__SERVICETYPE 	 0x03

//���������������ӱ�־
#define SERVER_ACCEPT_FINISH 0x02
//������������(����ж�̨���������, ����궨��������ֲ�ͬ�Ľ��������)
#define ACCESS_SRV_SEQ 0x01

//������Ϣ������
#define SERVER_ERR_MSG 0x10B
//��ϣ��Ĵ�С
#define     HASH_SIZE            256

#define Offset(s, m)  (unsigned int)&(((s *)0)->m)

#ifndef  MIN
#define MIN(a, b) \
	((a) < (b)) ? (a) : (b)
#endif

typedef struct
{
	pthread_t thread_id;
	long lTime;
}StClient_Socket_Info;

typedef struct
{
	pthread_mutex_t hash_mutex;
	StHash_Table *pTmp_clientsock_table;
}StTmp_clientSock_Hash;


//��ȡ�����·��
int GetProgramPath(char *pPath);

#endif 
