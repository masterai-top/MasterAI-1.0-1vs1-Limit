#include "../include/sysheads.h"
#include "../include/shlock_in.h"
#include "../include/shfun_in.h"
#include "../include/shlogimp.h"
#include "../include/shstd.h"
#include "./version.h"

using namespace std;
unsigned long shstd::log::CLog::MAX_LOG_FILE_SIZE = (2 * 1024 - 100) * 1024 * 1024L; //2G



//创建软连�?
static void CreateLink(const char *szDir, const char * szLogFile, const char *szModuleName, const char *szOwner)
{
	if (szLogFile == NULL || szModuleName == NULL) return;
	std::string strLogFile = szLogFile;
	std::string strFileName = strLogFile.substr(strLogFile.rfind('/') + 1);
	char szLink[1024] = {0};
	sprintf(szLink, "%s/%s.log", szDir, szModuleName);
	unlink(szLink);
	int nRet = symlink(strFileName.c_str(), szLink);

	if (nRet == 0)
	{
		shstd::fun::SetPathOwner(szLink, szOwner);
	}
	//char cmd[1024] = {0};
	//sprintf(cmd, "ln -sf %s %s/%s.log", strFileName.c_str(), szDir, szModuleName);
	//system(cmd);
}

static bool  g_bLogOpened = false;
static unsigned int g_nExtType = 0;

static shstd::log::CLog g_logExt1;


int shstd::log::Init(const char *pszDir, const char *szModuleName, const char *szLogLevels, const char *szOwner)
{
    if (shstd::log::CLog::Instance().Init(pszDir, szModuleName, szLogLevels, szOwner))
    {
    	g_bLogOpened = true;    	
        return 0;
    }
    else
    {
    	g_bLogOpened = false;
        return 1;
    }
}

int shstd::log::Init(const char *pszDir, const char *szModuleName, const char *szLogLevels, unsigned int nExtType, const char *szOwner)
{
	if (shstd::log::CLog::Instance().Init(pszDir, szModuleName, szLogLevels, szOwner))
    {
    	g_bLogOpened = true;

		g_nExtType = 0;
    	if (nExtType == 1)
    	{
    		if (g_logExt1.Init(pszDir, (string(szModuleName) + "_ext1").c_str(), szLogLevels, szOwner))
    		{
    			g_logExt1.SetAsynMode(100,1);
    			g_nExtType = nExtType;
    		}   		
    	}
  	 
    	
        return 0;
    }
    else
    {
    	g_bLogOpened = false;
        return 1;
    }
}



int shstd::log::IsOpen()
{
	return g_bLogOpened ? 0:1;
}



bool shstd::log::SetAsynMode(unsigned int nAsynFlushLogCnt, unsigned int nAsynFlushSeconds)
{
	 return shstd::log::CLog::Instance().SetAsynMode(nAsynFlushLogCnt, nAsynFlushSeconds);
}

bool shstd::log::SetSyncFlushOption(unsigned int nFlushLogCnt/* =1 */, unsigned int nFlushSecond /* = 3600 */)
{
	return shstd::log::CLog::Instance().SetSyncFlushOption(nFlushLogCnt, nFlushSecond);
}

bool shstd::log::GetAsynOption(unsigned int * pnAsynFlushLogCnt, unsigned int * pnAsynFlushSeconds)
{
	return shstd::log::CLog::Instance().GetAsynOption(pnAsynFlushLogCnt, pnAsynFlushSeconds);
}
bool shstd::log::CLog::GetAsynOption(unsigned int * pnAsynFlushLogCnt, unsigned int * pnAsynFlushSeconds)
{
	if (!m_bAsynMode) return false;
	if (pnAsynFlushLogCnt == NULL || pnAsynFlushSeconds == NULL) return false;

	*pnAsynFlushLogCnt = m_nFlushCnt;
	*pnAsynFlushSeconds = m_nFlushSecond;

	return true;
}


bool shstd::log::GetSyncFlushOption(unsigned int *pnSyncFlushLogCnt, unsigned int *pnSyncFlushSecond)
{
	return shstd::log::CLog::Instance().GetSyncFlushOption(pnSyncFlushLogCnt, pnSyncFlushSecond);    
}

bool shstd::log::CLog::GetSyncFlushOption(unsigned int *pnSyncFlushLogCnt, unsigned int *pnSyncFlushSecond)
{
	if (m_bAsynMode) return false;
	if (pnSyncFlushLogCnt == NULL || pnSyncFlushSecond == NULL) return false;
	*pnSyncFlushLogCnt = m_FLUSH_CNT_CONDITION;
	*pnSyncFlushSecond = m_FLUSH_SECOND_CONDITION;
	return true;
}


void shstd::log::Log(int nLogLevel, const char *szFileName, int nLineNum, int nMilliSecs,const char *szTransId, const char *fmt, ...)
{

    va_list vl;
    va_start(vl, fmt);
    shstd::log::CLog::Instance().Log(nLogLevel, szFileName, nLineNum, nMilliSecs, szTransId, fmt, vl);
    va_end(vl);

}


