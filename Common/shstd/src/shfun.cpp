#include "../include/sysheads.h"
#include "../include/shfun_in.h"
#include "../include/shfun.h"
#include "../include/shlock.h"
#include "../include/shlock_in.h"
#include "../include/shlog.h"
#include "../include/shlogimp.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

static char  MODULE_NAME[256]= {0};
static char  PROG_NAME[256] = {0};
static char  PROG_TID[64] = {0};
static pid_t MONITOR_PARENT_PID = -1;

void shstd_printf(const char *fmt, ...)
{
    if (shstd::fun::GetWorkHomePath(false) == NULL) return;

    if (PROG_TID[0] == '\0') //其它进程，没有设置这个信�?
    {
        sprintf(PROG_TID, "%05d-%lx", getpid(), time(NULL));
    }
    if (PROG_NAME[0] == '\0')
    {
        const char * p = shstd::fun::GetProgramPath();
        if (p != NULL)
        {
            p = strrchr((char *)p, '/');
            if (p != NULL)
            {
                strncpy(PROG_NAME, p+1, sizeof(PROG_NAME) - 1);
            }
        }
    }

    va_list vl;
    va_start(vl, fmt);

    timeval tv;
    gettimeofday(&tv, NULL);

    tm tmNow;
    localtime_r(&tv.tv_sec, &tmNow);

    char buf[1024] = {0};
    sprintf(buf, "[%02d:%02d:%02d.%03d|%s|%s|p:%d,t:%lx]:",
            tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec,(int)(tv.tv_usec/1000),
            PROG_TID,PROG_NAME,
            getpid(),pthread_self());

    //写到文件�?
    char szDayFileName[1024] = {0};


    sprintf(szDayFileName, "%s/logs/shstd.log.%04d-%02d-%02d",
            shstd::fun::GetWorkHomePath(),
            tmNow.tm_year + 1900, tmNow.tm_mon + 1, tmNow.tm_mday);
    int nIndex = 1;
    while(1)
    {
        struct stat st;
        if (0 == stat(szDayFileName, &st))
        {
            if (st.st_size > (2*1024 - 100)*1024*1024)
            {
                sprintf(szDayFileName, "%s/logs/shstd.log.%04d-%02d-%02d.%d",
                        shstd::fun::GetWorkHomePath(),
                        tmNow.tm_year + 1900, tmNow.tm_mon + 1, tmNow.tm_mday, ++nIndex);
            }
            else break;
        }
        else break;
    }

    FILE * fp = fopen(szDayFileName, "ab+");
    if (fp != NULL)
    {
        int len=strlen(buf);
        if (fwrite(buf, 1, len, fp) > 0)
        {
            if ( vfprintf(fp, fmt, vl) > 0)
            {
                fwrite("\n", 1, 1, fp);
            }
        }

        fclose(fp);

        fp = NULL;
    }
    va_end(vl);

    //创建符号映射
    char szLink[1024];
    sprintf(szLink, "%s/logs/shstd.log", shstd::fun::GetWorkHomePath());
    char *p = strrchr(szDayFileName, '/');
    if (p != NULL) p++;
    else p = szDayFileName;

    unlink(szLink);
    if (symlink(p, szLink) == 0)
    {
    }

    if (shstd::fun::GetRunUser() != NULL)
    {
        shstd::fun::SetPathOwner(szDayFileName, shstd::fun::GetRunUser());
    }

    chmod(szDayFileName, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
}

static int discryptstr (  int key, unsigned char *data,  int length )
{
    int     i;
    int     sum ;

    sum = key;

    while ( sum >= 10 )
        sum = sum/10 + sum%10;

    sum %= 8;

    if ( sum == 0 ) sum = 4;

    for (i=0; i<length; i++)
    {
        sum++;
        sum = sum%8;
        data[i] = ( data[i] << ( 8-sum ) ) | ( data[i] >> sum );

    }

    return 0;
}


const char * shstd::fun::DecodeDBPass(char *szEnPass, char * szDePass,  int key)
{
    if (szEnPass == NULL || szDePass == NULL) return NULL;

    char pm[3]= {0};
    int len = strlen(szEnPass);
    if (len == 0 || len % 2 != 0) return NULL;
    len /= 2;
    for (int  i=0; i<len; i++)
    {
        pm[0] = szEnPass[ i*2];
        pm[1] = szEnPass[ i*2+1];
        unsigned int dec = 0;
        if (0 == sscanf(pm,"%02x",&dec))
        {
            return NULL;
        }
        szDePass[i] = dec;
    }
    szDePass[len] = '\0';

    discryptstr(key, (unsigned char *)szDePass, len);
    return szDePass;
}

bool shstd::fun::IsIp(const string & strIp)
{
    if (IsIpV4(strIp.c_str()) || IsIpV6(strIp.c_str()) ) return true;
    else return false;
}


bool shstd::fun::IsIpV4(const char * ip) //是否是ipv4
{
    if (ip == NULL || strlen(ip) == 0) return false;
    in_addr in4;
    if (inet_pton(AF_INET, ip, &in4) > 0) //ipv4
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool shstd::fun::IsIpV6(const char * ip) //是否是ipv6
{
    if (ip == NULL || strlen(ip) == 0) return false;
    in6_addr in6;
    if (inet_pton(AF_INET6, ip, &in6) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool shstd::fun::IsConnected(const char *ip, unsigned short port)
{
    if (ip == NULL || strlen(ip) == 0 || port == 0) return false;
    if (IsIpV4(ip))
    {
        sockaddr_in si;
        memset(&si, 0, sizeof(si));

        inet_pton(AF_INET, ip, &si.sin_addr);
        si.sin_port = htons(port);
        si.sin_family = AF_INET;

        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) return false;
        if (0 != connect(fd, (sockaddr *)&si, sizeof(si)))
        {
            close(fd);
            return false;
        }

        close(fd);
        return true;
    }
    else if (IsIpV6(ip))
    {
        sockaddr_in6 si6;
        memset(&si6, 0, sizeof(si6));

        inet_pton(AF_INET6, ip, &si6.sin6_addr);
        si6.sin6_port = htons(port);
        si6.sin6_family = AF_INET6;

        int fd = socket(AF_INET6, SOCK_STREAM, 0);
        if (fd < 0) return false;
        if (0 != connect(fd, (sockaddr *)&si6, sizeof(si6)))
        {
            close(fd);
            return false;
        }

        close(fd);
        return true;
    }
    else
    {
        return false;
    }
}

//解析域名
int shstd::fun::ParseDns(const char * szDns, IP_INFO  ipinfos[], unsigned int nIpInfoSize, unsigned short port)
{
    if (szDns == NULL || strlen(szDns) == 0 || ipinfos == NULL || nIpInfoSize == 0)
    {
        return -1;
    }

    //if (shstd::fun::IsIp(szDns))
    //{
    //	strncpy(ipinfos[0].ip, szDns, sizeof(ipinfos[0].ip) - 1);
    //	return 1;
    //}

    unsigned int nIndex = 0;

    struct addrinfo *answer, hint, *curr;
    char ipstr[64];

    if (shstd::fun::IsIp(szDns))
    {
        strncpy(ipinfos[nIndex].ip, szDns, sizeof(ipinfos[nIndex].ip) -1);
        nIndex++;
        return  nIndex;
    }


    {
        //ipv4
        memset(&hint, 0, sizeof(hint));
        hint.ai_family = AF_INET;
        hint.ai_socktype = SOCK_STREAM;

        int ret = getaddrinfo(szDns, NULL, &hint, &answer);
        if (ret != 0)
        {
            return  -2;
        }

        for (curr = answer; curr != NULL; curr = curr->ai_next)
        {

            if (nIndex >= nIpInfoSize)
            {
                break;
            }
            memset(ipstr, 0, sizeof(ipstr));

            if (curr->ai_family == AF_INET
                    && inet_ntop(AF_INET, &(((struct sockaddr_in *)(curr->ai_addr))->sin_addr), ipstr, sizeof(ipstr) - 1) != NULL
               )
            {
                if (port)
                {
                    if (shstd::fun::IsConnected(ipstr, port))
                    {
                        strncpy(ipinfos[nIndex++].ip, ipstr, sizeof(IP_INFO));
                    }
                }
                else
                {
                    strncpy(ipinfos[nIndex++].ip, ipstr, sizeof(IP_INFO));
                }
            }
            else if (curr->ai_family == AF_INET6
                     && inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)(curr->ai_addr))->sin6_addr), ipstr, sizeof(ipstr) - 1) != NULL
                    )
            {
                if (port)
                {
                    if (shstd::fun::IsConnected(ipstr, port))
                    {
                        strncpy(ipinfos[nIndex++].ip, ipstr, sizeof(IP_INFO));
                    }
                }
                else
                {
                    strncpy(ipinfos[nIndex++].ip, ipstr, sizeof(IP_INFO));
                }
            }
            else
            {

            }
        }
        freeaddrinfo(answer);
    }

    if (nIndex < nIpInfoSize) //数组没满，继续ipv6解析
    {
        //ipv6
        memset(&hint, 0, sizeof(hint));
        hint.ai_family = AF_INET6;
        hint.ai_socktype = SOCK_STREAM;

        int ret = getaddrinfo(szDns, NULL, &hint, &answer);
        if (ret != 0)
        {
            if (nIndex == 0)
            {
                return  -3;
            }
        }
        else
        {
            for (curr = answer; curr != NULL; curr = curr->ai_next)
            {
                if (nIndex >= nIpInfoSize)
                {
                    break;
                }
                memset(ipstr, 0, sizeof(ipstr));

                if (curr->ai_family == AF_INET
                        && inet_ntop(AF_INET, &(((struct sockaddr_in *)(curr->ai_addr))->sin_addr), ipstr, sizeof(ipstr) - 1) != NULL
                   )
                {
                    if (port)
                    {
                        if (shstd::fun::IsConnected(ipstr, port))
                        {
                            strncpy(ipinfos[nIndex++].ip, ipstr, sizeof(IP_INFO));
                        }
                    }
                    else
                    {
                        strncpy(ipinfos[nIndex++].ip, ipstr, sizeof(IP_INFO));
                    }
                }
                else if (curr->ai_family == AF_INET6
                         && inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)(curr->ai_addr))->sin6_addr), ipstr, sizeof(ipstr) - 1) != NULL
                        )
                {
                    if (port)
                    {
                        if (shstd::fun::IsConnected(ipstr, port))
                        {
                            strncpy(ipinfos[nIndex++].ip, ipstr, sizeof(IP_INFO));
                        }
                    }
                    else
                    {
                        strncpy(ipinfos[nIndex++].ip, ipstr, sizeof(IP_INFO));
                    }
                }
                else
                {

                }
            }
            freeaddrinfo(answer);
        }
    }

    return nIndex;
}


