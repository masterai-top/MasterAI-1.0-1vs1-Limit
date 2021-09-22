

#ifndef __SH_STD_SAFE_LOCK__
#define __SH_STD_SAFE_LOCK__
#include <pthread.h>

namespace shstd
{
    namespace filelock
    {
        //返回被锁的进程id，负数：表示没有被锁或文件不存在
        pid_t GetFileLockPid(const char *szLockFile);

        //打开并锁定文件
        //返回-1:服务正在运行， >=0:服务没有运行， <-1:其它错误
        int GetFileLock(const char *szLockFile);
    }
}

#endif

