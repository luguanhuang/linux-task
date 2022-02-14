
#include "../util/service_global.h"
#include "../include/ydt_log.h"
#include "../interface/service_protocol.h"
#include "../DB/handle_table/service_handle_table.h"
#include "service_maintain_chargetypes.h"

#define ONE_HOUR (60 * 60)
#define ONE_DAY (24 * ONE_HOUR)

extern StCharge_Types_Info g_charge_types_info;

static int GetTime(int *pHour, int *pMinute)
{
	INFO("GetTime: func begin%s", "");
	if (NULL == pHour || NULL == pMinute)
	{
		ERROR("GetTime: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}
	
	time_t now;
	time(&now);
	struct tm *pTime = localtime(&now); 		//获取当前时间
	*pHour = pTime->tm_hour;
	*pMinute = pTime->tm_min;

	DEBUG("GetTime: [hour]=%d [minute]=%d", \
	*pHour, *pMinute);
	INFO("GetTime: func end%s", "");
	return TRUE;
}


void MaintainChargeTypesThread(void)
{
	INFO("MaintainChargeTypesThread: func begin%s", "");
	
	int nHour = 0;
	int nMinute = 0;
	int nDiff_hour = 0;
	int nRet = 0;
	BYTE bSleep_first_time = TRUE;

	GetTime(&nHour, &nMinute);

	if (nMinute > 30)
	{
		nHour++;
	}

	//现在到午夜两点之间的时间间隔
	nDiff_hour = 24 - nHour + 2;

	
	while (TRUE)
	{
		if (NULL != g_charge_types_info.pCharge_type)
		{
			INFO("MaintainChargeTypesThread: we will reload charge types again%s", "");
			pthread_mutex_lock(&g_charge_types_info.type_mutex);
			MM_FREE(g_charge_types_info.pCharge_type);
			g_charge_types_info.nCharge_type_num = 0;
			pthread_mutex_unlock(&g_charge_types_info.type_mutex);
		}
		else
		{
			WARN("MaintainChargeTypesThread: g_charge_types_info.pCharge_type is null"
				" maybe db is null%s", "");	
		}
		
		nRet = H_GetChargeTypes(&g_charge_types_info.pCharge_type, &g_charge_types_info.nCharge_type_num);
		if (TRUE != nRet)
		{
			ERROR("MaintainChargeTypesThread: Call H_GetChargeTypes error%s", "");
		}

		if (TRUE == bSleep_first_time)
		{
			INFO("MaintainChargeTypesThread: when the first time we sleep %d hour"
				" and then reload the charge types agasin", nDiff_hour);
			bSleep_first_time = FALSE;
			sleep(nDiff_hour * ONE_HOUR);		
		}
		else
		{
			INFO("MaintainChargeTypesThread: we will sleep one day and reload the charge types again%s", "");
			sleep(ONE_DAY);
			
		}
	}
	
	INFO("MaintainChargeTypesThread: func end%s", "");
}

 
