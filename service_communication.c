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

/*该文件主要是负责业务服务器通讯使用的
  */



//变量定义 

/*保存所有线程信息的结构(此变量主要是为了使得服务器可以动态地取消发送接收线程,  并且在程序发生错误的时候
  *可以动态地删除掉程序启动过程中分配的内存
  */

#define SIMPLE_MODE_SERVER 0x00
#define THREAD_POOL_MODE_SERVER 0x01

extern Threads_Info g_threads_info;							//线程ID信息
extern Server_Conf_Info g_srv_conf_info;

/*计算处理线程的总数(此变量是计算业务服务器创建的线程的总数
  *通过此变量来限制服务器创建的线程的数量)
  */
extern Process_Thread_Counter g_process_thread_counter;

//函数用途:启动服务器
//输入参数: 无
//输出参数: 无 
//返回值	:启动服务器成功,  返回TRUE,  启动服务器失败,   返回FALSE 

int StartServer(void)
{	
	INFO("StartServer: func begin%s", "");
	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	BYTE bServer_mode = g_srv_conf_info.bServer_mode;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);
	int nRet = 0;

	DEBUG("StartServer: [server mode]=%d", bServer_mode);

	//运行简单模式的服务器
	if (SIMPLE_MODE_SERVER == bServer_mode)
	{
		INFO("StartServer: we will run simple mode server%s", "");	
		nRet = RunSimpleModeServer();
	}
	else if (THREAD_POOL_MODE_SERVER == bServer_mode)	//运行线程池模式的服务器
	{
		INFO("StartServer: we will run thread pool mode server%s", "");	
		nRet = RunThreadPoolModeServer();
	}

	return nRet;
	INFO("StartServer: func end%s", "");
}

