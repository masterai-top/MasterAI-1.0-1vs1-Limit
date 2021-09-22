/**
* \file encoding.h
* \brief 字符串编码转换类
*
* 以 Unicode 为基准编码，提供字符串编码转换功能，
* 实现指定编码与 Unicode 编码之间的相互转换。
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __ENCODING_H__
#define __ENCODING_H__

#include "utildef.h"
#include "exception.h"
#include <string>

// 我们说的unicode, 其实就是utf-16, 但最通用的却是utf-8, 
// 原因: 英文占的比例比较大, 这样utf-8的存储优势比较明显, 因为utf-16是固定16位的(双字节),
// 而utf-8则是看情况而定, 即可变长度, 常规的128个ASCII只需要8位(单字节), 而汉字需要24位 
#ifdef WIN32
#define LOC_UNICODE "UTF-16LE"
#elif __GNUC__
#define LOC_UNICODE "WCHAR_T"
#endif

namespace dtutil
{
    /**
    * \defgroup encoding_group 字符串编码转换
    * encoding 提供了一组负责字符串编码转换的类，
    * 以 Unicode 为基准编码，实现指定编码与 Unicode 之间的相互转换
    *
    * 编码转换是通过 libiconv 库完成的，因此编译这个库是需要依赖 libiconv。
    *
    * 目前包括 Encoding(虚基类)， ANSIEncoding，UTF8Encoding，GBKEncoding，
    * 转换类实现为单实例模式，可以通过引用常量 ANSI，UTF8，GBK 访问
    * ANSIEncoding，UTF8Encoding 和 GBKEncoding。
    */

    /**
    * \class EncodingErrorException
    * \extends Exception
    * \brief 编码错误异常
    * \ingroup encoding_group
    *
    * 使用 EXCEPTION_DEF(EncodingErrorException) 定义的异常
    * \sa Exception
    */
    EXCEPTION_DEF(EncodingErrorException);      //!< 编码错误异常

    /**
    * \brief 字符串编码转换类
    * \ingroup encoding_group
    * 提供 Unicode 与指定编码之间的转换功能
    */
    class UTIL_EXPORT Encoding
    {
    public:
        /**
        * \brief ANSIEncoding 的单实例引用
        * \sa ANSIEncoding
        */
        static Encoding &ANSI;
        
        /**
        * \brief UTF8Encoding 的单实例引用
        * \sa UTF8Encoding
        */
        static Encoding &UTF8;

        /**
        * \brief GBKEncoding 的单实例引用
        * \sa GBKEncoding
        */
        static Encoding &GBK;

    public:
        /**
        * \brief 把 Unicode 字符串转换为指定编码的字符串
        * \param wstr Unicode字符串
        * \return 输出字符串
        */
        virtual std::string Transcode(const std::wstring &wstr) = 0;

        /**
        * \brief 把 Unicode 字符串转换为指定编码的字符串
        * \param wstr Unicode字符串
        * \return 输出字符串
        */
        virtual std::string Transcode(const wchar_t *wstr) = 0;

        /**
        * \brief 把 Unicode 字符串转换为指定编码的字符串
        * \param wstr Unicode字符串
        * \return 输出字符串
        */
        virtual std::string Transcode(wchar_t *wstr) = 0;

        /**
        * \brief 把指定编码的字符串转换为 Unicode 字符串
        * \param str 输入字符串
        * \return Unicode字符串
        */
        virtual std::wstring Transcode(const std::string &str) = 0;

        /**
        * \brief 把指定编码的字符串转换为 Unicode 字符串
        * \param str 输入字符串
        * \return Unicode字符串
        */
        virtual std::wstring Transcode(const char *str) = 0;

        /**
        * \brief 把指定编码的字符串转换为 Unicode 字符串
        * \param str 输入字符串
        * \return Unicode字符串
        */
        virtual std::wstring Transcode(char *str) = 0;

    protected:
        /**
        * \brief 把指定编码的字符串转换为 Unicode 字符串
        * \param str 输入字符串
        * \return Unicode字符串
        */
        virtual std::wstring Convert(const char *str) = 0;

        /**
        * \brief 把 Unicode 字符串转换为特定编码的字符串
        * \param wstr Unicode字符串
        * \return 输出字符串
        */
        virtual std::string Convert(const wchar_t *wstr) = 0;
    };
}

/**
* \brief GBK编码转换UTF8编码
* \ingroup encoding_group
*/
#define GBK_TO_UTF8(str) dtutil::Encoding::UTF8.Transcode(dtutil::Encoding::GBK.Transcode(str))
/**
* \brief UTF8编码转换GBK编码
* \ingroup encoding_group
*/
#define UTF8_TO_GBK(str) dtutil::Encoding::GBK.Transcode(dtutil::Encoding::UTF8.Transcode(str))
/**
* \brief ANSI编码转换UTF8编码
* \ingroup encoding_group
*/
#define ANSI_TO_UTF8(str) dtutil::Encoding::UTF8.Transcode(dtutil::Encoding::ANSI.Transcode(str))
/**
* \brief UTF8编码转换ASNI编码
* \ingroup encoding_group
*/
#define UTF8_TO_ANSI(str) dtutil::Encoding::ANSI.Transcode(dtutil::Encoding::UTF8.Transcode(str))
/**
* \brief ANSI编码转换GBK编码
* \ingroup encoding_group
*/
#define ANSI_TO_GBK(str) dtutil::Encoding::GBK.Transcode(dtutil::Encoding::ANSI.Transcode(str))
/**
* \brief GBK编码转换ANSI编码
* \ingroup encoding_group
*/
#define GBK_TO_ANSI(str) dtutil::Encoding::ANSI.Transcode(dtutil::Encoding::GBK.Transcode(str))

#endif // __ENCODING_H__