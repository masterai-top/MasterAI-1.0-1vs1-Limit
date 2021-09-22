/**
* \file pch.h
* \brief 预编译头文件
*
* pch.h : include file for standard system include files,
* or project specific include files that are used frequently, but
* are changed infrequently
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __PCH_H__
#define __PCH_H__

#ifdef WIN32

// 包括 SDKDDKVer.h 将定义可用的最高版本的 Windows 平台。
// 如果要为以前的 Windows 平台生成应用程序，请包括 WinSDKVer.h，并将
// WIN32_WINNT 宏设置为要支持的平台，然后再包括 SDKDDKVer.h。
#   include <SDKDDKVer.h>

// 从 Windows 头文件中排除极少使用的信息
#   define WIN32_LEAN_AND_MEAN

// Windows 头文件:
#   include <windows.h>
#   include <io.h>
#   include <winsock2.h>
#   include <time.h>
#   include <tlhelp32.h>
#   include <tchar.h>
#   include <psapi.h>

#   pragma comment(lib,"ws2_32.lib")
#   pragma comment(lib,"mswsock.lib")

#   pragma comment(lib,"libevent.lib")
#   pragma comment(lib,"libiconv.lib")
#   pragma comment(lib,"liblua.lib")
#   pragma comment(lib,"libtinyxml.lib")

#   ifdef _DEBUG
#       pragma comment(lib,"pthreadVC2d.lib")
#   else
#       pragma comment(lib,"pthreadVC2.lib")
#   endif

#else

#   include <unistd.h>
#   include <sys/time.h>
#   include <sys/stat.h>
#   include <sys/shm.h>
#   include <sys/types.h>
#   include <sys/sem.h>
#   include <sys/ipc.h>
#   include <cstring>
#   include <signal.h>
#   include <sys/user.h>
#   include <sys/socket.h>
#   include <sys/epoll.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <netinet/tcp.h>
#   include <sys/ioctl.h>
#   include <netdb.h>
#   include <dirent.h>

#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>
#include <wctype.h>
#include <assert.h>
#include <algorithm>
#include <fcntl.h>
#include <errno.h>

#endif // __PCH_H__