
#ifndef SERVICE_TABLE_USR_INFO
#define SERVICE_TABLE_USR_INFO


//�û�����Ⱥ��Ҫ�ĵȼ���
#define CLUSTER_LEVEL 8

//��ȡ�û���Ϣ������
int GetUsrInfoTableName(int nUsr_id, char *szTableName);

//��ȡ����ǩ����ͼ������
int GetSigAndPhotoIdx(int nUsr_id, char *szSelfSig, BYTE *pPhoto_idx, EnImage_Type *pImage_tyoe);


//��ȡͼƬ����
int GetPhotoIdx(int nUsr_id, BYTE *pPhoto_idx, EnImage_Type *pImage_type);

//�����û���Ϣ
int InsertUsrInfo(int nUsr_id, BYTE bSex);


//�ж��û��Ƿ����㹻��Ȩ�޴���Ⱥ
int IsEnoughClusterLevel(int nUsr_id);


//����ͼ������
int InsertImageData(int nUsr_id, BYTE *pImage_data, BYTE *pImage_postfix, int nImage_size);



#endif

