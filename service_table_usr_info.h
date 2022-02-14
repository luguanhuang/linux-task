
#ifndef SERVICE_TABLE_USR_INFO
#define SERVICE_TABLE_USR_INFO


//用户创建群需要的等级数
#define CLUSTER_LEVEL 8

//获取用户信息表名称
int GetUsrInfoTableName(int nUsr_id, char *szTableName);

//获取个性签名和图像索引
int GetSigAndPhotoIdx(int nUsr_id, char *szSelfSig, BYTE *pPhoto_idx, EnImage_Type *pImage_tyoe);


//获取图片索引
int GetPhotoIdx(int nUsr_id, BYTE *pPhoto_idx, EnImage_Type *pImage_type);

//插入用户信息
int InsertUsrInfo(int nUsr_id, BYTE bSex);


//判断用户是否有足够的权限创建群
int IsEnoughClusterLevel(int nUsr_id);


//插入图像数据
int InsertImageData(int nUsr_id, BYTE *pImage_data, BYTE *pImage_postfix, int nImage_size);



#endif

