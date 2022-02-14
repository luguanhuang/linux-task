
#ifndef SERVICE_TABLE_CLUSTER_DETAIL_H
#define SERVICE_TABLE_CLUSTER_DETAIL_H


//获取群详表名称
int GetClusterDetailTableName(int nUsr_id, char *szTableName);

//获取群成员数据
int GetClusterDetailData(int nUsr_id, int nCluster_id, Cluster_Member_Data *pCluster_member_data, WORD *pCount);

//插入群成员
int InsertClusterMember(int nCluster_id, int nUsr_id, BYTE bStatus);

//删除群成员
int DelClusterMember(int nCluster_id, int nUsr_id);

//删除所有的群成员
int DelAllClusterMember(int nCluster_id);


//删除用户属于那个群的所有数据
int GetUsrsId(int nCluster_id, int **ppUsr_id, int *pCount);

#endif
