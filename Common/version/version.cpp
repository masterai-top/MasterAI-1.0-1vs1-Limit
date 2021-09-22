#include "version.h"

const char *GetComVersionInfo()
{
    static char szVer[128];
    memset(szVer, 0, sizeof(szVer));
    snprintf(szVer, sizeof(szVer) - 1, "libcomm @Build %s %s", __DATE__, __TIME__);
    return szVer;
}

