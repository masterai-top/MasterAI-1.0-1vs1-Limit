/**
* \file ini_adapter.h
* \brief ini脚本适配器类头文件
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __INI_ADAPTER_H__
#define __INI_ADAPTER_H__

#include "ini_variant.h"
#include "ini_section.h"
#include <map>
#include <string>
#include "fileloader.h"
#include "strutil.h"
using namespace dtutil;

namespace dtscript
{
    //class FileLoader;

    /**
    * \brief ini脚本适配器
    * \ingroup script_group
    */
    class IniAdapter : public IIniIterator
    {
    public:
        /**
        * \brief 构造函数
        */
        IniAdapter(void);

        /**
        * \brief 析构函数
        */
        virtual ~IniAdapter(void);

    public:
        /**
        * \brief 根据指定段节点名称获得数据
        * \param szName 段节点名称
        * \return 指定段节点数据
        */
        virtual IniVariant & operator [] (const char *szName);

    public:
        /**
        * \brief 装载脚本
        * \param szFileName 脚本文件
        * \param bEncrypt 是否加密
        * \return 装载成功返回true，否则返回false
        */
        bool LoadScript(const char *szFileName, bool bEncrypt);

        /**
        * \brief 装载脚本数据
        * \param pData 脚本数据
        * \param nLen 数据大小
        * \return 装载成功返回true，否则返回false
        */
        bool LoadScript(LPCSTR pData, size_t nLen);

    private:
        /**
        * \brief 装载脚本文件
        * \param file 加载好的脚本文件
        * \return 装载成功返回true，否则返回false
        */
        bool LoadScriptFile(FileLoader *file);

        /**
        * \brief 根据指定段节点名称获得数据
        * \param szName 段节点名称
        * \return 指定段节点数据
        */
        IniVariant & GetData(const char *szName);

    private:
        IniSection                          m_NonameSection;    ///< 无段名称的段节点
        std::map<std::string, IniVariant>   m_SectionTable;     ///< 段节点列表
    };
}

#endif // __INI_ADAPTER_H__