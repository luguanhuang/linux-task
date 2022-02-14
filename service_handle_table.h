
#ifndef SERVICE_HANDLE_TABLE
#define SERVICE_HANDLE_TABLE

#ifdef _LIANTONG_SERVICE_
#include "service_handle_liantongtable.h"
#endif

#if defined(_WANWEI_LOGIN_SERVICE_) || defined(_WANWEI_QUERY_SERVICE_) || defined(_WANWEI_PUSH_SERVICE_)
#include "service_handle_wanweitable.h"
#endif


/*********************�������ƶ�����H ��ʼ��************************************************/

#ifdef _IM_SERVICE_

//�ж��ʺź������Ƿ���ȷ�ĺ���, �����ȷ, �򷵻ض�Ӧ���û���
int H_JudgeAccountAndPasswd(char *pUser_name, char *pPasswd, int *pUsr_id, char *pNickname);

//��ȡ��¼��ʱ�������
int H_GetLoginTmpTableName(int nUsr_id, char *szTableName);

//�ж��û��Ƿ��Ѿ���¼
int H_IsUsrLogin(int nUsr_id);

//�����¼��Ϣ����ʱ����
int H_InsertLoginInfoIntoDB(int nUsr_id, int nSvr_seq, int nSock);

//��ȡ������Ϣ������
int H_GetGroupDetailTableName(int nUsr_id, char *szTableName);

//��ȡ��½������
int H_GetLoginTableName(int nUsr_id, char *szTableName);

//��ȡ�û��ǳ�
int H_GetNickname(int nUsr_id, char *szNickname);

//��ȡ�û���Ϣ������
int H_GetUsrInfoTableName(int nUsr_id, char *szTableName);

//��ȡ����ǩ����ͼ������
int H_GetSigAndPhotoIdx(int nUsr_id, char *szSelfSig, BYTE *pPhoto_idx, EnImage_Type *pImage_tyoe);

//��ȡ�������
int H_GetGroupTableName(int nUsr_id, char *szTableName);

//�ж��û����Ƿ��ظ�
int H_IsTheSameUsrID(int nUsr_id);

//����ע����Ϣ
int H_InsertRegisInfoIntoDB(int nUsr_id, char *szNickname, char *szPasswd, Register_Type regis_type, char *szAccount);


//�ж��û��ʺ��Ƿ��ظ�
int H_IsTheSameAccount(char *szAccount);

//�ж��û�ID�Ƿ����
int H_IsUsrIDExistOrNot(int nUsr_id);


//�ж���ID�Ƿ����
int H_IsGroupIDExistOrNot(int nUsr_id, int nGroup_id);

//�жϺ��������Ƿ����
int H_IsFriendDataExistOrNot(unsigned int nUsr_id, unsigned int nGroup_id, unsigned int nFriend_id);


//�����������
int H_InsertFriendData(unsigned int nUsr_id, unsigned int nGroup_id, unsigned int nFriend_id);

//�������������
int H_InsertFriGroupData(unsigned int nGroup_id, unsigned int nUsr_id, char *szGroup_name);

//ɾ������������
int H_DeleteFriGroupData(unsigned int nGroup_id, unsigned int nUsr_id);

//ɾ����������
int H_DeleteFriendData(unsigned int nGroup_id, unsigned int nUsr_id, unsigned int nFriend_id);

//����������ID
int H_FindMaxGroupID(unsigned int nUsr_id, unsigned int *pMax_group_id);


//��ȡȺ����: ȺID  Ⱥ����
int H_GetClusterData(int nUsr_id, WORD *pCount, Cluster_Data *pCluster_data);

//��ȡͼƬ����
int H_GetPhotoIdx(int nUsr_id, BYTE *pPhoto_idx, EnImage_Type *pImage_type);


int H_GetClusterDetailData(int nUsr_id, int nCluster_id, Cluster_Member_Data *pCluster_member_data, WORD *pCount);

//ɾ����¼
int H_DeleteItemByUsrId(unsigned int nUsr_id);

//��ȡ������Ϣ
int H_GetFriendInfoFromDB(int nUsr_id, int nGroup_id, Fri_List_Data *pFri_list_data, WORD *pCount);

//�ж��û��Ƿ����㹻�Ĵ���Ⱥ��Ȩ��
int H_IsEnoughClusterLevel(int nUsr_id);

//����Ⱥ��Ϣ
int H_InsertClusterInfo(char *pCluster_name, int nUsr_id, BYTE bCluster_type, int *pCluster_id);

//�����û�ӵ�е�Ⱥ��Ϣ
int H_InsertOwnClusterInfo(int nUsr_id, int nCluster_id, char *pCluster_name, BYTE bStatus);

//��ȡȺ��������
int H_GetClusterTypeName(int nCluster_id, char *pType_name);

//��ȡ��������Ϣ
int H_GetFriendGroupInfo(int nUsr_id, WORD *pCount, Group_Data *pGroup_data);

//����Ⱥ��Ա
int H_InsertClusterMember(int nCluster_id, int nUsr_id, BYTE bStatus);

//��ȡȺ��������
int H_GetTypeName(BYTE bCluster_type, char *pType_name);

//����ͼ������
int H_InsertImageData(int nUsr_id, BYTE *pImage_data, BYTE *pImage_postfix, int nImage_size);

//�ж��û�ID�������Ƿ���ȷ
int H_IsUsridAndPasswdCorrect(int nUsr_id, char *pPasswd, char *pNickname);

//ɾ��Ⱥ��Ա
int H_DelClusterMember(int nCluster_id, int nUsr_id);

//ɾ���û�ӵ�е�Ⱥ��Ϣ
int H_DelOwnClusterInfo(int nUsr_id, int nCluster_id);

//�ж��û��Ƿ���Ȩ��ɾ��Ⱥ
int H_IsUsrHavePowerToDelCluster(int nUsr_id, int nCluster_id);

//��ȡȺ�������е��û�ID
int H_GetUsrsId(int nCluster_id, int **ppUsr_id, int *pCount);

//ɾ���û�ӵ�е�Ⱥ
int H_DeleteUsrsOwnCluster(int nUsrs_id, int nCluster_id);

//ɾ�����е�Ⱥ��Ա
int H_DelAllClusterMember(int nCluster_id);

//ɾ��Ⱥ
int H_DeleteCluster(int nCluster_id);

//�����û���Ϣ
int H_InsertUsrInfo(int nUsr_id, BYTE bSex);

int H_GetGroupNum(int nUsr_id, int *pGroup_num);

int H_GetMaxUsridInTheRange(int nUsrid_start, int nUsrid_end, int *pMax_usrid);


#endif

//�����ı�
int H_GetRegisterStartIDInfo(void);


#endif

