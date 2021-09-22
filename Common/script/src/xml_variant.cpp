/**
* \file xml_variant.cpp
* \brief 脚本文件变量类函数实现
*/

#include "pch.h"
#include "xml_variant.h"
#include "strutil.h"
using namespace dtutil;

namespace dtscript
{
    /// 空变量
    XmlVariant XmlVariant::s_EmptyVar;

    /**
    * \brief 构造函数
    */
    XmlVariant::XmlVariant(void)
    {
        m_Type = VT_EMPTY;
        m_Value.szValue = NULL;
    }

    /**
    * \brief 析构函数
    */
    XmlVariant::~XmlVariant(void)
    {
        m_Type = VT_EMPTY;
        m_Value.szValue = NULL;
    }

    /**
    * \brief 构造函数
    * \param itValue 多维脚本对象
    */
    XmlVariant::XmlVariant(IXmlIterator *itValue)
    {
        m_Type = VT_ELEMENT;
        m_Value.itValue = itValue;
    }

    /**
    * \brief 拷贝构造函数
    * \param other 拷贝对象
    */
    XmlVariant::XmlVariant(const XmlVariant &other)
    {
        m_Type = other.m_Type;
        m_Value.szValue = other.m_Value.szValue;
    }


    /**
    * \brief 强制类型转换
    * \return 字符串
    */
    XmlVariant::operator std::string ()
    {
        std::string str = m_Value.szValue;

        return str;
    }

    /**
    * \brief 强制类型转换
    * \return 字符串
    */
    XmlVariant::operator const char * ()
    {
        return m_Value.szValue;
    }

    /**
    * \brief 强制类型转换
    * \return 整型int8
    */
    XmlVariant::operator int8 ()
    {
        int32 nValue = StrUtil::strtoi32(m_Value.szValue);

        return (int8)(nValue);
    }

    /**
    * \brief 强制类型转换
    * \return 整型uint8
    */
    XmlVariant::operator uint8 ()
    {
        int32 nValue = StrUtil::strtoi32(m_Value.szValue);

        return (uint8)(nValue);
    }

    /**
    * \brief 强制类型转换
    * \return 整型int16
    */
    XmlVariant::operator int16 ()
    {
        int32 nValue = StrUtil::strtoi32(m_Value.szValue);

        return (int16)(nValue);
    }

    /**
    * \brief 强制类型转换
    * \return 整型uint16
    */
    XmlVariant::operator uint16 ()
    {
        int32 nValue = StrUtil::strtoi32(m_Value.szValue);

        return (uint16)(nValue);
    }

    /**
    * \brief 强制类型转换
    * \return 整型int32
    */
    XmlVariant::operator int32 ()
    {
        int32 nValue = StrUtil::strtoi32(m_Value.szValue);

        return nValue;
    }

    /**
    * \brief 强制类型转换
    * \return 整型uint32
    */
    XmlVariant::operator uint32 ()
    {
        int32 nValue = StrUtil::strtoi32(m_Value.szValue);

        return (uint32)(nValue);
    }

    /**
    * \brief 强制类型转换
    * \return 整型int64
    */
    XmlVariant::operator int64 ()
    {
        int64 nValue = StrUtil::strtoi64(m_Value.szValue);

        return nValue;
    }

    /**
    * \brief 强制类型转换
    * \return 整型uint64
    */
    XmlVariant::operator uint64 ()
    {
        int64 nValue = StrUtil::strtoi64(m_Value.szValue);

        return (uint64)(nValue);
    }

    /**
    * \brief 强制类型转换
    * \return 双精度double
    */
    XmlVariant::operator double ()
    {
        double dValue = StrUtil::strtodouble(m_Value.szValue);

        return (double)(dValue);
    }

    /**
    * \brief 强制类型转换
    * \return 单精度float
    */
    XmlVariant::operator float ()
    {
        double dValue = StrUtil::strtodouble(m_Value.szValue);

        return (float)(dValue);
    }

    /**
    * \brief 是否为空变量
    * \return 是空变量返回true，否则返回false
    */
    bool XmlVariant::Empty()
    {
        if (m_Type == VT_EMPTY)
        {
            return true;
        }

        return false;
    }

    /**
    * \brief 获得子节点数量
    * \param szName 子节点名称
    * \return 子节点数量
    */
    size_t XmlVariant::ChildElementCount(const char *szName)
    {
        if (m_Type == VT_ELEMENT)
        {
            return (*m_Value.itValue).ChildElementCount(szName);
        }

        return 0;
    }

    /**
    * \brief 获得指定名称指定索引的子节点
    * \param szName 子节点名称
    * \param nIndex 指定名称索引
    * \return 指定名称指定索引的子节点
    */
    XmlVariant & XmlVariant::ChildElement(const char *szName, uint32 nIndex)
    {
        if (m_Type == VT_ELEMENT)
        {
            return (*m_Value.itValue).ChildElement(szName, nIndex);
        }

        return s_EmptyVar;
    }

    /**
    * \brief 获得节点属性
    * \param szName 属性名称
    * \return 指定名称节点属性数据
    */
    XmlVariant & XmlVariant::operator [] (const char *szName)
    {
        if (m_Type == VT_ELEMENT)
        {
            return (*m_Value.itValue)[szName];
        }

        return s_EmptyVar;
    }
}