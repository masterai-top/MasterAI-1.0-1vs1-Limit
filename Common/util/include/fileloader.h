/**
* \file fileloader.h
* \brief 文件加载类
*
* 提供文件的操作功能。
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __FILE_LOADER_H__
#define __FILE_LOADER_H__

#include "utildef.h"
#include "typedef.h"
#include "exception.h"

namespace dtutil
{
    /**
    * \brief 文件编码类型
    * \ingroup file_group
    */
    enum FILE_ENCODING
    {
        FILE_ENCODING_UNKNOW    =   0,          ///< 未知格式编码
        FILE_ENCODING_UTF8,                     ///< UTF8格式编码
        FILE_ENCODING_UTF8_BOM,                 ///< UTF8 BOM格式编码
        FILE_ENCODING_GBK,                      ///< GBK格式编码
        FILE_ENCODING_UNICODE_BE,               ///< UNICODE BIG ENDIAN 格式编码
        FILE_ENCODING_UNICODE_LE,               ///< UNICODE LITTLE ENDIAN 格式编码

        FILE_ENCODING_MAX,
    };

    /**
    * \class FileEncodingErrorException
    * \extends Exception
    * \brief 文件编码错误异常
    * \ingroup file_group
    *
    * 使用 EXCEPTION_DEF(FileEncodingErrorException) 定义的异常
    * \sa Exception
    */
    EXCEPTION_DEF(FileEncodingErrorException);  ///< 文件编码错误异常

    /**
    * \brief 文件加载类
    * \ingroup file_group
    */
    class UTIL_EXPORT FileLoader
    {
    public:
        /**
        * \brief 构造函数
        */
        FileLoader(void);

        /**
        * \brief 析构函数
        */
        ~FileLoader(void);

    public:
        /**
        * \brief 加载文件，缓存文件内容
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool Load(LPCSTR filename);

        /**
        * \brief 加载数据，缓存文件内容
        * \param data 文件数据
        * \param len 数据长度
        * \return 成功返回true，否则返回false
        */
        bool Load(LPCSTR data, size_t len);

        /**
        * \brief 文件保存
        * \param filename 文件名
        * \param encoding 文件编码格式
        * \return 成功返回true，否则返回false
        */
        bool Save(LPCSTR filename, FILE_ENCODING encoding = FILE_ENCODING_UTF8);

        /**
        * \brief 改变文件编码格式
        * \param encoding 目标编码格式
        * \return 成功返回true，否则返回false
        */
        bool ChangeEncoding(FILE_ENCODING encoding);

        /**
        * \brief 获取文件编码
        * \return 文件编码
        */
        FILE_ENCODING GetEncoding();

        /**
        * \brief 是否读到文件尾
        * \return 是返回true，否则返回false
        */
        bool IsEOF();

        /**
        * \brief 获取一行数据
        * \param line 返回数据
        * \param encoding 编码格式
        * \return =0空行 >0成功 <0失败
        */
        int32 GetLine(std::string &line, FILE_ENCODING encoding = FILE_ENCODING_UTF8);

        /**
        * \brief 获取一行数据
        * \param wline 返回数据
        * \param encoding 编码格式
        * \return =0空行 >0成功 <0失败
        */
        int32 GetLineW(std::wstring &wline, FILE_ENCODING encoding = FILE_ENCODING_UNICODE_LE);

        /**
        * \brief 获得错误信息
        * \return 错误信息
        */
        const char * GetError();

        /**
        * \brief 获得错误码
        * \return 错误码
        */
        uint32 GetErrno();

    private:
        /**
        * \brief 检查编码格式
        * \return 返回文件编码格式
        */
        FILE_ENCODING CheckEncoding();

        /**
        * \brief 字符大小端转换
        * \param wstr 字符串
        * \return 成功返回true，否则返回false
        */
        bool CharEndianTranscode(std::wstring &wstr);

        /**
        * \brief 改变文件编码格式 UTF8->UTF8_BOM
        * \return 成功返回true，否则返回false
        */
        bool Utf8ChangeUtf8bom();

        /**
        * \brief 改变文件编码格式 UTF8->GBK
        * \return 成功返回true，否则返回false
        */
        bool Utf8ChangeGbk();

        /**
        * \brief 改变文件编码格式 UTF8->UNICODE_BE
        * \return 成功返回true，否则返回false
        */
        bool Utf8ChangeUnicodeBE();

        /**
        * \brief 改变文件编码格式 UTF8->UNICODE_LE
        * \return 成功返回true，否则返回false
        */
        bool Utf8ChangeUnicodeLE();

        /**
        * \brief 改变文件编码格式 UTF8_BOM->UTF8
        * \return 成功返回true，否则返回false
        */
        bool Utf8bomChangeUtf8();

        /**
        * \brief 改变文件编码格式 UTF8_BOM->GBK
        * \return 成功返回true，否则返回false
        */
        bool Utf8bomChangeGbk();

        /**
        * \brief 改变文件编码格式 UTF8_BOM->UNICODE_BE
        * \return 成功返回true，否则返回false
        */
        bool Utf8bomChangeUnicodeBE();

        /**
        * \brief 改变文件编码格式 UTF8_BOM->UNICODE_LE
        * \return 成功返回true，否则返回false
        */
        bool Utf8bomChangeUnicodeLE();

        /**
        * \brief 改变文件编码格式 GBK->UTF8
        * \return 成功返回true，否则返回false
        */
        bool GbkChangeUtf8();

        /**
        * \brief 改变文件编码格式 GBK->UTF8_BOM
        * \return 成功返回true，否则返回false
        */
        bool GbkChangeUtf8bom();

        /**
        * \brief 改变文件编码格式 GBK->UNICODE_BE
        * \return 成功返回true，否则返回false
        */
        bool GbkChangeUnicodeBE();

        /**
        * \brief 改变文件编码格式 GBK->UNICODE_LE
        * \return 成功返回true，否则返回false
        */
        bool GbkChangeUnicodeLE();

        /**
        * \brief 改变文件编码格式 UNICODE_BE->UTF8
        * \return 成功返回true，否则返回false
        */
        bool UnicodeBEChangeUtf8();

        /**
        * \brief 改变文件编码格式 UNICODE_BE->UTF8_BOM
        * \return 成功返回true，否则返回false
        */
        bool UnicodeBEChangeUtf8bom();

        /**
        * \brief 改变文件编码格式 UNICODE_BE->GBK
        * \return 成功返回true，否则返回false
        */
        bool UnicodeBEChangeGbk();

        /**
        * \brief 改变文件编码格式 UNICODE_BE->UNICODE_LE
        * \return 成功返回true，否则返回false
        */
        bool UnicodeBEChangeUnicodeLE();

        /**
        * \brief 改变文件编码格式 UNICODE_LE->UTF8
        * \return 成功返回true，否则返回false
        */
        bool UnicodeLEChangeUtf8();

        /**
        * \brief 改变文件编码格式 UNICODE_LE->UTF8_BOM
        * \return 成功返回true，否则返回false
        */
        bool UnicodeLEChangeUtf8bom();

        /**
        * \brief 改变文件编码格式 UNICODE_LE->GBK
        * \return 成功返回true，否则返回false
        */
        bool UnicodeLEChangeGbk();

        /**
        * \brief 改变文件编码格式 UNICODE_LE->UNICODE_LE
        * \return 成功返回true，否则返回false
        */
        bool UnicodeLEChangeUnicodeBE();

        /**
        * \brief 同编码文件存储
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool SameEncodingSave(LPCSTR filename);

        /**
        * \brief 文件保存 UTF8->UTF8_BOM
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool Utf8SaveUtf8bom(LPCSTR filename);

        /**
        * \brief 文件保存 UTF8->GBK
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool Utf8SaveGbk(LPCSTR filename);

        /**
        * \brief 文件保存 UTF8->UNICODE_BE
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool Utf8SaveUnicodeBE(LPCSTR filename);

        /**
        * \brief 文件保存 UTF8->UNICODE_LE
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool Utf8SaveUnicodeLE(LPCSTR filename);

        /**
        * \brief 文件保存 UTF8_BOM->UTF8
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool Utf8bomSaveUtf8(LPCSTR filename);

        /**
        * \brief 文件保存 UTF8_BOM->GBK
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool Utf8bomSaveGbk(LPCSTR filename);

        /**
        * \brief 文件保存 UTF8_BOM->UNICODE_BE
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool Utf8bomSaveUnicodeBE(LPCSTR filename);

        /**
        * \brief 文件保存 UTF8_BOM->UNICODE_LE
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool Utf8bomSaveUnicodeLE(LPCSTR filename);

        /**
        * \brief 文件保存 GBK->UTF8
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool GbkSaveUtf8(LPCSTR filename);

        /**
        * \brief 文件保存 GBK->UTF8_BOM
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool GbkSaveUtf8bom(LPCSTR filename);

        /**
        * \brief 文件保存 GBK->UNICODE_BE
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool GbkSaveUnicodeBE(LPCSTR filename);

        /**
        * \brief 文件保存 GBK->UNICODE_LE
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool GbkSaveUnicodeLE(LPCSTR filename);

        /**
        * \brief 文件保存 UNICODE_BE->UTF8
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool UnicodeBESaveUtf8(LPCSTR filename);

        /**
        * \brief 文件保存 UNICODE_BE->UTF8_BOM
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool UnicodeBESaveUtf8bom(LPCSTR filename);

        /**
        * \brief 文件保存 UNICODE_BE->GBK
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool UnicodeBESaveGbk(LPCSTR filename);

        /**
        * \brief 文件保存 UNICODE_BE->UNICODE_LE
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool UnicodeBESaveUnicodeLE(LPCSTR filename);

        /**
        * \brief 文件保存 UNICODE_LE->UTF8
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool UnicodeLESaveUtf8(LPCSTR filename);

        /**
        * \brief 文件保存 UNICODE_LE->UTF8_BOM
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool UnicodeLESaveUtf8bom(LPCSTR filename);

        /**
        * \brief 文件保存 UNICODE_LE->GBK
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool UnicodeLESaveGbk(LPCSTR filename);

        /**
        * \brief 文件保存 UNICODE_LE->UNICODE_LE
        * \param filename 文件名
        * \return 成功返回true，否则返回false
        */
        bool UnicodeLESaveUnicodeBE(LPCSTR filename);

        /**
        * \brief 读取一行数据
        * \param line 数据
        * \return =0空行 >0成功 <0失败
        */
        int32 ReadLine(std::string &line);

        /**
        * \brief 读取一行数据
        * \param wline 数据
        * \return =0空行 >0成功 <0失败
        */
        int32 ReadLineW(std::wstring &wline);

        /**
        * \brief 编码格式修正
        * 根据文件编码格式，修正文件内容初始buffer位置
        */
        void EncodingOffset();

    private:
        FILE                *m_hFile;       ///< 文件句柄
        FILE_ENCODING       m_nEncoding;    ///< 文件编码
        size_t              m_nFileSize;    ///< 文件大小
        char                *m_pBuffer;     ///< 文件缓存数据，保存了文件内容
        size_t              m_nBufSize;     ///< 缓存大小
        size_t              m_nOffset;      ///< 文件指针位置，用于GetLine的操作
    };

}

#endif // __FILE_LOADER_H__