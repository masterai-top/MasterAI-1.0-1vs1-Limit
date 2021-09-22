/**
* \file file.h
* \brief 文件封装类
*
* 提供文件的操作功能。
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __FILE_H__
#define __FILE_H__

#include "utildef.h"
#include "typedef.h"
#include <string>

namespace dtutil
{
    /**
    * \defgroup file_group 文件
    * file 封装了文件的打开、读取、保存等文件操作函数，
    * 以及文件路径操作函数。
    */	
	
    /**
    * \brief 文件封装类
    * \ingroup file_group
    */
    class UTIL_EXPORT File
    {
    public:
        /**
        * \brief 构造函数
        */
        File(void);

        /**
        * \brief 析构函数
        */
        ~File(void);

    public:
        /**
        * \brief 打开文件
        * 读取文件内容只能通过 std::string GetLine() 函数读取最多 500 字节的一行；
        * 可通过 void Print(const char* format, ...) 函数和 void Write(LPCSTR data, size_t len) 写文件
        * \param filename 文件名
        * \param mod 打开模式
        * \return 成功返回true，否则返回false
        */
        bool Open(const char *filename, const char *mod);

        /**
        * \brief 关闭文件
        */
        void Close();

        /**
        * \brief 文件是否打开
        * \return 打开返回true，否则返回false
        */
        bool IsOpen();

        /**
        * \brief 格式化写入文件
        * 由于fprintf写入时，对于整数来说，一位占一个字节，
        * 比如1，占1个字节；10，占2个字节；100，占3个字节，10000，占5个字节
        * 所以文件的大小会随数据的大小而改变，对大数据空间占用很大。
        * \param format 格式
        * \param ... 格式化参数
        */
        void Print(const char *format, ...);

        /**
        * \brief 写入文件
        * \param data 文件数据
        * \param len 数据长度
        */
        void Write(LPCSTR data, size_t len);

        /**
        * \brief 刷新输出缓存
        */
        void Flush();

        /**
        * \brief 获取一行数据
        * 注意：通过该函数获取一行数据，不能用二进制读文件，
        * 否则会产生乱码
        * \return 返回一行数据
        */
        std::string GetLine();

        /**
        * \brief 获取文件大小
        * \return 返回文件大小
        */
        size_t GetFileSize();

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
        FILE                *m_hFile;       ///< 文件句柄
        size_t              m_nFileSize;    ///< 文件大小
    };
}

#endif // __FILE_H__