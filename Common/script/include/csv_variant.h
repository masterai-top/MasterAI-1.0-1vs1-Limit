/**
* \file csv_variant.h
* \brief csv脚本变量类
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __CSV_VARIANT_H__
#define __CSV_VARIANT_H__

#include "typedef.h"
#include <string>
#include "scriptdef.h"

namespace dtscript
{
    class ICsvIterator;

    /**
    * \brief csv脚本文件变量类
    * \ingroup script_group
    */
    class SCRIPT_EXPORT CsvVariant
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
            const char      *szValue;   ///< 字符数据，指向CsvRow中存储的数据
            ICsvIterator    *itValue;   ///< 子脚本, 用来实现任意多维的脚本
        };

    public:
        /**
        * \brief 构造函数
        */
        CsvVariant(void);

        /**
        * \brief 析构函数
        */
        ~CsvVariant(void);

        /**
        * \brief 构造函数
        * \param itValue 多维脚本对象
        */
        CsvVariant(ICsvIterator *itValue);

        /**
        * \brief 拷贝构造函数
        * \param other 拷贝对象
        */
        CsvVariant(const CsvVariant &other);

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
        * \brief 获得多维脚本数据的个数
        * \return 多维脚本数据个数
        */
        size_t Size();

        /**
        * \brief 根据指定索引获得多维脚本数据
        * \param nIndex 指定索引
        * \return 指定索引多维脚本数据
        */
        CsvVariant & operator [] (size_t nIndex);

        /**
        * \brief 根据指定名称获得多维脚本数据
        * \param szName 指定名称
        * \return 指定名称多维脚本数据
        */
        CsvVariant & operator [] (const char *szName);

    public:
        _TYPE               m_Type;         ///< 数据类型
        _VALUE              m_Value;        ///< 数值
        static CsvVariant   s_EmptyVar;     ///< 空变量
    };


    /**
    * \brief csv脚本迭代器接口
    * \ingroup script_group
    * 用来实现脚本类型无关性
    */  
    class ICsvIterator
    {
    public:
        /**
        * \brief 获得文档内容的行数
        * \return 文档内容的行数
        */
        virtual size_t Size() = 0;

        /**
        * \brief 根据指定行数获得数据
        * \param nIndex 行数索引
        * \return 指定行数据
        */
        virtual CsvVariant & operator [] (size_t nIndex) = 0;

        /**
        * \brief 根据指定名称获得数据
        * \param szName 指定名称
        * \return 指定名称的数据
        */
        virtual CsvVariant & operator [] (const char *szName) = 0;

        virtual ~ICsvIterator() {}
    };
}

#endif // __CSV_VARIANT_H__