/**
* \file xml_adapter.cpp
* \brief xml脚本适配器类函数实现
*/

#include "pch.h"
#include "xml_adapter.h"
#include "xml_element.h"


namespace dtscript
{
    /**
    * \brief 构造函数
    */
    XmlAdapter::XmlAdapter(void)
    {
    }

    /**
    * \brief 析构函数
    */
    XmlAdapter::~XmlAdapter(void)
    {
    }

    /**
    * \brief 获得子节点数量
    * \param szName 子节点名称
    * \return 子节点数量
    */
    size_t XmlAdapter::ChildElementCount(const char *szName)
    {
        return m_RootElement.ChildElementCount(szName);
    }

    /**
    * \brief 获得指定名称指定索引的子节点
    * \param szName 子节点名称
    * \param nIndex 指定名称索引
    * \return 指定名称指定索引的子节点
    */
    XmlVariant & XmlAdapter::ChildElement(const char *szName, uint32 nIndex)
    {
        return m_RootElement.ChildElement(szName, nIndex);
    }

    /**
    * \brief 获得节点属性
    * \param szName 属性名称
    * \return 指定名称节点属性数据
    */
    XmlVariant & XmlAdapter::GetData(const char *szName)
    {
        return m_RootElement[szName];
    }

    /**
    * \brief 获得节点属性
    * \param szName 属性名称
    * \return 指定名称节点属性数据
    */
    XmlVariant & XmlAdapter::operator [] (const char *szName)
    {
        return GetData(szName);
    }

    /**
    * \brief 加载xml资源文件
    * \param szFileName 资源文件
    * \param bEncrypt 是否加密
    * \return 成功返回文件数据指针，否则返回NULL
    */
    bool XmlAdapter::LoadScript(const char *szFileName, bool bEncrypt)
    {
        // 装载数据
        if (!m_Document.LoadFile(szFileName))
        {
            return false;
        }

        // 获得根节点
        const TiXmlElement *pRootElement = m_Document.RootElement();
        if (NULL == pRootElement)
        {
            return false;
        }

        XmlElement *pRoot = new XmlElement();
        if (NULL == pRoot)
        {
            return false;
        }

        // 初始化根节点变量
        if (!pRoot->LoadData(pRootElement))
        {
            return false;
        }

        m_RootElement.m_Type = XmlVariant::VT_ELEMENT;
        m_RootElement.m_Value.itValue = pRoot;

        return true;
    }
}
