
#ifndef SERVICE_TABLE_CLUSTER_DETAIL_H
#define SERVICE_TABLE_CLUSTER_DETAIL_H


//��ȡȺ�������
int GetClusterDetailTableName(int nUsr_id, char *szTableName);

//��ȡȺ��Ա����
int GetClusterDetailData(int nUsr_id, int nCluster_id, Cluster_Member_Data *pCluster_member_data, WORD *pCount);

//����Ⱥ��Ա
int InsertClusterMember(int nCluster_id, int nUsr_id, BYTE bStatus);

//ɾ��Ⱥ��Ա
int DelClusterMember(int nCluster_id, int nUsr_id);

//ɾ�����е�Ⱥ��Ա
int DelAllClusterMember(int nCluster_id);


//ɾ���û������Ǹ�Ⱥ����������
int GetUsrsId(int nCluster_id, int **ppUsr_id, int *pCount);

#endif
