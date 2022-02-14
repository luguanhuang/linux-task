
#ifndef YDT_LOG_H_H
#define YDT_LOG_H_H

//modify by LiHuangYuan,2011/10/14 modify struct pack
#pragma pack()
#include <pthread.h>

/******该文件主要是用来记录日记信息*******/

#define TRACE_LENGTH 1024

#define LOG_DIRECTORY "LOG"
#define LOG_FILE_POFIX ".log"

//日志所在的文件名
#define FILE_NAME __FILE__
//日记所在的行号
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

//日记的类型
typedef enum
{
	EnFatal=1,
	EnError,					//错误日记
	EnWarn,
	EnDebug,				//调试日记
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

//写错误日记

void LOG_FATAL(char *pLogInfo, char *pFile_name, int nLine);
void LOG_ERROR(char *pLogInfo, char *pFile_name, int nLine);
void LOG_WARN(char *pLogInfo, char *pFile_name, int nLine);
void LOG_INFO(char *pLogInfo, char *pFile_name, int nLine);
//写调试日记
void LOG_DEBUG(char *pLogInfo, char *pFile_name, int nLine);

//写日记
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
