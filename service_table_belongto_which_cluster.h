
#ifndef SERVICE_TABLE_BELONG_TO_CLUSTER
#define SERVICE_TABLE_BELONG_TO_CLUSTER


//��ȡȺ�б�����
int GetBelongToClusterTableName(int nUsr_id, char *szTableName);

//��ȡ����Ⱥ��Ϣ
int GetClusterData(int nUsr_id, WORD *pCount, Cluster_Data *pCluster_data);

//�����û�ӵ�е�Ⱥ��Ϣ
int InsertOwnClusterInfo(int nUsr_id, int nCluster_id, char *pCluster_name, BYTE bStatus);


//ɾ���û�ӵ�е�Ⱥ��Ϣ
int DelOwnClusterInfo(int nUsr_id, int nCluster_id);

//�ж��û��Ƿ���Ȩ��ɾ��Ⱥ
int IsUsrHavePowerToDelCluster(int nUsr_id, int nCluster_id);

//ɾ���û����ڵ�ͬһȺ
int DeleteUsrsOwnCluster(int nUsr_id, int nCluster_id);

#endif
