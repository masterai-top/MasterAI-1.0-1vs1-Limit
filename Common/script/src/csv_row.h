/**
* \file csv_row.h
* \brief csv脚本文件行数据类头文件
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __CSV_ROW_H__
#define __CSV_ROW_H__

#include "csv_variant.h"

namespace dtscript
{
    class CsvAdapter;

    /**
    * \brief csv行数据类
    * \ingroup script_group
    */
    class CsvRow : public ICsvIterator
    {
    public:
        /**
        * \brief 构造函数
        */
        CsvRow(void);

        /**
        * \brief 析构函数
        */
        virtual ~CsvRow(void);

        /**
        * \brief 装载数据
        * \param pAdapter csv文件适配器
        * \param szData 行数据内容
        * \param cSeparator 数据分割符
        * \param bAllowSeries 是否允许连续出现，例：",,,"，允许的情况作为3个数据，不允许作为1个数据
        * \return 装载成功返回true，否则返回false
        */
        bool LoadData(CsvAdapter *pAdapter, const char *szData, char cSeparator = ',', bool bAllowSeries = true);

    public:
        /**
        * \brief 获得数据列数
        * \return 数据列数
        */
        virtual size_t Size();

        /**
        * \brief 获得指定列数据
        * \param nIndex 列数索引
        * \return 指定列数据
        */
        virtual CsvVariant & operator [] (size_t nIndex);

        /**
        * \brief 根据字段名称获得数据
        * \param szName 字段名称
        * \return 指定字段名称数据
        */
        virtual CsvVariant & operator [] (const char *szName);

    private:
        /**
        * \brief 获得指定列数据
        * \param nIndex 列数索引
        * \return 指定列数据
        */
        CsvVariant & GetData(size_t nIndex);

        /**
        * \brief 根据字段名称获得数据
        * \param szName 字段名称
        * \return 指定字段名称数据
        */
        CsvVariant & GetData(const char *szName);

    private:
        size_t          m_nSize;        ///< 数据个数
        CsvVariant      *m_pData;       ///< 数据列表，数据内容指向m_szData;
        char            *m_szData;      ///< 行数据内容
        CsvAdapter      *m_pAdapter;    ///< csv适配器
    };
}

#endif // __CSV_ROW_H__