//解析出服务的访问信息结构
bool shstd::fun::ParseDnsAndIp(const string & strAccessInfo, SVR_ACCESS_INFO &sai)
{
    string strAI = shstd::fun::CleanStr(strAccessInfo);
    //{dns=xxx;ip=xxx,xxx2,xxx3;localip=xxx}
    //ip=xxx;


    const char *pStart = strAI.c_str();
    if (strchr(pStart, '{') == NULL) //单一ip
    {
        //检测是否ip
        if (IsIp(strAI)) //是ip
        {
            sai.strDns = pStart;
            sai.strIp = pStart;
            return true;
        }
        else //不是正确的ip
        {
            return false;
        }
    }
    else //复杂
    {
        pStart = strchr(pStart, '{') + 1;
        char *pEnd = strchr((char *)pStart, '}');
        if (pEnd != NULL) *pEnd = '\0';

        //找ip
        char *ipStart = strcasestr((char *)pStart, "ip=");
        if (ipStart == NULL)
        {
            return false;
        }
        ipStart += strlen("ip=");
        char *ipEnd = strchr(ipStart, ';');
        if (ipEnd == NULL)
        {
            sai.strIp = ipStart;
        }
        else
        {
            sai.strIp.assign(ipStart, ipEnd);
        }

        if (!IsIp(sai.strIp))
        {
            return false;
        }

        //解析域名
        char *dnsStart = strcasestr((char *)pStart, "dns=");
        if (dnsStart == NULL)
        {
            if ( (dnsStart = strcasestr((char *)pStart, "dn=")) == NULL )
            {
                sai.strDns = sai.strIp;
            }
            else
            {
                dnsStart += strlen("dn=");
            }
        }
        else
        {
            dnsStart += strlen("dns=");
        }

        if  (dnsStart != NULL)
        {

            char *dnsEnd = strchr((char *)pStart, ';');
            if (dnsEnd == NULL)
            {
                sai.strDns = dnsStart;
            }
            else
            {
                sai.strDns.assign(dnsStart, dnsEnd);
            }
            if (sai.strDns.size() <= 1)
            {
                return false;
            }
        }
    }
    return true;
}

