
#ifndef SERVICE_TABLE_CLUSTER_H
#define SERVICE_TABLE_CLUSTER_H

#define CLUSTER_NUM_LEN 6

//����Ⱥ����
int GenerateClusterNum(int *pCluster_num);

//����Ⱥ��Ϣ
int InsertClusterInfo(char *pCluster_name, int nUsr_id, BYTE bCluster_type, int *pCluster_id);

//��ȡȺID
int GetClusterId(int *pCluster_id);

//��ȡȺ��������
int GetClusterTypeName(int nCluster_id, char *pType_name);

//ɾ��Ⱥ
int DeleteCluster(int nCluster_id);

#endif
