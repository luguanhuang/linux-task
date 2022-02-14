
#ifndef SERVICE_TABLE_GROUP_DETAIL_H
#define SERVICE_TABLE_GROUP_DETAIL_H

//用户状态
#define USR_ONLINE 0x00					//用户处于在线状态
#define USR_OFFLINE 0x01				//用户处于离线状态

//获取文件名称
int GetGroupDetailTableName(int nUsr_id, char *szTableName);

//判断好友数据是否存在
int IsFriendDataExistOrNot(unsigned int nUsr_id, unsigned int nGroup_id, unsigned int nFriend_id);

//插入好友数据
int InsertFriendData(unsigned int nUsr_id, unsigned int nGroup_id, unsigned int nFriend_id);

//删除好友数据
int DeleteFriendData(unsigned int nGroup_id, unsigned int nUsr_id, unsigned int nFriend_id);


//获取用户信息
int GetFriendInfoFromDB(int nUsr_id, int nGroup_id, Fri_List_Data *pFri_list_data, WORD *pCount);


#endif