void shstd::log::Log(int nLogLevel, const char *szFileName, int nLineNum, int nMilliSecs,const char *szTransId, unsigned int nExtType, const char *fmt, ...)
{

    va_list vl;
    va_start(vl, fmt);
    shstd::log::CLog::Instance().Log(nLogLevel, szFileName, nLineNum, nMilliSecs, szTransId, fmt, vl);
    if (nExtType > 0 && g_nExtType == 1)
    {
    	va_list vlext;
    	va_start(vlext, fmt);    	
    	g_logExt1.Log(nLogLevel, szFileName, nLineNum, nMilliSecs, szTransId, fmt, vlext);
    	va_end(vlext);
    }
    va_end(vl);
}


const char * shstd::log::GetLogType()
{
    return shstd::log::CLog::Instance().GetLogType();
}
void shstd::log::SetLogType(const char *szLogLevels)
{
    shstd::log::CLog::Instance().SetLogType(szLogLevels);
    if (g_nExtType)
    {
    	g_logExt1.SetLogType(szLogLevels);
    }
}
unsigned long shstd::log::SetLogMaxSize(unsigned long nMaxSize) //设置日志的最大大�? 默认2G - 100M
{
    return shstd::log::CLog::Instance().SetLogMaxSize(nMaxSize);
}


const char * shstd::log::CLog::GetLogType()
{
    static char szLogType[128];
    memset(szLogType, 0, sizeof(szLogType));

    if (m_nLogLevel & LOG_LEVEL_DEBUG)
    {
        if (strlen(szLogType) > 0) strcat(szLogType, ",");
        strcat(szLogType, "debug");
    }
    if (m_nLogLevel & LOG_LEVEL_INFO)
    {
        if (strlen(szLogType) > 0) strcat(szLogType, ",");
        strcat(szLogType, "info");
    }
    if (m_nLogLevel & LOG_LEVEL_WARN)
    {
        if (strlen(szLogType) > 0) strcat(szLogType, ",");
        strcat(szLogType, "warn");
    }
    if (m_nLogLevel & LOG_LEVEL_ERROR)
    {
        if (strlen(szLogType) > 0) strcat(szLogType, ",");
        strcat(szLogType, "error");
    }
    if (strlen(szLogType) == 0)
    {
        strcat(szLogType, "null");
    }

	if (m_bAsynMode)
	{
		strcat(szLogType, " [ASYN MODE]");
	}
	else
	{
		strcat(szLogType, " [SYNC MODE]");
	}
    return szLogType;
}


shstd::log::CLog::CLog()
: m_nFlushSecond(1), m_nFlushCnt(500), m_bShouldExit(false),  m_bLogBlock(false),
m_pAsynLog(NULL), m_bAsynMode(false), m_tidAsynThread(pthread_t(-1)), m_pidAsyn(pid_t(-1)),
m_tLastFlushTime(time(NULL)), m_nNotFlushCnt(0), m_FLUSH_CNT_CONDITION(0),m_FLUSH_SECOND_CONDITION(3600),
m_fp(NULL)
{
    time_t tNow = time(NULL);
    localtime_r(&tNow, &m_tmCur);
    pthread_mutex_init(&m_mutex, NULL);
	pthread_mutex_init(&m_mutexAsyn, NULL);	
}
int shstd::log::CLog::GetLogLevel()
{
    return m_nLogLevel;
}
bool shstd::log::CLog::SetLogType(int nLogLevel)
{
    m_nLogLevel = nLogLevel;
    return true;
}
unsigned long  shstd::log::CLog::SetLogMaxSize(unsigned long nMaxSize)
{
    if (nMaxSize >= 1024 && nMaxSize <= (10 * 1024 - 100) * 1024 * 1024L)
    {
        MAX_LOG_FILE_SIZE = nMaxSize;
    }
    return MAX_LOG_FILE_SIZE;
}

bool shstd::log::CLog::SetSyncFlushOption(unsigned int nFlushLogCnt, unsigned int nFlushSecond)
{
	if (m_bAsynMode) return false;

    m_FLUSH_CNT_CONDITION = nFlushLogCnt;
	m_FLUSH_SECOND_CONDITION = nFlushSecond;
	shstd_printf("SYNC_FLUSH_OPTION| flush_log_cnt=%u| flush_second=%u|", nFlushLogCnt, nFlushSecond);
	return true;
}

