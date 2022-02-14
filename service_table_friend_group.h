
#ifndef SERVICE_TABLE_FRIEND_GROUP
#define SERVICE_TABLE_FRIEND_GROUP


//获取组表名称
int GetGroupTableName(int nUsr_id, char *szTableName);

//判断组ID是否存在
int IsGroupIDExistOrNot(int nUsr_id, int nGroup_id);

//插入好友组数据
int InsertFriGroupData(unsigned int nGroup_id, unsigned int nUsr_id, char *szGroup_name);

//删除好友组数据
int DeleteFriGroupData(unsigned int nGroup_id, unsigned int nUsr_id);

//查找最大的组ID
int FindMaxGroupID(unsigned int nUsr_id, unsigned int *pMax_group_id);

//获取好友组信息
int GetFriendGroupInfo(int nUsr_id, WORD *pCount, Group_Data *pGroup_data);

//获取组数量
int GetGroupNum(int nUsr_id, int *pGroup_num);

#endif
