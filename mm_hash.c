
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#include "service_global.h"
extern "C"
{
	#include "mm_hash.h"	
}


//�ֽڶ���
//modify by LiHuangYuan,2011/10/14 modify struct pack
//#pragma pack(1)



/******���ļ���Ҫ�ǲ�����ϣ��ĸ��������**************/

//������;: ��ȡ��ϣ������±�
//�������: ��ֵ
//�������: ��
//����ֵ	: ��ϣ������±�

static int MM_Default_Hash_Func (char *pKey) 
{
	unsigned int nIdx = 0; 
	int rint=0;
	
	while (*pKey) 
	{
		nIdx ^= *pKey++;
		rint = nIdx%3;
		nIdx  <<= (rint==0)?1:((rint==1)?2:3);
	}
	
	//printf("\n--------MM_Default_Hash_Func nIdx=%d--------\n\n",nIdx);
	
	return nIdx;
}



//������;: ��ʼ����ϣ��
//�������: ��ϣ��ָ���ָ��,  ��ϣ��Ĵ�С,  ��ϣ��Ĭ�ϵĺ���ָ��
//�������: ��
//����ֵ	: ��ʼ���ɹ�,   ����TRUE�� ��ʼ��ʧ��,  ����FALSE

int MM_Hash_Init ( MM_HashTable **t, int bucketsize, int (* hash_func)(  char *) ) 
{
	if ( bucketsize <= 0 )
	{
		printf("MM_Hash_Init: bucketsize<=0 Param error\n" );
		return FALSE;   
	}	
	
	
	(*t) = (MM_HashTable *)malloc(sizeof(MM_HashTable));
	if ( (*t) == NULL )
	{
		printf("MM_Hash_Init: malloc MM_HashTable error\n" );
		return FALSE;
	}
	
	if (pthread_mutex_init(&(*t)->hash_mutex, NULL) != 0)
	{
		printf("MM_Hash_Init: Call pthread_mutex_init error");
		FREEIF((*t));
		return FALSE;
	}
	
	memset((*t), 0, sizeof(MM_HashTable));
	(*t)->bucketsize = bucketsize;
	(*t)->item = (MM_HashItem **)malloc(sizeof(MM_HashItem *)*bucketsize);
	if ( (*t)->item == NULL ) 
	{
		(*t)->bucketsize = 0;
		FREEIF((*t));
		printf("MM_Hash_Init: malloc sizeof(MM_HashItem *)*bucketsize error\n" );
		return FALSE;
	}
	
	memset((*t)->item, 0, sizeof(MM_HashItem *) * bucketsize);
	
	int i = 0;
	for (i=0; i<bucketsize; i++)
	{
		(*t)->item[i] = NULL;
	}
	
	if (hash_func)
	{
		(*t)->hash_func = hash_func;
	}
	else
	{
		(*t)->hash_func = MM_Default_Hash_Func;  
	}
	
	return TRUE;
}



//������;: �ͷ�������ϣ��̬������ڴ�
//�������: ��ϣ��ָ���ָ��, ��ϣ��Ĭ�ϵĺ���ָ��
//�������: ��
//����ֵ	: �ͷ��ڴ�ɹ�,  ����TRUE��  �ͷ�ʧ��,  ����FALSE
int MM_Hash_Destroy ( MM_HashTable **t, void (*free_func)(void *) )
{
	int i;
	MM_HashItem *h, *next = NULL;
	
	if ((*t) == NULL || (*t)->bucketsize <= 0 || (*t)->hash_func == NULL)
	{
		printf("MM_Hash_Destroy: Param error\n" );
		return FALSE; 
	}
	
	for ( i = 0; i < (*t)->bucketsize; i++ ) 
	{
		
		for ( h = (*t)->item[i]; h != NULL; h = next ) 
		{
			next = h->next;
			FREEIF(h->key)
			if (free_func)
			{
				free_func(h->matchMsg);
			}
			else {
				//����ͷţ�
				//FREEIF(h->matchMsg);
			}

			FREEIF(h);
		}
	}
	
	FREEIF((*t)->item);   
	(*t)->bucketsize = 0;
	pthread_mutex_destroy(&(*t)->hash_mutex);
	FREEIF((*t));
	return TRUE;
}

