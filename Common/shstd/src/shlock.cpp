
#include "../include/shstd.h"
#include "../include/sysheads.h"
#include "../include/shlock_in.h"
#include "../include/shfun_in.h"
#include "../include/shlock.h"
#include "../include/shfun.h"
#include "../include/shlog.h"
#include <fcntl.h>
#include <string>


int shstd::lock::CThreadCounter::m_nThreadCount[100] = {0};

shstd::lock::CMutex shstd::lock::CThreadCounter::m_mutex;

shstd::lock::CThreadCounter::CThreadCounter()
{
	m_nIndex = 0;
	CSafeMutex sm(m_mutex);
	m_nThreadCount[m_nIndex]++;
}
shstd::lock::CThreadCounter::CThreadCounter(unsigned int nIndex)
{
	m_nIndex = nIndex;
	if (m_nIndex >= 100) m_nIndex = 0;
    CSafeMutex sm(m_mutex);
    m_nThreadCount[m_nIndex]++;
}
shstd::lock::CThreadCounter::~CThreadCounter()
{
    CSafeMutex sm(m_mutex);
    m_nThreadCount[m_nIndex]--;
}
int shstd::lock::CThreadCounter::GetThreadCount(unsigned int nIndex)
{
    CSafeMutex sm(m_mutex);
    return m_nThreadCount[nIndex];
}

shstd::lock::CCond::CCond()
{
    pthread_cond_init(&m_tCond, NULL);
}
shstd::lock::CCond::~CCond()
{
    pthread_cond_destroy(&m_tCond);
}

bool shstd::lock::CCond::Wait(shstd::lock::CMutex & mutex, int nMillisecond)
{
    timespec to;
    to.tv_sec = nMillisecond / 1000;
    to.tv_nsec = (nMillisecond % 1000) * 1000000;

    int  nRet = pthread_cond_timedwait(&m_tCond, &mutex.m_tMutex, &to);
    return nRet == 0;
}

bool shstd::lock::CCond::Notify()
{
    bool bRet = pthread_cond_signal(&m_tCond);
    return bRet;
}


shstd::lock::CMutex::CMutex()
{
	pthread_mutex_init(&m_tMutex, NULL);
}

shstd::lock::CMutex::~CMutex()
{
	pthread_mutex_destroy(&m_tMutex);
}


bool shstd::lock::CMutex::Lock()
{
	return (0 == pthread_mutex_lock(&m_tMutex));
}


bool shstd::lock::CMutex::TryLock()
{
	return (0 == pthread_mutex_trylock(&m_tMutex));
}


bool shstd::lock::CMutex::Unlock()
{
	return (0 == pthread_mutex_unlock(&m_tMutex));
}


shstd::lock::CRWLock::CRWLock()
{
    pthread_rwlock_init(&m_rwlock, NULL);
}
shstd::lock::CRWLock::~CRWLock()
{
    pthread_rwlock_destroy(&m_rwlock);
}

bool shstd::lock::CRWLock::ReadLock()
{
    return (0 == pthread_rwlock_rdlock(&m_rwlock));
}
bool shstd::lock::CRWLock::WriteLock()
{
    return (0 == pthread_rwlock_wrlock(&m_rwlock));
}
bool shstd::lock::CRWLock::TryReadLock()
{
    return (0 == pthread_rwlock_tryrdlock(&m_rwlock));
}
bool shstd::lock::CRWLock::TryWriteLock()
{
    return (0 == pthread_rwlock_trywrlock(&m_rwlock));
}
bool shstd::lock::CRWLock::Unlock()
{
    return (0 == pthread_rwlock_unlock(&m_rwlock));
} 


