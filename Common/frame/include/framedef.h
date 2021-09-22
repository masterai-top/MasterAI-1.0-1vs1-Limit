/**
* \file frame.h
* \brief 框架库的导出定义
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __FRAME_DEF_H__
#define __FRAME_DEF_H__

/// win32
#ifdef WIN32

/// 编译动态库/静态库
#   ifdef BUILD_LIBFRAME
#       define FRAME_EXPORT __declspec(dllexport)
#       define FRAME_TEMPLATE_EXPORT __declspec(dllexport)
/// 使用静态库
#   elif USE_LIBFRAME
#       define FRAME_EXPORT
#       define FRAME_TEMPLATE_EXPORT
#       pragma comment(lib, "libframe.lib")
/// 使用动态库
#   else
#       define FRAME_EXPORT __declspec(dllimport)
#       define FRAME_TEMPLATE_EXPORT
#       pragma comment(lib, "libframe.lib")
#   endif

/// gcc
#else

/// 编译动态库
#   ifdef BUILD_LIBFRAME
#       define FRAME_EXPORT __attribute__((__visibility__("default")))
#       define FRAME_TEMPLATE_EXPORT __attribute__((__visibility__("default")))
/// 编译静态库/使用静态库/使用动态库
#   else
#       define FRAME_EXPORT
#       define FRAME_TEMPLATE_EXPORT
#   endif

#endif

#endif // __FRAME_DEF_H__