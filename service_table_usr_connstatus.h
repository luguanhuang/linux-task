
#ifndef SERVICE_TABLE_USR_CONNSTATUS_H
#define SERVICE_TABLE_USR_CONNSTATUS_H


//获取登录临时表名称
int GetLoginTmpTableName(int nUsr_id, char *szTableName);

//查询用户是否已经登录
int IsUsrLogin(int nUsr_id);

//插入登录信息到临时表中
int InsertLoginInfoIntoDB(int nUsr_id, int nSvr_seq, int nSock);

//删除一行记录
int DeleteItemByUsrId(unsigned int nUsr_id);

#endif
