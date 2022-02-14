#ifndef SEQUENCE_QUEUE_H
#define SEQUENCE_QUEUE_H
//Լ��: 
//��ͷָ����ڶ�βָ��ʱ������Ϊ�գ�
//��ͷ�ڶ�β��һ��ʱ������Ϊ����

//modify by LiHuangYuan,2011/10/14 modify struct pack
#pragma pack()

typedef void (*PFreeQueueItem)(void *);

//��Ϣ���нṹ
typedef struct
{
	void **ppBase;//������
	int nFront;//ͷָ��
	int nRear;//βָ��	
	int nQueue_size;
	int nQueue_len;
	pthread_mutex_t queue_mutex;
	PFreeQueueItem pFreeQueueItem;
}StSequence_Queue;

int InitSeqQueue(StSequence_Queue *pQueue, int nQueue_size, PFreeQueueItem pFreeQueueItem); //��ʼ������
int SeqQueueLength(StSequence_Queue queue);//���г���
int PutMsgToSeqQueue(StSequence_Queue *pQueue, void *pMsg);//����
int GetMsgFromSeqQueue (StSequence_Queue *pQueue, void **ppMsg);//����
int IsSeqQueueEmpty (StSequence_Queue *pQueue);//�ж϶����Ƿ�Ϊ��
//int IsSeqQueueFull (StSequence_Queue *pQueue); //�ж϶����Ƿ�Ϊ��
int DestroySeqQueue (StSequence_Queue *pQueue);//���ٶ���
void DisplayQueue (StSequence_Queue *pQueue);//��ʾ���е���������
#endif
