/**
* \file csv_row.cpp
* \brief csv脚本文件行数据类函数实现
*/

#include "pch.h"
#include "csv_row.h"
#include "csv_adapter.h"


namespace dtscript
{
    /**
    * \brief 构造函数
    */
    CsvRow::CsvRow(void)
    {
        m_pAdapter = NULL;
        m_pData = NULL;
        m_nSize = 0;
        m_szData = NULL;
    }

    /**
    * \brief 析构函数
    */
    CsvRow::~CsvRow(void)
    {
        if (NULL != m_pData)
        {
            delete [] m_pData;
            m_pData = NULL;
        }

        if (NULL != m_szData)
        {
            delete [] m_szData;
            m_szData = NULL;
        }

        m_nSize = 0;
        m_pAdapter = NULL;
    }

    /**
    * \brief 装载数据
    * \param pAdapter csv文件适配器
    * \param szData 行数据内容
    * \param cSeparator 数据分隔符
    * \param bAllowSeries 是否允许连续出现，例：",,,"，允许的情况作为3个数据，不允许作为1个数据
    * \return 装载成功返回true，否则返回false
    */
    bool CsvRow::LoadData(CsvAdapter *pAdapter, const char *szData, char cSeparator /*= ','*/, bool bAllowSeries /*= true*/)
    {
        const size_t VAR_MAX_PER_ROW = 1024;        // 一行变量数量最大数

        if (NULL == szData)
        {
            return false;
        }

        size_t nDataLen = strlen(szData);
        m_szData = new char[nDataLen + 1];  // '\0'
        memcpy(m_szData, szData, nDataLen);
        m_szData[nDataLen] = 0;

        bool bFlag = false;         // 引号开始标志
        bool bNewVar = true;        // 新变量开始标志

        size_t nVarOffset[VAR_MAX_PER_ROW] = { 0 };     // 保存每个变量的开始位置

        // 解析数据，将分隔符改为字符串结束标志'\0'，可以将一行数据分割成多个变量。
        // 引号内可能包含'\r'、'\n'这样的字符，所以引号内的换行并不是新行数据。

        size_t nRowLen = 0;             // 解析的字符长度
        char c = m_szData[nRowLen++];   // 当前解析的字符

        while ( c != 0 )
        {
            // 引号开始
            if (!bFlag && c == '\"')
            {
                bFlag = true;
            }
            // 引号结束
            else if (bFlag && c == '\"')
            {
                bFlag = false;
            }

            // 非引号内换行，已经读取完一行数据了，退出循环
            if ( (c == '\r' || c == '\n') && !bFlag )
            {
                break;
            }

            // 如果遇到分隔符
            if (c == cSeparator)
            {
                // 将分隔符改为字符串结束标志
                m_szData[nRowLen - 1] = 0;

                // 允许连续出现分隔符，每个分隔符之间是一个新变量
                if (bAllowSeries)
                {
                    // 变量第一次解析，进入这里表示刚开始解析","，或者刚解析完一个变量",,"；
                    // 允许连续出现分隔符，将分隔符前的空字符保存为一个新变量。
                    if (bNewVar)
                    {
                        nVarOffset[m_nSize++] = nRowLen - 1;
                    }
                    // 已经解析了变量部分数据，进入这里表示这个变量已经没有字符了，重置新变量开始标志
                    else
                    {
                        bNewVar = true;
                    }
                }
                // 不允许连续出现分隔符，后面连续的分隔符都已经改为了结束标志
                // 不保存新变量开始位置到nVarOffset中
                else
                {
                    bNewVar = true;
                }
            }
            // 如果不是分隔符
            else
            {
                // 变量第一次解析
                if (bNewVar)
                {
                    // 保存变量的开始位置
                    nVarOffset[m_nSize++] = nRowLen - 1;
                    // 后面的字符为当前变量的字符，新变量标志改为false
                    bNewVar = false;
                }
            }

            // 得到下一个字符
            c = m_szData[nRowLen++];

            // 变量数量已经达到最大值，退出循环
            if (m_nSize >= VAR_MAX_PER_ROW)
            {
                break;
            }
        }

        if (m_nSize == 0)
        {
            return false;
        }

        // 开始生成变量数据
        m_pData = new CsvVariant[m_nSize];
        if (NULL == m_pData)
        {
            return false;
        }

        // 遍历生成变量
        for (size_t i = 0; i < m_nSize; ++i)
        {
            m_pData[i].m_Type = CsvVariant::VT_VALUE;
            // 已经将m_szData按变量分隔符分成了若干字符串，按位置取字符串即可
            m_pData[i].m_Value.szValue = &m_szData[nVarOffset[i]];
        }

        m_pAdapter = pAdapter;

        return true;
    }

    /**
    * \brief 获得数据列数
    * \return 数据列数
    */
    size_t CsvRow::Size()
    {
        return m_nSize;
    }

    /**
    * \brief 获得指定列数据
    * \param nIndex 列数索引
    * \return 指定列数据
    */
    CsvVariant & CsvRow::operator [] (size_t nIndex)
    {
        return GetData(nIndex);
    }

    /**
    * \brief 根据字段名称获得数据
    * \param szName 字段名称
    * \return 指定字段名称数据
    */
    CsvVariant & CsvRow::operator [] (const char *szName)
    {
        return GetData(szName);
    }
    
    /**
    * \brief 获得指定列数据
    * \param nIndex 列数索引
    * \return 指定列数据
    */
    CsvVariant & CsvRow::GetData(size_t nIndex)
    {
        if (nIndex >= m_nSize)
        {
            return CsvVariant::s_EmptyVar;
        }

        return m_pData[nIndex];
    }

    /**
    * \brief 根据字段名称获得数据
    * \param szName 字段名称
    * \return 指定字段名称数据
    */
    CsvVariant & CsvRow::GetData(const char *szName)
    {
        if (NULL == m_pAdapter)
        {
            return CsvVariant::s_EmptyVar;
        }

        int32 nIndex = m_pAdapter->FindPropName(szName);
        if (nIndex < 0)
        {
            return CsvVariant::s_EmptyVar;
        }

        return GetData(nIndex);
    }
}
