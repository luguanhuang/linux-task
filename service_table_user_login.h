

#ifndef SERVICE_TABLE_USR_LOGIN
#define SERVICE_TABLE_USR_LOGIN

/****************该文件保存了对用户登录表的所有操作************************/

//获取登陆表名称
int GetLoginTableName(int nUsr_id, char *szTableName);

//判断帐号和密码是否正确的函数, 如果正确, 则返回对应的用户名
int JudgeAccountAndPasswd(char *pUser_name, char *pPasswd, int *pUsr_id, char *pNickname);


//从登陆表中获取帐号和昵称
int GetAccAndNickname(int nUsr_id, char *szAccount, char *szNickname);

//判断用户名(用户ID)是否重复
int IsTheSameUsrID(int nUsr_id);

//判断用户帐号是否重复
int IsTheSameAccount(char *szAccount);

//插入用户信息
int InsertRegisInfoIntoDB(int nUsr_id, char *szNickname, char *szPasswd, Register_Type regis_type, char *szAccount);

//判断用户ID是否存在
int IsUsrIDExistOrNot(int nUsr_id);

//获取用户昵称
int GetNickname(int nUsr_id, char *pNickname);

//获取用户备注名称
int GetRemarkName(int nUsr_id, char *pRemarkName);

//判断用户名和密码是否正确
int IsUsridAndPasswdCorrect(int nUsr_id, char *pPasswd, char *pNickname);

int GetMaxUsridInTheRange(int nUsrid_start, int nUsrid_end, int *pMax_usrid);


#endif
