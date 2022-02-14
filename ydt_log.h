
#ifndef YDT_LOG_H_H
#define YDT_LOG_H_H

//modify by LiHuangYuan,2011/10/14 modify struct pack
#pragma pack()
#include <pthread.h>

/******���ļ���Ҫ��������¼�ռ���Ϣ*******/

#define TRACE_LENGTH 1024

#define LOG_DIRECTORY "LOG"
#define LOG_FILE_POFIX ".log"

//��־���ڵ��ļ���
#define FILE_NAME __FILE__
//�ռ����ڵ��к�
#define FILE_LINE __LINE__

#define TWENTY_FOUR_HOUR (24 * 60 * 60)


//add by luguanhuang 20110901
#if 1
#define FATAL(log, ...) LOG(EnFatal, __FILE__, __LINE__, (log), ##__VA_ARGS__)
#define ERROR(log, ...) LOG(EnError, __FILE__, __LINE__, (log), ##__VA_ARGS__)
#define WARN(log, ...) LOG(EnWarn, __FILE__, __LINE__, (log), ##__VA_ARGS__)
#define INFO(log, ...) LOG(EnInfo, __FILE__, __LINE__, (log), ##__VA_ARGS__)
#define DEBUG(log, ...) LOG(EnDebug, __FILE__, __LINE__, (log), ##__VA_ARGS__)
#else
#define FATAL(log, ...) 
#define ERROR(log, ...) 
#define WARN(log, ...) 
#define INFO(log, ...) 
#define DEBUG(log, ...)
#endif




#define FUNC_BEGIN "func begin: "
#define FUNC_END "func end: "

#define FATAL_LOG "[FATAL]"
#define ERROR_LOG "[ERROR]"
#define WARN_LOG  "[WARN]"
#define INFO_LOG  "[INFO]"
#define DEBUG_LOG "[DEBUG]"

#define FUNC_ENTRY "==>"
#define FUNC_EXIT "<=="

void PrintToScreen(char *pFmt, ...);

//�ռǵ�����
typedef enum
{
	EnFatal=1,
	EnError,					//�����ռ�
	EnWarn,
	EnDebug,				//�����ռ�
	EnInfo
}EnLog_Type;

typedef enum
{	
	NOLOG_LEVEL_LOG=0,
	ONE_LEVEL_LOG,
	TWO_LEVEL_LOG,
	THREE_LEVEL_LOG,	
	FOUR_LEVEL_LOG,
	FIVE_LEVEL_LOG
}EnLevel;

typedef struct
{
	pthread_mutex_t log_level_mutex;	
	EnLevel enLevel;
}StLog_Level;


void LOG(EnLog_Type enLog_type, char *pFile_name, int nLine, char *pFmt, ...);

void SetMaxFileLineNum(int nFile_line);
void SetFileTime(long lFile_time);

void SetLogLevel(EnLevel enLevel);

//д�����ռ�

void LOG_FATAL(char *pLogInfo, char *pFile_name, int nLine);
void LOG_ERROR(char *pLogInfo, char *pFile_name, int nLine);
void LOG_WARN(char *pLogInfo, char *pFile_name, int nLine);
void LOG_INFO(char *pLogInfo, char *pFile_name, int nLine);
//д�����ռ�
void LOG_DEBUG(char *pLogInfo, char *pFile_name, int nLine);

//д�ռ�
void AddLog(char *pLog, EnLog_Type enLog_type);

class CTrace
{
public:
	CTrace(const char *pFuncName);
	~CTrace();
private:
	const char *m_pFuncName;
};

#define TRACE() CTrace trace(__FUNCTION__);
#define CLASS_REACE(CLASS_NAME) CTrace trace(CLASS_NAME);
#define CLASS_TRACE(CLASS_NAME) CTrace trace(CLASS_NAME);


#endif 
