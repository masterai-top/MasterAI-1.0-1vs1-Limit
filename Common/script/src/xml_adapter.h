/**
* \file xml_adapter.h
* \brief xml脚本适配器类头文件
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __XML_ADAPTER_H__
#define __XML_ADAPTER_H__

#include "xml_variant.h"
#include "tinyxml.h"
#include <vector>

namespace dtscript
{
    /**
    * \brief xml脚本适配器类
    * \ingroup script_group
    */
    class XmlAdapter : public IXmlIterator
    {
    public:
        /**
        * \brief 构造函数
        */
        XmlAdapter(void);

        /**
        * \brief 析构函数
        */
        virtual ~XmlAdapter(void);

    public:
        /**
        * \brief 获得子节点数量
        * \param szName 子节点名称
        * \return 子节点数量
        */
        virtual size_t ChildElementCount(const char *szName);

        /**
        * \brief 获得指定名称指定索引的子节点
        * \param szName 子节点名称
        * \param nIndex 指定名称索引
        * \return 指定名称指定索引的子节点
        */
        virtual XmlVariant & ChildElement(const char *szName, uint32 nIndex);

        /**
        * \brief 获得节点属性
        * \param szName 属性名称
        * \return 指定名称节点属性数据
        */
        virtual XmlVariant & GetData(const char *szName);

        /**
        * \brief 获得节点属性
        * \param szName 属性名称
        * \return 指定名称节点属性数据
        */
        virtual XmlVariant & operator [] (const char *szName);

    public:
        /**
        * \brief 加载xml资源文件
        * \param szFileName 资源文件
        * \param bEncrypt 是否加密
        * \return 成功返回文件数据指针，否则返回NULL
        */
        bool LoadScript(const char *szFileName, bool bEncrypt);

    private:
        TiXmlDocument   m_Document;         ///< tinyxml文档
        XmlVariant      m_RootElement;      ///< 根节点
    };
}

#endif // __XML_ADAPTER_H__