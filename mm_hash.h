#ifndef MM_HASH_H
#define MM_HASH_H

#include <pthread.h>


#if !defined(TRUE)
#define TRUE	1
#endif

#if !defined(FALSE)
#define FALSE	0
#endif



//哈希表里面相同的键已经存在
#define MM_HASH_SAMEKEY_EXIST 0x02
//#define MY_FREE(p) if(p){free(p);p=0;}
#define FREEIF(p) if(p){free(p);p=0;}


//哈希表的元素结构
typedef struct MM_hashitem
{	
	char *key;							//哈希表的键值
	void *matchMsg;						//哈希表的数值
	struct MM_hashitem *next;			//哈希表的下一项的指针
}MM_HashItem;

//哈希表的结构
typedef struct MM_hashtable 
{
	int    bucketsize;							//哈希表的大小(类似于链表的头指针大小)
	MM_HashItem  **item;					//哈希表的元素(头指针个数)
	int    (*hash_func)( char *);	 //定义一个函数指针 
	
	pthread_mutex_t hash_mutex;
	
}MM_HashTable; 

int MM_Hash_Init (MM_HashTable **t, int bucketsize, int (*hash_func)(char *));
int MM_Hash_Destroy (MM_HashTable **t, void (*free_func)(void *));
int MM_Is_Hash_Empty(MM_HashTable *t);
int MM_Hash_Count(MM_HashTable *pHashtable);
int MM_Hash_Insert (MM_HashTable **t, char *key, void *data);
void *MM_Hash_Del (MM_HashTable **t, char *key);
MM_HashItem *MM_Hash_GetHashItem(MM_HashTable *pHash_table, char *pKey);//获取哈希表的一项
void *MM_Hash_GetHashItemData ( MM_HashTable *t, char *key);



#endif
