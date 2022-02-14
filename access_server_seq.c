
#include "access_global.h"
#include "mm_memorymonitor.h"
//#include "../include/ydt_log.h"
#include "../communication/access_routing_maintain.h"
#include "access_server_seq.h"

//选择业务服务器的序号
extern Service_Server_Seq g_login_srv_seq;				//登录服务器序号
extern Service_Server_Seq g_register_srv_seq;			//注册服务器序号
extern Service_Server_Seq g_simplechat_srv_seq;			//单聊服务器序号
extern Service_Server_Seq g_clusterchat_srv_seq;		//群聊服务器序号
extern Service_Server_Seq g_liantong_srv_seq;		//联通服务器序号
extern Service_Server_Seq g_wanwei_loginsrv_seq;			//万维登录服务器序号
extern Service_Server_Seq g_wanwei_report_gps_srvseq;			//万维登录服务器数量
extern Service_Server_Seq g_wanwei_querypush_srvseq;			

//业务服务器数量
extern Service_Server_Num g_login_srv_num;				//登录服务器数量
extern Service_Server_Num g_register_srv_num;;			//注册服务器数量
extern Service_Server_Num g_simplechat_srv_num;			//单聊服务器数量
extern Service_Server_Num g_clusterchat_srv_num;		//群聊服务器数量
extern Service_Server_Num g_liantong_srv_num;		//联通服务器数量
extern Service_Server_Num g_wanwei_loginsrv_num;			//万维登录服务器数量
extern Service_Server_Num g_wanwei_report_gps_srvnum;			//万维登录服务器数量
extern Service_Server_Num g_wanwei_querypush_srvnum;		

static int GetImServerSeq(BYTE bSub_service_code)
{
	INFO("GetImServerSeq: func begin%s", "");
	int nService_srv_seq = 0;
	
	if (LOGIN_SERVICE_TYPE == bSub_service_code)
	{
		
		pthread_mutex_lock(&g_login_srv_seq.srv_seq_mutex);
		if (0 == g_login_srv_num.service_server_num)
		{
			nService_srv_seq = 0;
		}
		else
		{
			nService_srv_seq = (g_login_srv_seq.nService_server_seq % g_login_srv_num.service_server_num + 1);	
			g_login_srv_seq.nService_server_seq++;
			g_login_srv_seq.nService_server_seq = g_login_srv_seq.nService_server_seq % g_login_srv_num.service_server_num;
		}
		pthread_mutex_unlock(&g_login_srv_seq.srv_seq_mutex);
	}
	else if (REGISTER_SERVICE_TYPE == bSub_service_code)
	{
		pthread_mutex_lock(&g_register_srv_seq.srv_seq_mutex);
		if (0 == g_register_srv_num.service_server_num)
		{
			nService_srv_seq = 0;
		}
		else
		{
			nService_srv_seq = (g_register_srv_seq.nService_server_seq % g_register_srv_num.service_server_num + 1);	
			g_register_srv_seq.nService_server_seq++;
			g_register_srv_seq.nService_server_seq = g_register_srv_seq.nService_server_seq % g_register_srv_num.service_server_num;
		}
		pthread_mutex_unlock(&g_register_srv_seq.srv_seq_mutex);
	}
	else if (SIMPLECHAT_SERVICE_TYPE == bSub_service_code)
	{
		pthread_mutex_lock(&g_simplechat_srv_seq.srv_seq_mutex);
		if (0 == g_simplechat_srv_num.service_server_num)
		{
			nService_srv_seq = 0;
		}
		else
		{
			nService_srv_seq = (g_simplechat_srv_seq.nService_server_seq % g_simplechat_srv_num.service_server_num + 1);	
			g_simplechat_srv_seq.nService_server_seq++;
			g_simplechat_srv_seq.nService_server_seq = g_simplechat_srv_seq.nService_server_seq % g_simplechat_srv_num.service_server_num;
		}
		pthread_mutex_unlock(&g_simplechat_srv_seq.srv_seq_mutex);
	}
	else if (CLUSTERCHAT_SERVICE_TYPE == bSub_service_code)
	{
		pthread_mutex_lock(&g_clusterchat_srv_seq.srv_seq_mutex);
		if (0 == g_clusterchat_srv_num.service_server_num)
		{
			nService_srv_seq = 0;
		}
		else
		{
			nService_srv_seq = (g_clusterchat_srv_seq.nService_server_seq % g_clusterchat_srv_num.service_server_num + 1);	
			g_clusterchat_srv_seq.nService_server_seq++;
			g_clusterchat_srv_seq.nService_server_seq = g_clusterchat_srv_seq.nService_server_seq % g_clusterchat_srv_num.service_server_num;
		}
		pthread_mutex_unlock(&g_clusterchat_srv_seq.srv_seq_mutex);
	}
	
	INFO("GetImServerSeq: func end%s", "");
	return nService_srv_seq;
}

