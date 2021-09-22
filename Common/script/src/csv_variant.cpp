/**
* \file csv_variant.cpp
* \brief csv脚本文件变量类函数实现
*/

#include "pch.h"
#include "csv_variant.h"
#include "strutil.h"
using namespace dtutil;

namespace dtscript
{
    /// 空变量
    CsvVariant CsvVariant::s_EmptyVar;

    /**
    * \brief 构造函数
    */
    CsvVariant::CsvVariant(void)
    {
        m_Type = VT_EMPTY;
        m_Value.szValue = NULL;
    }

    /**
    * \brief 析构函数
    */
    CsvVariant::~CsvVariant(void)
    {
        m_Type = VT_EMPTY;
        m_Value.szValue = NULL;
    }

    /**
    * \brief 构造函数
    * \param itValue 多维脚本对象
    */
    CsvVariant::CsvVariant(ICsvIterator *itValue)
    {
        m_Type = VT_ITERATOR;
        m_Value.itValue = itValue;
    }

    /**
    * \brief 拷贝构造函数
    * \param other 拷贝对象
    */
    CsvVariant::CsvVariant(const CsvVariant &other)
    {
        m_Type = other.m_Type;
        m_Value.szValue = other.m_Value.szValue;
    }

    /**
    * \brief 强制类型转换
    * \return 字符串
    */
    CsvVariant::operator std::string ()
    {
        std::string str = m_Value.szValue;

        return str;
    }

    /**
    * \brief 强制类型转换
    * \return 字符串
    */
    CsvVariant::operator const char * ()
    {
        return m_Value.szValue;
    }

    /**
    * \brief 强制类型转换
    * \return 整型int8
    */
    CsvVariant::operator int8 ()
    {
        int32 nValue = StrUtil::strtoi32(m_Value.szValue);

        return (int8)(nValue);
    }

    /**
    * \brief 强制类型转换
    * \return 整型uint8
    */
    CsvVariant::operator uint8 ()
    {
        int32 nValue = StrUtil::strtoi32(m_Value.szValue);

        return (uint8)(nValue);
    }

    /**
    * \brief 强制类型转换
    * \return 整型int16
    */
    CsvVariant::operator int16 ()
    {
        int32 nValue = StrUtil::strtoi32(m_Value.szValue);

        return (int16)(nValue);
    }

    /**
    * \brief 强制类型转换
    * \return 整型uint16
    */
    CsvVariant::operator uint16 ()
    {
        int32 nValue = StrUtil::strtoi32(m_Value.szValue);

        return (uint16)(nValue);
    }

    /**
    * \brief 强制类型转换
    * \return 整型int32
    */
    CsvVariant::operator int32 ()
    {
        int32 nValue = StrUtil::strtoi32(m_Value.szValue);

        return nValue;
    }

    /**
    * \brief 强制类型转换
    * \return 整型uint32
    */
    CsvVariant::operator uint32 ()
    {
        int32 nValue = StrUtil::strtoi32(m_Value.szValue);

        return (uint32)(nValue);
    }

    /**
    * \brief 强制类型转换
    * \return 整型int64
    */
    CsvVariant::operator int64 ()
    {
        int64 nValue = StrUtil::strtoi64(m_Value.szValue);

        return nValue;
    }

    /**
    * \brief 强制类型转换
    * \return 整型uint64
    */
    CsvVariant::operator uint64 ()
    {
        int64 nValue = StrUtil::strtoi64(m_Value.szValue);

        return (uint64)(nValue);
    }

    /**
    * \brief 强制类型转换
    * \return 双精度double
    */
    CsvVariant::operator double ()
    {
        double dValue = StrUtil::strtodouble(m_Value.szValue);

        return (double)(dValue);
    }

    /**
    * \brief 强制类型转换
    * \return 单精度float
    */
    CsvVariant::operator float ()
    {
        double dValue = StrUtil::strtodouble(m_Value.szValue);

        return (float)(dValue);
    }

    /**
    * \brief 是否为空变量
    * \return 是空变量返回true，否则返回false
    */
    bool CsvVariant::Empty()
    {
        if (m_Type == VT_EMPTY)
        {
            return true;
        }

        return false;
    }

    /**
    * \brief 获得多维脚本数据的个数
    * \return 多维脚本数据个数
    */
    size_t CsvVariant::Size()
    {
        if (m_Type == VT_ITERATOR)
        {
            return m_Value.itValue->Size();
        }

        return 0;
    }

    /**
    * \brief 根据指定索引获得多维脚本数据
    * \param nIndex 指定索引
    * \return 指定索引多维脚本数据
    */
    CsvVariant & CsvVariant::operator [] (size_t nIndex)
    {
        if (m_Type == VT_ITERATOR)
        {
            return (*m_Value.itValue)[nIndex];
        }

        return s_EmptyVar;
    }

    /**
    * \brief 根据指定名称获得多维脚本数据
    * \param szName 指定名称
    * \return 指定名称多维脚本数据
    */
    CsvVariant & CsvVariant::operator [] (const char *szName)
    {
        if (m_Type == VT_ITERATOR)
        {
            return (*m_Value.itValue)[szName];
        }

        return s_EmptyVar;
    }
}