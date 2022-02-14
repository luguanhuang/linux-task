#ifndef LINK_QUEUE_H
#define LINK_QUEUE_H
//约定: 
//队头指针等于队尾指针时，队列为空；
//队头在队尾下一个时，队列为满；

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
