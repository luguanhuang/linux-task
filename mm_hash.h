#ifndef MM_HASH_H
#define MM_HASH_H

#include <pthread.h>


#if !defined(TRUE)
#define TRUE	1
#endif

#if !defined(FALSE)
#define FALSE	0
#endif



//��ϣ��������ͬ�ļ��Ѿ�����
#define MM_HASH_SAMEKEY_EXIST 0x02
//#define MY_FREE(p) if(p){free(p);p=0;}
#define FREEIF(p) if(p){free(p);p=0;}


//��ϣ���Ԫ�ؽṹ
typedef struct MM_hashitem
{	
	char *key;							//��ϣ��ļ�ֵ
	void *matchMsg;						//��ϣ�����ֵ
	struct MM_hashitem *next;			//��ϣ�����һ���ָ��
}MM_HashItem;

//��ϣ��Ľṹ
typedef struct MM_hashtable 
{
	int    bucketsize;							//��ϣ��Ĵ�С(�����������ͷָ���С)
	MM_HashItem  **item;					//��ϣ���Ԫ��(ͷָ�����)
	int    (*hash_func)( char *);	 //����һ������ָ�� 
	
	pthread_mutex_t hash_mutex;
	
}MM_HashTable; 

int MM_Hash_Init (MM_HashTable **t, int bucketsize, int (*hash_func)(char *));
int MM_Hash_Destroy (MM_HashTable **t, void (*free_func)(void *));
int MM_Is_Hash_Empty(MM_HashTable *t);
int MM_Hash_Count(MM_HashTable *pHashtable);
int MM_Hash_Insert (MM_HashTable **t, char *key, void *data);
void *MM_Hash_Del (MM_HashTable **t, char *key);
MM_HashItem *MM_Hash_GetHashItem(MM_HashTable *pHash_table, char *pKey);//��ȡ��ϣ���һ��
void *MM_Hash_GetHashItemData ( MM_HashTable *t, char *key);



#endif
