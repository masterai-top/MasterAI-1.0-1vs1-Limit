/**
* \file encoding_gbk.h
* \brief GBK 编码转换类
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __GBK_ENCODING_H__
#define __GBK_ENCODING_H__

#include "encoding.h"

namespace dtutil
{
    /**
    * \brief GBK 编码转换类
    * \ingroup encoding_group
    *
    * 这是一个单实例的类，通过 Encoding::GBK 访问
    */
    class GBKEncoding : public Encoding
    {
    public:
        /**
        * \brief 把 Unicode 字符串转换为指定编码的字符串
        * \param wstr Unicode字符串
        * \return 输出字符串
        */
        virtual std::string Transcode(const std::wstring &wstr);

        /**
        * \brief 把 Unicode 字符串转换为指定编码的字符串
        * \param wstr Unicode字符串
        * \return 输出字符串
        */
        virtual std::string Transcode(const wchar_t *wstr);

        /**
        * \brief 把 Unicode 字符串转换为指定编码的字符串
        * \param wstr Unicode字符串
        * \return 输出字符串
        */
        virtual std::string Transcode(wchar_t *wstr);

        /**
        * \brief 把指定编码的字符串转换为 Unicode 字符串
        * \param str 输入字符串
        * \return Unicode字符串
        */
        virtual std::wstring Transcode(const std::string &str);

        /**
        * \brief 把指定编码的字符串转换为 Unicode 字符串
        * \param str 输入字符串
        * \return Unicode字符串
        */
        virtual std::wstring Transcode(const char *str);

        /**
        * \brief 把指定编码的字符串转换为 Unicode 字符串
        * \param str 输入字符串
        * \return Unicode字符串
        */
        virtual std::wstring Transcode(char *str);

        /**
        * \brief 获取编码实例
        * \return 编码实例
        */
        static Encoding & GetInstance();

        /**
        * \brief 析构函数
        */
        virtual ~GBKEncoding();

    protected:
        /**
        * \brief 把指定编码的字符串转换为 Unicode 字符串
        * \param str 输入字符串
        * \return Unicode字符串
        */
        virtual std::wstring Convert(const char *str);

        /**
        * \brief 把 Unicode 字符串转换为特定编码的字符串
        * \param wstr Unicode字符串
        * \return 输出字符串
        */
        virtual std::string Convert(const wchar_t *wstr);

    private:
        /**
        * \brief 构造函数
        * 私有构造函数，使该类不被继承
        */
        GBKEncoding();

        /**
        * \brief 拷贝构造函数
        */
        GBKEncoding(const GBKEncoding &);

        /**
        * \brief =赋值重载
        */
        GBKEncoding & operator = (const GBKEncoding &);

    private:
        void    *m_pToGBK;          ///< 其它编码转换成GBK编码
        void    *m_pFromGBK;        ///< GBK编码转换成其它编码
    };
}

#endif // __GBK_ENCODING_H__