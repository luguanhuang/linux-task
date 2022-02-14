#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <errno.h> 
#include <stdarg.h>
#include <unistd.h>
//#include "access_global.h"

#include "mm_memorymonitor.h"

#ifdef __DEBUG

//字节对齐
//modify by LiHuangYuan,2011/10/14 modify struct pack
//#pragma pack(1)


MM_HashTable* mm_memorystatus;
int mm_sum_malloc_mem;
int mm_sum_free_mem;
pthread_mutex_t mm_mutex_malloc;
pthread_mutex_t mm_mutex_free;
pthread_mutex_t mm_mutex_log;
FILE *mm_logfile;	


void MM_LOG(char *pFmt, ...)
{
	char szContent[1024] = {0};
	
	va_list args;
	va_start(args, pFmt);
	vsnprintf(szContent, sizeof(szContent) - 1, pFmt, args);
	va_end(args);

	#if 1
	printf("%s",szContent);	
	#endif
	
	#if 0
	pthread_mutex_lock(&mm_mutex_log);
	fwrite(szContent, strlen(szContent),1, mm_logfile);
	fflush(mm_logfile);
	pthread_mutex_unlock(&mm_mutex_log);
	#endif
}

/*文件相关*/
static int GetProgramPath(char *pPath)
{
	char szPath[512] = {0};
	getcwd(szPath, sizeof(szPath));
	strcpy(pPath, szPath);
//	printf("GetProgramPath szPath=%s\n", szPath);
	return TRUE;
}


static void GetToday(char *pToday)
{
	char szToday[256] = {0};
	time_t now;
	
	time(&now);
	struct tm *pTime = localtime(&now);
	int nYear = pTime->tm_year + 1900;
	int nMonth = pTime->tm_mon + 1;
	int nDay = pTime->tm_mday;
	int nHour = pTime->tm_hour;
	int nMinute = pTime->tm_min;
	int nSecond = pTime->tm_sec;
	
	snprintf(szToday, 256 - 1, "%d%02d%02d_%02d%02d%02d", nYear, nMonth, nDay, nHour, nMinute, nSecond);
	strcpy(pToday, szToday);
}

static int GetFilePath(char *pFilePath)
{
	char szFilePath[512] = {0};
	char szPath[512] = {0};
	char szToday[256] = {0};
	
	GetProgramPath(szPath);
	GetToday(szToday);
	
	snprintf(szFilePath, sizeof(szFilePath) - 1, "%s/%s%s.log", szPath, MM_LOGFILENAME,szToday);
	strcpy(pFilePath, szFilePath);
	return TRUE;
}



void MY_INIT() 
{
	int ret =0;
	
	if (pthread_mutex_init(&mm_mutex_malloc, NULL) != 0)
	{
		printf("MY_INIT: pthread_mutex_init mm_mutex_malloc error");
		return;
	}
	
	if (pthread_mutex_init(&mm_mutex_free, NULL) != 0)
	{
		printf("MY_INIT: pthread_mutex_init mm_mutex_free error");
		return;
	}
	
	if (pthread_mutex_init(&mm_mutex_log, NULL) != 0)
	{
		printf("MY_INIT: pthread_mutex_init mm_mutex_log error");
		return;
	}
	
	ret = MM_Hash_Init(&mm_memorystatus, 1024*1024, NULL);
	mm_sum_malloc_mem = 0;
	mm_sum_free_mem = 0;
	
	char szFilePath[1024] = {0};
	GetFilePath(szFilePath);
	printf("MY_INIT logfile name is szFilePath=%s\n", szFilePath);
	mm_logfile =  fopen(szFilePath, "a");;
	if (NULL == mm_logfile)
	{
		printf("MY_INIT OpenLogFile error,errno=%d\n", errno);
		return ;	
	}
	
	MM_LOG("MY_INIT %s,mm_memorystatus=0x%08x\n",ret?"success":"fail",(unsigned int)mm_memorystatus);
	MY_MEMORYSTATUS();
}