//������;: �жϹ�ϣ���Ƿ�Ϊ��
//�������: ��ϣ���ָ��
//�������: ��
//����ֵ	: ���Ϊ��,   ����TRUE�� �����Ϊ��,  ����FALSE
int MM_Is_Hash_Empty(MM_HashTable *pHashtable)
{
	int i = 0;
	
	pthread_mutex_lock(&pHashtable->hash_mutex);
	for (i=0; i<pHashtable->bucketsize; i++)
	{
		if (pHashtable->item[i] != NULL)
		{
			pthread_mutex_unlock(&pHashtable->hash_mutex);
			return FALSE;
		}
	}
	pthread_mutex_unlock(&pHashtable->hash_mutex);
	return TRUE;
}


int MM_Hash_Count(MM_HashTable *pHashtable)
{
	int i = 0;
	MM_HashItem *pItem = NULL;
	int sum = 0;
	
	pthread_mutex_lock(&pHashtable->hash_mutex);
	for (i=0; i<pHashtable->bucketsize; i++)
	{
		pItem = pHashtable->item[i];
		while (pItem != NULL)
		{
			sum++;
			pItem = pItem->next;
		}
	}
	pthread_mutex_unlock(&pHashtable->hash_mutex);
	
	return sum;
}



//������;: �����ϣ�������һ��
//�������: ��ϣ��ָ�� ��ָ��,  ��ϣ��ļ�ָ��, ��ϣ�����ֵָ��
//�������: ��
//����ֵ	: ����ɹ�,  ����TRUE,  ����ʧ��,  ����FALSE
//��ע: ��Ϊ���һ���ڵ�û�м����Ƿ���ͬ,  ���޸�����
int MM_Hash_Insert(MM_HashTable **t,   char *key, void *data) 
{
	unsigned int index;
	MM_HashItem *h;
	
	if ((*t)== NULL || (*t)->bucketsize <= 0 || (*t)->hash_func == NULL || key == NULL || data == NULL )
	{	
		printf("MM_Hash_Insert: Param error\n" );
		return FALSE;
	}
	
	pthread_mutex_lock(&(*t)->hash_mutex);
	
	index = (*t)->hash_func(key);
	index %= (*t)->bucketsize;
	
	/* if index is free, fill it */
	if ( (*t)->item[index] == NULL ) 
	{ 
		(*t)->item[index] = (MM_HashItem *)malloc(sizeof(MM_HashItem));
		if ( (*t)->item[index] == NULL )
		{
			printf("MM_Hash_Insert: malloc MM_HashItem error 1\n" );
			pthread_mutex_unlock(&(*t)->hash_mutex);
			return FALSE;
		}
		
		(*t)->item[index]->key = strdup(key);
		if ( !(*t)->item[index]->key )
		{
			printf("MM_Hash_Insert: strdup key error 1\n" );
			FREEIF((*t)->item[index]);
			pthread_mutex_unlock(&(*t)->hash_mutex);
			return FALSE;
		}
		
		(*t)->item[index]->matchMsg = data;
		(*t)->item[index]->next = NULL;
		
		pthread_mutex_unlock(&(*t)->hash_mutex);
		return TRUE;
	}
	
	
	/* no, then find if the key already exists, and reach the last link */
	for ( h = (*t)->item[index]; h->next != NULL; h = h->next )
	{
		if ( strcmp(key, h->key) == 0 )
		{	
			printf("MM_Hash_Insert : the same key already exists\n" );
			pthread_mutex_unlock(&(*t)->hash_mutex);
			return MM_HASH_SAMEKEY_EXIST; 
		}
	}
	
	if (0 == strcmp(key, h->key))
	{
		printf("MM_Hash_Insert : the same key already exists\n" );
		pthread_mutex_unlock(&(*t)->hash_mutex);
		return MM_HASH_SAMEKEY_EXIST; 
	}
	
	h->next = (MM_HashItem *)malloc(sizeof(MM_HashItem));
	if ( h->next == NULL )
	{
		printf("MM_Hash_Insert: malloc MM_HashItem error 2\n" );
		pthread_mutex_unlock(&(*t)->hash_mutex);
		return FALSE;  
	}
	
	h->next->key = strdup(key);
	if ( !h->next->key )
	{
		printf("MM_Hash_Insert: strdup key error 2\n" );
		FREEIF(h->next);
		pthread_mutex_unlock(&(*t)->hash_mutex);
		return FALSE;
	}
	
	h->next->matchMsg = data;
	h->next->next = NULL;
	pthread_mutex_unlock(&(*t)->hash_mutex);
	return TRUE;
}


