/**
* \file system.cpp
* \brief 系统封装类实现代码
*/

#include "pch.h"
#include "system.h"
#include "strutil.h"
#include "file.h"


namespace dtutil
{
    /**
    * \brief 线程休眠
    * \param msecs 休眠时间(毫秒)
    */
    void System::SleepMS(uint32 msecs)
    {
        struct timeval time;
        time.tv_sec     = msecs/1000;           //seconds
        time.tv_usec    = (msecs%1000) * 1000;  //microsecond
        select(0, NULL, NULL, NULL, &time);
    }

    /**
    * \brief 获取进程ID字符串
    * \return 进程ID字符串
    */
    std::string System::GetPIDStr()
    {
        std::string pid = "0";

#ifdef WIN32
        StrUtil::strformat(pid, "%06d", GetCurrentProcessId());
#else
        StrUtil::strformat(pid, "%06d", getpid());
#endif

        return pid;
    }

    /**
    * \brief 获取进程ID
    * \return 进程ID
    */
    uint32 System::GetPID()
    {
        uint32 pid = 0;

#ifdef WIN32
        pid = GetCurrentProcessId();
#else
        pid = getpid();
#endif

        return pid;
    }

    /**
    * \brief 获取进程名称
    * \return 进程名称
    */
    std::string System::GetProcessName()
    {
        std::string name = "main";
        char buf[260] = { 0 };

#ifdef WIN32
        if (GetModuleFileNameA(NULL, buf, 259) > 0)
        {
            name = buf;
        }
        std::string::size_type pos = name.rfind("\\");
        if (pos != std::string::npos)
        {
            name = name.substr(pos+1, std::string::npos);
        }
        pos = name.rfind(".");
        if (pos != std::string::npos)
        {
            name = name.substr(0, pos);
        }
#elif defined __APPLE__
        proc_name(getpid(), buf, 260);
        name = buf;
#else
        sprintf(buf, "/proc/%d/cmdline", getpid());
        File file;
        file.Open(buf, "r");
        if (file.IsOpen())
        {
            name = file.GetLine();
            file.Close();
            std::string::size_type pos = name.rfind("/");
            if (pos != std::string::npos)
            {
                name = name.substr(pos+1, std::string::npos);
            }
        }
#endif
        return name;
    }

    /**
    * \brief 根据进程名称获得进程ID
    * \param pname 进程名称
    * \param fpid 返回的进程ID列表
    * \return 成功找到返回true，否则返回fasle
    */
    bool System::GetPIDByName(const char *pname, int32 *fpid)
    {
#ifdef WIN32
        PROCESSENTRY32 proc;            //存放快照进程信息的一个结构体
        proc.dwSize = sizeof(proc);     //在使用这个结构之前，先设置它的大小

        HANDLE hProc = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);//给系统内的全部进程拍一个快照 
        if (hProc == INVALID_HANDLE_VALUE)  
        {  
            return false;  
        }

        int32 i = 0;

        BOOL bMore = Process32First(hProc, &proc);       //获取第一个进程信息
        
        while(bMore)
        {
#ifdef UNICODE
            if (strcmp(Encoding::UTF8.Transcode(proc.szExeFile).c_str(), pname) == 0)     //比较是否存在此进程
#else
            if (strcmp(proc.szExeFile, pname) == 0)     //比较是否存在此进程
#endif
            {
                fpid[i++] = proc.th32ProcessID;
            }
            bMore = Process32Next(hProc, &proc);
        }

        fpid[i] = 0;
        CloseHandle(hProc);

        if (i > 0)
        {
            return true;
        }
#else
        /* Open the /proc directory. */
        DIR *dir = opendir("/proc");
        if (NULL == dir)
        {
            return false;
        }

        fpid[0] = 0;
        int32 pnamelen = strlen(pname);

        /* Walk through the directory. */
        struct dirent *d;
        int32 i = 0;
        while((d = readdir(dir)) != NULL)
        {
            char exe[PATH_MAX+1];
            char path[PATH_MAX+1];

            /* See if this is a process */
            int32 pid = atoi(d->d_name);
            if (pid == 0)
            {
                continue;
            }

            snprintf(exe, sizeof(exe), "/proc/%s/exe", d->d_name);
            int32 len = readlink(exe, path, PATH_MAX);
            if (len < 0)
            {
                continue;
            }
            path[len] = '\0';

            /* Find ProcName */
            char *s = strrchr(path, '/');
            if (NULL == s)
            {
                continue;
            }
            ++s;

            /* we don't need small name len */
            int32 namelen = strlen(s);
            if (namelen < pnamelen)
            {
                continue;
            }

            if(0 == strncmp(pname, s, pnamelen))
            {
                /* to avoid subname like search proc tao but proc taolinke matched */
                if(s[pnamelen] == ' ' || s[pnamelen] == '\0')
                {
                    fpid[i++] = pid;
                }
            }
        }

        fpid[i] = 0;
        closedir(dir);

