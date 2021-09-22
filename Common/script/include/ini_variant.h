/**
* \file ini_variant.h
* \brief ini脚本变量类
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __INI_VARIANT_H__
#define __INI_VARIANT_H__

#include "typedef.h"
#include <string>
#include "scriptdef.h"

namespace dtscript
{
    class IIniIterator;

    /**
    * \brief ini脚本文件变量类
    * \ingroup script_group
    */
    class SCRIPT_EXPORT IniVariant
    {
    public:
        /**
        * \brief 变量类型
        */
        enum _TYPE
        {
            VT_EMPTY = 0,               ///< 空类型
            VT_VALUE,                   ///< 值类型
            VT_ITERATOR,                ///< 迭代类型
        };
        /**
        * \brief 变量值
        */
        union _VALUE
        {
            const char      *szValue;   ///< 字符数据，指向IniSection中存储的数据
            IIniIterator    *itValue;   ///< 子脚本, 用来实现任意多维的脚本
        };

    public:
        /**
        * \brief 构造函数
        */
        IniVariant(void);

        /**
        * \brief 析构函数
        */
        ~IniVariant(void);

        /**
        * \brief 构造函数
        * \param itValue 多维脚本对象
        */
        IniVariant(IIniIterator *itValue);

        /**
        * \brief 拷贝构造函数
        * \param other 拷贝对象
        */
        IniVariant(const IniVariant &other);

    public:
        /**
        * \brief 强制类型转换
        * \return 字符串
        */
        operator std::string ();

        /**
        * \brief 强制类型转换
        * \return 字符串
        */
        operator const char * ();

        /**
        * \brief 强制类型转换
        * \return 整型int8
        */
        operator int8 ();

        /**
        * \brief 强制类型转换
        * \return 整型uint8
        */
        operator uint8 ();

        /**
        * \brief 强制类型转换
        * \return 整型int16
        */
        operator int16 ();

        /**
        * \brief 强制类型转换
        * \return 整型uint16
        */
        operator uint16 ();

        /**
        * \brief 强制类型转换
        * \return 整型int32
        */
        operator int32 ();

        /**
        * \brief 强制类型转换
        * \return 整型uint32
        */
        operator uint32 ();

        /**
        * \brief 强制类型转换
        * \return 整型int64
        */
        operator int64 ();

        /**
        * \brief 强制类型转换
        * \return 整型uint64
        */
        operator uint64 ();

        /**
        * \brief 强制类型转换
        * \return 双精度double
        */
        operator double ();

        /**
        * \brief 强制类型转换
        * \return 单精度float
        */
        operator float ();

    public:
        /**
        * \brief 是否为空变量
        * \return 是空变量返回true，否则返回false
        */
        bool Empty();

        /**
        * \brief 根据指定名称获得多维脚本数据
        * \param szName 指定名称
        * \return 指定名称多维脚本数据
        */
        IniVariant & operator [] (const char *szName);

    public:
        _TYPE               m_Type;         ///< 数据类型
        _VALUE              m_Value;        ///< 数值
        static IniVariant   s_EmptyVar;     ///< 空变量
    };


    /**
    * \brief ini脚本迭代器接口
    * \ingroup script_group
    * 用来实现脚本类型无关性
    */  
    class IIniIterator
    {
    public:
        /**
        * \brief 根据指定名称获得数据
        * \param szName 指定名称
        * \return 指定名称的数据
        */
        virtual IniVariant & operator [] (const char *szName) = 0;

        virtual ~IIniIterator() {}
    };
}

#endif // __INI_VARIANT_H__