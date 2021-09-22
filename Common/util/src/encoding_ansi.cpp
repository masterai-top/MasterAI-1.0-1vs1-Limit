/**
* \file encoding_ansi.cpp
* \brief ANSI 编码转换实现代码
*/

#include "pch.h"
#include "encoding_ansi.h"


namespace dtutil
{
    /**
    * \brief 获取编码实例
    * \return 编码实例
    */
    Encoding & ANSIEncoding::GetInstance()
    {
        static ANSIEncoding ansi;
        return ansi;
    }

    /**
    * \brief 把 Unicode 字符串转换为指定编码的字符串
    * \param wstr Unicode字符串
    * \return 输出字符串
    */
    std::string ANSIEncoding::Transcode(const std::wstring &wstr)
    {
        return Convert(wstr.c_str());
    }

    /**
    * \brief 把 Unicode 字符串转换为指定编码的字符串
    * \param wstr Unicode字符串
    * \return 输出字符串
    */
    std::string ANSIEncoding::Transcode(const wchar_t *wstr)
    {
        return Convert(wstr);
    }

    /**
    * \brief 把 Unicode 字符串转换为指定编码的字符串
    * \param wstr Unicode字符串
    * \return 输出字符串
    */
    std::string ANSIEncoding::Transcode(wchar_t *wstr)
    {
        return Convert(wstr);
    }

    /**
    * \brief 把指定编码的字符串转换为 Unicode 字符串
    * \param str 输入字符串
    * \return Unicode字符串
    */
    std::wstring ANSIEncoding::Transcode(const std::string &str)
    {
        return Convert(str.c_str());
    }

    /**
    * \brief 把指定编码的字符串转换为 Unicode 字符串
    * \param str 输入字符串
    * \return Unicode字符串
    */
    std::wstring ANSIEncoding::Transcode(const char *str)
    {
        return Convert(str);
    }

    /**
    * \brief 把指定编码的字符串转换为 Unicode 字符串
    * \param str 输入字符串
    * \return Unicode字符串
    */
    std::wstring ANSIEncoding::Transcode(char *str)
    {
        return Convert(str);
    }

    /**
    * \brief 把指定编码的字符串转换为 Unicode 字符串
    * \param str 输入字符串
    * \return Unicode字符串
    */
    std::wstring ANSIEncoding::Convert(const char *str)
    {
        setlocale(LC_CTYPE, "");    // 使用系统默认编码

        size_t size = strlen(str) + 1;
        wchar_t *buf = new wchar_t[size];   // wcs 的字符数要比 mbs 的字节数少
        memset(buf, 0, sizeof(wchar_t) * size);
        size = mbstowcs(buf, str, size);
        if ((size_t)(-1) == size)
        {
            throw EncodingErrorException("invalid multibyte character");
        }

        std::wstring wstr = buf;
        delete [] buf;

        return wstr;
    }

    /**
    * \brief 把 Unicode 字符串转换为特定编码的字符串
    * \param wstr Unicode字符串
    * \return 输出字符串
    */
    std::string ANSIEncoding::Convert(const wchar_t *wstr)
    {
        setlocale(LC_CTYPE, "");    // 使用系统默认编码

        size_t size = wcstombs(NULL, wstr, 0) + 1;  // 取得需要的尺寸
        char *buf = new char[size];
        memset(buf, 0, size);

        size = wcstombs(buf, wstr, size);
        if ((size_t)(-1) == size)
        {
            throw EncodingErrorException("some wide character cannot be convert to multibyte character.");
        }

        std::string str = buf;
        delete [] buf;

        return str;
    }
}