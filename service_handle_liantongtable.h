
#ifndef SERVICE_HANDLE_LIANTONGTABLE
#define SERVICE_HANDLE_LIANTONGTABLE

#include "../../service/liantong_service/service_record_charge_info.h"
#include "../../service/liantong_service/service_old_record_chargeinfo.h"
#include "../../service/liantong_service/service_usershare_clickrecord.h"

#include "../../service/handle_service/service_handle_service.h"

#include "../liantong_table/service_table_chinese_code.h"
#include "../liantong_table/service_table_phone_register.h"
#include "../liantong_table/service_table_snd_task_a.h"
#include "../liantong_table/translate_table/service_table_software_tack.h"
#include "../liantong_table/translate_table/service_table_book_tack.h"
#include "../liantong_table/translate_table/service_table_weibo.h"
#include "../liantong_table/service_table_Client_update.h"
#include "../liantong_table/translate_table/service_table_advertise.h"
#include "../liantong_table/translate_table/service_table_new_preference.h"
#include "../liantong_table/translate_table/service_table_recomm_url.h"
#include "../../thread/service_handle_pushmsg_thread.h"

/*********************函数名称都是以H 开始的************************************************/

/**********联通业务操作数据库表操作**********/

#ifdef _LIANTONG_SERVICE_
int H_BindUUIDAndPhoneNum(int nUUID, char *pPhone_num);
int H_ReportAndroidInfo(StReport_Android_Info *pReport_android_info);
int H_ReportIPhoneInfo(int nUUID, char *pPhone_type, char *pOS_version);
int H_CheckAndroidSoftVersion(int nUUID, char *pVersion, BYTE *pUpdate_status, StSoftware_UpdateInfo **ppSoft_updateinfo);
int H_RecordClientUpdateInfo(StSoftware_update_record *pUpdate_record);

int H_GetTaskSendQueryData(int nUUID, StTask_Send_Query_Data *pTask_snd_query_data, int *pTask_num);

int H_InsertClientFeedbackInfo(StClient_Feedback *pClient_feedback);

int H_InsertTaskTouchRecordInfo(int nUUID, char *pPhone_num, int nTask_id, BYTE bTask_type);
int H_InsertNewTaskTouchRecordInfo(StTask_Click_Info *pTask_click_info);
int H_DelTasksSndRecord(int nUUID, char *pTime);

int H_IsUUIDExist(int nUUID);

int H_IsUUIDExist(int nUUID);

int H_InsertPhoneRegisterInfo(EnPhoneRegisterType enRegis_type, char *pData, int nUUID);

int H_GetChargeTypes(StCharge_Type **ppCharge_type, int *pCharge_type_num);

int H_InsertClientChargeInfo(StCharge_Info *pCharge_info);
int H_InsertOldClientChargeInfo(StOld_Charge_Info *pCharge_info);

int H_GetMaxUUID(int nUsrid_begin, int *pMax_usrid);

int H_InsertUsrIdOutofrangeInfo(void);

void H_GetImageData(char **ppImage_data, int *pSize);

int H_SetUsridFullUseFlag(int nBegin_usrid, int nEnd_usrid);

int H_GetChineseCodeData(int nCode_id, StSystem_Tip_Info *pSystem_tip_info, BYTE *pHas_data);

int H_FindMaxUsrIdEnd(int *pMax_usr_id);

int H_ReportIPhoneMoreInfo(StIPhone_MoreInfo *pIphone_moreinfo);

int H_RecordUsrLoginInfo(StUsr_Login_Info *pUsr_login_info);
int H_DelUsrLoginInfo(int nUUID);

int H_GetRegisterCount(int nUUID_begin, int *pCount);
int H_GetNewRegisterCount(int nUUID_begin, char *pCur_time, int *pCount);
int H_TranslateTaskPushData(int *pCount);
int H_TransNewRegisUsrTaskPushData(char *pCur_time, int *pCount);
int H_GetRegisterUsrsInfo(StRegis_table_info *pRegis_table_info, void *pTask_a_data, StRegister_Datas *pRegister_data);
int H_GetNewRegisUsrsInfo(StRegis_table_info *pRegis_table_info, void *pData, char *pCur_time, StRegister_Datas *pRegister_data);
int H_InsertTaskData(StTask_A_Datas *pTask_a_data, StRegister_Datas *pRegister_datas);
int H_BatchInsertTaskData(StTask_A_Datas *pTask_a_data, StRegister_Datas *pRegister_datas);
int H_BatchInsertTaskAppleData(StTask_A_Datas *pTask_a_data, StRegister_Datas *pRegister_datas);
int H_InsertExactSndTaskAppleData(StTask_A_Info *pTask_a_info);
int H_InsertTagClickRecordInfo(StTag_Click_Info *pTag_click_info);
int H_GetExactSndRegisInfo(int nUUID_begin, StPhone_Num_Datas *pPhone_num_datas, void *pParam);
int H_InsertExactSndTaskData(StTask_A_Info *pTask_a_info);
int H_GetBlackLstInfo(StTask_A_Datas *pTask_a_datas);
int H_InsertUsrShareClickRecordInfo(StUsrShare_ClickInfo *pUsrshare_clickinfo);
int H_InsertMalfuncReportInfo(StMalfunc_Charge_Info *pMalfunc_charge_info);
int H_GetUsrLoginNum(int *pCount);
int H_GetUsrLoginInfos(StLogin_Table_Info *pLogin_table_info);
int H_TestTableNormal(StConn_Info *pConn_info);


/**********翻译线程获取数据开始**********/
int H_GetSoftwareOneLineData(int nID, StSoftware_Data *pSoft_data);
int H_GetBookOneLineData(int nID, StBook_Data *pBook_data);
int H_GetWeiboOneLineData(int nID, StWeibo_Data *pWeibo_data);
int H_GetClientUpdateOneLineData(int nID, StSof_Update_Data *pSoft_update_data);
int H_GetAdvertiseOneLineData(int nID, StAdvertise_Data *pAd_data);
int H_GetNewPreferenceData(int nID, StNew_Preference_Data *pNew_preference);
int H_GetRecommUrlOneLineData(int nID, StRecomm_Url_Data *pRecomm_url_data);

int H_GetNewTaskSendData(int nUUID, StTask_Snd_Datas *pTask_snd_query_data, int *pTask_num);

/**********翻译线程获取数据结束**********/


#endif


#endif

