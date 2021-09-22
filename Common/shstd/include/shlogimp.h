
//-------------------------  公共库日志模块头文件-------------------------

/*----------- 使用方法 ---------------------------------

第一步：初始化
shstd::CLog::Instance().Init(日志存放目录, 模块名, 输出的日志类型),
输出的日志类型为组合方式,可以为"debug, info, warn, error" 四种类型的任意组合,也可以为空，为空即不输出日志
例如：LOGINIT("/home/mylocal/logs", "fts", "warn,info,error");
如要将日志内容同时打印到屏幕上一份，可以加上print,即
LOGINIT("/home/mylocal/logs", "fts", "warn,info,error,print");

第二步:写普通日志
日志初始化成功之后，就可以开始写日志，为了简化编码，用LOG宏来实现
写DEBUG日志：
LOG(LT_DEBUG, "miid=%d", miid);
写INFO日志:
LOG(LT_INFO, "miid=%d", miid);
写WARN日志:
LOG(LT_WARN, "miid=%d", miid);
写ERROR日志:
LOG(LT_ERROR, "miid=%d|msid=%d", miid, msid);

第二步.1:写含有dealtime和transid的日志
LOG(LT_DEBUG_ALL,nDealTimeMillSeconds, szTransId, "miid=%d", miid);
LOG(LT_INFO_ALL, 1,szTransId, "miid=%d", miid);

第二步.2:写含有transid不含dealtime的日志
LOG(LT_INFO_TRANS, szTransId, "usere=%s | uin=%u", username, uin);


如要改变日志输出类型为"warn,err"
SET_LOG_LEVEL("warn,err"); 
设置切换大小:
SET_LOG_MAX_SIZE(nSize); //默认2G-100M
--------------------------------------------------------*/

#ifndef __SH_STD_LOG_
#define __SH_STD_LOG_
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <sstream>
#include "shasynlogdsdef.h"

namespace shstd
{ 
    namespace log
    {
        class CLog
        {
        public:
            const static  int LOG_LEVEL_NULL = 0;     //不输出任何日志
            const static  int LOG_LEVEL_DEBUG = 1;    //输出debug日志
            const static  int LOG_LEVEL_INFO = 2;       //输出info日志
            const static  int LOG_LEVEL_WARN = 4;       //输出warn日志
            const static  int LOG_LEVEL_ERROR = 8;      //输出err日志
            const static  int LOG_LEVEL_PRINT = 1024;   //将要输出的日志同时打印到标准输出
            const static  int LOG_PRINT_LINE  = 2048;   //所有的都输出行号
			
		private://异步相关	
			unsigned int m_nFlushSecond;
			unsigned int m_nFlushCnt;
			volatile bool m_bShouldExit; 
			bool m_bLogBlock;

			ASYN_LOG *m_pAsynLog;
			bool m_bAsynMode;

			void WriteToFile(ASYN_LOG & al);//把异步的日志写到文件中 
			static void * ASYN_LOG_THREAD_FUN(void *pThis); //异步线程类
			pthread_t m_tidAsynThread;
			pid_t m_pidAsyn; //创建异步进程的pid

        private:
			unsigned int m_tLastFlushTime; //最后刷新时间
			unsigned int m_nNotFlushCnt; //未刷数量
			unsigned int m_FLUSH_CNT_CONDITION; //flush数量阈值
			unsigned int m_FLUSH_SECOND_CONDITION; //时间阈值

            static unsigned long MAX_LOG_FILE_SIZE; //2G - 100M, 以支持32位机器
            int m_nLogLevel;
            std::string m_strDir;
            FILE *m_fp;
            pthread_mutex_t m_mutex;
			pthread_mutex_t m_mutexAsyn;
            tm m_tmCur;
            std::string m_strModuleName;
            std::string m_strOwner;
        private:
			void WriteToBuf(timeval &tv, const char *szHead, int nHeadLen, const char *fmt, va_list vl);
            void WriteToFile(timeval &tv, const char *szHead, int nHeadLen, const char *fmt, va_list vl);        
            const char * GetLevelName(int nLogLevel);
            std::string  GetLogFileName(tm & tmNow);
            bool Open(const char *pszDir, const char *szModuleName, int nLogLevel = LOG_LEVEL_NULL);
            bool SetLogType(int nLogLevel);
            int GetLogLevel();
			void Close(); //关闭文件句柄
			bool IsOpen(); //判断是否打开
        public:    
            CLog();
            ~CLog();
            static  CLog & Instance();
        public:
            bool Init(const char *pszDir, const char *szModuleName, const char *szLogLevels, const char *szOwner); //初始化日志, 只能调用一次
            void Log(int nLogLevel, const char *szFileName, int nLineNum, int nMilliSecs,const char *szTransId, const char *fmt, ...);
            void Log(int nLogLevel, const char *szFileName, int nLineNum, int nMilliSecs,const char *szTransId, const char *fmt, va_list vl); //写日志函数        
            void Log(char buf, unsigned int nBufLen);
            void SetLogType(const char *szLogLevels); 
            const char * GetLogType();
            unsigned long  SetLogMaxSize(unsigned long nMaxSize = (2 * 1024 - 100) * 1024 * 1024L); //设置日志的最大大小, 默认2G - 100M
			bool SetAsynMode(unsigned int nAsynFlushLogCnt, unsigned int nAsynFlushSeconds);			
			bool SetSyncFlushOption(unsigned int nFlushLogCnt=1, unsigned int nFlushSecond = 3600);

			bool GetAsynOption(unsigned int * pnAsynFlushLogCnt, unsigned int * pnAsynFlushSeconds);
			bool GetSyncFlushOption(unsigned int *pnSyncFlushLogCnt, unsigned int *pnSyncFlushSeconds);
        };

    } 
}

#endif
 