void MM_FREE_Node_MemoryStatus(void* p)
{
	MY_MemoryStatus *pms = p;
	
	if (p) 
	{
		pthread_mutex_lock(&mm_mutex_free);
		mm_sum_free_mem+=pms->size;
		pthread_mutex_unlock(&mm_mutex_free);
		
		FREEIF(pms->desc);
		FREEIF(pms);
	}
	
	return;
}

void MY_DESTORY() 
{
	int ret = MM_Hash_Destroy ( &mm_memorystatus, MM_FREE_Node_MemoryStatus ) ;
	MM_LOG("MY_DESTORY %s\n",ret?"success":"fail" );
	pthread_mutex_destroy(&mm_mutex_malloc);
	pthread_mutex_destroy(&mm_mutex_free);
	pthread_mutex_destroy(&mm_mutex_log);
	MY_MEMORYSTATUS();
	//close(mm_logfile);
	fclose(mm_logfile);
}

void* MY_MALLOC(int size,char* desc) 
{
	void *p = NULL;
	char addr[8+1];
	MY_MemoryStatus *pms = NULL;
	int iret=0;
	
	if(size < 0 || NULL == desc)
	{
		MM_LOG("MY_MALLOC: error param\n");	
		return NULL;
	}
	
	p = malloc(size);
	
	if(p)
	{
		snprintf(addr,sizeof(addr),"%08x",(unsigned int)p);
		pms = (MY_MemoryStatus *)malloc(sizeof(MY_MemoryStatus));
		if (!pms) 
		{
			MM_LOG("MY_MALLOC: malloc MY_MemoryStatus error\n");	
			FREEIF(p);
			return NULL;
		}
		pms->size = size ;
		pms->desc = strdup(desc);
		if (!pms->desc) 
		{
			MM_LOG("MY_MALLOC:  strdup(desc) error\n");	
			FREEIF(pms);
			FREEIF(p);
			return NULL;
		}
		
		iret = MM_Hash_Insert(&mm_memorystatus, addr, pms);	
		if (iret == TRUE) 
		{
			pthread_mutex_lock(&mm_mutex_malloc);
			mm_sum_malloc_mem += size;
			pthread_mutex_unlock(&mm_mutex_malloc);
		}
		//如果键键存在, 就会分配不到内存了
		else
		{
			MM_LOG("MY_MALLOC:  MM_Hash_Insert mm_memorystatus error\n");	
			FREEIF(pms->desc);
			FREEIF(pms);
			FREEIF(p);
			//不用return 函数
			return NULL;
		}

		MM_LOG("MY_MALLOC: malloc %dbytes(%s) mem(0x%08x) success\n",size,desc,(unsigned int)p);	
		MY_MEMORYSTATUS();
	}
	else 
	{
		MM_LOG("MY_MALLOC: error(no memory)\n");	
	}

	return p;
}



int MY_FREE(void* p)
{
	char addr[8+1];
	MY_MemoryStatus *pms = 0;
	
	if(!p)
	{
		MM_LOG("MY_FREE: null param\n");	
		return FALSE;
	}
	else 
	{
		snprintf(addr,sizeof(addr),"%08x",(unsigned int)p);
		pms = (MY_MemoryStatus *)MM_Hash_Del(&mm_memorystatus, addr);
		if (NULL == pms)
		{
			MM_LOG("MY_FREE: we can't find the item from hashtable, then exit the MY_FREE func [addr]=%s\n", addr);
			return FALSE;
		}
		
		pthread_mutex_lock(&mm_mutex_free);
		mm_sum_free_mem+=pms->size;
		pthread_mutex_unlock(&mm_mutex_free);
		
		MM_LOG("MY_FREE: free %dbytes(%s) mem(0x%08x) success\n",pms->size,pms->desc,(unsigned int)p);	
		
		FREEIF(p);
		FREEIF(pms->desc);
		FREEIF(pms);
		
		MY_MEMORYSTATUS();
	}
	return TRUE;
}


void MY_MEMORYSTATUS()
{
	MM_LOG("MY_MEMORYSTATUS: sum_malloc_mem=%d sum_free_mem=%d sum_used_mem_of_mine=%d\n",
		   mm_sum_malloc_mem,mm_sum_free_mem,mm_sum_malloc_mem-mm_sum_free_mem);
}
#endif
