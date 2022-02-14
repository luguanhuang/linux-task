#ifndef SEQUENCE_QUEUE_H
#define SEQUENCE_QUEUE_H
//约定: 
//队头指针等于队尾指针时，队列为空；
//队头在队尾下一个时，队列为满；

//modify by LiHuangYuan,2011/10/14 modify struct pack
#pragma pack()

typedef void (*PFreeQueueItem)(void *);

//消息队列结构
typedef struct
{
	void **ppBase;//数组名
	int nFront;//头指针
	int nRear;//尾指针	
	int nQueue_size;
	int nQueue_len;
	pthread_mutex_t queue_mutex;
	PFreeQueueItem pFreeQueueItem;
}StSequence_Queue;

int InitSeqQueue(StSequence_Queue *pQueue, int nQueue_size, PFreeQueueItem pFreeQueueItem); //初始化队列
int SeqQueueLength(StSequence_Queue queue);//队列长度
int PutMsgToSeqQueue(StSequence_Queue *pQueue, void *pMsg);//进队
int GetMsgFromSeqQueue (StSequence_Queue *pQueue, void **ppMsg);//出队
int IsSeqQueueEmpty (StSequence_Queue *pQueue);//判断队列是否为空
//int IsSeqQueueFull (StSequence_Queue *pQueue); //判断队列是否为满
int DestroySeqQueue (StSequence_Queue *pQueue);//销毁队列
void DisplayQueue (StSequence_Queue *pQueue);//显示队列的所有数据
#endif
