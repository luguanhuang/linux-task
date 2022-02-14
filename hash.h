#ifndef HASH_H_H
#define HASH_H_H

//哈希表里面相同的键已经存在
#ifndef HASH_SAMEKEY_EXIST
#define HASH_SAMEKEY_EXIST 0x02
#endif

//哈希表的元素结构
typedef struct _StHash_Item
{	
	char *pKey;							//哈希表的键值
	void *pMatch_msg;						//哈希表的数值
	struct _StHash_Item *pNext;			//哈希表的下一项的指针
}StHash_Item;

//哈希表的结构
typedef struct
{
	int nBucket_size;							//哈希表的大小(类似于链表的头指针大小)
	int nHashtable_len;						//元素个数
	StHash_Item **ppItem;					//哈希表的元素(头指针个数)
	int    (*hash_func)(char *);	 //定义一个函数指针
	pthread_mutex_t hash_mutex;
}StHash_Table;

int HashInit(StHash_Table **t, int size, int (*hash_func)(char *));
StHash_Item *HashGetItem(StHash_Table *pHash_table, char *pKey);		//获取哈希表的一项
void FreeFunc(void * data);
int  HashFunc(char *key);
void *HashData(StHash_Table *t, char *key);
void *HashDel(StHash_Table **t, char *key);
int HashFree(StHash_Table **t, void (*free_func)(void *));
int HashInsert(StHash_Table **t, char *key, void *data);
int IsHashEmpty(StHash_Table *pHash_table);
int HashtableLen(StHash_Table *pHashtable);

#endif