//������;: ɾ����ϣ�������һ��
//�������: ��ϣ��ָ���ָ��,  ��ϣ��ļ�ָ��
//�������: ��
//����ֵ	: ɾ���ɹ�,  ����ɾ������ֵ��ַ,   ɾ��ʧ��,  ����NULL
void* MM_Hash_Del(MM_HashTable **t,   char *key) 
{
	unsigned int index;
	void *data;
	MM_HashItem *h, *last = NULL;
	
	if ( (*t) == NULL || (*t)->bucketsize <= 0 || (*t)->hash_func == NULL || key == NULL )
	{
		printf("MM_Hash_Del: null key or hashtable have a problem\n" );
		return NULL; 
	}
	
	pthread_mutex_lock(&(*t)->hash_mutex);
	index = (*t)->hash_func(key);
	index %= (*t)->bucketsize;
	
	for ( h = (*t)->item[index]; h != NULL; h = h->next ) 
	{
		
		if ( strcmp(key, h->key) == 0 ) 
		{
			/* fix list */
			if (last)
			{
				last->next = h->next;
			}
			else /* is first item */
			{
				(*t)->item[index] = h->next;
			}
			
			FREEIF(h->key);
			data = h->matchMsg;	
			FREEIF(h);
			
			pthread_mutex_unlock(&(*t)->hash_mutex);
			return data;
			
		}
		last = h;
	} 
	
	pthread_mutex_unlock(&(*t)->hash_mutex);
	return NULL;
}


//������;: ���ݹ�ϣ��ļ���ȡ��Ӧ�Ĺ�ϣ�����
//�������: ��ϣ���ָ��,  ��ϣ��ļ�ָ��
//�������: ��
//����ֵ	: ��ȡ�ɹ�,  ���ػ�ȡ������ֵ��ַ,   ��ȡʧ��,  ����NULL
MM_HashItem *MM_Hash_GetHashItem(MM_HashTable *pHash_table,   char *pKey)
{
	int nIdx = 0;
	MM_HashItem *pItem = NULL;
	if (NULL ==  pHash_table || pHash_table->bucketsize <= 0 || NULL == pHash_table->hash_func || NULL == pKey)
	{
		printf("MM_Hash_Item: hashtable have a problem or param error\n" );
		return NULL; 
	}
	
	pthread_mutex_lock(&pHash_table->hash_mutex);
	
	nIdx = pHash_table->hash_func(pKey);
	nIdx %= pHash_table->bucketsize;
	
	for (pItem=pHash_table->item[nIdx]; pItem!=NULL; pItem=pItem->next)
	{
		if (0 == strcmp(pKey, pItem->key))
		{
			pthread_mutex_unlock(&pHash_table->hash_mutex);
			return pItem;
		}
	}
	
	pthread_mutex_unlock(&pHash_table->hash_mutex);
	return NULL;
}


//������;: ���ݹ�ϣ��ļ���ȡ��Ӧ�Ĺ�ϣ�����ֵ
//�������: ��ϣ���ָ��,  ��ϣ��ļ�ָ��
//�������: ��
//����ֵ	: ��ȡ�ɹ�,  ���ػ�ȡ������ֵ��ַ,   ��ȡʧ��,  ����NULL
void *MM_Hash_GetHashItemData ( MM_HashTable *t,   char *key) 
{
	unsigned int index;
	MM_HashItem *h;
	
	if ( t == NULL || t->bucketsize <= 0 || t->hash_func == NULL || key == NULL )
	{
		printf("MM_Hash_GetHashItemData: Param error\n" );
		return NULL; 
	}
	
	pthread_mutex_lock(&t->hash_mutex);
	index = t->hash_func(key);   
	index %= t->bucketsize;
	
	for ( h = t->item[index]; h != NULL; h = h->next )
	{
		if ( strcmp(key, h->key) == 0 )
		{
			pthread_mutex_unlock(&t->hash_mutex);
			return h->matchMsg;
		}
	}
	
	pthread_mutex_unlock(&t->hash_mutex);
	return NULL;
}




