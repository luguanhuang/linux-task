
#ifndef SERVICE_THREADPOOL_MODE_SERVER
#define SERVICE_THREADPOOL_MODE_SERVER

#include "service_thread_pool.h"
#include "../include/ydt_hash.h"
#include "../include/hash.h"

typedef struct
{
	pthread_mutex_t hash_mutex;
	pthread_cond_t hash_cond;
	StHash_Table *pSave_threadres_hash;				
}StSave_ThreadRes_Hash;

typedef struct
{
	pthread_mutex_t mutex;
	StHash_Table *pAccesssrv_seq_hash;				
}StAccessSrv_Seq_Hash;

int RunThreadPoolModeServer(void);
void SetCancelThreadFlg(void);

int GetAccessSrvConnInfo(Client_Server_Msg *pClient_server_msg, WORD *pCmd_id, WORD *pAccess_srv_seq);


#endif
