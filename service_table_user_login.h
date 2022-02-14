

#ifndef SERVICE_TABLE_USR_LOGIN
#define SERVICE_TABLE_USR_LOGIN

/****************���ļ������˶��û���¼������в���************************/

//��ȡ��½������
int GetLoginTableName(int nUsr_id, char *szTableName);

//�ж��ʺź������Ƿ���ȷ�ĺ���, �����ȷ, �򷵻ض�Ӧ���û���
int JudgeAccountAndPasswd(char *pUser_name, char *pPasswd, int *pUsr_id, char *pNickname);


//�ӵ�½���л�ȡ�ʺź��ǳ�
int GetAccAndNickname(int nUsr_id, char *szAccount, char *szNickname);

//�ж��û���(�û�ID)�Ƿ��ظ�
int IsTheSameUsrID(int nUsr_id);

//�ж��û��ʺ��Ƿ��ظ�
int IsTheSameAccount(char *szAccount);

//�����û���Ϣ
int InsertRegisInfoIntoDB(int nUsr_id, char *szNickname, char *szPasswd, Register_Type regis_type, char *szAccount);

//�ж��û�ID�Ƿ����
int IsUsrIDExistOrNot(int nUsr_id);

//��ȡ�û��ǳ�
int GetNickname(int nUsr_id, char *pNickname);

//��ȡ�û���ע����
int GetRemarkName(int nUsr_id, char *pRemarkName);

//�ж��û����������Ƿ���ȷ
int IsUsridAndPasswdCorrect(int nUsr_id, char *pPasswd, char *pNickname);

int GetMaxUsridInTheRange(int nUsrid_start, int nUsrid_end, int *pMax_usrid);


#endif
