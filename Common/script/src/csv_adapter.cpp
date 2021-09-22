/**
* \file csv_adapter.cpp
* \brief csv脚本适配器类函数实现
*/

#include "pch.h"
#include "csv_adapter.h"


namespace dtscript
{
    /**
    * \brief 构造函数
    */
    CsvAdapter::CsvAdapter(void)
    {
    }

    /**
    * \brief 析构函数
    */
    CsvAdapter::~CsvAdapter(void)
    {
        // 释放行数据
        for (std::vector<CsvVariant>::iterator it = m_RowTable.begin(); it != m_RowTable.end(); ++it)
        {
            CsvVariant &row = *it;
            if (row.m_Type == CsvVariant::VT_ITERATOR && row.m_Value.itValue != NULL)
            {
                delete row.m_Value.itValue;
            }
        }
        m_RowTable.clear();
    }

    /**
    * \brief 获得文档内容的行数
    * \return 文档内容的行数
    */
    size_t CsvAdapter::Size()
    {
        return m_RowTable.size();
    }

    /**
    * \brief 根据指定行数获得数据
    * \param nIndex 行数索引
    * \return 指定行数据
    */
    CsvVariant & CsvAdapter::operator [] (size_t nIndex)
    {
        return GetData(nIndex);
    }

    /**
    * \brief 根据指定名称获得数据
    * \param szName 指定名称
    * \return 指定名称的数据
    */
    CsvVariant & CsvAdapter::operator [] (const char *szName)
    {
        return GetData(szName);
    }

    /**
    * \brief 装载脚本
    * \param szFileName 脚本文件
    * \param bEncrypt 是否加密
    * \return 装载成功返回true，否则返回false
    */
    bool CsvAdapter::LoadScript(const char *szFileName, bool bEncrypt)
    {
        if (bEncrypt)
        {
            return LoadScriptCse(szFileName);
        }

        return LoadScriptCsv(szFileName);
    }

    /**
    * \brief 装载脚本数据
    * \param pData 脚本数据
    * \param nLen 数据大小
    * \return 装载成功返回true，否则返回false
    */
    bool CsvAdapter::LoadScript(LPCSTR pData, size_t nLen)
    {
        FileLoader file;
        if (!file.Load(pData, nLen))
        {
            return false;
        }

        return LoadScriptFile(&file);
    }

    /**
    * \brief 根据参数名称查找参数在第几列
    * \param szName 列名称
    * \return 参数所在列索引，非列表参数返回-1
    */
    int32 CsvAdapter::FindPropName(const char *szName)
    {
        size_t nLen = strlen(szName);

        // 先遍历英文名称
        for(size_t i = 0; i < m_Title[1].Size(); ++i)
        {
            if (StrUtil::strncmp(szName, m_Title[1][i].m_Value.szValue, nLen) == 0)
            {
                return (int32)(i);
            }
        }

        // 再遍历中文名称
        for(size_t i = 0; i < m_Title[0].Size(); ++i)
        {
            if (StrUtil::strncmp(szName, m_Title[0][i].m_Value.szValue, nLen) == 0)
            {
                return (int32)(i);
            }
        }

        // 最后遍历protobuf名称
        for(size_t i = 0; i < m_Title[2].Size(); ++i)
        {
            if (StrUtil::strncmp(szName, m_Title[2][i].m_Value.szValue, nLen) == 0)
            {
                return (int32)(i);
            }
        }

        return -1;
    }

    /**
    * \brief 装载csv脚本
    * \param szFileName 脚本文件
    * \return 装载成功返回true，否则返回false
    */
    bool CsvAdapter::LoadScriptCsv(const char *szFileName)
    {
        FileLoader file;
        if (!file.Load(szFileName))
        {
            return false;
        }

        return LoadScriptFile(&file);
    }

    /**
    * \brief 装载cse脚本(加密csv脚本)
    * \param szFileName 脚本文件
    * \return 装载成功返回true，否则返回false
    */
    bool CsvAdapter::LoadScriptCse(const char *szFileName)
    {
        FileLoader file;
        if (!file.Load(szFileName))
        {
            return false;
        }

        // 文件解密处理 ...

        return LoadScriptFile(&file);
    }

    /**
    * \brief 装载脚本文件
    * \param file 加载好的脚本文件
    * \return 装载成功返回true，否则返回false
    */
    bool CsvAdapter::LoadScriptFile(FileLoader *file)
    {
        if (NULL == file)
        {
            return false;
        }

        std::string strRow = "";

        // 装载标题行
        for (size_t i = 0; i < CSV_TITLE_ROW_COUNT; ++i)
        {
            // 空行，继续读下一行
            if (file->GetLine(strRow) == 0)
            {
                continue;
            }

            // 装载行数据
            if (!m_Title[i].LoadData(this, strRow.c_str()))
            {
                // 装载失败或没变量数据，继续读下一行
                continue;
            }
        }

        size_t nRow = CSV_TITLE_ROW_COUNT;

        // 装载行数据
        while (!file->IsEOF())
        {
            ++nRow;

            // 空行，继续读下一行
            if (file->GetLine(strRow) == 0)
            {
                continue;
            }

            CsvRow *pRow = new CsvRow();
            if (NULL == pRow)
            {
                return false;
            }

            // 装载行数据
            if (!pRow->LoadData(this, strRow.c_str()))
            {
                // 装载失败或没变量数据，继续读下一行
                delete pRow;
                continue;
            }

            CsvVariant var;
            var.m_Type = CsvVariant::VT_ITERATOR;
            var.m_Value.itValue = pRow;

            m_RowTable.push_back(var);
            // 将行的第一列作为key存储起来，以便根据名称查找所在行，多字段复合key的将无意义
            //m_ObjectNameTable.push_back(pRow->GetData(0).m_Value.szValue);
        }

        return true;
    }
    
    /**
    * \brief 根据指定行数获得数据
    * \param nIndex 行数索引
    * \return 指定行数据
    */
    CsvVariant & CsvAdapter::GetData(size_t nIndex)
    {
        if (nIndex >= m_RowTable.size())
        {
            return CsvVariant::s_EmptyVar;
        }

        return m_RowTable[nIndex];
    }

    /**
    * \brief 根据指定名称获得数据
    * \param szName 指定名称
    * \return 指定名称的数据
    */
    CsvVariant & CsvAdapter::GetData(const char *szName)
    {
        // 无意义，除非每一行都有一个唯一key，传入的szName就是这个key
        return CsvVariant::s_EmptyVar;
    }
}
