
#ifndef SERVICE_TABLE_FRIEND_GROUP
#define SERVICE_TABLE_FRIEND_GROUP


//��ȡ�������
int GetGroupTableName(int nUsr_id, char *szTableName);

//�ж���ID�Ƿ����
int IsGroupIDExistOrNot(int nUsr_id, int nGroup_id);

//�������������
int InsertFriGroupData(unsigned int nGroup_id, unsigned int nUsr_id, char *szGroup_name);

//ɾ������������
int DeleteFriGroupData(unsigned int nGroup_id, unsigned int nUsr_id);

//����������ID
int FindMaxGroupID(unsigned int nUsr_id, unsigned int *pMax_group_id);

//��ȡ��������Ϣ
int GetFriendGroupInfo(int nUsr_id, WORD *pCount, Group_Data *pGroup_data);

//��ȡ������
int GetGroupNum(int nUsr_id, int *pGroup_num);

#endif
