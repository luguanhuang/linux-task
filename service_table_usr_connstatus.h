
#ifndef SERVICE_TABLE_USR_CONNSTATUS_H
#define SERVICE_TABLE_USR_CONNSTATUS_H


//��ȡ��¼��ʱ������
int GetLoginTmpTableName(int nUsr_id, char *szTableName);

//��ѯ�û��Ƿ��Ѿ���¼
int IsUsrLogin(int nUsr_id);

//�����¼��Ϣ����ʱ����
int InsertLoginInfoIntoDB(int nUsr_id, int nSvr_seq, int nSock);

//ɾ��һ�м�¼
int DeleteItemByUsrId(unsigned int nUsr_id);

#endif
