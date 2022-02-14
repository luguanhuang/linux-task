
#include "access_global.h"
//#include "../include/ydt_log.h"
#include "../interface/access_protocol.h"

/*该文件主要是定义了一些公用的全局变量与全局函数
  *
  */

extern Server_Conf_Info g_srv_conf_info;

//函数用途: 获取程序的当前路径
//输入参数:  无
//输出参数:  存放程序名的缓存
//返回值	: 返回TRUE

int GetProgramPath(char *pPath)
{
	char szPath[512] = {0};
	getcwd(szPath, sizeof(szPath));

	strcpy(pPath, szPath);
	return TRUE;
}



