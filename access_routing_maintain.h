	
#ifndef ACCESS_ROUTING_MAINTAIN_H
#define ACCESS_ROUTING_MAINTAIN_H

#include "../util/access_global.h"

/*********���ļ���Ҫ��������·��ά���̵߳�*********/

//����ҵ���������Ϣ���̲߳����ṹ
typedef struct
{
	int nSock;							//�����������ҵ�������֮�������socket id
	BYTE bMain_service_code;			//��ҵ����
	BYTE bSub_service_code;				//��ҵ����
	WORD wService_srv_seq;				//ҵ������������
}Recv_Thread_Param;

//�˽ṹ��������ά���߳�������ҵ�������������
typedef struct
{
	pthread_mutex_t awake_flg_mutex;			//�����̻߳������
	unsigned int nAwake_thread_flg;				//�����̱߳�־
}Awake_Thread_Flg;

typedef struct
{
	pthread_mutex_t routing_mutex;				//·��ά���������
	StHash_Table *pRouting_table;				//·��ά����ϣ��	
}StRouting_Table;

//·��ά���߳�
void *RoutingMaintainThread(void *);

#endif
