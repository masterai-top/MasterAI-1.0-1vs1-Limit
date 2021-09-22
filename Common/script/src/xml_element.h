/**
* \file xml_element.h
* \brief xml脚本文件节点类头文件
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __XML_ELEMENT_H__
#define __XML_ELEMENT_H__

#include "xml_variant.h"
#include "tinyxml.h"
#include <map>
#include <vector>

namespace dtscript
{
    /**
    * \brief xml节点类
    * \ingroup script_group
    */
    class XmlElement : public IXmlIterator
    {
    public:
        /**
        * \brief 构造函数
        */
        XmlElement(void);

        /**
        * \brief 析构函数
        */
        virtual ~XmlElement(void);

        /**
        * \brief 装载数据
        * \param pElement tinyxml节点
        * \return 装载成功返回true，否则返回false
        */
        bool LoadData(const TiXmlElement *pElement);

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
       virtual  XmlVariant & ChildElement(const char *szName, uint32 nIndex);

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

    private:
        std::string                                         m_strName;          ///< 节点名称
        std::map<std::string, XmlVariant>                   m_mapAttribute;     ///< 属性列表
        std::map<std::string, std::string>                  m_mapAttrData;      ///< 属性数据列表
        std::map< std::string, std::vector<XmlVariant> >    m_mapElement;       ///< 子节点列表
        std::map< std::string, std::vector<XmlElement *> >  m_mapElementData;   ///< 子节点数据列表
    };
}

#endif // __XML_ELEMENT_H__