/**
* \file encoding_gbk.cpp
* \brief GBK 编码转换实现代码
*/

#include "pch.h"
#include "encoding_gbk.h"
#include "iconv.h"

namespace dtutil
{
    /**
    * \brief 构造函数
    * 私有构造函数，使该类不被继承
    */
    GBKEncoding::GBKEncoding()
        : m_pToGBK(NULL), m_pFromGBK(NULL)
    {
        iconv_t rs = (iconv_t)(-1);
        rs = iconv_open("GBK", LOC_UNICODE);
        if ((iconv_t)(-1) == rs)
        {
            throw EncodingErrorException("to gbk code error.");
        }
        else
        {
            m_pToGBK = rs;
        }

        rs = iconv_open(LOC_UNICODE, "GBK");
        if ((iconv_t)(-1) == rs)
        {
            throw EncodingErrorException("from gbk code error.");
        }
        else
        {
            m_pFromGBK = rs;
        }
    }

    /**
    * \brief 析构函数
    */
    GBKEncoding::~GBKEncoding()
    {
        if (m_pToGBK)
        {
            iconv_close(m_pToGBK);
        }

        if (m_pFromGBK)
        {
            iconv_close(m_pFromGBK);
        }
    }

    /**
    * \brief 获取编码实例
    * \return 编码实例
    */
    Encoding& GBKEncoding::GetInstance()
    {
        static GBKEncoding gbk;
        return gbk;
    }

    /**
    * \brief 把 Unicode 字符串转换为指定编码的字符串
    * \param wstr Unicode字符串
    * \return 输出字符串
    */
    std::string GBKEncoding::Transcode(const std::wstring &wstr)
    {
        return Convert(wstr.c_str());
    }

    /**
    * \brief 把 Unicode 字符串转换为指定编码的字符串
    * \param wstr Unicode字符串
    * \return 输出字符串
    */
    std::string GBKEncoding::Transcode(const wchar_t *wstr)
    {
        return Convert(wstr);
    }

    /**
    * \brief 把 Unicode 字符串转换为指定编码的字符串
    * \param wstr Unicode字符串
    * \return 输出字符串
    */
    std::string GBKEncoding::Transcode(wchar_t *wstr)
    {
        return Convert(wstr);
    }

    /**
    * \brief 把指定编码的字符串转换为 Unicode 字符串
    * \param str 输入字符串
    * \return Unicode字符串
    */
    std::wstring GBKEncoding::Transcode(const std::string &str)
    {
        return Convert(str.c_str());
    }

    /**
    * \brief 把指定编码的字符串转换为 Unicode 字符串
    * \param str 输入字符串
    * \return Unicode字符串
    */
    std::wstring GBKEncoding::Transcode(const char *str)
    {
        return Convert(str);
    }

    /**
    * \brief 把指定编码的字符串转换为 Unicode 字符串
    * \param str 输入字符串
    * \return Unicode字符串
    */
    std::wstring GBKEncoding::Transcode(char *str)
    {
        return Convert(str);
    }

    /**
    * \brief 把指定编码的字符串转换为 Unicode 字符串
    * \param str 输入字符串
    * \return Unicode字符串
    */
    std::wstring GBKEncoding::Convert(const char *str)
    {
        const char *in = str;
        size_t inlen = strlen(str) + 1;
        size_t outlen = inlen * sizeof(wchar_t);

        wchar_t *out = new wchar_t[outlen];
        memset(out, 0, outlen);

        // 使用iconv时会改变输出指针的值，为保证正确输出，可以使用指针拷贝作为iconv的参数
        wchar_t *tmp = out;

        // 实编码转换
        size_t rs = iconv(m_pFromGBK, (char **)&in, &inlen, (char **)&tmp, &outlen);
        if ((size_t)(-1) == rs)
        {
            throw EncodingErrorException("cant convert code from gbk to unicode.");
        }

        std::wstring wstr = out;
        delete [] out;

        return wstr;
    }

    /**
    * \brief 把 Unicode 字符串转换为特定编码的字符串
    * \param wstr Unicode字符串
    * \return 输出字符串
    */
    std::string GBKEncoding::Convert(const wchar_t *wstr)
    {
        const size_t GBK_CHAR_SIZE = 2;  // gbk 转换时需要的最大字符长度

        const wchar_t *in = wstr;
        size_t inlen = wcslen(wstr) * sizeof(wchar_t);
        size_t outlen = inlen * GBK_CHAR_SIZE + 1;

        char *out = new char[outlen];
        memset(out, 0, outlen);

        // 使用iconv时会改变输出指针的值，为保证正确输出，可以使用指针拷贝作为iconv的参数
        char *tmp = out;

        // 实编码转换
        size_t rs = iconv(m_pToGBK, (char **)&in, &inlen, (char **)&tmp, &outlen);
        if ((size_t)(-1) == rs)
        {
            throw EncodingErrorException("cant convert code from unicode to gbk.");
        }

        std::string str = out;
        delete [] out;

        return str;
    }
}