bool shstd::log::CLog::SetAsynMode(unsigned int nAsynFlushLogCnt, unsigned int nAsynFlushSeconds)
{
    m_nFlushCnt = nAsynFlushLogCnt;
	m_nFlushSecond = nAsynFlushSeconds;
	if (m_nFlushCnt == 0 || m_nFlushCnt > 10000) m_nFlushCnt = 500;
	if (m_nFlushSecond == 0 || m_nFlushSecond > 120) m_nFlushSecond = 1;

	if (m_bAsynMode)
	{
		shstd_printf("SET_LOG_ASYN_MODE| flush_cnt=%u| flush_second=%u| param_flush_cnt=%u| param_flush_second=%u| msg=retry set|",
			m_nFlushCnt, m_nFlushSecond, nAsynFlushLogCnt, nAsynFlushSeconds);
		return true;
	}

	if (shstd::fun::GetMonitorParentPid() != getppid()) 
	{
		shstd_printf("SET_LOG_ASYN_MODE| flush_cnt=%u| flush_second=%u| param_flush_cnt=%u| param_flush_second=%u| get_monitor_pid=%d| ppid=%d|msg=failed|",
			m_nFlushCnt, m_nFlushSecond, nAsynFlushLogCnt, nAsynFlushSeconds,
			shstd::fun::GetMonitorParentPid(), getppid());
		return false;
	}

	shstd::lock::CSafeGuard sg(m_mutex);
	if (sg.IsLocked())
	{
		if (0 == pthread_create(&m_tidAsynThread, NULL, ASYN_LOG_THREAD_FUN, this))
		{
			while(1)
			{
				if (m_pAsynLog != NULL)  break;
				else usleep(10);
			}
			m_bAsynMode = true;
			m_pidAsyn = getpid();
		}
		else
		{
			m_tidAsynThread = pthread_t(-1);
			shstd_printf("CREATE_THREAD| FILE=%s:%u| msg=failed to create asyn thread.|", __FILE__, __LINE__);
		}
	}
	else
	{
		shstd_printf("LOCK_FAILED| FILE=%s:%u| msg=failed to lock when set asyn mode.|", __FILE__, __LINE__);
	}

	shstd_printf("SET_LOG_ASYN_MODE| flush_cnt=%u| flush_second=%u| param_flush_cnt=%u| param_flush_second=%u| get_monitor_pid=%d| ppid=%d|msg=%s|",
		m_nFlushCnt, m_nFlushSecond, nAsynFlushLogCnt, nAsynFlushSeconds,
		shstd::fun::GetMonitorParentPid(), getppid(),
		m_bAsynMode ? "success":"failed");
	return m_bAsynMode;
}

shstd::log::CLog::~CLog()
{
	m_bShouldExit = true;
	if (m_pidAsyn == getpid())
	{
		if (m_bAsynMode) pthread_join(m_tidAsynThread, NULL);  
	}
    pthread_mutex_destroy(&m_mutex);
	pthread_mutex_destroy(&m_mutexAsyn);
}


shstd::log::CLog & shstd::log::CLog::Instance()
{
    static CLog ins;
    return ins;
}

bool shstd::log::CLog::IsOpen()
{
    return (m_fp != NULL);
}
void shstd::log::CLog::Close()
{
	shstd::lock::CSafeGuard sg(m_mutex);
	if (sg.IsLocked())
	{
		if (m_bAsynMode) //异步模式
		{
			shstd::lock::CSafeGuard sgasyn(m_mutexAsyn);
			if (sgasyn.IsLocked())
			{
				if (m_fp)
				{
					fclose(m_fp);
					m_fp = NULL;
				}
			}
			else
			{
				shstd_printf("CLOSE_LOG| msg=failed to lock asyn.|");
			}
		}
		else //同步模式
		{
			if (m_fp)
			{
				fclose(m_fp);
				m_fp = NULL;
			}
		}        
	}
	else
	{
		shstd_printf("CLOSE_LOG| msg=failed to lock.|");
	}
}



bool shstd::log::CLog::Init(const char *pszDir, const char *szModuleName, const char *szLogLevels, const char *szOwner)
{
    if (pszDir == NULL || szModuleName == NULL || szLogLevels == NULL || szOwner == NULL) return false;
    m_strOwner = szOwner;

	Close();

    //解析日志级别
    SetLogType(szLogLevels); 
    bool bRet =Open(pszDir, szModuleName, m_nLogLevel);
	if (bRet)
	{
		Log(LT_INFO, "shstd_LIB_INFO| info=support asyn log| ver=%s|", GetShstdVersionInfo());		
	}

    return bRet;
}

void shstd::log::CLog::SetLogType(const char *szLogLevels)
{   

    if (szLogLevels != NULL && strlen(szLogLevels) > 0)
    {
        int nLogLevel = LOG_LEVEL_NULL;
        if (strcasestr(szLogLevels, "de") != NULL)
        {
            nLogLevel = nLogLevel|LOG_LEVEL_DEBUG;
        }
        if (strcasestr(szLogLevels, "info") != NULL)
        {
            nLogLevel = nLogLevel|LOG_LEVEL_INFO;
        }
        if (strcasestr(szLogLevels, "warn") != NULL)
        {
            nLogLevel = nLogLevel|LOG_LEVEL_WARN;
        }
        if (strcasestr(szLogLevels, "err") != NULL)
        {
            nLogLevel = nLogLevel|LOG_LEVEL_ERROR;
        }
        if (strcasestr(szLogLevels, "pri") != NULL)
        {
            nLogLevel = nLogLevel|LOG_LEVEL_PRINT;
        }
        if (strcasestr(szLogLevels, "printline") != NULL)
        {
        	nLogLevel = nLogLevel | LOG_PRINT_LINE;
        }
        m_nLogLevel = nLogLevel;
    }
    
}

bool shstd::log::CLog::Open(const char *pszDir, const char *szModuleName, int nLogLevel) //初始化日�?
{
    shstd::lock::CSafeGuard sg(m_mutex);
    if (pszDir == NULL || strlen(pszDir) <= 0) return false;
    if (szModuleName == NULL || strlen(szModuleName) <= 0) return false;

    m_strModuleName = szModuleName;
    m_strDir = pszDir;
    if (m_strDir[m_strDir.size() - 1] == '/') 
    {
        m_strDir = m_strDir.substr(0, m_strDir.size() - 1);
    }
    m_nLogLevel = nLogLevel;
    return shstd::fun::MakeDir(m_strDir.c_str());
}

