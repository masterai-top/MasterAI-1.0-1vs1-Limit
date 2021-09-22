/**
* \file xml_element.cpp
* \brief xml脚本文件节点数据类函数实现
*/

#include "pch.h"
#include "xml_element.h"


namespace dtscript
{
    /**
    * \brief 构造函数
    */
    XmlElement::XmlElement(void)
    {
        m_strName = "";
    }

    /**
    * \brief 析构函数
    */
    XmlElement::~XmlElement(void)
    {
        for (std::map< std::string, std::vector<XmlElement *> >::iterator it = m_mapElementData.begin(); it != m_mapElementData.end(); ++it)
        {
            std::vector<XmlElement *> &vecElement = it->second;

            for(std::vector<XmlElement *>::iterator iter = vecElement.begin(); iter != vecElement.end(); ++iter)
            {
                XmlElement *pElement = *iter;
                if (NULL != pElement)
                {
                    delete pElement;
                }
            }
        }
        m_mapElementData.clear();

        m_mapElement.clear();
        m_mapAttrData.clear();
        m_mapAttribute.clear();

        m_strName = "";
    }

    /**
    * \brief 装载数据
    * \param pElement tinyxml节点
    * \return 装载成功返回true，否则返回false
    */
    bool XmlElement::LoadData(const TiXmlElement *pElement)
    {
        if (NULL == pElement)
        {
            return false;
        }

        m_strName = pElement->Value();

        // 节点属性
        const TiXmlAttribute *pAttribute = pElement->FirstAttribute();
        while(NULL != pAttribute)
        {
            std::string strAttrName = pAttribute->Name();   // 属性
            std::string strAttrValue = pAttribute->Value(); // 值

            // 存储属性数据
            m_mapAttrData[strAttrName] = strAttrValue;

            // 映射属性变量
            XmlVariant var;
            var.m_Type = XmlVariant::VT_ATTRIBUTE;
            var.m_Value.szValue = m_mapAttrData[strAttrName].c_str();
            m_mapAttribute[strAttrName] = var;

            // 获得下一个属性
            const TiXmlAttribute *pNext = pAttribute->Next();
            pAttribute = pNext;
        }

        // 子节点
        const TiXmlElement *pChildElement = pElement->FirstChildElement();
        while(NULL != pChildElement)
        {
            XmlElement *pChild = new XmlElement();
            if (NULL == pChild)
            {
                return false;
            }

            std::string strElementName = pChildElement->Value();    // 节点名称

            if (pChild->LoadData(pChildElement))
            {
                // 存储子节点数据
                m_mapElementData[strElementName].push_back(pChild);

                // 映射节点变量
                XmlVariant var;
                var.m_Type = XmlVariant::VT_ELEMENT;
                var.m_Value.itValue = pChild;

                m_mapElement[strElementName].push_back(var);
            }

            // 获得下一个子节点
            pChildElement = pElement->NextSiblingElement();
        }

        return true;
    }

    /**
    * \brief 获得子节点数量
    * \param szName 子节点名称
    * \return 子节点数量
    */
    size_t XmlElement::ChildElementCount(const char *szName)
    {
        std::map< std::string, std::vector<XmlVariant> >::iterator it = m_mapElement.find(szName);
        if (it == m_mapElement.end())
        {
            return 0;
        }

        std::vector<XmlVariant> &vecChildElement = it->second;

        return vecChildElement.size();
    }

    /**
    * \brief 获得指定名称指定索引的子节点
    * \param szName 子节点名称
    * \param nIndex 指定名称索引
    * \return 指定名称指定索引的子节点
    */
    XmlVariant & XmlElement::ChildElement(const char *szName, uint32 nIndex)
    {
        std::map< std::string, std::vector<XmlVariant> >::iterator it = m_mapElement.find(szName);
        if (it == m_mapElement.end())
        {
            return XmlVariant::s_EmptyVar;
        }

        std::vector<XmlVariant> &vecChildElement = it->second;

        if (nIndex >= vecChildElement.size())
        {
            return XmlVariant::s_EmptyVar;
        }

        return vecChildElement[nIndex];
    }

    /**
    * \brief 获得节点属性
    * \param szName 属性名称
    * \return 指定名称节点属性数据
    */
    XmlVariant & XmlElement::GetData(const char *szName)
    {
        std::map<std::string, XmlVariant>::iterator it = m_mapAttribute.find(szName);
        if (it == m_mapAttribute.end())
        {
            return XmlVariant::s_EmptyVar;
        }

        return it->second;
    }

    /**
    * \brief 获得节点属性
    * \param szName 属性名称
    * \return 指定名称节点属性数据
    */
    XmlVariant & XmlElement::operator [] (const char *szName)
    {
        return GetData(szName);
    }
}
