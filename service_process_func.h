
#ifndef SERVICE_PROCESS_FUNC
#define SERVICE_PROCESS_FUNC

#include "../util/service_global.h"
#include "../util/service_types.h"

typedef int (*PProcessFunc)(void *, StSequence_Queue *, pthread_mutex_t *, pthread_cond_t *);

typedef struct
{
	WORD wCmd_id;
	PProcessFunc pProcessFunc;
}StProcFunc_MapTable;

#include "service_wanwei_processfunc.h"


#define UUID_ISNOT_EXIST_ERROR		0x07			//UUID不存在错误

#define CLIENT_SERVER_MSG 0x01			//客户端到服务器的消息类型
#define SERVER_CLIENT_MSG 0x02 			//服务器到客户端的消息类型
#define FORWARD_SRV_CLIENT_MSG 0x03		//中转消息
#define PUSHTO_CLIENT_RESPONSE 0x0B 

#define NOTIFY_DISCONNECT_WITHCLIENT_MSG 0x08
#define PUSHTO_CLIENT_MSG				 0x0A

//定义的消息类型宏
#define APPLY_TATTEDCODE_MSG 0x01		//申请随机码消息类型
#define LOGIN_MSG 0x02					//登录消息类型
#define FRI_GROUP_MSG 0x03				//申请好友组列表消息类型
#define FRI_LIST_MSG 0x04				//申请好友列表消息类型
#define USRID_REGISTER_MSG 0x05	   //用户ID注册消息类型
#define CHAT_MSG 0x07					//聊天消息类型
#define ACC_REGISTER_MSG 0x08		//注册(帐号注册)消息类型
#define ADD_FRIEND_GROUP_MSG 0x09	//添加好友组消息类型
#define DEL_FRIEND_GROUP_MSG 0x0A	//删除好友组消息类型
#define ADD_FRINED_MSG		 0x0B	//添加好友消息类型
#define DEL_FRIEND_MSG		 0x0C  //删除好友消息类型
#define CREATE_CLUSTER		0x10	//创建群
#define CLUSTER_LIST		0x11    //请求群列表
#define CLUSTER_MEMBER_LIST 0x12	//请求群成员列表类型
#define INVITE_JOIN_CLUSTER 0x13	//邀请好友加入群消息类型
#define CHANGE_IMAGE 		0x14	//改变头像消息类型
#define USRID_LOGIN_MSG		0x19	//用户ID登录消息
#define ADD_CLUSTER_MEMBER		0x21	//添加群成员消息类型
#define DELETE_CLUSTER_MEMBER   0x22	//删除群成员
#define DISMISS_CLUSTER			0x23	//解散群
#define EXIT_CLUSTER			  0x25	//群成员退群
#define HEARTBEAT_DETECT		  0x28	//心跳包检测
#define APPLICATION_EXIT 0x2A //程序退出

//联通的消息类型宏定义
#define ANDROID_REGISTER_MSG 		0x100			//android注册消息类型
#define IPHONE_REGISTER_MSG 		0x101			//用户注册消息类型
#define REPORT_ANDROID_PHONEINFO	0x102			//报告android手机信息
#define REPORT_IPHONE_PHONEINFO		0x103			//报告IPHONE手机信息
#define PHONE_LOGIN_MSG				0x104			//手机用户登录消息类型
#define CHECK_ANDROID_VERSION    	0x105			//(android)客户端版本升级检查
#define REPORT_VERSION_UPDATE		0x106			//报告软件升级
#define TASKS_SEND_QUERY			0x107			//任务推送查询
#define DEL_TASK_RECORD				0x108			//删除任务记录
#define CLIENT_FEEDBACK				0x109			//客户信息反馈
#define TASK_TOUCH_RECORD			0x10A			//任务点击记录
#define SERVER_ERROR				0x10B			// 服务器错误
#define GET_CHARGE_TYPE 			0x10C			//获取投诉类型
#define RECORD_CHARGE_INFO			0x10D			//记录投诉信息		
#define HEARTBEAT_DEC_PACKET		0x10E			//心跳检测包
#define GET_IMAGE					0x1FF
#define REPORTIPHONE_MOREINFO		0x110			//报告iphone手机等多信息
#define BIND_UUID_SOCKETID			0x111			//绑定socketID
#define NEW_TASK_PUSH				0x112
#define TAG_CLICK_RECORD			0x113
#define PUSHMSG_TYPE				0x114
#define USRSHARE_CLICKRECORD		0x115
#define RECORD_MALFUNC_REPORT 		0x116
#define REQ_HALL_INFO		 		0x117


int YDTProcessFunc(WORD wCmd_id, Client_Server_Msg *pClient_srv_msg, StSequence_Queue *pQueue, pthread_mutex_t *pMutex, pthread_cond_t *pCond);
int LTProcessFunc(WORD wCmd_id, Client_Server_Msg *pClient_srv_msg, StSequence_Queue *pQueue, pthread_mutex_t *pMutex, pthread_cond_t *pCond);
int DefProcessFunc(WORD wCmd_id, Client_Server_Msg *pClient_srv_msg, StSequence_Queue *pQueue, pthread_mutex_t *pMutex, pthread_cond_t *pCond);

#endif