pid_t shstd::fun::GetMonitorParentPid()
{
    return MONITOR_PARENT_PID;
}

bool shstd::fun::SetMonitorParentPid(pid_t ppid)
{
    if (MONITOR_PARENT_PID <= 0)
    {
        MONITOR_PARENT_PID = ppid;
        return true;
    }
    else
    {
        return false;
    }
}

char * shstd::fun::TrimMobile(char *szMobile)
{
    //全部是数�?
    char *p = szMobile;
    while (*p != '\0')
    {
        if (*p == '+' || isdigit(*p)) p++;
        else return szMobile;
    }

    if (strlen(szMobile) < 11) return szMobile;

    if (strncmp(szMobile, "86", 2) == 0)
    {
        memmove(szMobile, szMobile + 2, strlen(szMobile) - 1);
    }
    else if (strncmp(szMobile, "0086", 4) == 0)
    {
        memmove(szMobile, szMobile + 4, strlen(szMobile) - 3);
    }
    else if (strncmp(szMobile, "++86", 4) == 0)
    {
        memmove(szMobile, szMobile + 4, strlen(szMobile) - 3);
    }

    else if (strncmp(szMobile, "+86", 3) == 0)
    {
        memmove(szMobile, szMobile + 3, strlen(szMobile) - 2);
    }
    else
    {}

    return szMobile;
}


char * shstd::fun::TrimUsername(char *szUser)
{
    if (szUser == NULL) return NULL;
    char *pRet = szUser;
    while(*pRet != '\0')
    {
        if (*pRet == ' ' || *pRet == '\"' || *pRet == '\'') pRet++;
        else break;
    }
    char *pEnd = pRet + strlen(pRet) - 1;
    while (pEnd > pRet)
    {
        if (*pEnd == ' ' || *pEnd == '\"' || *pEnd == '\'') *pEnd-- = '\0';
        else break;
    }
    memmove(szUser, pRet, strlen(pRet) + 1);
    return szUser;
}

unsigned int g_md_cnt = 0;
const char * shstd::fun::GetPeerIp(int nSocket)
{
    sockaddr_in si;
    memset(&si, 0, sizeof(si));
    socklen_t nLen = sizeof(si);
    getpeername(nSocket, (sockaddr *)&si, &nLen);
    return inet_ntoa(si.sin_addr);
}
bool shstd::fun::SetPathOwner(const string &strPath, const string &strUser)
{
    bool bRet = false;
    struct passwd   *pwd = getpwnam(strUser.c_str());
    if (pwd != NULL)
    {
        if (0 == chown(strPath.c_str(), pwd->pw_uid, pwd->pw_gid))
        {
            bRet = true;
        }
    }
    return bRet;
}

bool shstd::fun::SetPathMod(const string & strPath, int nMode)
{
    bool bRet = false;
    if (0 == chmod(strPath.c_str(), nMode))
    {
        bRet = true;
    }
    return bRet;
}

int shstd::fun::GetParamPos(int argc, const char **argv, const char *szFind)
{
    int nRet = -1;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], szFind) == 0)
        {
            nRet = i;
            break;
        }
    }
    return nRet;
}

bool shstd::fun::GetPathOwner(const string &strPath, string &strUser)
{
    struct stat st;
    if (0 == stat(strPath.c_str(), &st))
    {
        struct passwd *passwd;
        passwd = getpwuid(st.st_uid);
        if (passwd != NULL)
        {
            strUser = passwd->pw_name;
            return true;
        }
    }

    return false;
}

//当前仅支持取/home/ 目录的下一级路�?
const char * shstd::fun::GetRunUser()
{
    static char  szRunUser[1024] = {0};
    if (strlen(szRunUser) == 0)
    {
        const char *pWorkPath = GetWorkHomePath();

        //仅支持以第二个路径作为用户名的情�? �? /mroot/user/awas/.. 目录,用户名为user
        if (pWorkPath != NULL && pWorkPath[0] == '/')
        {
            const char *start = NULL;
            const char *end   = NULL;
            if(NULL != (start = strstr(pWorkPath+1, "/")))
            {
                start += 1;
                if(NULL != (end = strstr(start, "/")))
                {
                    strncpy(szRunUser, start, end - start);
                }
            }
        }
    }

    if (strlen(szRunUser) > 0) {
        return szRunUser;
    }
    else {
        return NULL;
    }
}