void shstd::log::CLog::Log(int nLogLevel, const char *szFileName, int nLineNum, int nMilliSecs,const char *szTransId, const char *fmt, ...)
{
	time_t tStart = time(NULL);
    va_list vl;
    va_start(vl, fmt);
    Log(nLogLevel, szFileName, nLineNum, nMilliSecs, szTransId, fmt, vl);
    va_end(vl);
	time_t tEnd = time(NULL);
    
	if ( (tEnd - tStart) > 3)
	{
		//记录写日志请求超时情�?
		shstd_printf("LOG_BLOCK_TIME_OUT| FILE=%s:%u| time=%lds| file=%s| line=%d| tid=%s|",
			__FILE__, __LINE__,
			(long)(tEnd - tStart), szFileName, nLineNum,
			szTransId == NULL ? "":szTransId);
	}
	
}

void shstd::log::CLog::Log(int nLogLevel, const char *szFileName, int nLineNum, int nMilliSecs,const char *szTransId,  const char *fmt, va_list vl) //写日志函�?
{
    if (!(nLogLevel & m_nLogLevel) ) return; //这个语句往前移

    const int HEAD_BUF_SIZE = 512 - 5;
    char szHead[HEAD_BUF_SIZE + 5] = {0};

    //取得日期时间
    //int gettimeofday(struct timeval *tv, struct timezone *tz)

    timeval tv;
    gettimeofday(&tv, NULL);

    tm tmNow;
	localtime_r(&tv.tv_sec, &tmNow); 

    const char *pFileName = strrchr(szFileName, '/');
    if (pFileName == NULL)
    {
        pFileName = szFileName;
    }
    else
    {
        pFileName += 1;
    }

    //[等级][时间][事务ID] [线程ID][文件�?类名：行号] [t:处理时间(单位)]:logid 日志内容
    int nHeadLen = 0;
    if (nMilliSecs >= 0)
    {
        if ( (nLogLevel & LOG_LEVEL_INFO) && !(m_nLogLevel & LOG_PRINT_LINE) )
        {
            nHeadLen = snprintf(szHead, HEAD_BUF_SIZE, "[%s][%02d:%02d:%02d.%06d][%s][%lx]:", 
                GetLevelName(nLogLevel), 
                tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec,
                (int)(tv.tv_usec),
                szTransId != NULL ? szTransId:"",
                pthread_self());
        }
        else
        {
            nHeadLen = snprintf(szHead, HEAD_BUF_SIZE, "[%s][%02d:%02d:%02d.%06d][%s][%lx][%s:%d][%d]:", 
                GetLevelName(nLogLevel), 
                tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec,
                (int)(tv.tv_usec),
                szTransId != NULL ? szTransId:"",
                pthread_self(),        
                pFileName,nLineNum,
                nMilliSecs
                );
        }
        
    }
    else
    {
        if ( (nLogLevel & LOG_LEVEL_INFO) && !(m_nLogLevel & LOG_PRINT_LINE) )
        {
            nHeadLen = snprintf(szHead, HEAD_BUF_SIZE, "[%s][%02d:%02d:%02d.%06d][%s][%lx]:", 
                GetLevelName(nLogLevel), 
                tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec,
                (int)(tv.tv_usec),
                szTransId != NULL ? szTransId:"",
                pthread_self());
        }
        else
        {
            nHeadLen = snprintf(szHead, HEAD_BUF_SIZE, "[%s][%02d:%02d:%02d.%06d][%s][%lx][%s:%d][]:", 
                GetLevelName(nLogLevel), 
                tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec,
                (int)(tv.tv_usec),
                szTransId != NULL ? szTransId:"",
                pthread_self(),        
                pFileName,nLineNum
                );
        }
    } 

    const unsigned int HARD_MAX_CNT = 60000;
	unsigned int SOFT_MAX_CNT = 50000;

	bool bContinue = false;
	do 
	{
		if (bContinue)
		{
			usleep(50000); //如果存在阻塞，则休眠一�?
		}
		shstd::lock::CSafeGuard sg(m_mutex);
		if (sg.IsLocked())
		{
			if (m_bAsynMode)
			{
				if ( (*m_pAsynLog).GetTotalLogCnt() > SOFT_MAX_CNT)
				{					
					if (!m_bLogBlock)
					{	
						m_bLogBlock = true;
						shstd_printf("ASYN_LOG_BLOCK| main_log_cnt=%u| soft=%u| hard=%u| msg=block start|", 
							(*m_pAsynLog).GetTotalLogCnt(),
							SOFT_MAX_CNT, HARD_MAX_CNT);
					}	
					SOFT_MAX_CNT = HARD_MAX_CNT; //超出软限制，则阻�?.05秒，超出硬限制，则一直阻�?
					bContinue = true;
					continue;
				}
				else
				{
					if (m_bLogBlock && (SOFT_MAX_CNT < HARD_MAX_CNT) ) //阻塞结束
					{
						m_bLogBlock = false;
						shstd_printf("ASYN_LOG_BLOCK| main_log_cnt=%u| soft=%u| hard=%u| msg=block end|", 
							(*m_pAsynLog).GetTotalLogCnt(),
							SOFT_MAX_CNT, HARD_MAX_CNT);
					}
                    WriteToBuf(tv, szHead, nHeadLen, fmt, vl);
					bContinue = false;
				}				
			}
			else
			{
				WriteToFile(tv,szHead,nHeadLen,fmt,vl);
				bContinue = false;
			}
		}
		else //加锁错误
		{
			shstd_printf("LOCK_FAILED| dir=%s| FILE=%s:%u| msg=throw log{%s}",
				m_strDir.c_str(), __FILE__, __LINE__, szHead);
			bContinue = false;
		}
	} while (bContinue);

}

