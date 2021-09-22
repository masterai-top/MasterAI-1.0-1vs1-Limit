/**
* \file ini_adapter.cpp
* \brief ini脚本适配器类函数实现
*/

#include "pch.h"
#include "ini_adapter.h"



namespace dtscript
{
    /**
    * \brief 构造函数
    */
    IniAdapter::IniAdapter(void)
    {
    }

    /**
    * \brief 析构函数
    */
    IniAdapter::~IniAdapter(void)
    {
        // 释放段节点数据
        for (std::map<std::string, IniVariant>::iterator it = m_SectionTable.begin(); it != m_SectionTable.end(); ++it)
        {
            IniVariant &section = it->second;
            if (section.m_Type == IniVariant::VT_ITERATOR && section.m_Value.itValue != NULL)
            {
                delete section.m_Value.itValue;
            }
        }
        m_SectionTable.clear();
    }

    /**
    * \brief 根据指定段节点名称获得数据
    * \param szName 段节点名称
    * \return 指定段节点数据
    */
    IniVariant & IniAdapter::operator [] (const char *szName)
    {
        return GetData(szName);
    }

    /**
    * \brief 装载脚本
    * \param szFileName 脚本文件
    * \param bEncrypt 是否加密
    * \return 装载成功返回true，否则返回false
    */
    bool IniAdapter::LoadScript(const char *szFileName, bool bEncrypt)
    {
        FileLoader file;
        if (!file.Load(szFileName))
        {
            return false;
        }

        return LoadScriptFile(&file);
    }

    /**
    * \brief 装载脚本数据
    * \param pData 脚本数据
    * \param nLen 数据大小
    * \return 装载成功返回true，否则返回false
    */
    bool IniAdapter::LoadScript(LPCSTR pData, size_t nLen)
    {
        FileLoader file;
        if (!file.Load(pData, nLen))
        {
            return false;
        }

        return LoadScriptFile(&file);
    }

    /**
    * \brief 装载脚本文件
    * \param file 加载好的脚本文件
    * \return 装载成功返回true，否则返回false
    */
    bool IniAdapter::LoadScriptFile(FileLoader *file)
    {
        if (NULL == file)
        {
            return false;
        }

        std::string strRow = "";
        IniSection *pCurSection = NULL;     ///< 当前段节点

        // 读其它所有行
        while (!file->IsEOF())
        {
            // 空行，继续读下一行
            if (file->GetLine(strRow) == 0)
            {
                continue;
            }

            //截断注释行
            size_t nFound = strRow.find("#", 3);
            if(nFound != std::string::npos)
            {
                strRow = strRow.substr(0, nFound);
            }

            //删除tab
            StrUtil::trimex(strRow, "\t");

            // 去掉前后空格
            strRow = StrUtil::trim(strRow);

            // strstr(str1,str2) 函数用于判断字符串str2是否是str1的子串。如果是，则该函数返回str2在str1中首次出现的地址；否则，返回NULL。
            // 段节点名称
            if (strstr(strRow.c_str(), "[") && strstr(strRow.c_str(), "]"))
            {
                pCurSection = new IniSection();
                if (NULL == pCurSection)
                {
                    return false;
                }

                //用str替换指定字符串从起始位置pos开始长度为len的字符 
                //string& replace (size_t pos, size_t len, const string& str); 
                strRow = strRow.replace(strRow.find("["), 1, "");   // 去掉左[
                strRow = strRow.replace(strRow.rfind("]"), 1, "");  // 去掉右]

                IniVariant var;
                var.m_Type = IniVariant::VT_ITERATOR;
                var.m_Value.itValue = pCurSection;

                m_SectionTable[strRow] = var;
            }
            // 数据节点
            else if (strstr(strRow.c_str(), "="))
            {
                size_t pos = strRow.find("=", 0);
                std::string strKey = StrUtil::trim(strRow.substr(0, pos));                      // "="左边字符
                std::string strValue = StrUtil::trim(strRow.substr(pos+1, strRow.length()));    // "="右边字符

                if (NULL == pCurSection)
                {
                    pCurSection = &m_NonameSection;
                }

                pCurSection->AddNode(strKey.c_str(), strValue.c_str());
            }
            // 不是段节点，也不是数据节点，忽略
            else
            {

            }
        }

        return true;
    }
    
    /**
    * \brief 根据指定段节点名称获得数据
    * \param szName 段节点名称
    * \return 指定段节点数据
    */
    IniVariant & IniAdapter::GetData(const char *szName)
    {
        std::map<std::string, IniVariant>::iterator it = m_SectionTable.find(szName);
        if (it == m_SectionTable.end())
        {
            return IniVariant::s_EmptyVar;
        }

        return it->second;
    }
}