const char * shstd::fun::GetModuleName()
{
    return MODULE_NAME[0]?MODULE_NAME:NULL;
}
const char * shstd::fun::GetProgramPath()
{
    static char buf[2048] = {0};
    if (strlen(buf) == 0)
    {
        int count;
        count = readlink( "/proc/self/exe", buf, sizeof(buf) );

        if ( count < 0 || count >= (int)sizeof(buf) )
        {
            memset(buf, 0, sizeof(buf));
        }
        buf[ count ] = '\0';
    }

    if (strlen(buf) == 0) return NULL;
    else return buf;
}


const char * shstd::fun::GetWorkHomePath(bool bPrintErr)
{
    static char szHomePath[2048] = {0};

    if (strlen(szHomePath) == 0 && GetProgramPath() != NULL)
    {
        strcpy(szHomePath, GetProgramPath());
        char *p = NULL;

        while(1) {
            p = strrchr((char *)szHomePath, '/');
            if (p == NULL) //没有找到，则跳出
            {
                memset(szHomePath, 0, sizeof(szHomePath));
                break;
            }
            else
            {
                *p = '\0';  
                string strNewconf = string(szHomePath) + "/conf";
                string strBin = string(szHomePath) + "/bin";
                string strSbin = string(szHomePath) + "/sbin";

                if ( shstd::fun::IsDirExist(strNewconf.c_str())
                        && shstd::fun::IsDirExist(strBin.c_str())
                        && shstd::fun::IsDirExist(strSbin.c_str())
                   )
                {
                    break;
                }
            }
        }
    }
    if (strlen(szHomePath) == 0)
    {
        if (bPrintErr)
        {
            fprintf(stderr, "Work home directory should include subdir(bin,sbin,conf) must exist. please check.\n");
        }
        return NULL;
    }
    else return szHomePath;
}

bool shstd::fun::SvrStdInit(int argc, const char **argv, const char *szModuleName, const char *szUser, int nochdir, int noclose)
{
    if (argc < 1 || argv == NULL || szModuleName == NULL || szUser == NULL) return false;

    if (GetParamPos(argc, argv, "--debug") > 0)
    {
        return true;
    }

    char szPidFile[1024] = {0};
    sprintf(szPidFile, "%s/var/%s/%s.pid", shstd::fun::GetWorkHomePath(), szModuleName, szModuleName);
    strncpy(MODULE_NAME, szModuleName, sizeof(szModuleName) - 2);

    string strCmd;
    for (int i=0; i < argc; i++)
    {
        strCmd += argv[i];
        strCmd += " ";
    }
    shstd_printf("SVR_STD_INIT| cmd=%s| user=%s| pid=%s|", strCmd.c_str(), szUser, szPidFile);

    pid_t pidLock = shstd::filelock::GetFileLockPid(szPidFile);
    if (pidLock >= 0) //服务已经被启�?
    {
        if (GetParamPos(argc, argv, "--restart") > 0)
        {
            char szStop[1024] = {0};
            sprintf(szStop,"%s --stop > /dev/null", argv[0]);
            system(szStop);
        }
        else if (GetParamPos(argc, argv, "--stop") > 0)
        {
            FILE *pIn = fopen(szPidFile, "rb");
            if (pIn != NULL)
            {
                fclose(pIn);
                if (!shstd::fun::KillTree(pidLock))
                {
                    fprintf(stderr, "failed to stop svr\n");
                    shstd_printf("SVR_STD_INIT| cmd=%s| user=%s| pid=%d| msg=failed to stop svr|", strCmd.c_str(), szUser, pidLock);
                    exit(1);
                }
                else
                {
                    //删除pid文件
                    unlink(szPidFile);
                    exit(0);
                }

            }
            else
            {
                printf("failed to open pidfile.(%s).\n", szPidFile);
                shstd_printf("SVR_STD_INIT| cmd=%s| user=%s| pid=%d| msg=failed to open pidfile.(%s)|", strCmd.c_str(), szUser, pidLock, szPidFile);
                exit(1);
            }
        }
        else
        {
            printf("*** %s is running.\n", strrchr((char *)argv[0], '/') == NULL ? argv[0]:(strrchr(argv[0], '/') + 1));
            exit(1);
        }
    }
    else //没出现异常，可以正常启动服务
    {
        if (GetParamPos(argc, argv, "--stop") > 0)
        {
            exit(0);
        }

        if (argc > 1 && GetParamPos(argc, argv, "--start") < 0 && GetParamPos(argc, argv, "--restart") < 0 ) //操作错误
        {
            printf("not found param --start/--restart/--debug\n");
            exit(1);
        }
    }

    if (0 != daemon(nochdir, noclose))
    {
        printf("failed to demonize.\n");
        shstd_printf("SVR_STD_INIT| cmd=%s| user=%s| msg=failed to domonize.|", strCmd.c_str(), szUser);
        exit(1);
    }

    int nLockFile = -1;
    int nCount = 0;
    while(nLockFile < 0)
    {
        nCount++;
        nLockFile = shstd::filelock::GetFileLock(szPidFile);
        if (nLockFile < 0)
        {
            fprintf(stderr, "failed to open pid file(%s) ret=%d\n", szPidFile, nLockFile);
            shstd_printf("SVR_STD_INIT| cmd=%s| user=%s| nLockFile=%d| nCount=%d| msg=failed to open pidfile for new svr.(%s)|", strCmd.c_str(), szUser, nLockFile, nCount, szPidFile);
            if (nCount > 5) exit(1);
            else sleep(1);
        }
    }

    ftruncate(nLockFile, 0);
    char pid[1024] = {0};
    sprintf(pid, "%u", getpid());
    write(nLockFile, pid, strlen(pid));

    //监控自己
    shstd::fun::MonitorSelf(argc, argv);

    return true;
}