void shstd::log::CLog::WriteToFile(timeval &tv, const char *szHead, int nHeadLen, const char *fmt, va_list vl)
{  
	//time_t tLockStart = tv.tv_sec;	
	//time_t tLockEnd = time(NULL);

	tm tmNow;
	localtime_r(&tv.tv_sec, &tmNow);  

	if (m_fp == NULL)
	{
		string strLogFileName = GetLogFileName(tmNow);
		m_fp = fopen(strLogFileName.c_str(), "ab+");        
		if (m_fp != NULL)
		{
			chmod(strLogFileName.c_str(), S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
			shstd::fun::SetPathOwner(strLogFileName.c_str(), m_strOwner.c_str());
			//软链�?
			CreateLink(m_strDir.c_str(), strLogFileName.c_str(), m_strModuleName.c_str(), m_strOwner.c_str());   
		}         
		else
		{
			//写系统日�?
			shstd_printf("OPEN_FILE_FAILED| dir=%s| FILE=%s:%u| fun=fopen| logfile=%s|",
				m_strDir.c_str(), __FILE__, __LINE__, strLogFileName.c_str());
		}
	}
	else
	{        

		if (tmNow.tm_mday != m_tmCur.tm_mday) //是否跨日期了
		{
			if (m_fp != NULL) 
			{
				fclose(m_fp);
				m_fp = NULL;
			}

			string strLogFileName = GetLogFileName(tmNow);
			m_fp = fopen(strLogFileName.c_str(), "ab+");        
			if (m_fp != NULL)
			{
				chmod(strLogFileName.c_str(), S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
				shstd::fun::SetPathOwner(strLogFileName.c_str(), m_strOwner.c_str());
				//软链�?
				CreateLink(m_strDir.c_str(), strLogFileName.c_str(), m_strModuleName.c_str(), m_strOwner.c_str());   
			}  
			if (m_fp != NULL)
			{
				m_tmCur = tmNow;
			}
		}
		else //检查大�?
		{
			struct stat file_stats;
			if(fstat(fileno(m_fp), &file_stats) < 0) //检查大小出�?
			{
				fclose(m_fp);
				m_fp = NULL;
			}
			else
			{
				if (file_stats.st_size > (long)MAX_LOG_FILE_SIZE)
				{
					if (m_fp != NULL) 
					{
						fclose(m_fp);
						m_fp = NULL;
					}

					string strLogFileName = GetLogFileName(tmNow);
					m_fp = fopen(strLogFileName.c_str(), "ab+");        
					if (m_fp != NULL)
					{
						chmod(strLogFileName.c_str(), S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
						shstd::fun::SetPathOwner(strLogFileName.c_str(), m_strOwner.c_str());
						//软链�?
						CreateLink(m_strDir.c_str(), strLogFileName.c_str(), m_strModuleName.c_str(), m_strOwner.c_str());   
					}  
					else
					{
						//写系统日�?
						shstd_printf("OPEN_FILE_FAILED| dir=%s| FILE=%s:%u| fun=fopen| logfile=%s| errno=%d| errmsg=%s|",
							m_strDir.c_str(), __FILE__, __LINE__,  strLogFileName.c_str(),
							errno, strerror(errno));
					}
				}
			}
		}
	}

	if (m_fp != NULL)
	{
		int nHeadSize = 0;
		//写头�?
		if ((nHeadSize = fwrite(szHead, 1, nHeadLen, m_fp)) <= 0)
		{
			fclose(m_fp);
			m_fp = NULL;
			return;            
		}


		int nBodySize = 0;
		if ((nBodySize = vfprintf(m_fp, fmt, vl)) < 0)
		{
			fclose(m_fp);
			m_fp = NULL;
			return;   
		}
		fwrite("\n", 1, 1, m_fp);

		m_nNotFlushCnt++;
		if (m_nNotFlushCnt >= m_FLUSH_CNT_CONDITION || (time(NULL) - m_tLastFlushTime) >= m_FLUSH_SECOND_CONDITION)
		{
			if (0 != fflush(m_fp))
			{
				shstd_printf("FLUSH_FILE_FAILED| dir=%s| FILE=%s:%u| opt=fflush| errno=%d| errmsg=%s| msg=maybe throw %u logs|",
					m_strDir.c_str(), __FILE__, __LINE__,
					errno, strerror(errno),
					m_nNotFlushCnt);

				fclose(m_fp);
				m_fp = NULL;			
			}
			
			m_nNotFlushCnt = 0;
			m_tLastFlushTime = time(NULL);
		}
	}

} 


const char * shstd::log::CLog::GetLevelName(int nLogLevel)
{
    static char szDebug[] = "DEBUG";
    static char szInfo[] = "INFO";
    static char szWarn[] = "WARN";
    static char szError[] = "ERROR";
    static char szUnknown[] = "UNKNOWN";
    switch(nLogLevel)
    {
    case LOG_LEVEL_DEBUG:
        return szDebug;
        break;
    case LOG_LEVEL_INFO:
        return szInfo;
        break;
    case LOG_LEVEL_WARN:
        return szWarn;
        break;
    case LOG_LEVEL_ERROR:
        return szError;
        break;
    default:
        return szUnknown;
        break;
    }    

}


static void ListDir(const char *srcDir, const char *szFindStr, std::set<string> &setFiles)
{
    struct dirent* ent = NULL;
    DIR *pDir = opendir(srcDir);
    if (pDir != NULL)
    {
        while (NULL != (ent=readdir(pDir))) 
        {

            if (ent->d_type == 8) //文件
            {
            	
                if (strncmp(ent->d_name, szFindStr, strlen(szFindStr)) == 0 )
                {
                	if (strcmp(ent->d_name, szFindStr) == 0) setFiles.insert(ent->d_name);
					else if ( (strlen(ent->d_name) - strlen(szFindStr) ) == 7)
					{
						char *p = ent->d_name+strlen(szFindStr);
						if (*p++ != '.') continue;
						if (!isdigit(*p++)) continue;
						if (!isdigit(*p++)) continue;
						if (!isdigit(*p++)) continue;
						if (!isdigit(*p++)) continue;
						if (!isdigit(*p++)) continue;
						if (!isdigit(*p++)) continue;						
						
						setFiles.insert(ent->d_name);
					}
					else{}
                    
                }
            }
            else
            {
            }
        }

        if (0 != closedir(pDir))
        {
        }
    }
    else
    {
    }

}

std::string  shstd::log::CLog::GetLogFileName(tm & tmNow)
{
    char szDayFileName[2048] = {0};
    sprintf(szDayFileName, "%s.log.%04d-%02d-%02d", m_strModuleName.c_str(), tmNow.tm_year + 1900,
        tmNow.tm_mon + 1, tmNow.tm_mday);
    //找最后一个文�?
    std::set<string> setFiles;
    ListDir(m_strDir.c_str(), szDayFileName, setFiles);

    char szFilePath[2048] = {0};

    if (setFiles.size() <= 1)
    {
        sprintf(szFilePath, "%s/%s", m_strDir.c_str(), szDayFileName);
    }
    else
    {
        sprintf(szFilePath, "%s/%s", m_strDir.c_str(), (*setFiles.rbegin()).c_str());
    }

    if (access(szFilePath, F_OK) == 0)  //文件已经存在
    {       

        //检查文件属�?
        struct stat file_stats;
        if( (stat(szFilePath, &file_stats) == 0) && (file_stats.st_size < (long)MAX_LOG_FILE_SIZE)) //检查大小出�?
        {
            //不用改变
        }
        else
        {
            //printf("add time to file name.file_stats.st_size=%ld\n", file_stats.st_size);
            //加时间戳
            sprintf(szFilePath, "%s/%s.%02d%02d%02d", m_strDir.c_str(), szDayFileName, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec);
        }
    }


    FILE *fp = fopen(szFilePath, "ab+");
    if (fp != NULL)
    { 		
        fclose(fp);
		chmod(szFilePath, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
		if (shstd::fun::GetRunUser() != NULL)
		{
            shstd::fun::SetPathOwner(szFilePath, shstd::fun::GetRunUser());
		}
		
    }
    else
    {
		shstd_printf("OPEN_FILE_FAILED| path=%s| FILE=%s:%u| opt=fopen| logfile=%s| errno=%d| errmsg=%s|",
			m_strDir.c_str(), __FILE__, __LINE__, szFilePath,
			errno, strerror(errno));
        sprintf(szFilePath, "%s/%s.%02d%02d%02d", m_strDir.c_str(), szDayFileName, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec+2);
    }

    return std::string(szFilePath);
}

void shstd::log::CLog::WriteToFile(shstd::ASYN_LOG & al)//把异步的日志写到文件�? 在异步线程中执行，不需要加�?
{
	if (al.GetTotalLogCnt() == 0) return;

	time_t tNow = time(NULL);
	tm tmNow;
	localtime_r(&tNow, &tmNow);  

	if (m_fp == NULL)
	{
		string strLogFileName = GetLogFileName(tmNow);
		m_fp = fopen(strLogFileName.c_str(), "ab+");        
		if (m_fp != NULL)
		{
			m_tmCur = tmNow;
			chmod(strLogFileName.c_str(), S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
			shstd::fun::SetPathOwner(strLogFileName.c_str(), m_strOwner.c_str());
			//软链�?
			CreateLink(m_strDir.c_str(), strLogFileName.c_str(), m_strModuleName.c_str(), m_strOwner.c_str());   
		}         
		else
		{
			//写系统日�?
			shstd_printf("OPEN_FILE_FAILED| path=%s| FILE=%s:%u| opt=fopen| logfile=%s| errno=%d| errmsg=%s|",
				m_strDir.c_str(), __FILE__, __LINE__, strLogFileName.c_str(),
				errno, strerror(errno));
		}
	}
	else
	{   
		if (tmNow.tm_yday != m_tmCur.tm_yday) //是否跨日期了
		{
			if (m_fp != NULL) 
			{
				fclose(m_fp);
				m_fp = NULL;
			}

			string strLogFileName = GetLogFileName(tmNow);
			m_fp = fopen(strLogFileName.c_str(), "ab+");        
			if (m_fp != NULL)
			{
				m_tmCur = tmNow;

				chmod(strLogFileName.c_str(), S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
				shstd::fun::SetPathOwner(strLogFileName.c_str(), m_strOwner.c_str());
				//软链�?
				CreateLink(m_strDir.c_str(), strLogFileName.c_str(), m_strModuleName.c_str(), m_strOwner.c_str());   
			}
			else
			{
				shstd_printf("OPEN_FILE_FAILED| path=%s| FILE=%s:%u| opt=fopen| logfile=%s| errno=%d| errmsg=%s|",
					m_strDir.c_str(), __FILE__, __LINE__, strLogFileName.c_str(),
					errno, strerror(errno));
			}

		}
		else //检查大�?
		{
			struct stat file_stats;
			if(fstat(fileno(m_fp), &file_stats) < 0) //检查大小出�?
			{
				shstd_printf("STAT_FILE_FAILED| path=%s| FILE=%s:%u| opt=fstat| msg=shstd::log| errno=%d| errmsg=%s",
					m_strDir.c_str(), __FILE__, __LINE__,
					errno, strerror(errno));

				fclose(m_fp);
				m_fp = NULL;
				
			}
			else
			{
				if (file_stats.st_size > (long)MAX_LOG_FILE_SIZE)
				{
					if (m_fp != NULL) 
					{
						fclose(m_fp);
						m_fp = NULL;
					}

					string strLogFileName = GetLogFileName(tmNow);
					m_fp = fopen(strLogFileName.c_str(), "ab+");        
					if (m_fp != NULL)
					{
						chmod(strLogFileName.c_str(), S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
						shstd::fun::SetPathOwner(strLogFileName.c_str(), m_strOwner.c_str());
						//软链�?
						CreateLink(m_strDir.c_str(), strLogFileName.c_str(), m_strModuleName.c_str(), m_strOwner.c_str());   
					}  
					else
					{
						//写系统日�?
						shstd_printf("OPEN_FILE_FAILED| dir=%s| FILE=%s:%u| fun=fopen| logfile=%s| errno=%d| errmsg=%s",
							m_strDir.c_str(), __FILE__, __LINE__, strLogFileName.c_str(),
							errno, strerror(errno));
					}
				}
			}

		}
	}


	//将流中的内容写到文件�?
	if (m_fp != NULL && al.sslog.cnt > 0)
	{
		al.sslog.ss.seekg(0, ios_base::end);
		unsigned int length = al.sslog.ss.tellg();
		unsigned int nWriteLen = fwrite(al.sslog.ss.str().c_str(), 1, length, m_fp) ;
		if (nWriteLen != length)
		{
			shstd_printf("WRITE_FILE_FAILED| dir=%s| FILE=%s:%u| opt=fwrite| length=%u| write=%u| errno=%d| errmsg=%s| msg=maybe throw %u logs|",
				m_strDir.c_str(), __FILE__, __LINE__,
				length, nWriteLen,
				errno, strerror(errno),
				al.GetTotalLogCnt());

			fclose(m_fp);
			m_fp = NULL;			
		}
		else
		{
			al.sslog.Clear();
		}
	}

	//将buf日志写到文件�?
	if (m_fp != NULL && al.buflog.cnt > 0)
	{
		unsigned int nWriteLen = fwrite(al.buflog.logbuf, 1, al.buflog.nSize, m_fp) ;
		if (nWriteLen != al.buflog.nSize)
		{
			shstd_printf("WRITE_FILE_FAILED| dir=%s| FILE=%s:%u| opt=fwrite| length=%u| write=%u| errno=%d| errmsg=%s| msg=maybe throw %u logs|",
				m_strDir.c_str(), __FILE__, __LINE__,
				al.buflog.nSize, nWriteLen,
				errno, strerror(errno),
				al.buflog.cnt);

			fclose(m_fp);
			m_fp = NULL;			
		}
		else
		{
			al.buflog.Clear();
		}

	}

	if (m_fp != NULL && 0 != fflush(m_fp))
	{
		shstd_printf("FLUSH_FILE_FAILED| dir=%s| FILE=%s:%u| fun=fflush| errno=%d| errmsg=%s|",
			m_strDir.c_str(), __FILE__, __LINE__,
			errno, strerror(errno));

		fclose(m_fp);
		m_fp = NULL;
	}

}

static void ChangeAyncIndex(unsigned int & nAsyncInex, unsigned int & nMainIndex)
{
	unsigned int n = nAsyncInex;
    nAsyncInex = nMainIndex;
	nMainIndex = n;
}
void * shstd::log::CLog::ASYN_LOG_THREAD_FUN(void *pThis) //异步线程函数
{
	CLog * ins = (CLog *)pThis;
	
	ASYN_LOG *arrAL = new ASYN_LOG[2];
	unsigned int nAsynIndex=0;
	unsigned int nMainIndex=1;
	unsigned int nSleepStep = 0;
	ChangeAyncIndex(nAsynIndex, nMainIndex);
	
	ins->m_pAsynLog = &arrAL[nMainIndex];

	{
		shstd::lock::CSafeGuard sm(ins->m_mutex);
	}
	
	while(!ins->m_bShouldExit)
	{
		time_t tStart = time(NULL);

		if (arrAL[nAsynIndex].GetTotalLogCnt() == 0) //如果不为空，则不交换
		{
			shstd::lock::CSafeGuard sm(ins->m_mutex);
			if (sm.IsLocked())
			{
				//交换
				ChangeAyncIndex(nAsynIndex, nMainIndex);
				ins->m_pAsynLog = & arrAL[nMainIndex];

			}	
			else
			{
				shstd_printf("LOCK_FAILED| dir=%s| FILE=%s:%u|",
					ins->m_strDir.c_str(), __FILE__, __LINE__);
			}
		}

		time_t tEnd = time(NULL);
		if ( (tEnd - tStart) > 3 )
		{
			shstd_printf("PICKUP_LOG_TIME_OUT| dir=%s| FILE=%s:%u| time=%lds| msg=get log from buf.",
				ins->m_strDir.c_str(), __FILE__, __LINE__,
				long(tEnd - tStart));
		}                

		tStart = time(NULL);
		unsigned int nLogCnt = arrAL[nAsynIndex].GetTotalLogCnt();
		if (arrAL[nAsynIndex].GetTotalLogCnt() != 0)
		{
			shstd::lock::CSafeGuard sgasyn(ins->m_mutexAsyn);
			if (sgasyn.IsLocked())
			{
				//将日志写到文�?
				ins->WriteToFile(arrAL[nAsynIndex]);
				arrAL[nAsynIndex].Clear(); //清空，给下次轮换�?
			}
			else
			{
				shstd_printf("LOCK_ASYN_FAILED| FILE=%s:%u| msg=failed to lock asyn.|",
					__FILE__, __LINE__);
			}
			
		}
		tEnd = time(NULL);
		if ( (tEnd - tStart) > 3 )
		{
			shstd_printf("WRITE_LOG_TIME_OUT| FILE=%s:%u| log_cnt=%u| time=%lds|", 
				__FILE__, __LINE__,
				nLogCnt, (long)(tEnd - tStart));
		}		

		
		time_t tWaitStart = time(NULL);
		nSleepStep = 0;
		while ( (time(NULL) - tWaitStart) < ins->m_nFlushSecond && !ins->m_bShouldExit)
		{
			if (arrAL[nMainIndex].GetTotalLogCnt() > ins->m_nFlushCnt) break; //如果发现有一千条日志，就去拿过来�?
			time_t tNow = time(NULL);
			tm tmNow;
			localtime_r(&tNow, &tmNow);
			if (tmNow.tm_hour == 23 && tmNow.tm_min == 59 && tmNow.tm_sec > 30) //快临近第二天�?
			{
				usleep(100);
				break;
			}
			if (tmNow.tm_yday != ins->m_tmCur.tm_yday)
			{
				usleep(100);
				break; //跨天了，就重写文�?
			}

			if (++nSleepStep > 500) nSleepStep = 500;
            usleep(100*nSleepStep);
			
		}
	}   


	{
		shstd::lock::CSafeGuard sm(ins->m_mutex);
		if (sm.IsLocked())
		{
			//交换
			ChangeAyncIndex(nAsynIndex, nMainIndex);
			ins->m_pAsynLog = & arrAL[nMainIndex];

		}	
		else
		{
			shstd_printf("LOCK_FAILED| dir=%s| FILE=%s:%u|",
				ins->m_strDir.c_str(), __FILE__, __LINE__);
		}

		if (arrAL[nAsynIndex].GetTotalLogCnt() != 0)
		{
			//将日志写到文�?
			ins->WriteToFile(arrAL[nAsynIndex]);
			arrAL[nAsynIndex].Clear();
		}
	}
    

	if (ins->m_fp != NULL)
	{
		fclose(ins->m_fp);
		ins->m_fp = NULL;
	}

	delete[] arrAL;
	arrAL = NULL;

	return NULL;
}

void shstd::log::CLog::WriteToBuf(timeval &tv, const char *szHead, int nHeadLen, const char *fmt, va_list vl)
{
		ASYN_LOG & al = *m_pAsynLog;
		memcpy(al.buflog.logbuf + al.buflog.nSize, szHead, nHeadLen); //拷贝�?
		al.buflog.nSize += nHeadLen;
		int nBodySize = vsnprintf(al.buflog.logbuf + al.buflog.nSize, sizeof(al.buflog.logbuf) - 1024 - al.buflog.nSize, fmt, vl);
		if (nBodySize < 0)
		{
			char *pBody = strcpy(al.buflog.logbuf + al.buflog.nSize, "failed to vsnprintf body.");
			nBodySize = strlen(pBody);
		}
		if (nBodySize >= int(sizeof(al.buflog.logbuf) - 1024 - al.buflog.nSize)) //写的数据太多,缓存容不�?
		{
            shstd_printf("DATA_OVERFLOW| buf_remain_size=%u| data_size=%d|", sizeof(al.buflog.logbuf) - 1024 - al.buflog.nSize, nBodySize);		
			al.buflog.nSize = strlen(al.buflog.logbuf);
		}
		else
		{
            al.buflog.nSize += nBodySize;
		}		
		
		if (al.buflog.logbuf[al.buflog.nSize - 1] != '\n')
		{			
			al.buflog.logbuf[al.buflog.nSize] = '\n';	
			al.buflog.nSize++;
		}
		al.buflog.logbuf[al.buflog.nSize] = '\0';

		al.buflog.cnt++;

		al.CheckOverflow(); //检查是否到了警戒线	
} 