static int GetWanweiServerSeq(BYTE bSub_service_code)
{
	INFO("GetWanweiServerSeq: func begin%s", "");
	int nService_srv_seq = 0;
	
	if (WANWEI_LOGIN_SUB_SERVICETYPE == bSub_service_code)
	{
		pthread_mutex_lock(&g_wanwei_loginsrv_seq.srv_seq_mutex);
		if (0 == g_wanwei_loginsrv_num.service_server_num)
		{
			nService_srv_seq = 0;
		}
		else
		{
			nService_srv_seq = (g_wanwei_loginsrv_seq.nService_server_seq % g_wanwei_loginsrv_num.service_server_num + 1);
			g_wanwei_loginsrv_seq.nService_server_seq++;
			g_wanwei_loginsrv_seq.nService_server_seq = g_wanwei_loginsrv_seq.nService_server_seq % g_wanwei_loginsrv_num.service_server_num;
		}
		pthread_mutex_unlock(&g_wanwei_loginsrv_seq.srv_seq_mutex);
	}
	else if (WANWEI_REPORT_GPS_SERVICETYPE == bSub_service_code)
	{
		pthread_mutex_lock(&g_wanwei_report_gps_srvseq.srv_seq_mutex);
		if (0 == g_wanwei_report_gps_srvnum.service_server_num)
		{
			nService_srv_seq = 0;
		}
		else
		{
			nService_srv_seq = (g_wanwei_report_gps_srvseq.nService_server_seq % g_wanwei_report_gps_srvnum.service_server_num + 1);
			g_wanwei_report_gps_srvseq.nService_server_seq++;
			g_wanwei_report_gps_srvseq.nService_server_seq = g_wanwei_report_gps_srvseq.nService_server_seq % g_wanwei_report_gps_srvnum.service_server_num;
		}
		pthread_mutex_unlock(&g_wanwei_report_gps_srvseq.srv_seq_mutex);
	}
	else if (WANWEI_QUERYPUSH__SERVICETYPE == bSub_service_code)
	{
		pthread_mutex_lock(&g_wanwei_querypush_srvseq.srv_seq_mutex);
		if (0 == g_wanwei_querypush_srvnum.service_server_num)
		{
			nService_srv_seq = 0;
		}
		else
		{
			nService_srv_seq = (g_wanwei_querypush_srvseq.nService_server_seq % g_wanwei_querypush_srvnum.service_server_num + 1);
			g_wanwei_querypush_srvseq.nService_server_seq++;
			g_wanwei_querypush_srvseq.nService_server_seq = g_wanwei_querypush_srvseq.nService_server_seq % g_wanwei_querypush_srvnum.service_server_num;
		}
		pthread_mutex_unlock(&g_wanwei_querypush_srvseq.srv_seq_mutex);	
	}
	
	INFO("GetWanweiServerSeq: func end%s", "");
	return nService_srv_seq;
}

static int GetLiantongServerSeq(BYTE bSub_service_code)
{
	INFO("GetLiantongServerSeq: func begin%s", "");
	int nService_srv_seq = 0;
	
	if (LIANTONG_SPECIAL_SERVICE_SUBTYPE == bSub_service_code || \
		QUERY_HALL_INFO == bSub_service_code)
	{
		pthread_mutex_lock(&g_liantong_srv_seq.srv_seq_mutex);
		if (0 == g_liantong_srv_num.service_server_num)
		{
			nService_srv_seq = 0;
		}
		else
		{
			nService_srv_seq = (g_liantong_srv_seq.nService_server_seq % g_liantong_srv_num.service_server_num + 1);	
			g_liantong_srv_seq.nService_server_seq++;
			g_liantong_srv_seq.nService_server_seq = g_liantong_srv_seq.nService_server_seq % g_liantong_srv_num.service_server_num;
		}
		pthread_mutex_unlock(&g_liantong_srv_seq.srv_seq_mutex);
	}
	INFO("GetLiantongServerSeq: func end%s", "");
	return nService_srv_seq;
}


//函数用途: 获取业务服务器的序号(如果没有业务服务器可用, 就返回零)
//输入参数: 主业务码,  子业务码
//输出参数:  无
//返回值	: 获取成功,  返回获取到的业务服务器序号， 获取失败,  返回零

int GetServiceSrvSeq(BYTE bMain_service_code, BYTE bSub_service_code)
{
	INFO("GetServiceSrvSeq: func begin%s", "");

	DEBUG("GetServiceSrvSeq: [main service code]=%d [sub service code]=%d", \
		bMain_service_code, bSub_service_code);
	int nService_srv_seq = 0;

	if (IM_SERVICE_TYPE == bMain_service_code)
	{
		nService_srv_seq = GetImServerSeq(bSub_service_code);
	}
	else if (LIANTONG_SERVICE_TYPE == bMain_service_code)			//群聊服务器
	{
		nService_srv_seq = GetLiantongServerSeq(bSub_service_code);
	}
	else if (WANWEI_SERVICE_TYPE == bMain_service_code)
	{
		nService_srv_seq = GetWanweiServerSeq(bSub_service_code);
	}

	DEBUG("GetServiceSrvSeq: [service server sequence]=%d", nService_srv_seq);
	INFO("GetServiceSrvSeq: func end%s", "");
	return nService_srv_seq;
}

