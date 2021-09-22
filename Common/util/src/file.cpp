/**
* \file file.cpp
* \brief 文件封装类函数的实现
*/

#include "pch.h"
#include "file.h"
#include "strutil.h"


namespace dtutil
{
    /**
    * \brief 构造函数
    */
    File::File(void) : m_hFile(NULL), m_nFileSize(0)
    {
    }

    /**
    * \brief 析构函数
    */
    File::~File(void)
    {
        Close();
    }

    /**
    * \brief 打开文件
    * 读取文件内容只能通过 std::string GetLine() 函数读取最多 500 字节的一行；
    * 可通过 void Print(LPCSTR data, size_t len) 函数和 void Write(LPCSTR data, size_t len) 写文件
    * \param filename 文件名
    * \param mod 打开模式
    * \return 成功返回true，否则返回false
    */
    bool File::Open(const char *filename, const char *mod)
    {
        if (NULL != m_hFile)
        {
            fclose(m_hFile);
            m_hFile = NULL;
        }

        m_hFile = fopen(filename, mod);
        if (NULL == m_hFile)
        {
            return false;
        }

        fseek(m_hFile, 0, SEEK_END);
        m_nFileSize = ftell(m_hFile);
        fseek(m_hFile, 0, SEEK_SET);

        return true;
    }

    /**
    * \brief 关闭文件
    */
    void File::Close()
    {
        if (NULL != m_hFile)
        {
            fclose(m_hFile);
            m_hFile = NULL;
        }

        m_nFileSize = 0;
    }

    /**
    * \brief 文件是否打开
    * \return 打开返回true，否则返回false
    */
    bool File::IsOpen()
    {
        return m_hFile != NULL;
    }

    /**
    * \brief 格式化写入文件
    * 由于fprintf写入时，对于整数来说，一位占一个字节，
    * 比如1，占1个字节；10，占2个字节；100，占3个字节，10000，占5个字节
    * 所以文件的大小会随数据的大小而改变，对大数据空间占用很大。
    * \param format 格式
    * \param ... 格式化参数
    */
    void File::Print(const char *format, ...)
    {
        if (NULL == m_hFile)
        {
            return;
        }

        va_list ap;
        va_start(ap, format);

        std::string str;
        if (StrUtil::strformat(str, format, ap) < 0)
        {
            va_end(ap);
            return;
        }

        if ( fprintf(m_hFile, "%s", str.c_str()) != (int32)(str.length()) )
        {
            Close();
        }

        va_end(ap);
    }

    /**
    * \brief 写入文件
    * \param data 文件数据
    * \param len 数据长度
    */
    void File::Write(LPCSTR data, size_t len)
    {
        if (m_hFile && data != NULL && len > 0)
        {
            if (fwrite(data, len, 1, m_hFile) != 1)
            {
                Close();
            }
        }
    }

    /**
    * \brief 刷新输出缓存
    */
    void File::Flush()
    {
        if (m_hFile)
        {
            fflush(m_hFile);
        }
    }

    /**
    * \brief 获取一行数据
    * 注意：通过该函数获取一行数据，不能用二进制读文件，
    * 否则会产生乱码
    * \return 返回一行数据
    */
    std::string File::GetLine()
    {
        char buf[500] = { 0 };
        if (m_hFile && fgets(buf, 500, m_hFile) != NULL)
        {
            return std::string(buf);
        }

        return std::string();
    }

    /**
    * \brief 获取文件大小
    * \return 返回文件大小
    */
    size_t File::GetFileSize()
    {
        return m_nFileSize;
    }

    /**
    * \brief 获得错误信息
    * \return 错误信息
    */
    const char * File::GetError()
    {
#ifdef WIN32
        return strerror(GetLastError());
#else
        return strerror(errno);
#endif
    }

    /**
    * \brief 获得错误码
    * \return 错误码
    */
    uint32 File::GetErrno()
    {
#ifdef WIN32
        return GetLastError();
#else
        return errno;
#endif
    }
}