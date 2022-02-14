
#ifndef SERVICE_TABLE_GROUP_DETAIL_H
#define SERVICE_TABLE_GROUP_DETAIL_H

//�û�״̬
#define USR_ONLINE 0x00					//�û���������״̬
#define USR_OFFLINE 0x01				//�û���������״̬

//��ȡ�ļ�����
int GetGroupDetailTableName(int nUsr_id, char *szTableName);

//�жϺ��������Ƿ����
int IsFriendDataExistOrNot(unsigned int nUsr_id, unsigned int nGroup_id, unsigned int nFriend_id);

//�����������
int InsertFriendData(unsigned int nUsr_id, unsigned int nGroup_id, unsigned int nFriend_id);

//ɾ����������
int DeleteFriendData(unsigned int nGroup_id, unsigned int nUsr_id, unsigned int nFriend_id);


//��ȡ�û���Ϣ
int GetFriendInfoFromDB(int nUsr_id, int nGroup_id, Fri_List_Data *pFri_list_data, WORD *pCount);


#endif