pid_t shstd::filelock::GetFileLockPid(const char *szLockFile)
{
   
#ifdef __DEBUG__
    LOG(LT_DEBUG, "FIND_PID_TREE| szLockFile=%s..", szLockFile);
#endif

    pid_t pid = -1;
    if (szLockFile != NULL && shstd::fun::IsFileExist(szLockFile))
    {
        int fd = open(szLockFile, O_RDONLY);
        if (fd >= 0) 
        {
            struct flock ret;
            memset(&ret, 0, sizeof(ret));
            ret.l_pid = -1;

            fcntl(fd, F_GETLK, &ret);//记录锁

            if (ret.l_pid != -1)
            {
                LOG(LT_DEBUG, "GETFILELOCKPID | success fcntl |");
                pid = ret.l_pid;
            }
            else
            {
                char szPid[1024] = {0};
                if (read(fd, szPid, sizeof(szPid) - 2) > 0)
                {
                    LOG(LT_DEBUG, "GETFILELOCKPID | szPid=%s|", szPid);
                    int nLock = flock(fd, LOCK_EX|LOCK_NB);
                    if (nLock == 0) //锁了，就要释放
                    {                        
                        flock(fd, LOCK_UN);
                        LOG(LT_DEBUG, "GETFILELOCKPID| file(%s) is unlocked,|", szLockFile);
                        if (strstr(szPid, "(") != NULL) //不能锁定文件类型
                        {
                            set<pid_t> setPid;
#ifdef __DEBUG__
                            LOG(LT_DEBUG, "FIND_PID_TREE| pid=%u", atol(szPid));
#endif  
                            if (shstd::fun::GetPTree(atol(szPid), setPid)) //进程存在，判断关键串
                            {
                                char cmd[1024*5] = {0};
                                char *pEnd = strstr(szPid, ")");
                                if (pEnd != NULL)
                                {
                                    *pEnd = '\0';
                                    char *pStart = strstr(szPid, "(");
                                    if (pStart != NULL)
                                    {
                                        sprintf(cmd, "ps -eo pid,cmd|grep '%s'|grep -v 'ps -eo pid,cmd'|grep %ld", pStart + 1, atol(szPid));

                                       // LOG(LT_DEBUG, "NOT_LOCK_FILE| cmd=%s", cmd);

                                        FILE *pFile = popen(cmd, "r");
                                        if (pFile != NULL)
                                        {
                                            memset(cmd, 0, sizeof(cmd));
                                            while(!feof(pFile) &&  strlen(cmd) < (sizeof(cmd) - 10) )
                                            {
                                                if (fread(cmd + strlen(cmd), 1, (sizeof(cmd) - 10)  - strlen(cmd), pFile) <= 0) break;
                                            }
                                            pclose(pFile);


                                            //LOG(LT_DEBUG, "NOT_LOCK_FILE| psinfo=%s| szpid=%s|", cmd, szPid);


                                            if (atol(szPid) == atol(cmd)) //进程确实存在
                                            {
                                                pid = atol(szPid);
                                            }
                                        }

                                    }                           
                                    
                                }
                                
                            }
                        }
                        else
                        {
#ifdef __DEBUG__
                            LOG(LT_DEBUG, "FIND_PID_TREE| not found (");
#endif  
                        }
                    }
                    else if (-1 == nLock  && errno == EWOULDBLOCK)  //被锁
                    {
                        LOG(LT_DEBUG, "GetFileLockPid | file(%s) is locked |", szLockFile);
                        set<pid_t> setPid;
                        if (shstd::fun::GetPTree(atol(szPid), setPid))
                        {
                            pid = atol(szPid);
                            LOG(LT_DEBUG, "GETFILELOCKPID | file(%s) is locked | pid=%d |", szLockFile, pid);
                        }                        
                    }
                }
            }
            close(fd);
        }  

    }
    return pid;    
}

int shstd::filelock::GetFileLock(const char *szPath)
{
    if (szPath == NULL)
    {
        return -2;
    }

    char *pFind = strrchr((char *)szPath, '/');
    std::string strDir = szPath;
    if (pFind != NULL)
    {
        strDir.assign(szPath, (int)(pFind - szPath));
        if (!shstd::fun::MakeDir(strDir.c_str()))
        {
            return -4;
        }
    }


    
    int fd = open(szPath, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
    if (fd < 0) 
	{

		return -3;
	}

    const char *user = shstd::fun::GetRunUser();
	if (user != NULL)
	{
		fun::SetPathOwner(szPath, user);
	}

	if (1) //修改为flock
	{

		if (0 == flock(fd, LOCK_EX|LOCK_NB))
		{
			return fd;
		}
		else
		{
			close(fd);
			return -1;
		}

	}
	else
	{

		struct flock ret;
		ret.l_type = F_WRLCK ;
		ret.l_start = 0;
		ret.l_whence = SEEK_SET;
		ret.l_len = 100;
		ret.l_pid = getpid();

		if (0 == fcntl(fd, F_SETLK, &ret))
		{

			return fd;
		}
		else
		{
			close(fd);
			return -1;
		}
	}

}


