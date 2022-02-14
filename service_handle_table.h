
#ifndef SERVICE_HANDLE_TABLE
#define SERVICE_HANDLE_TABLE

#ifdef _LIANTONG_SERVICE_
#include "service_handle_liantongtable.h"
#endif

#if defined(_WANWEI_LOGIN_SERVICE_) || defined(_WANWEI_QUERY_SERVICE_) || defined(_WANWEI_PUSH_SERVICE_)
#include "service_handle_wanweitable.h"
#endif


/*********************函数名称都是以H 开始的************************************************/

#ifdef _IM_SERVICE_

//判断帐号和密码是否正确的函数, 如果正确, 则返回对应的用户名
int H_JudgeAccountAndPasswd(char *pUser_name, char *pPasswd, int *pUsr_id, char *pNickname);

//获取登录临时表的名称
int H_GetLoginTmpTableName(int nUsr_id, char *szTableName);

//判断用户是否已经登录
int H_IsUsrLogin(int nUsr_id);

//插入登录信息到临时表中
int H_InsertLoginInfoIntoDB(int nUsr_id, int nSvr_seq, int nSock);

//获取好友信息表名称
int H_GetGroupDetailTableName(int nUsr_id, char *szTableName);

//获取登陆表名称
int H_GetLoginTableName(int nUsr_id, char *szTableName);

//获取用户昵称
int H_GetNickname(int nUsr_id, char *szNickname);

//获取用户信息表名称
int H_GetUsrInfoTableName(int nUsr_id, char *szTableName);

//获取个性签名和图像索引
int H_GetSigAndPhotoIdx(int nUsr_id, char *szSelfSig, BYTE *pPhoto_idx, EnImage_Type *pImage_tyoe);

//获取组表名称
int H_GetGroupTableName(int nUsr_id, char *szTableName);

//判断用户名是否重复
int H_IsTheSameUsrID(int nUsr_id);

//插入注册信息
int H_InsertRegisInfoIntoDB(int nUsr_id, char *szNickname, char *szPasswd, Register_Type regis_type, char *szAccount);


//判断用户帐号是否重复
int H_IsTheSameAccount(char *szAccount);

//判断用户ID是否存在
int H_IsUsrIDExistOrNot(int nUsr_id);


//判断组ID是否存在
int H_IsGroupIDExistOrNot(int nUsr_id, int nGroup_id);

//判断好友数据是否存在
int H_IsFriendDataExistOrNot(unsigned int nUsr_id, unsigned int nGroup_id, unsigned int nFriend_id);


//插入好友数据
int H_InsertFriendData(unsigned int nUsr_id, unsigned int nGroup_id, unsigned int nFriend_id);

//插入好友组数据
int H_InsertFriGroupData(unsigned int nGroup_id, unsigned int nUsr_id, char *szGroup_name);

//删除好友组数据
int H_DeleteFriGroupData(unsigned int nGroup_id, unsigned int nUsr_id);

//删除好友数据
int H_DeleteFriendData(unsigned int nGroup_id, unsigned int nUsr_id, unsigned int nFriend_id);

//查找最大的组ID
int H_FindMaxGroupID(unsigned int nUsr_id, unsigned int *pMax_group_id);


//获取群数据: 群ID  群名称
int H_GetClusterData(int nUsr_id, WORD *pCount, Cluster_Data *pCluster_data);

//获取图片索引
int H_GetPhotoIdx(int nUsr_id, BYTE *pPhoto_idx, EnImage_Type *pImage_type);


int H_GetClusterDetailData(int nUsr_id, int nCluster_id, Cluster_Member_Data *pCluster_member_data, WORD *pCount);

//删除记录
int H_DeleteItemByUsrId(unsigned int nUsr_id);

//获取好友信息
int H_GetFriendInfoFromDB(int nUsr_id, int nGroup_id, Fri_List_Data *pFri_list_data, WORD *pCount);

//判断用户是否有足够的创建群的权限
int H_IsEnoughClusterLevel(int nUsr_id);

//插入群信息
int H_InsertClusterInfo(char *pCluster_name, int nUsr_id, BYTE bCluster_type, int *pCluster_id);

//插入用户拥有的群信息
int H_InsertOwnClusterInfo(int nUsr_id, int nCluster_id, char *pCluster_name, BYTE bStatus);

//获取群类型名称
int H_GetClusterTypeName(int nCluster_id, char *pType_name);

//获取好友组信息
int H_GetFriendGroupInfo(int nUsr_id, WORD *pCount, Group_Data *pGroup_data);

//插入群成员
int H_InsertClusterMember(int nCluster_id, int nUsr_id, BYTE bStatus);

//获取群类型名称
int H_GetTypeName(BYTE bCluster_type, char *pType_name);

//插入图像数据
int H_InsertImageData(int nUsr_id, BYTE *pImage_data, BYTE *pImage_postfix, int nImage_size);

//判断用户ID和密码是否正确
int H_IsUsridAndPasswdCorrect(int nUsr_id, char *pPasswd, char *pNickname);

//删除群成员
int H_DelClusterMember(int nCluster_id, int nUsr_id);

//删除用户拥有的群信息
int H_DelOwnClusterInfo(int nUsr_id, int nCluster_id);

//判断用户是否有权限删除群
int H_IsUsrHavePowerToDelCluster(int nUsr_id, int nCluster_id);

//获取群里面所有的用户ID
int H_GetUsrsId(int nCluster_id, int **ppUsr_id, int *pCount);

//删除用户拥有的群
int H_DeleteUsrsOwnCluster(int nUsrs_id, int nCluster_id);

//删除所有的群成员
int H_DelAllClusterMember(int nCluster_id);

//删除群
int H_DeleteCluster(int nCluster_id);

//插入用户信息
int H_InsertUsrInfo(int nUsr_id, BYTE bSex);

int H_GetGroupNum(int nUsr_id, int *pGroup_num);

int H_GetMaxUsridInTheRange(int nUsrid_start, int nUsrid_end, int *pMax_usrid);


#endif

//公共的表
int H_GetRegisterStartIDInfo(void);


#endif

