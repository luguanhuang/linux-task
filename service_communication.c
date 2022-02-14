#include "../util/service_global.h"
#include "../include/ydt_log.h"
#include "../interface/service_protocol.h"
#include "../thread/service_heartbeat_detectthread.h"
#include "service_process_func.h"
#include "service_maintaininfo_thread.h"
#include "service_sock_info.h"
#include "service_simplemode_server.h"
#include "service_threadpoolmode_server.h"
#include "service_communication.h"

#include <pthread.h>
#include <time.h>

/*���ļ���Ҫ�Ǹ���ҵ�������ͨѶʹ�õ�
  */



//�������� 

/*���������߳���Ϣ�Ľṹ(�˱�����Ҫ��Ϊ��ʹ�÷��������Զ�̬��ȡ�����ͽ����߳�,  �����ڳ����������ʱ��
  *���Զ�̬��ɾ�����������������з�����ڴ�
  */

#define SIMPLE_MODE_SERVER 0x00
#define THREAD_POOL_MODE_SERVER 0x01

extern Threads_Info g_threads_info;							//�߳�ID��Ϣ
extern Server_Conf_Info g_srv_conf_info;

/*���㴦���̵߳�����(�˱����Ǽ���ҵ��������������̵߳�����
  *ͨ���˱��������Ʒ������������̵߳�����)
  */
extern Process_Thread_Counter g_process_thread_counter;

//������;:����������
//�������: ��
//�������: �� 
//����ֵ	:�����������ɹ�,  ����TRUE,  ����������ʧ��,   ����FALSE 

int StartServer(void)
{	
	INFO("StartServer: func begin%s", "");
	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	BYTE bServer_mode = g_srv_conf_info.bServer_mode;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);
	int nRet = 0;

	DEBUG("StartServer: [server mode]=%d", bServer_mode);

	//���м�ģʽ�ķ�����
	if (SIMPLE_MODE_SERVER == bServer_mode)
	{
		INFO("StartServer: we will run simple mode server%s", "");	
		nRet = RunSimpleModeServer();
	}
	else if (THREAD_POOL_MODE_SERVER == bServer_mode)	//�����̳߳�ģʽ�ķ�����
	{
		INFO("StartServer: we will run thread pool mode server%s", "");	
		nRet = RunThreadPoolModeServer();
	}

	return nRet;
	INFO("StartServer: func end%s", "");
}

