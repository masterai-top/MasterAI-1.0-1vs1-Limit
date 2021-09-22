/**
* \file utildef.h
* \brief 框架库的导出定义
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __UTIL_DEF_H__
#define __UTIL_DEF_H__

/// win32
#ifdef WIN32

/// 编译动态库/静态库
#   ifdef BUILD_LIBUTIL
#       define UTIL_EXPORT __declspec(dllexport)
#       define UTIL_TEMPLATE_EXPORT __declspec(dllexport)
/// 使用静态库
#   elif USE_LIBENCODE
#       define UTIL_EXPORT
#       define UTIL_TEMPLATE_EXPORT
#       pragma comment(lib, "libdtutil.lib")
/// 使用动态库
#   else
#       define UTIL_EXPORT __declspec(dllimport)
#       define UTIL_TEMPLATE_EXPORT
#       pragma comment(lib, "libdtutil.lib")
#   endif

/// gcc
#else

/// 编译动态库
#   ifdef BUILD_LIBUTIL
#       define UTIL_EXPORT __attribute__((__visibility__("default")))
#       define UTIL_TEMPLATE_EXPORT __attribute__((__visibility__("default")))
/// 编译静态库/使用静态库/使用动态库
#   else
#       define UTIL_EXPORT
#       define UTIL_TEMPLATE_EXPORT
#   endif

#endif

#endif // __FRAME_DEF_H__