bool shstd::fun::ChangeRunUser(const char *szUser)
{
    bool bRet = false;
    if (szUser != NULL)
    {
        struct passwd   *pwd = getpwnam(szUser);
        if (pwd != NULL)
        {
            if (pwd->pw_gid == getgid())
            {
                if (pwd->pw_uid == getuid())
                {
                    bRet = true;
                }
                else
                {
                    bRet = (0 == setuid(pwd->pw_uid));
                }

            }
            else
            {
                if (0 == setgid(pwd->pw_gid))
                {
                    if (pwd->pw_uid == getuid())
                    {
                        bRet = true;
                    }
                    else
                    {
                        bRet = (0 == setuid(pwd->pw_uid));
                    }

                }

            }

        }
    }

    return bRet;
}

bool shstd::fun::ParseGetElems(const string &str, vector<string> &vecElem, char chSeparator)
{
    vecElem.clear();
    char *pStart = (char *)str.c_str();
    string strElem;
    while (1)
    {
        char *pEnd = strchr(pStart, chSeparator);
        if (pEnd == NULL)
        {
            strElem = pStart;
        }
        else
        {
            strElem.assign(pStart, pEnd);
            pStart = pEnd + 1;
        }
        if (!strElem.empty())
        {
            vecElem.push_back(strElem);
        }
        if (pEnd == NULL)
        {
            break;
        }
    }
    return (!vecElem.empty());
}

bool shstd::fun::GetPTree(pid_t pid, set<pid_t> & setPTree)
{
    setPTree.clear();
    //给进程树发一个信�?
    char szCmd[1024]= {0};
    sprintf(szCmd, "pstree -p %u", pid);
    FILE *pIn = popen(szCmd, "r");
    if (pIn != NULL)
    {
        string strTreeInfo;
        while (!feof(pIn))
        {
            char buf[1024] = {0};
            int nRead = fread(buf, 1, sizeof(buf) - 2, pIn);
            if (nRead <= 0) break;
            strTreeInfo += buf;
        }
        pclose(pIn);


        const char *pFind = strTreeInfo.c_str();
        while ( (pFind = strchr(pFind, '(')) != NULL)
        {
            if ( *(pFind - 1) != '}')
            {
                pid_t pid = atol(pFind + 1);
                if (pid != 0) setPTree.insert(pid);
            }
            pFind++;
        }
    }
    return (setPTree.size() > 0);
}

bool shstd::fun::KillTree(pid_t pid, unsigned int nMaxWaitSeconds)
{
    if (pid == 0) return true;
    bool bRet = true;
    //给进程树发一个信�?
    set<pid_t> setPTree;
    if (GetPTree(pid, setPTree))
    {
        kill(pid, SIGINT);
        set<pid_t>::iterator it;
        for (it = setPTree.begin(); it != setPTree.end(); it++)
        {
            int nKillRet = kill((*it), SIGINT);
            stringstream ss;
            ss<<"to_pid="<<*it<<"| signal="<<SIGINT<<"| ret="<<nKillRet;
            shstd_printf("SEND_SIGNAL| %s|", ss.str().c_str());

            //杀父进程后，确保父进程不存在，然后再杀其它进程
        }


        set<pid_t> setTmp;
        nMaxWaitSeconds *= 10;
        while (nMaxWaitSeconds > 0 && !setPTree.empty())
        {
            if (!GetPTree(*setPTree.begin(), setTmp))
            {
                setPTree.erase(setPTree.begin());
            }
            usleep(100000);
            nMaxWaitSeconds--;
        }

        bool bSendKill = false;
        //强制杀�?
        for (it = setPTree.begin(); it != setPTree.end(); it++)
        {

            int nKillRet = kill((*it), SIGKILL);

            stringstream ss;
            ss<<"to_pid="<<*it<<"| signal="<<SIGKILL<<"| ret="<<nKillRet;
            shstd_printf("SEND_SIGNAL| %s|", ss.str().c_str());
            bSendKill = true;
        }

        if (bSendKill) sleep(2);


        //检查进程是否还存在
        for (set<pid_t>::iterator it = setPTree.begin(); it != setPTree.end(); it++)
        {
            set<pid_t> tmp;
            if (shstd::fun::GetPTree(*it, tmp))
            {
                bRet = false;
                break;
            }

        }



    }

    return bRet;
}

static unsigned int g_nForkCnt = 0;
static time_t g_tParentStartup = time(NULL);
static time_t g_tWorkStartup = time(NULL);

unsigned int shstd::fun::GetForkCnt()
{
    return g_nForkCnt;
}
time_t shstd::fun::GetParentProgressStartup() //父进程启动时�?
{
    return g_tParentStartup;
}
time_t shstd::fun::GetWorkProgressStartup() //子进程启动时�?
{
    return g_tWorkStartup;
}


static void SigChld(int sig) //消灭所有僵�?
{
    int nStatus;
    int nTimes = 1000;
    while (nTimes-- > 0)
    {
        pid_t pid = waitpid(-1, &nStatus, WNOHANG);
        if (pid < 0) break;
    }
}

void shstd::fun::MonitorSelf(unsigned int nSleepSeconds)
{
    MonitorSelf(0, NULL, nSleepSeconds);
}

