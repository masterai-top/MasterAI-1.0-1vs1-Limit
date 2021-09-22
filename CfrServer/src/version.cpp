#include "version.h"

const char *GetSvrVersionInfo()
{
    static char szVer[128];
    memset(szVer, 0, sizeof(szVer));
    snprintf(szVer, sizeof(szVer) - 1, "Server @Build %s %s", __DATE__, __TIME__);
    return szVer;
}

