#ifndef HASH_H_H
#define HASH_H_H

//��ϣ��������ͬ�ļ��Ѿ�����
#ifndef HASH_SAMEKEY_EXIST
#define HASH_SAMEKEY_EXIST 0x02
#endif

//��ϣ���Ԫ�ؽṹ
typedef struct _StHash_Item
{	
	char *pKey;							//��ϣ��ļ�ֵ
	void *pMatch_msg;						//��ϣ�����ֵ
	struct _StHash_Item *pNext;			//��ϣ�����һ���ָ��
}StHash_Item;

//��ϣ��Ľṹ
typedef struct
{
	int nBucket_size;							//��ϣ��Ĵ�С(�����������ͷָ���С)
	int nHashtable_len;						//Ԫ�ظ���
	StHash_Item **ppItem;					//��ϣ���Ԫ��(ͷָ�����)
	int    (*hash_func)(char *);	 //����һ������ָ��
	pthread_mutex_t hash_mutex;
}StHash_Table;

int HashInit(StHash_Table **t, int size, int (*hash_func)(char *));
StHash_Item *HashGetItem(StHash_Table *pHash_table, char *pKey);		//��ȡ��ϣ���һ��
void FreeFunc(void * data);
int  HashFunc(char *key);
void *HashData(StHash_Table *t, char *key);
void *HashDel(StHash_Table **t, char *key);
int HashFree(StHash_Table **t, void (*free_func)(void *));
int HashInsert(StHash_Table **t, char *key, void *data);
int IsHashEmpty(StHash_Table *pHash_table);
int HashtableLen(StHash_Table *pHashtable);

#endif
