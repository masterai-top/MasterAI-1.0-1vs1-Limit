/**
* \file scriptdef.h
* \brief 框架库的导出定义
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __SCRIPT_DEF_H__
#define __SCRIPT_DEF_H__

/// win32
#ifdef WIN32

/// 编译动态库/静态库
#   ifdef BUILD_LIBSCRIPT
#       define SCRIPT_EXPORT __declspec(dllexport)
#       define SCRIPT_TEMPLATE_EXPORT __declspec(dllexport)
/// 使用静态库
#   elif USE_LIBENCODE
#       define SCRIPT_EXPORT
#       define SCRIPT_TEMPLATE_EXPORT
#       pragma comment(lib, "libdtscript.lib")
/// 使用动态库
#   else
#       define SCRIPT_EXPORT __declspec(dllimport)
#       define SCRIPT_TEMPLATE_EXPORT
#       pragma comment(lib, "libdtscript.lib")
#   endif

/// gcc
#else

/// 编译动态库
#   ifdef BUILD_LIBSCRIPT
#       define SCRIPT_EXPORT __attribute__((__visibility__("default")))
#       define SCRIPT_TEMPLATE_EXPORT __attribute__((__visibility__("default")))
/// 编译静态库/使用静态库/使用动态库
#   else
#       define SCRIPT_EXPORT
#       define SCRIPT_TEMPLATE_EXPORT
#   endif

#endif

#endif // __SCRIPT_DEF_H__
