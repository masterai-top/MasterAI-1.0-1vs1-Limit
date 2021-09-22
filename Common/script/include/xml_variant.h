/**
* \file xml_variant.h
* \brief xml脚本变量类
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __XML_VARIANT_H__
#define __XML_VARIANT_H__


#include "typedef.h"
#include <string>
#include "scriptdef.h"

namespace dtscript
{
    class IXmlIterator;

    /**
    * \brief xml脚本变量类
    * \ingroup script_group
    */
    class SCRIPT_EXPORT XmlVariant
    {
    public:
        /**
        * \brief 变量类型
        */
        enum _TYPE
        {
            VT_EMPTY = 0,               ///< 空类型
            VT_ATTRIBUTE,               ///< 属性类型
            VT_ELEMENT,                 ///< 节点类型
        };
        /**
        * \brief 变量值
        */
        union _VALUE
        {
            const char      *szValue;   ///< 字符数据，指向XmlElement中存储的数据
            IXmlIterator    *itValue;   ///< 节点数据, 用来实现任意多维的脚本
        };

    public:
        /**
        * \brief 构造函数
        */
        XmlVariant(void);

        /**
        * \brief 析构函数
        */
        ~XmlVariant(void);

        /**
        * \brief 构造函数
        * \param itValue 多维脚本对象
        */
        XmlVariant(IXmlIterator *itValue);

        /**
        * \brief 拷贝构造函数
        * \param other 拷贝对象
        */
        XmlVariant(const XmlVariant &other);

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
        * \brief 获得子节点数量
        * \param szName 子节点名称
        * \return 子节点数量
        */
        size_t ChildElementCount(const char *szName);

        /**
        * \brief 获得指定名称指定索引的子节点
        * \param szName 子节点名称
        * \param nIndex 指定名称索引
        * \return 指定名称指定索引的子节点
        */
        XmlVariant & ChildElement(const char *szName, uint32 nIndex);

        /**
        * \brief 获得节点属性
        * \param szName 属性名称
        * \return 指定名称节点属性数据
        */
        XmlVariant & operator [] (const char *szName);

    public:
        _TYPE               m_Type;         ///< 数据类型
        _VALUE              m_Value;        ///< 数值
        static XmlVariant   s_EmptyVar;     ///< 空变量
    };


    /**
    * \brief 脚本迭代器接口
    * \ingroup script_group
    * 用来实现脚本类型无关性
    */  
    class IXmlIterator
    {
    public:
        /**
        * \brief 获得子节点数量
        * \param szName 子节点名称
        * \return 子节点数量
        */
        virtual size_t ChildElementCount(const char *szName) = 0;

        /**
        * \brief 获得指定名称指定索引的子节点
        * \param szName 子节点名称
        * \param nIndex 指定名称索引
        * \return 指定名称指定索引的子节点
        */
        virtual XmlVariant & ChildElement(const char *szName, uint32 nIndex) = 0;

        /**
        * \brief 获得节点属性
        * \param szName 属性名称
        * \return 指定名称节点属性数据
        */
        virtual XmlVariant & GetData(const char *szName) = 0;

        /**
        * \brief 获得节点属性
        * \param szName 属性名称
        * \return 指定名称节点属性数据
        */
        virtual XmlVariant & operator [] (const char *szName) = 0;
    };
}

#endif // __XML_VARIANT_H__
