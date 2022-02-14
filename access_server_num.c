
#include "access_global.h"
#include "mm_memorymonitor.h"
//#include "../include/ydt_log.h"
#include "../communication/access_routing_maintain.h"
#include "access_server_num.h"

//选择业务服务器的序号
extern Service_Server_Seq g_login_srv_seq;				//登录服务器序号
extern Service_Server_Seq g_register_srv_seq;			//注册服务器序号
extern Service_Server_Seq g_simplechat_srv_seq;			//单聊服务器序号
extern Service_Server_Seq g_clusterchat_srv_seq;		//群聊服务器序号
extern Service_Server_Seq g_liantong_srv_seq;		//联通服务器序号
extern Service_Server_Seq g_wanwei_loginsrv_seq;			//万维登录服务器序号
extern Service_Server_Seq g_wanwei_report_gps_srvseq;			//万维登录服务器数量

//业务服务器数量
extern Service_Server_Num g_login_srv_num;				//登录服务器数量
extern Service_Server_Num g_register_srv_num;;			//注册服务器数量
extern Service_Server_Num g_simplechat_srv_num;			//单聊服务器数量
extern Service_Server_Num g_clusterchat_srv_num;		//群聊服务器数量
extern Service_Server_Num g_liantong_srv_num;		//联通服务器数量
extern Service_Server_Num g_wanwei_loginsrv_num;			//万维登录服务器数量
extern Service_Server_Num g_wanwei_report_gps_srvnum;			//万维登录服务器数量
extern Service_Server_Num g_wanwei_querypush_srvnum;		

extern Server_Conf_Info g_srv_conf_info;

static void IncWanweiServiceSrvNum(BYTE bSubservice_code)
{
	INFO("IncWanweiServiceSrvNum: func begin%s", "");
	if (WANWEI_LOGIN_SUB_SERVICETYPE == bSubservice_code)
	{
		pthread_mutex_lock(&g_wanwei_loginsrv_num.num_mutex);
		g_wanwei_loginsrv_num.service_server_num++;
		pthread_mutex_unlock(&g_wanwei_loginsrv_num.num_mutex);
		INFO("IncWanweiServiceSrvNum: [wanwei login server num]=%d", \
			g_wanwei_loginsrv_num.service_server_num);
	}
	else if (WANWEI_REPORT_GPS_SERVICETYPE == bSubservice_code)
	{
		pthread_mutex_lock(&g_wanwei_report_gps_srvnum.num_mutex);
		g_wanwei_report_gps_srvnum.service_server_num++;
		pthread_mutex_unlock(&g_wanwei_report_gps_srvnum.num_mutex);
		INFO("IncWanweiServiceSrvNum: [wanwei report gps server num]=%d", \
			g_wanwei_report_gps_srvnum.service_server_num);
	}
	else if (WANWEI_QUERYPUSH__SERVICETYPE == bSubservice_code)
	{
		pthread_mutex_lock(&g_wanwei_querypush_srvnum.num_mutex);
		g_wanwei_querypush_srvnum.service_server_num++;
		pthread_mutex_unlock(&g_wanwei_querypush_srvnum.num_mutex);	
		INFO("IncWanweiServiceSrvNum: [wanwei query push server num]=%d", \
			g_wanwei_querypush_srvnum.service_server_num);
	}
	
	INFO("IncWanweiServiceSrvNum: func end%s", "");
}



static void IncImServiceSrvNum(BYTE bSubservice_code)
{
	INFO("IncImServiceSrvNum: func begin%s", "");
	if (LOGIN_SERVICE_TYPE == bSubservice_code)
	{
		pthread_mutex_lock(&g_login_srv_num.num_mutex);
		g_login_srv_num.service_server_num++;
		pthread_mutex_unlock(&g_login_srv_num.num_mutex);
	}
	else if (REGISTER_SERVICE_TYPE == bSubservice_code)
	{
		pthread_mutex_lock(&g_register_srv_num.num_mutex);
		g_register_srv_num.service_server_num++;
		pthread_mutex_unlock(&g_register_srv_num.num_mutex);
	}
		
	else if (SIMPLECHAT_SERVICE_TYPE == bSubservice_code)
	{
		pthread_mutex_lock(&g_simplechat_srv_num.num_mutex);
		g_simplechat_srv_num.service_server_num++;
		pthread_mutex_unlock(&g_simplechat_srv_num.num_mutex);
	}
	else if (CLUSTERCHAT_SERVICE_TYPE == bSubservice_code)
	{
		pthread_mutex_lock(&g_clusterchat_srv_num.num_mutex);
		g_clusterchat_srv_num.service_server_num++;
		pthread_mutex_unlock(&g_clusterchat_srv_num.num_mutex);
	}
	INFO("IncImServiceSrvNum: func end%s", "");
}


void IncServiceSrvNum(BYTE bMainservice_code, BYTE bSubservice_code)
{
	INFO("IncServiceSrvNum: func begin%s", "");
	BYTE bSet_liantong_srv_num = FALSE;
	
	if (IM_SERVICE_TYPE == bMainservice_code)
	{
		IncImServiceSrvNum(bSubservice_code);
	}
	else if (LIANTONG_SERVICE_TYPE == bMainservice_code && LIANTONG_SPECIAL_SERVICE_SUBTYPE == bSubservice_code)
	{
		pthread_mutex_lock(&g_liantong_srv_num.num_mutex);
		if (FALSE == bSet_liantong_srv_num)
		{
			g_liantong_srv_num.service_server_num = g_srv_conf_info.nLiantong_srv_num;
			bSet_liantong_srv_num = TRUE;
		}
	
		pthread_mutex_unlock(&g_liantong_srv_num.num_mutex);			
	}
	else if (WANWEI_SERVICE_TYPE == bMainservice_code)
	{
		INFO("IncServiceSrvNum: [wanwei main service code]=%d [sub service code]=%d", \
				bMainservice_code, bSubservice_code);
		IncWanweiServiceSrvNum(bSubservice_code);
	}
	
	INFO("IncServiceSrvNum: func end%s", "");
}


void  UpdateServiceSrvNum(BYTE bMainservice_code, BYTE bSubservice_code)
{
	INFO("UpdateServiceSrvNum: func begin%s", "");
	if (IM_SERVICE_TYPE == bMainservice_code)
	{
		IncImServiceSrvNum(bSubservice_code);
	}
	else if (LIANTONG_SERVICE_TYPE ==bMainservice_code && LIANTONG_SPECIAL_SERVICE_SUBTYPE == bSubservice_code)
	{
		pthread_mutex_lock(&g_liantong_srv_num.num_mutex);
		g_liantong_srv_num.service_server_num++;
		pthread_mutex_unlock(&g_liantong_srv_num.num_mutex);

		pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
		g_srv_conf_info.nLiantong_srv_num++;
		pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);
	}
	else if (WANWEI_SERVICE_TYPE == bMainservice_code)
	{
		INFO("UpdateServiceSrvNum: [wanwei main service code]=%d [sub service code]=%d", \
			bMainservice_code, bSubservice_code);
		IncWanweiServiceSrvNum(bSubservice_code);
	}
	
	INFO("UpdateServiceSrvNum: func end%s", "");
}

