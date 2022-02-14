#ifndef LINK_QUEUE_H
#define LINK_QUEUE_H
//Լ��: 
//��ͷָ����ڶ�βָ��ʱ������Ϊ�գ�
//��ͷ�ڶ�β��һ��ʱ������Ϊ����

//modify by LiHuangYuan,2011/10/14 modify struct pack
#pragma pack()

typedef struct _StNode
{
	unsigned int nData;
	struct _StNode *pNext;
}StNode;

typedef struct
{
	StNode *pFront;
	StNode *pRear;
	int nQueue_len;
	pthread_mutex_t queue_mutex;
}StLinkQueue;

void InitLinkQueue(StLinkQueue *pLinkQueue);
int InsertItemToQueue(StLinkQueue *pLinkQueue, int nData);
int DelFromQueuetByValue(StLinkQueue *pLinkQueue, int nData);
int DelFromQueuetByItem(StLinkQueue *pLinkQueue, StNode *pNode);
StNode *SearchFromLinkQueue(StLinkQueue *pLinkQueue, int nData);
int LinkQueueLen(StLinkQueue *pLinkQueue);
int IsLinkQueueEmpty(StLinkQueue *pLinkQueue);


#endif
