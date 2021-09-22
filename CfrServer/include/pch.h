/**
* \file pch.h
* \brief 预编译头文件
*
* stdafx.h : include file for standard system include files,
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


#ifdef _WIN32

	// 包括 SDKDDKVer.h 将定义可用的最高版本的 Windows 平台。
	// 如果要为以前的 Windows 平台生成应用程序，请包括 WinSDKVer.h，并将
	// WIN32_WINNT 宏设置为要支持的平台，然后再包括 SDKDDKVer.h。
#   include <SDKDDKVer.h>

	//  从 Windows 头文件中排除极少使用的信息
#   define WIN32_LEAN_AND_MEAN

	// Windows 头文件:
#   include <windows.h>
#   include <winsock2.h>

#   pragma comment(lib,"ws2_32.lib")
#   pragma comment(lib,"mswsock.lib")

#   pragma comment(lib,"libacpc.lib")
#   pragma comment(lib,"libframe.lib")

#else

#   include <unistd.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <fcntl.h>
#   include <errno.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>

#endif // _WIN32


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "frame.h"
using namespace frame;

#endif //__PCH_H__