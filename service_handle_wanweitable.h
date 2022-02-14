
#ifndef SERVICE_HANDLE_WANWEITABLE
#define SERVICE_HANDLE_WANWEITABLE

#include "../wanwei_table/service_table_usr_followbusi.h" 
#include "../../service/handle_service/service_handle_service.h"
#include "../wanwei_table/service_table_touch_tmp.h" 

int H_HandlePhoneLogin(StPhone_LoginInfo *pPhone_logininfo);
int H_InsertUsrGPSInfo(StUsr_GPS_Info *pUsr_gps_info);
int H_GetBusinessAreasInfo(void);
int H_GetBusiAreaIDs(int nUUID, int **ppID, int *pNum);
int H_TranslateBusiActivityTask(StActivity_Query_Info *pActivity_query_info, StBusi_Activity_PushData *pBusi_activity_data);
int H_GetBusiActivityInfo(StActivity_Query_Info *pActivity_query_info, int nActivity_id, StBusi_Activity_Info *pBusi_activity);
int H_GetInterestBusiActivityInfo(StActivity_Query_Info *pActivity_query_info, int nActivity_id, StBusi_Activity_Info *pBusi_activity, StFollow_Busi_Info *pFollow_busi_info);
int H_GetBusiName(int nBusi_id, char *pBusi_name, int nLen, int nCity_id);
int H_InsertBusiActivityPushData(StBusi_Activity_Data *pActivity_data);
int H_DelPushTask(StDel_PushTask *pDel_pushtask);
int H_GetTaskPushData(int nUUID, StPush_Task_Data *pPush_task_data);
int H_GetUsrFollowBusiID(int nUUID, StFollow_Busi_Info *pFollow_busi_info);
int H_TranslateInterestBusiActivityTask(StActivity_Query_Info *pActivity_query_info, StBusi_Activity_PushData *pBusi_activity_pushdata);
int H_InsertTouchInfo(StTouch_Info *pTouch_info);
int H_GetTouchInfo(StBusi_Touch_Info *pBusi_touch_info, StTouch_Match_Data *pTouch_match_data);
int H_GetBusiTouchInfo(StTouch_Info *pTouch_info, StTouch_Match_Data *pTouch_match_data);
int H_GetUsrConnInfo(int *pUUID, int nUUID_count, StUsr_login_info *pUsr_login_info, int *pConn_cnt);
int H_InsertBusiTouchInfo(StBusi_Touch_Info *pBusi_touch_info);

int H_GetConnInfo(int nUUID, StUsr_login_info *pUsr_login_info, int *pCount);


#endif

