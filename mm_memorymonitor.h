#ifndef MM_MEMORYMONITOR_H
#define MM_MEMORYMONITOR_H

extern "C"
{
	#include "mm_hash.h"	
}



#define MM_LOGFILENAME "mm"

//#define IS_USED_IN_IPHONE /*��iphone��ʹ�ô򿪴˺�*/
//#define IS_USED_IN_LINUX /*��linux��ʹ�ô򿪴˺�*/

//#define __DEBUG

//��ϣ���Ԫ�ؽṹ
typedef struct __MM_MemoryStatus__
{	
	char *desc;	/*�ÿ��ڴ������*/ 
	int size;/*�ÿ��ڴ�Ĵ�С*/ 
}MY_MemoryStatus;


#ifdef __DEBUG
#define MM_MALLOC_WITHOUT_DESC(size) MM_MALLOC(size,"no mem desc")  
#define MM_MALLOC_WITH_DESC(size,desc) MY_MALLOC(size,desc)  
#define MM_FREE(p) MY_FREE(p) 


void MY_INIT() ;
void MY_DESTORY(); 
 
void* MY_MALLOC(int size,char* desc);
int MY_FREE(void* p);
void MY_MEMORYSTATUS();

#else
#define MM_MALLOC_WITH_DESC(size,desc) malloc(size)
#define MM_FREE(p) FREEIF(p) 
#define MY_INIT()  //
#define MY_DESTORY()  //
#define MY_MEMORYSTATUS()  //
#endif


#endif
