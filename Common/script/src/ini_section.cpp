/**
* \file ini_section.cpp
* \brief ini脚本文件段节点类函数实现
*/

#include "pch.h"
#include "ini_section.h"


namespace dtscript
{
    /**
    * \brief 构造函数
    */
    IniSection::IniSection(void)
    {
    }

    /**
    * \brief 析构函数
    */
    IniSection::~IniSection(void)
    {
        for (std::map<std::string, IniVariant>::iterator it = m_mapNodes.begin(); it != m_mapNodes.end(); ++it)
        {
            IniVariant &var = it->second;
            if (var.m_Type == IniVariant::VT_ITERATOR && var.m_Value.itValue != NULL)
            {
                delete var.m_Value.itValue;
            }
        }
        m_mapNodes.clear();
    }

    /**
    * \brief 添加节点
    * \param szKey 节点名称
    * \param szValue 节点值
    * \return 添加成功返回true，否则返回false
    */
    bool IniSection::AddNode(const char *szKey, const char *szValue)
    {
        if (NULL == szKey || NULL == szValue)
        {
            return false;
        }

        // 存储内容
        m_mapData[szKey] = szValue;

        IniVariant var;
        var.m_Type = IniVariant::VT_VALUE;
        var.m_Value.szValue = m_mapData[szKey].c_str(); // 变量绑定存储内容

        // 映射节点
        m_mapNodes[szKey] = var;

        return true;
    }

    /**
    * \brief 根据节点名称获得节点值
    * \param szName 节点名称
    * \return 指定节点名称的值
    */
    IniVariant & IniSection::operator [] (const char *szName)
    {
        return GetData(szName);
    }

    /**
    * \brief 根据节点名称获得节点值
    * \param szName 节点名称
    * \return 指定节点名称的值
    */
    IniVariant & IniSection::GetData(const char *szName)
    {
        std::map<std::string, IniVariant>::iterator it = m_mapNodes.find(szName);
        if (it == m_mapNodes.end())
        {
            return IniVariant::s_EmptyVar;
        }

        return it->second;
    }
}