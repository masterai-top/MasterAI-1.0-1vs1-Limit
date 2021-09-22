/**
* \file ini_section.h
* \brief ini脚本文件段节点类头文件
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __INI_SECTION_H__
#define __INI_SECTION_H__

#include "ini_variant.h"
#include <map>
#include <string>

namespace dtscript
{
    /**
    * \brief ini段节点类
    * \ingroup script_group
    */
    class IniSection : public IIniIterator
    {
    public:
        /**
        * \brief 构造函数
        */
        IniSection(void);

        /**
        * \brief 析构函数
        */
        virtual ~IniSection(void);

        /**
        * \brief 添加节点
        * \param szKey 节点名称
        * \param szValue 节点值
        * \return 添加成功返回true，否则返回false
        */
        bool AddNode(const char *szKey, const char *szValue);

    public:
        /**
        * \brief 根据节点名称获得节点值
        * \param szName 节点名称
        * \return 指定节点名称的值
        */
        virtual IniVariant & operator [] (const char *szName);

    private:
        /**
        * \brief 根据节点名称获得节点值
        * \param szName 节点名称
        * \return 指定节点名称的值
        */
        IniVariant & GetData(const char *szName);

    private:
        std::map<std::string, IniVariant>       m_mapNodes;     ///< 节点列表
        std::map<std::string, std::string>      m_mapData;      ///< 节点内容
    };
}

#endif // __INI_SECTION_H__