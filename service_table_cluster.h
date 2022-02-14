
#ifndef SERVICE_TABLE_CLUSTER_H
#define SERVICE_TABLE_CLUSTER_H

#define CLUSTER_NUM_LEN 6

//生成群号码
int GenerateClusterNum(int *pCluster_num);

//插入群信息
int InsertClusterInfo(char *pCluster_name, int nUsr_id, BYTE bCluster_type, int *pCluster_id);

//获取群ID
int GetClusterId(int *pCluster_id);

//获取群类型名称
int GetClusterTypeName(int nCluster_id, char *pType_name);

//删除群
int DeleteCluster(int nCluster_id);

#endif
