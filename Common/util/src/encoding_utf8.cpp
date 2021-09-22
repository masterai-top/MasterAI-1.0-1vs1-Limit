/**
* \file encoding_utf8.cpp
* \brief UTF8 编码转换实现代码
*/

#include "pch.h"
#include "encoding_utf8.h"
#include "iconv.h"

namespace dtutil
{
    /**
    * \brief 构造函数
    * 私有构造函数，使该类不被继承
    */
    UTF8Encoding::UTF8Encoding()
        : m_pToUTF8(NULL), m_pFromUTF8(NULL)
    {
        iconv_t rs = (iconv_t)(-1);
        rs = iconv_open("UTF-8", LOC_UNICODE);
        if ((iconv_t)(-1) == rs)
        {
            throw EncodingErrorException("to utf8 code error.");
        }
        else
        {
            m_pToUTF8 = rs;
        }

        rs = iconv_open(LOC_UNICODE, "UTF-8");
        if ((iconv_t)(-1) == rs)
        {
            throw EncodingErrorException("from utf8 code error.");
        }
        else
        {
            m_pFromUTF8 = rs;
        }
    }

    /**
    * \brief 析构函数
    */
    UTF8Encoding::~UTF8Encoding()
    {
        if (m_pToUTF8)
        {
            iconv_close(m_pToUTF8);
        }

        if (m_pFromUTF8)
        {
            iconv_close(m_pFromUTF8);
        }
    }

    /**
    * \brief 获取编码实例
    * \return 编码实例
    */
    Encoding& UTF8Encoding::GetInstance()
    {
        static UTF8Encoding utf8;
        return utf8;
    }

    /**
    * \brief 把 Unicode 字符串转换为指定编码的字符串
    * \param wstr Unicode字符串
    * \return 输出字符串
    */
    std::string UTF8Encoding::Transcode(const std::wstring &wstr)
    {
        return Convert(wstr.c_str());
    }

    /**
    * \brief 把 Unicode 字符串转换为指定编码的字符串
    * \param wstr Unicode字符串
    * \return 输出字符串
    */
    std::string UTF8Encoding::Transcode(const wchar_t *wstr)
    {
        return Convert(wstr);
    }

    /**
    * \brief 把 Unicode 字符串转换为指定编码的字符串
    * \param wstr Unicode字符串
    * \return 输出字符串
    */
    std::string UTF8Encoding::Transcode(wchar_t *wstr)
    {
        return Convert(wstr);
    }

    /**
    * \brief 把指定编码的字符串转换为 Unicode 字符串
    * \param str 输入字符串
    * \return Unicode字符串
    */
    std::wstring UTF8Encoding::Transcode(const std::string &str)
    {
        return Convert(str.c_str());
    }

    /**
    * \brief 把指定编码的字符串转换为 Unicode 字符串
    * \param str 输入字符串
    * \return Unicode字符串
    */
    std::wstring UTF8Encoding::Transcode(const char *str)
    {
        return Convert(str);
    }

    /**
    * \brief 把指定编码的字符串转换为 Unicode 字符串
    * \param str 输入字符串
    * \return Unicode字符串
    */
    std::wstring UTF8Encoding::Transcode(char *str)
    {
        return Convert(str);
    }

    /**
    * \brief 把指定编码的字符串转换为 Unicode 字符串
    * \param str 输入字符串
    * \return Unicode字符串
    */
    std::wstring UTF8Encoding::Convert(const char *str)
    {
        const char *in = str;
        size_t inlen = strlen(str) + 1;
        size_t outlen = inlen * sizeof(wchar_t);

        wchar_t *out = new wchar_t[outlen];
        memset(out, 0, outlen);

        // 使用iconv时会改变输出指针的值，为保证正确输出，可以使用指针拷贝作为iconv的参数
        wchar_t *tmp = out;

        // 实编码转换
        size_t rs = iconv(m_pFromUTF8, (char **)&in, &inlen, (char **)&tmp, &outlen);
        if ((size_t)(-1) == rs)
        {
            throw EncodingErrorException("cant convert code from utf8 to unicode.");
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
    std::string UTF8Encoding::Convert(const wchar_t *wstr)
    {
        const size_t UTF8_CHAR_SIZE = 4;  // utf8 转换时需要的最大字符长度

        const wchar_t *in = wstr;
        size_t inlen = wcslen(wstr) * sizeof(wchar_t);
        size_t outlen = inlen * UTF8_CHAR_SIZE + 1;

        char *out = new char[outlen];
        memset(out, 0, outlen);

        // 使用iconv时会改变输出指针的值，为保证正确输出，可以使用指针拷贝作为iconv的参数
        char *tmp = out;

        // 实编码转换
        size_t rs = iconv(m_pToUTF8, (char **)&in, &inlen, (char **)&tmp, &outlen);
        if ((size_t)(-1) == rs)
        {
            throw EncodingErrorException("cant convert code from unicode to utf8.");
        }

        std::string str = out;
        delete [] out;

        return str;
    }
}