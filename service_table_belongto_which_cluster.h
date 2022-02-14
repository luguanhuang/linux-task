
#ifndef SERVICE_TABLE_BELONG_TO_CLUSTER
#define SERVICE_TABLE_BELONG_TO_CLUSTER


//获取群列表名称
int GetBelongToClusterTableName(int nUsr_id, char *szTableName);

//获取各个群信息
int GetClusterData(int nUsr_id, WORD *pCount, Cluster_Data *pCluster_data);

//插入用户拥有的群信息
int InsertOwnClusterInfo(int nUsr_id, int nCluster_id, char *pCluster_name, BYTE bStatus);


//删除用户拥有的群信息
int DelOwnClusterInfo(int nUsr_id, int nCluster_id);

//判断用户是否有权限删除群
int IsUsrHavePowerToDelCluster(int nUsr_id, int nCluster_id);

//删除用户属于的同一群
int DeleteUsrsOwnCluster(int nUsr_id, int nCluster_id);

#endif