void shstd::fun::MonitorSelf(int argc, const char **argv, unsigned int nSleepSeconds) //监控自己
{
    if (nSleepSeconds == 0) nSleepSeconds = 3;

    if (argc > 0 && argv != NULL && strlen(PROG_NAME) == 0)
    {
        sprintf(PROG_TID, "%05d-%lx", getpid(), time(NULL));

        const char *prog = strrchr((char *)argv[0], '/');
        if (prog == NULL) prog = argv[0];
        else prog++;
        strncpy(PROG_NAME, prog, sizeof(PROG_NAME) - 1);

        string strCmd;
        for (int i=0; i < argc; i++)
        {
            strCmd += argv[i];
            strCmd += " ";
        }
        shstd_printf("CMD| cmd=%s|", strCmd.c_str());
    }
    MONITOR_PARENT_PID = getpid();

    const int TIME_SPAN = 10; //10分钟之内�?
    const int NORMAL_TIME_SPAN = 10;
    long nTimes = 0;
    time_t tTimeSpan = 0;
    while (1)
    {
        time_t tLast = time(NULL);
        g_nForkCnt++;
        pid_t subpid = fork();
        if (subpid < 0)
        {
            exit(1);
        }
        else if (subpid != 0) //父进�?
        {
            SigChld(0); //将所有僵死进程回收干净

            shstd_printf("MON_FORK_CHILD| parent_pid=%d| sub_pid=%d|", getpid(), subpid);
            signal(SIGCHLD, SigChld);
            bool bContinue = true;
            while (bContinue)
            {
                int nStatus;
                pid_t pidWait = waitpid(subpid, &nStatus, 0);

                shstd_printf("MON_PARENT_WAIT| parent_pid=%d| wait_pid=%d| sub_pid=%d| errno=%d| errmsg=%s|"
                             " WIFEXITED=%s| WEXITSTATUS=%d|"
                             " WIFSIGNALED=%s| WTERMSIG=%d|"
                             " WCOREDUMP=%s| WIFSTOPPED=%s|"
                             " WSTOPSIG=%d| WIFCONTINUED=%s| ",
                             getpid(), pidWait, subpid, errno, strerror(errno),
                             WIFEXITED(nStatus)?"true":"false", WEXITSTATUS(nStatus),
                             WIFSIGNALED(nStatus)?"true":"false", WTERMSIG(nStatus),
                             WCOREDUMP(nStatus)?"true":"false",WIFSTOPPED(nStatus)?"true":"false",
                             WSTOPSIG(nStatus), WIFCONTINUED(nStatus)?"true":"false"
                            );

                if (pidWait == subpid)
                {
                    if (WIFEXITED(nStatus)) //正常退�? 进程每隔三秒启动
                    {
                        shstd_printf("WAIT_EXIT| parent_pid=%d| msg=exit or _exit|", getpid());
                        bContinue = false;
                        nTimes = 1;
                    }
                    else if (WIFSIGNALED(nStatus)) //崩溃或接收信号退�?
                    {
                        shstd_printf("WAIT_SIGNAL| parent_pid=%d| msg=signal exit|", getpid());
                        bContinue = false;
                    }
                    else //休眠或被唤醒
                    {
                        shstd_printf("WAIT_STOP_WAKE| parent_pid=%d| msg=stop or wake|", getpid());
                    }
                }
                else
                {
                    //取得进程�?
                    set<pid_t>  setPTree;
                    if (shstd::fun::GetPTree(getpid(), setPTree))
                    {
                        shstd_printf("GET_PTREE| parent_pid=%d| tree_size=%u|", getpid(), setPTree.size());
                        if (setPTree.find(subpid) == setPTree.end())  //子进程已经不在了
                        {
                            bContinue = false; //发生异常，没发现子进�?
                            nTimes = 1;
                        }
                    }
                    else
                    {
                        shstd_printf("GET_PTREE| parent_pid=%d| msg=failed to get ptree.|", getpid());
                    }
                }
            }
            signal(SIGCHLD, SIG_IGN);
        }
        else //子进�?
        {
            g_tWorkStartup = time(NULL);
            break;
        }

        tTimeSpan = time(NULL) - tLast;


        //tLast = time(NULL);
        if ( tTimeSpan > NORMAL_TIME_SPAN) //一个小时没core
        {
            nTimes = 0;
        }

        if ( tTimeSpan < TIME_SPAN)
        {
            if (nTimes > 0)
            {

                int nTotalSeconds = nTimes* nSleepSeconds;
                shstd_printf("MONITOR_PROGRESS| sleep_second=%d|", nTotalSeconds);
                while(nTotalSeconds-- > 0) sleep(1);
            }

            if (nTimes == 0) nTimes = 1;
            else nTimes *= 2;
        }

    }
}

char * shstd::fun::cp2ap(char *path) //相对路径到绝对路�?
{
    char p[1024] = {0};
    if (path[0] == '/') strcpy(p, path);
    else
    {
        getcwd(p, sizeof(p));
        strcat(p, "/");
        strcat(p, path);
    }
#ifdef __DEBUG__
    fprintf(stderr, "init,path=%s\n", p);
#endif
    //对p进行处理
    char *pFind = NULL;
    while ( (pFind = strstr(p, "/./")) != NULL)
    {
        memmove(pFind, pFind+2, strlen(pFind+2) + 1);
    }
#ifdef __DEBUG__
    fprintf(stderr, "remove /./,path=%s\n", p);
#endif
    while( (pFind = strstr(p, "/../")) != NULL)
    {
        char *pNext = pFind+3;
        *pFind = '\0';
        char *pPrev = strrchr(p, '/');
        if (pPrev == NULL)
        {
            *pFind = '/';
            break;
        }
        memmove(pPrev, pNext, strlen(pNext) + 1);
    }
#ifdef __DEBUG__
    fprintf(stderr, "remove /../,path=%s\n", p);
#endif
    strcpy(path, p);
    return path;
}


string shstd::fun::CleanStr(const string & str)
{
    string strRet;
    int nCount = str.size();
    for (int i = 0; i < nCount; i++)
    {
        if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n' && str[i] != '\r')
        {
            strRet += str[i];
        }
    }

    return strRet;
}