        if (i > 0)
        {
            return true;
        }
#endif
        return false;
    }

    /**
    * \brief 获得指定进程ID的进程名称
    * \param pid 进程ID
    * \return 进程名称
    */
    std::string System::GetPIDName(int32 pid)
    {
        std::string name = "";
        char buf[260] = { 0 };

#ifdef WIN32
        PROCESSENTRY32 proc;            //存放快照进程信息的一个结构体
        proc.dwSize = sizeof(proc);     //在使用这个结构之前，先设置它的大小

        HANDLE hProc = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);//给系统内的全部进程拍一个快照 
        if (hProc != INVALID_HANDLE_VALUE)  
        {  
            BOOL bMore = Process32First(hProc, &proc);      //获取第一个进程信息

            while(bMore)
            {
                if (proc.th32ProcessID == pid)              //比較是否存在此进程
                {
#ifdef UNICODE
                    name = Encoding::UTF8.Transcode(proc.szExeFile);
#else
                    name = proc.szExeFile;
#endif
                    break;
                }
                bMore = Process32Next(hProc, &proc);
            }

            CloseHandle(hProc);
        }
#elif defined __APPLE__
        proc_name(pid, buf, 260);
        name = buf;
#else
        sprintf(buf, "/proc/%d/cmdline", pid);
        File file;
        file.Open(buf, "r");
        if (file.IsOpen())
        {
            name = file.GetLine();
            file.Close();
            std::string::size_type pos = name.rfind("/");
            if (pos != std::string::npos)
            {
                name = name.substr(pos+1, std::string::npos);
            }
        }
#endif
        return name;
    }

    /**
    * \brief 查找进程是否存在
    * \param pid 进程ID
    * \return 进程存在返回true，否则返回false
    */
    bool System::FindPID(int32 pid)
    {
        bool bFind = false;

#ifdef WIN32
        PROCESSENTRY32 proc;            //存放快照进程信息的一个结构体
        proc.dwSize = sizeof(proc);     //在使用这个结构之前，先设置它的大小

        HANDLE hProc = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);//给系统内的全部进程拍一个快照 
        if (hProc == INVALID_HANDLE_VALUE)  
        {  
            return false;  
        }

        BOOL bMore = Process32First(hProc, &proc);      //获取第一个进程信息

        while(bMore)
        {
            if (proc.th32ProcessID == pid)              //比較是否存在此进程
            {
                bFind = true;
                break;
            }
            bMore = Process32Next(hProc, &proc);
        }

        CloseHandle(hProc);
#else
        char cmd[128] = { 0 };
        char buf[128] = { 0 };

        sprintf(cmd, "ps -ef|grep %d|wc -l", pid);
        FILE *file = popen(cmd, "r");
        if (NULL == file)
        {
            return false;
        }

        if (NULL != fgets(buf, sizeof(buf), file))
        {
            int32 count = atoi(buf);
            if (count - 2 != 0)     // 进程包括grep和自身进程组，所以多两个进程
            {
                bFind = true;
            }
        }
        pclose(file);
#endif

        return bFind;
    }

    /**
    * \brief 杀掉进程
    * \param pid 进程ID
    * \return 成功返回true，否则返回false
    */
    bool System::KillPID(int32 pid)
    {
#ifdef WIN32
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE,FALSE,pid);
        TerminateProcess(hProcess,0);
        CloseHandle(hProcess);
#else
        kill(pid, SIGTERM);
#endif
        return true;
    }

    /**
    * \brief 获取线程ID字符串
    * \return 线程ID字符串
    */
    std::string System::GetTIDStr()
    {
        std::string tid = "0";

#ifdef WIN32
        StrUtil::strformat(tid, "%08X", GetCurrentThreadId());
#else
        StrUtil::strformat(tid, "%08X", pthread_self());
#endif

        return tid;
    }

    /**
    * \brief 获取线程ID
    * \return 线程ID
    */
    uint32 System::GetTID()
    {
        uint32 tid = 0;

#ifdef WIN32
        tid = GetCurrentThreadId();
#else
        tid = pthread_self();
#endif

        return tid;
    }

    /**
    * \brief 开启守护进程模式
    */
    void System::OpenDaemon()
    {
#ifdef WIN32
#else
        pid_t pid;
        if ( (pid = fork()) != 0 )
        {
            exit(0);
        }

        setsid();   // become session leader

        if ( (pid = fork()) != 0 )
        {
            exit(0);
        }

        umask(0);   // clear our file mode creation mask
#endif
    }

    /**
    * \brief 得到CPU核数
    * \return CPU核数
    */
    uint32 System::GetCPUNum()
    {
#ifdef WIN32
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        return info.dwNumberOfProcessors;
#else
        return sysconf(_SC_NPROCESSORS_ONLN);
#endif
    }

    /**
    * \brief 得到物理内存大小
    * \return 物理内存大小
    */
    uint64 System::GetMemorySize()
    {
#ifdef WIN32
        MEMORYSTATUSEX info = { sizeof(MEMORYSTATUSEX) };
        GlobalMemoryStatusEx(&info);
        return info.ullTotalPhys;
#else
        return sysconf(_SC_PHYS_PAGES) * (unsigned long long) PAGE_SIZE;
#endif
    }
}