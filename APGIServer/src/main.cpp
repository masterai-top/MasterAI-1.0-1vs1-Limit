#include "pch.h"
#include "apgi_svr.h"
#include "version.h"
#include "../../Common/version/version.h"

int main(int argc, char *argv[])
{
    if ( argc < 2 )
    {
        fprintf(stderr, "\n Usage: %s  start/stop/restart/-v  [path] [User] '.....\n\n", *argv);
        exit(0);
    }

    if(argc > 1 && 0 == StrUtil::strnicmp(argv[1], "-v", strlen("-v")))
    {
        printf("ver=%s, com=%s\r\n\r\n", GetSvrVersionInfo(), GetComVersionInfo());
        exit(0);
    }

    const char *szRunUser = shstd::fun::GetRunUser();
    if(NULL == szRunUser)
    {
        fprintf(stderr, "Get run user failed...\r\n\r\n");
        exit(0);
    }

    if (!shstd::fun::ChangeRunUser(szRunUser))
    {
        fprintf(stderr, "Change run user failed| user=%s\r\n\r\n", szRunUser);
        exit(0);
    }

    if(!CRunConfig::Instance()->Init(argv[0]))
    {
        printf( "Run config init failed\r\n");
        exit(0);
    }
    if (!shstd::fun::SvrStdInit(argc, (const char **)argv, CRunConfig::Instance()->GetRunName().c_str(), szRunUser, 1, 1))
    {
        fprintf(stderr, "SvrStdInit failed\n");
        exit(0);
    }

    bool bRet = false;
    try
    {
        bRet = CAPGISvr::Instance().Init(argc, argv);
    }
    catch(Exception &e)
    {
        printf("server init exception| msg=%s\r\n", e.what());
        _exit(0);
        return 0;
    }
    catch(...)
    {
        printf("server init unknow exception\r\n");
        _exit(0);
        return 0;
    }

    if (bRet)
    {
        CAPGISvr::Instance().Loop();
    }

    sleep(1);
    CAPGISvr::Instance().Restore();

    return 0;
}