unsigned long shstd::fun::Hex2Int(const char *szHex, unsigned int nSize)
{
    unsigned long nRet = 0;
    for (unsigned int i = 0; i < nSize; i++)
    {
        if (szHex[i] >= '0' && szHex[i] <= '9')
        {
            nRet *= 16;
            nRet += szHex[i] - '0';
        }
        else if (szHex[i] >= 'a' && szHex[i] <= 'f')
        {
            nRet *= 16;
            nRet += szHex[i] - 'a' + 10;

        }
        else if (szHex[i] >= 'A' && szHex[i] <= 'F')
        {
            nRet *= 16;
            nRet += szHex[i] - 'A' + 10;
        }
        else
        {
            break;
        }
    }

    return nRet;
}

std::string   shstd::fun::MakeUpper(const std::string & strSrc)
{
    string strDest = strSrc;
    int nSize = strDest.size();
    const char *szDest = strDest.c_str();
    while (nSize-- > 0)
    {
        if (szDest[nSize] >= 'a' && szDest[nSize] <= 'z')
        {
            strDest[nSize] = 'A' + (szDest[nSize] - 'a');
        }
    }
    return strDest;
}


std::string  shstd::fun::MakeLower(const std::string & strSrc)
{
    string strDest = strSrc;
    int nSize = strDest.size();
    const char *szDest = strDest.c_str();
    while (nSize-- > 0)
    {
        if (szDest[nSize] >= 'A' && szDest[nSize] <= 'Z')
        {
            strDest[nSize] = 'a' + (szDest[nSize] - 'A');
        }
    }

    return strDest;
}

void shstd::fun::MakeLower(char *szStr, unsigned int nLen)
{

    if (nLen <= 0)
    {
        while (szStr != NULL && *szStr != '\0')
        {
            if (*szStr >= 'A' && *szStr <= 'Z') //是大写字�?
            {
                *szStr += 'a' - 'A';
            }
            szStr++;
        }
    }
    else
    {
        while (szStr != NULL && *szStr != '\0' && nLen-- > 0)
        {
            if (*szStr >= 'A' && *szStr <= 'Z') //是大写字�?
            {
                *szStr += 'a' - 'A';
            }
            szStr++;
        }
    }
}
void shstd::fun::MakeUpper(char *szStr, unsigned int nLen)
{
    if (nLen <= 0)
    {
        while (szStr != NULL && *szStr != '\0')
        {
            if (*szStr >= 'a' && *szStr <= 'z') //是大写字�?
            {
                *szStr += 'A' - 'a';
            }

            szStr++;
        }
    }
    else
    {
        while (szStr != NULL && *szStr != '\0' && nLen-- > 0)
        {
            if (*szStr >= 'a' && *szStr <= 'z') //是大写字�?
            {
                *szStr += 'A' - 'a';
            }

            szStr++;
        }
    }

}

bool shstd::fun::IsDirExist(const char *szDir) //判断问目录是否存�?
{
    bool bRet = false;

    struct stat st;
    if (stat(szDir, &st) == 0)
    {
        if (S_ISDIR(st.st_mode))
        {
            bRet = true;
        }
    }

    return bRet;
}

bool shstd::fun::IsFileExist(const char *szFile) //判断文件是否存在
{
    bool bRet = false;

    if (szFile != NULL && strlen(szFile) > 0)
    {
        struct stat st;
        if (stat(szFile, &st) == 0)
        {
            if (S_ISREG(st.st_mode))
            {
                bRet = true;
            }
        }
    }

    return bRet;
}


bool shstd::fun::DelFile(const char *szFile) //删除文件
{
    bool bRet = false;

    if (szFile != NULL && strlen(szFile) > 0)
    {
        bRet = (unlink(szFile) == 0);
    }

    return bRet;
}

int  shstd::fun::GetFileSize(const char * file)
{
    struct stat fs;
    if (0 == stat(file, &fs))
    {
        return fs.st_size;
    }
    return -1;
}

bool shstd::fun::DelDir(const char *szDir) //递归删除目录
{
    if (!IsDirExist(szDir)) return false;
    bool bRet = false;
    char szCommand[1024];
    sprintf(szCommand, "rm -fr %s", szDir);
    system(szCommand);
    bRet = !IsDirExist(szDir);
    return bRet;
}

bool shstd::fun::MakeDir(const char *szDstDir) //递归创建目录
{
    bool bRet = false;
    char bufDir[1024] = {0};

    char *pFind = (char *)szDstDir;
    while ( (pFind = strchr(pFind + 1, '/')) != NULL )
    {
        memset(bufDir, 0, 1024);
        strncpy(bufDir, szDstDir, pFind - szDstDir);
        if (IsDirExist(bufDir))
        {
            continue;
        }
        mkdir(bufDir,  S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IXOTH);
        if (shstd::fun::GetRunUser() != NULL)
        {
            SetPathOwner(bufDir, shstd::fun::GetRunUser());
        }
    }

    if (!IsDirExist(szDstDir))
    {
        mkdir(szDstDir, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IXOTH);
        if (shstd::fun::GetRunUser() != NULL)
            SetPathOwner(szDstDir, shstd::fun::GetRunUser());
    }


    ////判断文件夹是否存�?
    if (IsDirExist(szDstDir))
    {
        bRet = (access(szDstDir, W_OK) == 0);
    }

    return bRet;
}



std::string shstd::fun::URLdecode(const std::string & strSrc)
{

    string strDst;
    unsigned int uiSize = 0;
    const unsigned int VAL_TWO = 2;
    unsigned char  ucChar = 0;

    uiSize = strSrc.size();
    strDst.clear();

    for ( unsigned int i=0; i<uiSize; )
    {
        ucChar = strSrc[i];

        if ( '%' == ucChar )
        {
            if ( ( i + VAL_TWO ) >= uiSize )
            {
                break;
            }

            char szBuf[VAL_TWO+1] = {0};
            unsigned int iVal = 0;

            szBuf[0] = strSrc[++i];
            szBuf[1] = strSrc[++i];
            szBuf[2] = '\0';

            iVal = strtol(szBuf, NULL, 16);
            strDst.append(1,static_cast<char>(iVal));
        }
        else if ( '+' == ucChar )
        {
            strDst.append( 1, ' ' );
        }
        else
        {
            strDst.append(1,static_cast<char>(ucChar));
        }

        i++;
    }

    return strDst;
}

