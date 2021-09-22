/**
* \file typedef.h
* \brief 基础类型定义
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __TYPE_DEF_H__
#define __TYPE_DEF_H__

/// NULL定义
#ifndef NULL
#   ifdef __cplusplus
#       define NULL 0
#   else
#       define NULL ((void *)0)
#   endif
#endif

/// 整型定义
typedef char                int8;
typedef unsigned char       uint8;
typedef short               int16;
typedef unsigned short      uint16;
typedef int                 int32;
typedef unsigned int        uint32;

/// buffer定义
typedef unsigned char       byte;
typedef const char *        LPCSTR;

/// 浮点型定义
typedef float               float32;
typedef double              float64;

/// 64位定义
#ifdef WIN32
#   ifndef int64
        typedef __int64             int64;
#   endif
#   ifndef uint64
        typedef unsigned __int64    uint64;
#   endif
#else
#   ifndef int64
        typedef long long           int64;
#   endif
#   ifndef uint64
        typedef unsigned long long  uint64;
#   endif
#endif

#if defined(_MSC_VER)
#   define strtoll _strtoi64
#   define wcstoll _wcstoi64
#endif


/// 字节对齐，align必须是2的幂，一般4字节或8字节对齐即可
#define ALIGN_SIZE(size, align) (((size) + (align - 1)) & ~(align - 1))
#define ALIGN_SIZE_8(size) (ALIGN_SIZE(size, 8))
#define ALIGN_SIZE_16(size) (ALIGN_SIZE(size, 16))
#define ALIGN_SIZE_32(size) (ALIGN_SIZE(size, 32))
#define ALIGN_SIZE_64(size) (ALIGN_SIZE(size, 64))

/// 不定参数格式化拷贝
#ifndef va_copy
#   ifdef __va_copy
#       define va_copy __va_copy
#   else
#       define va_copy(a, b)  memcpy(&(a), &(b), sizeof(va_list))
#   endif
#endif

#endif // __TYPE_DEF_H__