
#include "access_global.h"
//#include "../include/ydt_log.h"
#include "../interface/access_protocol.h"

/*���ļ���Ҫ�Ƕ�����һЩ���õ�ȫ�ֱ�����ȫ�ֺ���
  *
  */

extern Server_Conf_Info g_srv_conf_info;

//������;: ��ȡ����ĵ�ǰ·��
//�������:  ��
//�������:  ��ų������Ļ���
//����ֵ	: ����TRUE

int GetProgramPath(char *pPath)
{
	char szPath[512] = {0};
	getcwd(szPath, sizeof(szPath));

	strcpy(pPath, szPath);
	return TRUE;
}