std::string shstd::fun::URLencode(const std::string& strSrc)
{
    const unsigned int MAX_BUF_SIZE = 1024*1024;
    std::string strDst;
    unsigned int iSize = 0;
    char  szBuf[4]= {0};
    unsigned char ucTmp = '\0';

    iSize = strSrc.length();
    iSize = iSize >= MAX_BUF_SIZE ? (MAX_BUF_SIZE-1) : iSize;

    strDst.clear();

    for ( unsigned int i=0; i<iSize; i++ )
    {
        ucTmp = strSrc[i];

        /*�ַ����ֲ�ת��*/
        if ( ( 'a' <= ucTmp && 'z' >= ucTmp )
                ||( 'A' <= ucTmp && 'Z' >= ucTmp )
                ||( '0' <= ucTmp && '9' >= ucTmp )
                || '-' == ucTmp
                || '_' == ucTmp )
        {
            strDst.append(1,static_cast<char>(ucTmp));
        }
        /*�ո�ת����'+'*/
        else if ( ' ' == ucTmp )
        {
            strDst.append("+");
        }
        /*����ascii��ת����%xx��ʽ*/
        else
        {

            snprintf(szBuf,4,"%%%02X",ucTmp);
            strDst.append(szBuf);
        }
    }

    return strDst;
}



/*
 * encode base64 string
 */
int shstd::fun::Base64Enc(unsigned char *buf,const unsigned char*text,int size)
{
    static const char *base64_encoding =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int buflen = 0;

    while(size>0)
    {
        *buf++ = base64_encoding[ (text[0] >> 2 ) & 0x3f];
        if(size>2)
        {
            *buf++ = base64_encoding[((text[0] & 3) << 4) | (text[1] >> 4)];
            *buf++ = base64_encoding[((text[1] & 0xF) << 2) | (text[2] >> 6)];
            *buf++ = base64_encoding[text[2] & 0x3F];
        }
        else
        {
            switch(size)
            {
            case 1:
                *buf++ = base64_encoding[(text[0] & 3) << 4 ];
                *buf++ = '=';
                *buf++ = '=';
                break;
            case 2:
                *buf++ = base64_encoding[((text[0] & 3) << 4) | (text[1] >> 4)];
                *buf++ = base64_encoding[((text[1] & 0x0F) << 2) | (text[2] >> 6)];
                *buf++ = '=';
                break;
            }
        }
        text +=3;
        size -=3;
        buflen +=4;
    }

    *buf = 0;
    return buflen;
}

static char GetBase64Value(char ch)
{
    if ((ch >= 'A') && (ch <= 'Z'))
        return ch - 'A';
    if ((ch >= 'a') && (ch <= 'z'))
        return ch - 'a' + 26;
    if ((ch >= '0') && (ch <= '9'))
        return ch - '0' + 52;
    switch (ch)
    {
    case '+':
        return 62;
    case '/':
        return 63;
    case '=': /* base64 padding */
        return 0;
    default:
        return 0;
    }
}

/*
����base64��������Ӧ����4�ı���(����mime��׼)
�������4�������ش���
ע�� ��������һ���ַ� ��ô���Ȳ�׼�� ���ܻ��1
����buf����
*/
int shstd::fun::Base64Dec(unsigned char *buf,const unsigned char*text,int size)
{
    unsigned char chunk[4];
    int parsenum=0;
    if(size%4)
        return -1;

    while(size>0)
    {
        chunk[0] = GetBase64Value(text[0]);
        chunk[1] = GetBase64Value(text[1]);
        chunk[2] = GetBase64Value(text[2]);
        chunk[3] = GetBase64Value(text[3]);

        *buf++ = (chunk[0] << 2) | (chunk[1] >> 4);
        *buf++ = (chunk[1] << 4) | (chunk[2] >> 2);
        *buf++ = (chunk[2] << 6) | (chunk[3]);

        text+=4;
        size-=4;
        parsenum+=3;
    }

    return parsenum;
}


void shstd::fun::replace_all(std::string &str, const std::string &old_value, const std::string &new_value)
{
    while(true)   
    {                         
        std::string::size_type pos(0);     
        if((pos = str.find(old_value)) != std::string::npos)     {
            str.replace(pos, old_value.length(), new_value);
        }
        else {   
            break;
        }
    }
}

std::string shstd::fun::CurrentDate()
{
    time_t rt       = time(NULL);
    struct tm *ptm  = localtime( &rt);

    char szDate[22] = {0};
    snprintf(szDate, sizeof(szDate) - 1, "%d-%d-%d", ptm->tm_year+1900, ptm->tm_mon + 1, ptm->tm_mday);
    return std::string(szDate);
}

/*���ַ�����ȡָ��keyֵ������
*    ������delimiter�ָ�
*    ��ʽ��key1=value1 key2=value2 ...
*/
std::string shstd::fun::GetValueBykey(const char *szBuf, const char *key, const char* delimiter)
{
    if(NULL == szBuf || NULL == key || NULL == delimiter)
    {
        return std::string("");
    }

    const char *start = NULL, *end = NULL;
    if(NULL == (start = strstr(szBuf, key)))
    {
        return std::string("");
    }

    start += (strlen(key) + 1);
    if(NULL == (end = strstr(start, delimiter)))
    {
        if(NULL == (end = strstr(start, "\r\n")))
        {
            return std::string(start);
        }
    }
    
    std::string s(start, end -start);
    
    return s;  
}











