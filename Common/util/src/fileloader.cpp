/**
* \file fileloader.cpp
* \brief 文件加载类函数的实现
*/

#include "pch.h"
#include "fileloader.h"
#include "encoding.h"


namespace dtutil
{
    /**
    * \brief 构造函数
    */
    FileLoader::FileLoader(void) : m_hFile(NULL), m_nEncoding(FILE_ENCODING_UNKNOW), m_nFileSize(0), m_pBuffer(NULL), m_nBufSize(0), m_nOffset(0)
    {
    }

    /**
    * \brief 析构函数
    */
    FileLoader::~FileLoader(void)
    {
        if (NULL != m_pBuffer)
        {
            delete [] m_pBuffer;
            m_pBuffer = NULL;
        }
        m_nBufSize = 0;

        if (m_hFile)
        {
            fclose(m_hFile);
            m_hFile = NULL;
        }
    }

    /**
    * \brief 加载文件，缓存文件内容
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Load(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        if (m_hFile != NULL)
        {
            fclose(m_hFile);
            m_hFile = NULL;
        }

        m_hFile = fopen(filename, "rb");
        if (m_hFile == NULL)
        {
            return false;
        }

        // 获取文件长度
        fseek(m_hFile, 0, SEEK_END);
        m_nFileSize = ftell(m_hFile);
        fseek(m_hFile, 0, SEEK_SET);

        m_nOffset = 0;
        size_t nFileSize = m_nFileSize + 1 * sizeof(wchar_t);   // 为了后面加入'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);                   // 8字节对齐

        // 缓存保存文件内容
        m_pBuffer = new char[m_nBufSize];
        if (NULL == m_pBuffer)
        {
            return false;
        }
        memset(m_pBuffer, 0, m_nBufSize);

        // 读入缓存
        if (1 != fread(m_pBuffer, m_nFileSize, 1, m_hFile))
        {
            fclose(m_hFile);
            m_hFile = NULL;
            delete [] m_pBuffer;
            m_pBuffer = NULL;
            return false;
        }

        // 关闭文件
        fclose(m_hFile);
        m_hFile = NULL;

        // 检查文件编码格式
        m_nEncoding = CheckEncoding();

        // 没有结束标志，加一个
        if (m_pBuffer[m_nFileSize - 1] != '\0')
        {
            if(m_nEncoding == FILE_ENCODING_UNICODE_BE || m_nEncoding == FILE_ENCODING_UNICODE_LE)
            {
                wchar_t flag = L'\0';
                memcpy(m_pBuffer+m_nFileSize, (const void *)&flag, sizeof(flag));
            }
            else
            {
                char flag = '\0';
                memcpy(m_pBuffer+m_nFileSize, (const void *)&flag, sizeof(flag));
            }
        }

        // UNICODE BIG ENDIAN 转 UNICODE LITTLE ENDIAN
        // 转了后取行才能判断行结束标志L'\r''\n'
        if (m_nEncoding == FILE_ENCODING_UNICODE_BE)
        {
            ChangeEncoding(FILE_ENCODING_UNICODE_LE);
        }

        return true;
    }

    /**
    * \brief 加载数据，缓存文件内容
    * \param data 文件数据
    * \param len 数据长度
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Load(LPCSTR data, size_t len)
    {
        if (NULL == data || 0 == len)
        {
            return false;
        }

        m_nOffset = 0;
        m_nFileSize = len;
        size_t nFileSize = m_nFileSize + 1 * sizeof(wchar_t);   // 为了后面加入'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);                   // 8字节对齐

        // 缓存保存文件内容
        m_pBuffer = new char[m_nBufSize];
        if (NULL == m_pBuffer)
        {
            return false;
        }
        memset(m_pBuffer, 0, m_nBufSize);

        memcpy(m_pBuffer, data, len);

        // 检查文件编码格式
        m_nEncoding = CheckEncoding();

        // 没有结束标志，加一个
        if (m_pBuffer[m_nFileSize - 1] != '\0')
        {
            if(m_nEncoding == FILE_ENCODING_UNICODE_BE || m_nEncoding == FILE_ENCODING_UNICODE_LE)
            {
                wchar_t flag = L'\0';
                memcpy(m_pBuffer+m_nFileSize, (const void *)&flag, sizeof(flag));
            }
            else
            {
                char flag = '\0';
                memcpy(m_pBuffer+m_nFileSize, (const void *)&flag, sizeof(flag));
            }
        }

        // UNICODE BIG ENDIAN 转 UNICODE LITTLE ENDIAN
        // 转了后取行才能判断行结束标志L'\r''\n'
        if (m_nEncoding == FILE_ENCODING_UNICODE_BE)
        {
            ChangeEncoding(FILE_ENCODING_UNICODE_LE);
        }

        return true;
    }

    typedef bool (FileLoader::*fnSave)(LPCSTR filename);
    /**
    * \brief 文件保存
    * \param filename 文件名
    * \param encoding 文件编码格式
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Save(LPCSTR filename, FILE_ENCODING encoding)
    {
        static fnSave fnFileSave[FILE_ENCODING_MAX][FILE_ENCODING_MAX] =
        {
            {   &FileLoader::SameEncodingSave,                  //  UNKNOW          ->      UNKNOW
                &FileLoader::SameEncodingSave,                  //  UNKNOW          ->      UTF8
                &FileLoader::SameEncodingSave,                  //  UNKNOW          ->      UTF8_BOM
                &FileLoader::SameEncodingSave,                  //  UNKNOW          ->      GBK
                &FileLoader::SameEncodingSave,                  //  UNKNOW          ->      UNICODE_BE
                &FileLoader::SameEncodingSave },                //  UNKNOW          ->      UNICODE_LE
            {   &FileLoader::SameEncodingSave,                  //  UTF8            ->      UNKNOW
                &FileLoader::SameEncodingSave,                  //  UTF8            ->      UTF8
                &FileLoader::Utf8SaveUtf8bom,                   //  UTF8            ->      UTF8_BOM
                &FileLoader::Utf8SaveGbk,                       //  UTF8            ->      GBK
                &FileLoader::Utf8SaveUnicodeBE,                 //  UTF8            ->      UNICODE_BE
                &FileLoader::Utf8SaveUnicodeLE },               //  UTF8            ->      UNICODE_LE
            {   &FileLoader::SameEncodingSave,                  //  UTF8_BOM        ->      UNKNOW
                &FileLoader::Utf8bomSaveUtf8,                   //  UTF8_BOM        ->      UTF8
                &FileLoader::SameEncodingSave,                  //  UTF8_BOM        ->      UTF8_BOM
                &FileLoader::Utf8bomSaveGbk,                    //  UTF8_BOM        ->      GBK
                &FileLoader::Utf8bomSaveUnicodeBE,              //  UTF8_BOM        ->      UNICODE_BE
                &FileLoader::Utf8bomSaveUnicodeLE },            //  UTF8_BOM        ->      UNICODE_LE
            {   &FileLoader::SameEncodingSave,                  //  GBK             ->      UNKNOW
                &FileLoader::GbkSaveUtf8,                       //  GBK             ->      UTF8
                &FileLoader::GbkSaveUtf8bom,                    //  GBK             ->      UTF8_BOM
                &FileLoader::SameEncodingSave,                  //  GBK             ->      GBK
                &FileLoader::GbkSaveUnicodeBE,                  //  GBK             ->      UNICODE_BE
                &FileLoader::GbkSaveUnicodeLE },                //  GBK             ->      UNICODE_LE
            {   &FileLoader::SameEncodingSave,                  //  UNICODE_BE      ->      UNKNOW
                &FileLoader::UnicodeBESaveUtf8,                 //  UNICODE_BE      ->      UTF8
                &FileLoader::UnicodeBESaveUtf8bom,              //  UNICODE_BE      ->      UTF8_BOM
                &FileLoader::UnicodeBESaveGbk,                  //  UNICODE_BE      ->      GBK
                &FileLoader::SameEncodingSave,                  //  UNICODE_BE      ->      UNICODE_BE
                &FileLoader::UnicodeBESaveUnicodeLE },          //  UNICODE_BE      ->      UNICODE_LE
            {   &FileLoader::SameEncodingSave,                  //  UNICODE_LE      ->      UNKNOW
                &FileLoader::UnicodeLESaveUtf8,                 //  UNICODE_LE      ->      UTF8
                &FileLoader::UnicodeLESaveUtf8bom,              //  UNICODE_LE      ->      UTF8_BOM
                &FileLoader::UnicodeLESaveGbk,                  //  UNICODE_LE      ->      GBK
                &FileLoader::UnicodeLESaveUnicodeBE,            //  UNICODE_LE      ->      UNICODE_BE
                &FileLoader::SameEncodingSave },                //  UNICODE_LE      ->      UNICODE_LE
        };

        // 同编码存储文件
        if (fnFileSave[m_nEncoding][encoding] == NULL)
        {
            return SameEncodingSave(filename);
        }

        return (this->*fnFileSave[m_nEncoding][encoding])(filename);
    }

    typedef bool (FileLoader::*fnChangeEncoding)();
    /**
    * \brief 改变文件编码格式
    * \param encoding 目标编码格式
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::ChangeEncoding(FILE_ENCODING encoding)
    {
        static fnChangeEncoding fnEncodingChange[FILE_ENCODING_MAX][FILE_ENCODING_MAX] =
        {
            {   NULL,                                           //  UNKNOW          ->      UNKNOW
                NULL,                                           //  UNKNOW          ->      UTF8
                NULL,                                           //  UNKNOW          ->      UTF8_BOM
                NULL,                                           //  UNKNOW          ->      GBK
                NULL,                                           //  UNKNOW          ->      UNICODE_BE
                NULL },                                         //  UNKNOW          ->      UNICODE_LE
            {   NULL,                                           //  UTF8            ->      UNKNOW
                NULL,                                           //  UTF8            ->      UTF8
                &FileLoader::Utf8ChangeUtf8bom,                 //  UTF8            ->      UTF8_BOM
                &FileLoader::Utf8ChangeGbk,                     //  UTF8            ->      GBK
                &FileLoader::Utf8ChangeUnicodeBE,               //  UTF8            ->      UNICODE_BE
                &FileLoader::Utf8ChangeUnicodeLE },             //  UTF8            ->      UNICODE_LE
            {   NULL,                                           //  UTF8_BOM        ->      UNKNOW
                &FileLoader::Utf8bomChangeUtf8,                 //  UTF8_BOM        ->      UTF8
                NULL,                                           //  UTF8_BOM        ->      UTF8_BOM
                &FileLoader::Utf8bomChangeGbk,                  //  UTF8_BOM        ->      GBK
                &FileLoader::Utf8bomChangeUnicodeBE,            //  UTF8_BOM        ->      UNICODE_BE
                &FileLoader::Utf8bomChangeUnicodeLE },          //  UTF8_BOM        ->      UNICODE_LE
            {   NULL,                                           //  GBK             ->      UNKNOW
                &FileLoader::GbkChangeUtf8,                     //  GBK             ->      UTF8
                &FileLoader::GbkChangeUtf8bom,                  //  GBK             ->      UTF8_BOM
                NULL,                                           //  GBK             ->      GBK
                &FileLoader::GbkChangeUnicodeBE,                //  GBK             ->      UNICODE_BE
                &FileLoader::GbkChangeUnicodeLE },              //  GBK             ->      UNICODE_LE
            {   NULL,                                           //  UNICODE_BE      ->      UNKNOW
                &FileLoader::UnicodeBEChangeUtf8,               //  UNICODE_BE      ->      UTF8
                &FileLoader::UnicodeBEChangeUtf8bom,            //  UNICODE_BE      ->      UTF8_BOM
                &FileLoader::UnicodeBEChangeGbk,                //  UNICODE_BE      ->      GBK
                NULL,                                           //  UNICODE_BE      ->      UNICODE_BE
                &FileLoader::UnicodeBEChangeUnicodeLE },        //  UNICODE_BE      ->      UNICODE_LE
            {   NULL,                                           //  UNICODE_LE      ->      UNKNOW
                &FileLoader::UnicodeLEChangeUtf8,               //  UNICODE_LE      ->      UTF8
                &FileLoader::UnicodeLEChangeUtf8bom,            //  UNICODE_LE      ->      UTF8_BOM
                &FileLoader::UnicodeLEChangeGbk,                //  UNICODE_LE      ->      GBK
                &FileLoader::UnicodeLEChangeUnicodeBE,          //  UNICODE_LE      ->      UNICODE_BE
                NULL },                                         //  UNICODE_LE      ->      UNICODE_LE
        };

        if (m_nEncoding == encoding)
        {
            return true;
        }

        if (fnEncodingChange[m_nEncoding][encoding] == NULL)
        {
            return false;
        }

        return (this->*fnEncodingChange[m_nEncoding][encoding])();
    }

    /**
    * \brief 获取文件编码
    * \return 文件编码
    */
    FILE_ENCODING FileLoader::GetEncoding()
    {
        return m_nEncoding;
    }

    /**
    * \brief 是否读到文件尾
    * \return 是返回true，否则返回false
    */
    bool FileLoader::IsEOF()
    {
        return m_nOffset >= m_nFileSize;
    }

    /**
    * \brief 获取一行数据
    * \param line 返回数据
    * \param encoding 编码格式
    * \return =0空行 >0成功 <0失败
    */
    int32 FileLoader::GetLine(std::string &line, FILE_ENCODING encoding /*= FILE_ENCODING_UTF8*/)
    {
        // 该函数只支持获取非Unicode编码一行数据，若想获取Unicode编码数据，请用GetLineW接口
        if (encoding == FILE_ENCODING_UNICODE_BE || encoding == FILE_ENCODING_UNICODE_LE)
        {
            throw FileEncodingErrorException("getline in unicode error! if you want getline in unicode, please use function GetLineW()!");
        }

        line.clear();
        // 文件结束
        if (IsEOF())
        {
            return -1;
        }

        while(1)
        {
            // 当前文件编码是Unicode
            if (m_nEncoding == FILE_ENCODING_UNICODE_BE || m_nEncoding == FILE_ENCODING_UNICODE_LE)
            {
                std::wstring wstr;
                int32 result = ReadLineW(wstr);
                if (result <= 0)
                {
                    return result;
                }

                if (m_nEncoding == FILE_ENCODING_UNICODE_BE && !CharEndianTranscode(wstr))
                {
                    throw FileEncodingErrorException("unicode big endian transcode to unicode little endian failed!");
                }

                // 注释行
                if (wstr[0] == L'#')
                {
                    wstr.clear();
                    continue;
                }

                // UNICODE --> UTF8
                if (encoding == FILE_ENCODING_UTF8 || encoding == FILE_ENCODING_UTF8_BOM)
                {
                    line = Encoding::UTF8.Transcode(wstr);
                }
                // UNICODE --> GBK
                else if (encoding == FILE_ENCODING_GBK)
                {
                    line = Encoding::GBK.Transcode(wstr);
                }
                // UNICODE --> xxx
                else
                {
                    throw FileEncodingErrorException("not support unicode transcode to dest encoding!");
                }
            }
            // 当前文件编码不是Unicode
            else
            {
                std::string str;
                int32 result = ReadLine(str);
                if (result <= 0)
                {
                    return result;
                }
                // 注释行
                if (str[0] == '#')
                {
                    str.clear();
                    continue;
                }

                // GBK --> UTF8
                if (m_nEncoding == FILE_ENCODING_GBK && (encoding == FILE_ENCODING_UTF8 || encoding == FILE_ENCODING_UTF8_BOM))
                {
                    line = GBK_TO_UTF8(str);
                }
                // UTF8 --> GBK
                else if ((m_nEncoding == FILE_ENCODING_UTF8 || m_nEncoding == FILE_ENCODING_UTF8_BOM) && encoding == FILE_ENCODING_GBK)
                {
                    line = UTF8_TO_GBK(str);
                }
                // GBK --> GBK or UTF8 --> UTF8 or GBK --> xxx or UTF8 --> xxx
                else
                {
                    line = str;
                }
            }

            break;
        }

        return 1;
    }

    /**
    * \brief 获取一行数据
    * \param wline 返回数据
    * \param encoding 编码格式
    * \return =0空行 >0成功 <0失败
    */
    int32 FileLoader::GetLineW(std::wstring &wline, FILE_ENCODING encoding /*= FILE_ENCODING_UNICODE_LE*/)
    {
        // 该函数只支持获取Unicode编码一行数据，若想获取非Unicode编码数据，请用GetLine接口
        if (encoding != FILE_ENCODING_UNICODE_BE && encoding != FILE_ENCODING_UNICODE_LE)
        {
            throw FileEncodingErrorException("getline not unicode error! if you want getline not unicode, please use function GetLine()!");
        }

        wline.clear();
        // 文件结束
        if (IsEOF())
        {
            return -1;
        }

        while(1)
        {
            // 当前文件编码是Unicode Little Endian
            if (m_nEncoding == FILE_ENCODING_UNICODE_LE)
            {
                std::wstring wstr;
                int32 result = ReadLineW(wstr);
                if (result <= 0)
                {
                    return result;
                }

                // 注释行
                if (wstr[0] == L'#')
                {
                    wstr.clear();
                    continue;
                }

                if (encoding == FILE_ENCODING_UNICODE_BE && !CharEndianTranscode(wstr))
                {
                    throw FileEncodingErrorException("unicode little endian transcode to unicode big endian failed!");
                }

                wline = wstr;
            }
            // 当前文件编码是Unicode Big Endian
            else if (m_nEncoding == FILE_ENCODING_UNICODE_BE)
            {
                std::wstring wstr;
                int32 result = ReadLineW(wstr);
                if (result <= 0)
                {
                    return result;
                }

                // 注释行
                if (wstr[0] == L'#')
                {
                    wstr.clear();
                    continue;
                }

                if (encoding == FILE_ENCODING_UNICODE_LE && !CharEndianTranscode(wstr))
                {
                    throw FileEncodingErrorException("unicode big endian transcode to unicode little endian failed!");
                }

                wline = wstr;
            }
            // 当前文件编码不是Unicode
            else
            {
                std::string str;
                int32 result = ReadLine(str);
                if (result <= 0)
                {
                    return result;
                }

                // 注释行
                if (str[0] == '#')
                {
                    str.clear();
                    continue;
                }

                // GBK --> UNICODE_BE
                if (m_nEncoding == FILE_ENCODING_GBK && encoding == FILE_ENCODING_UNICODE_BE)
                {
                    wline = Encoding::GBK.Transcode(str);
                    if (!CharEndianTranscode(wline))
                    {
                        throw FileEncodingErrorException("unicode little endian transcode to unicode big endian failed!");
                    }
                }
                // GBK -->UNICODE_LE
                else if (m_nEncoding == FILE_ENCODING_GBK && encoding == FILE_ENCODING_UNICODE_LE)
                {
                    wline = Encoding::GBK.Transcode(str);
                }
                // UTF8 --> UNICODE_BE
                else if ((m_nEncoding == FILE_ENCODING_UTF8 || m_nEncoding == FILE_ENCODING_UTF8_BOM) && encoding == FILE_ENCODING_UNICODE_BE)
                {
                    wline = Encoding::UTF8.Transcode(str);
                    if (!CharEndianTranscode(wline))
                    {
                        throw FileEncodingErrorException("unicode little endian transcode to unicode big endian failed!");
                    }
                }
                // UTF8 --> UNICODE_LE
                else if ((m_nEncoding == FILE_ENCODING_UTF8 || m_nEncoding == FILE_ENCODING_UTF8_BOM) && encoding == FILE_ENCODING_UNICODE_LE)
                {
                    wline = Encoding::UTF8.Transcode(str);
                }
                // xxx --> UNICODE
                else
                {
                    throw FileEncodingErrorException("not support encoding transcode to unicode encoding!");
                }
            }

            break;
        }

        return 1;
    }

    /**
    * \brief 获得错误信息
    * \return 错误信息
    */
    const char * FileLoader::GetError()
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
    uint32 FileLoader::GetErrno()
    {
#ifdef WIN32
        return GetLastError();
#else
        return errno;
#endif
    }

    /**
    * \brief 检查编码格式
    * \return 返回文件编码格式
    */
    FILE_ENCODING FileLoader::CheckEncoding()
    {
        if (NULL == m_pBuffer)
        {
            return FILE_ENCODING_UNKNOW;
        }

        int32 flag = 0;
        memcpy(&flag, m_pBuffer, 2);
        if (flag == 0xFEFF)
        {
            return FILE_ENCODING_UNICODE_LE;
        }
        else if(flag == 0xFFFE)
        {
            return FILE_ENCODING_UNICODE_BE;
        }

        flag = 0;
        memcpy(&flag, m_pBuffer, 3);
        if (flag == 0xBFBBEF)
        {
            return FILE_ENCODING_UTF8_BOM;
        }

        FILE_ENCODING ret = FILE_ENCODING_UTF8;
        const char* offset = (const char *)m_pBuffer;

        // 遍历每一个字符，检查是否UTF8编码
        while(*offset != 0 && offset < m_pBuffer + m_nBufSize)
        {
            if (*offset < 0x80)                                     // UTF8一字节      0000 - 007F     0xxxxxxx
            {
                offset++;
            }
            else if ( (*offset >= 0xC0) && (*offset < 0xE0) )
            {
                if ( (offset[1] >= 0x80) && (offset[1] < 0xC0) )    // UTF8二字节      0080 - 07FF     110xxxxx 10xxxxxx
                {
                    offset += 2;
                }
                else
                {
                    ret = FILE_ENCODING_GBK;
                    break;
                }
            }
            else if ( (*offset >= 0xE0) && (*offset < 0xF0) )
            {
                if ( (offset[1] >= 0x80) && (offset[1] < 0xC0)      // UTF8三字节      0800 - FFFF     1110xxxx 10xxxxxx 10xxxxxx
                  && (offset[2] >= 0x80) && (offset[2] < 0xC0) )
                {
                    offset += 3;
                }
                else
                {
                    ret = FILE_ENCODING_GBK;
                    break;
                }
            }
            else if ( (*offset >= 0xF0) && (*offset < 0xF8) )
            {
                if ( (offset[1] >= 0x80) && (offset[1] < 0xC0)      // UTF8四字节      10000 - 1FFFF     11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                  && (offset[2] >= 0x80) && (offset[2] < 0xC0)
                  && (offset[3] >= 0x80) && (offset[3] < 0xC0) )
                {
                    offset += 4;
                }
                else
                {
                    ret = FILE_ENCODING_GBK;
                    break;
                }
            }
            else if ( (*offset >= 0xF8) && (*offset < 0xFC) )
            {
                if ( (offset[1] >= 0x80) && (offset[1] < 0xC0)      // UTF8五字节      20000 - 3FFFF     111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
                    && (offset[2] >= 0x80) && (offset[2] < 0xC0)
                    && (offset[3] >= 0x80) && (offset[3] < 0xC0)
                    && (offset[4] >= 0x80) && (offset[4] < 0xC0) )
                {
                    offset += 5;
                }
                else
                {
                    ret = FILE_ENCODING_GBK;
                    break;
                }
            }
            else if ( (*offset >= 0xFC) && (*offset < 0xFE) )
            {
                if ( (offset[1] >= 0x80) && (offset[1] < 0xC0)      // UTF8六字节      40000 - 7FFFF     1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
                    && (offset[2] >= 0x80) && (offset[2] < 0xC0)
                    && (offset[3] >= 0x80) && (offset[3] < 0xC0)
                    && (offset[4] >= 0x80) && (offset[4] < 0xC0)
                    && (offset[5] >= 0x80) && (offset[5] < 0xC0) )
                {
                    offset += 6;
                }
                else
                {
                    ret = FILE_ENCODING_GBK;
                    break;
                }
            }
            else        // (>=80 && <C0) || >=FE
            {
                ret = FILE_ENCODING_GBK;
                break;
            }
        }

        return ret;
    }

    /**
    * \brief 字符大小端转换
    * \param wstr 字符串
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::CharEndianTranscode(std::wstring &wstr)
    {
        const wchar_t *pSrcBuf = wstr.c_str();

        size_t nCharCount = wstr.length();
        wchar_t *pDstBuf = new wchar_t[nCharCount+1];       // 加上结束符
        if (NULL == pSrcBuf || NULL == pDstBuf)
        {
            return false;
        }
        memset( pDstBuf, 0, sizeof(wchar_t)*(nCharCount+1) );

        size_t nSrcOffset = 0;
        size_t nDstOffset = 0;

        while(nSrcOffset < sizeof(wchar_t)*nCharCount)      // 需要处理的字节数
        {
            for(int32 i=sizeof(wchar_t)-1; i>0; i--)
            {
                memcpy(pDstBuf+nDstOffset, pSrcBuf+nSrcOffset+i, 1);
                nDstOffset += 1;
            }
            nSrcOffset += sizeof(wchar_t);
        }

        wstr = pDstBuf;

        delete [] pDstBuf;

        return true;
    }

    /**
    * \brief 改变文件编码格式 UTF8->UTF8_BOM
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Utf8ChangeUtf8bom()
    {
        char *pOldBuffer = m_pBuffer;
        size_t nOldFileSize = m_nFileSize;

        m_nFileSize = nOldFileSize + 3;                 // 加上编码头
        size_t nFileSize = m_nFileSize + sizeof(char);  // 加上'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);           // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        // UTF8_BOM 编码头
        unsigned char flag[3] = { 0xEF, 0xBB, 0xBF };
        memcpy(m_pBuffer, flag, sizeof(flag));                      // 文件头
        memcpy(m_pBuffer+sizeof(flag), pOldBuffer, nOldFileSize);   // 文件内容

        m_nEncoding = FILE_ENCODING_UTF8_BOM;
        m_nOffset = 0;

        delete [] pOldBuffer;
        
        return true;
    }

    /**
    * \brief 改变文件编码格式 UTF8->GBK
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Utf8ChangeGbk()
    {
        char *pOldBuffer = m_pBuffer;
        std::string text = UTF8_TO_GBK(m_pBuffer);

        m_nFileSize = text.length();
        size_t nFileSize = m_nFileSize + sizeof(char);  // 加上'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);           // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        memcpy(m_pBuffer, text.c_str(), text.length());  // 文件内容

        m_nEncoding = FILE_ENCODING_GBK;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 UTF8->UNICODE_BE
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Utf8ChangeUnicodeBE()
    {
        char *pOldBuffer = m_pBuffer;
        std::wstring text = Encoding::UTF8.Transcode(m_pBuffer);

        // Unicode Little Endian --> Unicode Big Endian
        if (!CharEndianTranscode(text))
        {
            return false;
        }

        size_t nTextSize = text.length() * sizeof(wchar_t);

        m_nFileSize = nTextSize + 2;                        // 加上编码头
        size_t nFileSize = m_nFileSize + sizeof(wchar_t);   // 加上L'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);               // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        // UnicodeBE 编码头
        unsigned char flag[2] = { 0xFE, 0xFF };
        memcpy(m_pBuffer, flag, sizeof(flag));                  // 文件头
        memcpy(m_pBuffer+sizeof(flag), (const char *)text.c_str(), nTextSize);  // 文件内容

        m_nEncoding = FILE_ENCODING_UNICODE_BE;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 UTF8->UNICODE_LE
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Utf8ChangeUnicodeLE()
    {
        char *pOldBuffer = m_pBuffer;
        std::wstring text = Encoding::UTF8.Transcode(m_pBuffer);

        size_t nTextSize = text.length() * sizeof(wchar_t);

        m_nFileSize = nTextSize + 2;                        // 加上编码头
        size_t nFileSize = m_nFileSize + sizeof(wchar_t);   // 加上L'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);               // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        // UnicodeLE 编码头
        unsigned char flag[2] = { 0xFF, 0xFE };
        memcpy(m_pBuffer, flag, sizeof(flag));                  // 文件头
        memcpy(m_pBuffer+sizeof(flag), (const char *)text.c_str(), nTextSize);  // 文件内容

        m_nEncoding = FILE_ENCODING_UNICODE_LE;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 UTF8_BOM->UTF8
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Utf8bomChangeUtf8()
    {
        char *pOldBuffer = m_pBuffer;

        m_nFileSize -= 3;                               // 去掉编码头
        size_t nFileSize = m_nFileSize + sizeof(char);  // 加上'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);           // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        memcpy(m_pBuffer, pOldBuffer+3, m_nFileSize);  // 文件内容

        m_nEncoding = FILE_ENCODING_UTF8;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 UTF8_BOM->GBK
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Utf8bomChangeGbk()
    {
        char *pOldBuffer = m_pBuffer;
        std::string text = UTF8_TO_GBK(m_pBuffer+3);

        m_nFileSize = text.length();
        size_t nFileSize = m_nFileSize + sizeof(char);  // 加上'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);           // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        memcpy(m_pBuffer, text.c_str(), text.length());  // 文件内容

        m_nEncoding = FILE_ENCODING_GBK;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 UTF8_BOM->UNICODE_BE
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Utf8bomChangeUnicodeBE()
    {
        char *pOldBuffer = m_pBuffer;
        std::wstring text = Encoding::UTF8.Transcode(m_pBuffer+3);

        // Unicode Little Endian --> Unicode Big Endian
        if (!CharEndianTranscode(text))
        {
            return false;
        }

        size_t nTextSize = text.length() * sizeof(wchar_t);

        m_nFileSize = nTextSize + 2;                        // 加上编码头
        size_t nFileSize = m_nFileSize + sizeof(wchar_t);   // 加上L'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);               // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        // UnicodeBE 编码头
        unsigned char flag[2] = { 0xFE, 0xFF };
        memcpy(m_pBuffer, flag, sizeof(flag));                  // 文件头
        memcpy(m_pBuffer+sizeof(flag), (const char *)text.c_str(), nFileSize);  // 文件内容

        m_nEncoding = FILE_ENCODING_UNICODE_BE;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 UTF8_BOM->UNICODE_LE
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Utf8bomChangeUnicodeLE()
    {
        char *pOldBuffer = m_pBuffer;
        std::wstring text = Encoding::UTF8.Transcode(m_pBuffer+3);

        size_t nTextSize = text.length() * sizeof(wchar_t);

        m_nFileSize = nTextSize + 2;                        // 加上编码头
        size_t nFileSize = m_nFileSize + sizeof(wchar_t);   // 加上L'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);               // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        // UnicodeLE 编码头
        unsigned char flag[2] = { 0xFF, 0xFE };
        memcpy(m_pBuffer, flag, sizeof(flag));                  // 文件头
        memcpy(m_pBuffer+sizeof(flag), (const char *)text.c_str(), nTextSize);  // 文件内容

        m_nEncoding = FILE_ENCODING_UNICODE_LE;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 GBK->UTF8
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::GbkChangeUtf8()
    {
        char *pOldBuffer = m_pBuffer;
        std::string text = GBK_TO_UTF8(m_pBuffer);

        m_nFileSize = text.length();
        size_t nFileSize = m_nFileSize + sizeof(char);  // 加上'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);           // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        memcpy(m_pBuffer, text.c_str(), text.length());  // 文件内容

        m_nEncoding = FILE_ENCODING_UTF8;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 GBK->UTF8_BOM
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::GbkChangeUtf8bom()
    {
        char *pOldBuffer = m_pBuffer;
        std::string text = GBK_TO_UTF8(m_pBuffer);

        m_nFileSize = text.length() + 3;                // 加上编码头
        size_t nFileSize = m_nFileSize + sizeof(char);  // 加上'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);           // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        // UTF8_BOM 编码头
        unsigned char flag[3] = { 0xEF, 0xBB, 0xBF };
        memcpy(m_pBuffer, flag, sizeof(flag));                          // 文件头
        memcpy(m_pBuffer+sizeof(flag), text.c_str(), text.length());    // 文件内容

        m_nEncoding = FILE_ENCODING_UTF8_BOM;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 GBK->UNICODE_BE
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::GbkChangeUnicodeBE()
    {
        char *pOldBuffer = m_pBuffer;
        std::wstring text = Encoding::GBK.Transcode(m_pBuffer);

        // Unicode Little Endian --> Unicode Big Endian
        if (!CharEndianTranscode(text))
        {
            return false;
        }

        size_t nTextSize = text.length() * sizeof(wchar_t);

        m_nFileSize = nTextSize + 2;                        // 加上编码头
        size_t nFileSize = m_nFileSize + sizeof(wchar_t);   // 加上L'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);               // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        // UnicodeBE 编码头
        unsigned char flag[2] = { 0xFE, 0xFF };
        memcpy(m_pBuffer, flag, sizeof(flag));                  // 文件头
        memcpy(m_pBuffer+sizeof(flag), (const char *)text.c_str(), nTextSize);  // 文件内容

        m_nEncoding = FILE_ENCODING_UNICODE_BE;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 GBK->UNICODE_LE
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::GbkChangeUnicodeLE()
    {
        char *pOldBuffer = m_pBuffer;
        std::wstring text = Encoding::GBK.Transcode(m_pBuffer);

        size_t nTextSize = text.length() * sizeof(wchar_t);

        m_nFileSize = nTextSize + 2;                        // 加上编码头
        size_t nFileSize = m_nFileSize + sizeof(wchar_t);   // 加上L'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);               // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        // UnicodeLE 编码头
        unsigned char flag[2] = { 0xFF, 0xFE };
        memcpy(m_pBuffer, flag, sizeof(flag));                  // 文件头
        memcpy(m_pBuffer+sizeof(flag), (const char *)text.c_str(), nTextSize);  // 文件内容

        m_nEncoding = FILE_ENCODING_UNICODE_LE;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 UNICODE_BE->UTF8
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::UnicodeBEChangeUtf8()
    {
        char *pOldBuffer = m_pBuffer;

        std::wstring wstr = (wchar_t *)(m_pBuffer + 2);
        // Unicode Big Endian --> Unicode Little Endian
        if (!CharEndianTranscode(wstr))
        {
            return false;
        }

        std::string text = Encoding::UTF8.Transcode(wstr);

        m_nFileSize = text.length();
        size_t nFileSize = m_nFileSize + sizeof(char);  // 加上'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);           // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        memcpy(m_pBuffer, text.c_str(), text.length());  // 文件内容

        m_nEncoding = FILE_ENCODING_UTF8;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 UNICODE_BE->UTF8_BOM
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::UnicodeBEChangeUtf8bom()
    {
        char *pOldBuffer = m_pBuffer;

        std::wstring wstr = (wchar_t *)(m_pBuffer + 2);
        // Unicode Big Endian --> Unicode Little Endian
        if (!CharEndianTranscode(wstr))
        {
            return false;
        }

        std::string text = Encoding::UTF8.Transcode(wstr);

        m_nFileSize = text.length() + 3;                // 加上编码头
        size_t nFileSize = m_nFileSize + sizeof(char);  // 加上'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);           // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        // UTF8_BOM 编码头
        unsigned char flag[3] = { 0xEF, 0xBB, 0xBF };
        memcpy(m_pBuffer, flag, sizeof(flag));                          // 文件头
        memcpy(m_pBuffer+sizeof(flag), text.c_str(), text.length());    // 文件内容

        m_nEncoding = FILE_ENCODING_UTF8_BOM;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 UNICODE_BE->GBK
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::UnicodeBEChangeGbk()
    {
        char *pOldBuffer = m_pBuffer;

        std::wstring wstr = (wchar_t *)(m_pBuffer + 2);
        // Unicode Big Endian --> Unicode Little Endian
        if (!CharEndianTranscode(wstr))
        {
            return false;
        }

        std::string text = Encoding::UTF8.Transcode(wstr);

        m_nFileSize = text.length();
        size_t nFileSize = m_nFileSize + sizeof(char);  // 加上'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);           // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        memcpy(m_pBuffer, text.c_str(), text.length());  // 文件内容

        m_nEncoding = FILE_ENCODING_GBK;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 UNICODE_BE->UNICODE_LE
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::UnicodeBEChangeUnicodeLE()
    {
        char *pOldBuffer = m_pBuffer;

        std::wstring wstr = (wchar_t *)(m_pBuffer + 2);
        // Unicode Big Endian --> Unicode Little Endian
        if (!CharEndianTranscode(wstr))
        {
            return false;
        }

        size_t nTextSize = wstr.length() * sizeof(wchar_t);

        m_nFileSize = nTextSize + 2;                        // 加上编码头
        size_t nFileSize = m_nFileSize + sizeof(wchar_t);   // 加上L'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);               // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        // UnicodeLE 编码头
        unsigned char flag[2] = { 0xFF, 0xFE };
        memcpy(m_pBuffer, flag, sizeof(flag));                  // 文件头
        memcpy(m_pBuffer+sizeof(flag), (const char *)wstr.c_str(), nTextSize);  // 文件内容

        m_nEncoding = FILE_ENCODING_UNICODE_LE;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 UNICODE_LE->UTF8
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::UnicodeLEChangeUtf8()
    {
        char *pOldBuffer = m_pBuffer;

        std::wstring wstr = (wchar_t *)(m_pBuffer + 2);

        std::string text = Encoding::UTF8.Transcode(wstr);

        m_nFileSize = text.length();
        size_t nFileSize = m_nFileSize + sizeof(char);  // 加上'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);           // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        memcpy(m_pBuffer, text.c_str(), text.length());  // 文件内容

        m_nEncoding = FILE_ENCODING_UTF8;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 UNICODE_LE->UTF8_BOM
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::UnicodeLEChangeUtf8bom()
    {
        char *pOldBuffer = m_pBuffer;

        std::wstring wstr = (wchar_t *)(m_pBuffer + 2);

        std::string text = Encoding::UTF8.Transcode(wstr);

        m_nFileSize = text.length() + 3;                // 加上编码头
        size_t nFileSize = m_nFileSize + sizeof(char);  // 加上'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);           // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        // UTF8_BOM 编码头
        unsigned char flag[3] = { 0xEF, 0xBB, 0xBF };
        memcpy(m_pBuffer, flag, sizeof(flag));                          // 文件头
        memcpy(m_pBuffer+sizeof(flag), text.c_str(), text.length());    // 文件内容

        m_nEncoding = FILE_ENCODING_UTF8_BOM;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 UNICODE_LE->GBK
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::UnicodeLEChangeGbk()
    {
        char *pOldBuffer = m_pBuffer;

        std::wstring wstr = (wchar_t *)(m_pBuffer + 2);

        std::string text = Encoding::UTF8.Transcode(wstr);

        m_nFileSize = text.length();
        size_t nFileSize = m_nFileSize + sizeof(char);  // 加上'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);           // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        memcpy(m_pBuffer, text.c_str(), text.length());  // 文件内容

        m_nEncoding = FILE_ENCODING_GBK;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 改变文件编码格式 UNICODE_LE->UNICODE_LE
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::UnicodeLEChangeUnicodeBE()
    {
        char *pOldBuffer = m_pBuffer;

        std::wstring wstr = (wchar_t *)(m_pBuffer + 2);
        // Unicode Big Endian --> Unicode Little Endian
        if (!CharEndianTranscode(wstr))
        {
            return false;
        }

        size_t nTextSize = wstr.length() * sizeof(wchar_t);

        m_nFileSize = nTextSize + 2;                        // 加上编码头
        size_t nFileSize = m_nFileSize + sizeof(wchar_t);   // 加上L'\0'
        m_nBufSize = ALIGN_SIZE_8(nFileSize);               // 8字节对齐
        m_pBuffer = new char[m_nBufSize];
        memset(m_pBuffer, 0, m_nBufSize);

        // UnicodeBE 编码头
        unsigned char flag[2] = { 0xFE, 0xFF };
        memcpy(m_pBuffer, flag, sizeof(flag));                  // 文件头
        memcpy(m_pBuffer+sizeof(flag), (const char *)wstr.c_str(), nTextSize);  // 文件内容

        m_nEncoding = FILE_ENCODING_UNICODE_BE;
        m_nOffset = 0;

        delete [] pOldBuffer;

        return true;
    }

    /**
    * \brief 同编码文件存储
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::SameEncodingSave(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        if (1 != fwrite(m_pBuffer, m_nFileSize, 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 UTF8->UTF8_BOM
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Utf8SaveUtf8bom(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        // 文件编码
        unsigned char flag[3] = { 0xEF, 0xBB, 0xBF };
        if (1 != fwrite(flag, sizeof(flag), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        // 文件内容
        if (1 != fwrite(m_pBuffer, m_nFileSize, 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 UTF8->GBK
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Utf8SaveGbk(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        std::string text = UTF8_TO_GBK(m_pBuffer);
        // 文件内容
        if (1 != fwrite(text.c_str(), text.length(), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 UTF8->UNICODE_BE
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Utf8SaveUnicodeBE(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        // 文件编码
        unsigned char flag[2] = { 0xFE, 0xFF };
        if (1 != fwrite(flag, sizeof(flag), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        std::wstring wtext = Encoding::UTF8.Transcode(m_pBuffer);
        // Unicode Little Endian --> Unicode Big Endian
        if (!CharEndianTranscode(wtext))
        {
            fclose(hFile);
            return false;
        }

        // 文件内容
        size_t nTextSize = wtext.length() * sizeof(wchar_t);
        if (1 != fwrite(wtext.c_str(), nTextSize, 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 UTF8->UNICODE_LE
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Utf8SaveUnicodeLE(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        // 文件编码
        unsigned char flag[2] = { 0xFF, 0xFE };
        if (1 != fwrite(flag, sizeof(flag), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        // 文件内容
        std::wstring wtext = Encoding::UTF8.Transcode(m_pBuffer);
        size_t nTextSize = wtext.length() * sizeof(wchar_t);
        if (1 != fwrite(wtext.c_str(), nTextSize, 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 UTF8_BOM->UTF8
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Utf8bomSaveUtf8(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        // 文件内容
        if (1 != fwrite(m_pBuffer+3, m_nFileSize-3, 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 UTF8_BOM->GBK
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Utf8bomSaveGbk(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        // 文件内容
        std::string text = UTF8_TO_GBK(m_pBuffer+3);
        if (1 != fwrite(text.c_str(), text.length(), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 UTF8_BOM->UNICODE_BE
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Utf8bomSaveUnicodeBE(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        // 文件编码
        unsigned char flag[2] = { 0xFE, 0xFF };
        if (1 != fwrite(flag, sizeof(flag), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        std::wstring wtext = Encoding::UTF8.Transcode(m_pBuffer+3);
        // Unicode Little Endian --> Unicode Big Endian
        if (!CharEndianTranscode(wtext))
        {
            fclose(hFile);
            return false;
        }

        // 文件内容
        size_t nTextSize = wtext.length() * sizeof(wchar_t);
        if (1 != fwrite(wtext.c_str(), nTextSize, 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 UTF8_BOM->UNICODE_LE
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::Utf8bomSaveUnicodeLE(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        // 文件编码
        unsigned char flag[2] = { 0xFF, 0xFE };
        if (1 != fwrite(flag, sizeof(flag), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        // 文件内容
        std::wstring wtext = Encoding::UTF8.Transcode(m_pBuffer+3);
        size_t nTextSize = wtext.length() * sizeof(wchar_t);
        if (1 != fwrite(wtext.c_str(), nTextSize, 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 GBK->UTF8
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::GbkSaveUtf8(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        std::string text = GBK_TO_UTF8(m_pBuffer);
        // 文件内容
        if (1 != fwrite(text.c_str(), text.length(), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 GBK->UTF8_BOM
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::GbkSaveUtf8bom(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        // 文件编码
        unsigned char flag[3] = { 0xEF, 0xBB, 0xBF };
        if (1 != fwrite(flag, sizeof(flag), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        // 文件内容
        std::string text = GBK_TO_UTF8(m_pBuffer);
        if (1 != fwrite(text.c_str(), text.length(), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 GBK->UNICODE_BE
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::GbkSaveUnicodeBE(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        // 文件编码
        unsigned char flag[2] = { 0xFE, 0xFF };
        if (1 != fwrite(flag, sizeof(flag), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        std::wstring wtext = Encoding::GBK.Transcode(m_pBuffer);
        // Unicode Little Endian --> Unicode Big Endian
        if (!CharEndianTranscode(wtext))
        {
            fclose(hFile);
            return false;
        }

        // 文件内容
        size_t nTextSize = wtext.length() * sizeof(wchar_t);
        if (1 != fwrite(wtext.c_str(), nTextSize, 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 GBK->UNICODE_LE
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::GbkSaveUnicodeLE(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        // 文件编码
        unsigned char flag[2] = { 0xFF, 0xFE };
        if (1 != fwrite(flag, sizeof(flag), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        // 文件内容
        std::wstring wtext = Encoding::GBK.Transcode(m_pBuffer);
        size_t nTextSize = wtext.length() * sizeof(wchar_t);
        if (1 != fwrite(wtext.c_str(), nTextSize, 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 UNICODE_BE->UTF8
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::UnicodeBESaveUtf8(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        std::wstring wtext = (wchar_t *)(m_pBuffer+2);
        if (!CharEndianTranscode(wtext))
        {
            fclose(hFile);
            return false;
        }

        // 文件内容
        std::string text = Encoding::UTF8.Transcode(wtext);
        if (1 != fwrite(text.c_str(), text.length(), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 UNICODE_BE->UTF8_BOM
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::UnicodeBESaveUtf8bom(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        std::wstring wtext = (wchar_t *)(m_pBuffer+2);
        if (!CharEndianTranscode(wtext))
        {
            fclose(hFile);
            return false;
        }

        // 文件编码
        unsigned char flag[3] = { 0xEF, 0xBB, 0xBF };
        if (1 != fwrite(flag, sizeof(flag), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        // 文件内容
        std::string text = Encoding::UTF8.Transcode(wtext);
        if (1 != fwrite(text.c_str(), text.length(), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 UNICODE_BE->GBK
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::UnicodeBESaveGbk(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        std::wstring wtext = (wchar_t *)(m_pBuffer+2);
        if (!CharEndianTranscode(wtext))
        {
            fclose(hFile);
            return false;
        }

        // 文件内容
        std::string text = Encoding::GBK.Transcode(wtext);
        if (1 != fwrite(text.c_str(), text.length(), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 UNICODE_BE->UNICODE_LE
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::UnicodeBESaveUnicodeLE(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        std::wstring wtext = (wchar_t *)(m_pBuffer+2);
        if (!CharEndianTranscode(wtext))
        {
            fclose(hFile);
            return false;
        }

        // 文件编码
        unsigned char flag[2] = { 0xFF, 0xFE };
        if (1 != fwrite(flag, sizeof(flag), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        // 文件内容
        size_t nTextSize = wtext.length() * sizeof(wchar_t);
        if (1 != fwrite(wtext.c_str(), nTextSize, 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 UNICODE_LE->UTF8
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::UnicodeLESaveUtf8(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        std::wstring wtext = (wchar_t *)(m_pBuffer+2);

        // 文件内容
        std::string text = Encoding::UTF8.Transcode(wtext);
        if (1 != fwrite(text.c_str(), text.length(), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 UNICODE_LE->UTF8_BOM
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::UnicodeLESaveUtf8bom(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        std::wstring wtext = (wchar_t *)(m_pBuffer+2);

        // 文件编码
        unsigned char flag[3] = { 0xEF, 0xBB, 0xBF };
        if (1 != fwrite(flag, sizeof(flag), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        // 文件内容
        std::string text = Encoding::UTF8.Transcode(wtext);
        if (1 != fwrite(text.c_str(), text.length(), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 UNICODE_LE->GBK
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::UnicodeLESaveGbk(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        std::wstring wtext = (wchar_t *)(m_pBuffer+2);

        // 文件内容
        std::string text = Encoding::GBK.Transcode(wtext);
        if (1 != fwrite(text.c_str(), text.length(), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 文件保存 UNICODE_LE->UNICODE_LE
    * \param filename 文件名
    * \return 成功返回true，否则返回false
    */
    bool FileLoader::UnicodeLESaveUnicodeBE(LPCSTR filename)
    {
        if (NULL == filename)
        {
            return false;
        }

        FILE *hFile = fopen(filename, "wb");
        if (NULL == hFile)
        {
            return false;
        }

        std::wstring wtext = (wchar_t *)(m_pBuffer+2);
        if (!CharEndianTranscode(wtext))
        {
            fclose(hFile);
            return false;
        }

        // 文件编码
        unsigned char flag[2] = { 0xFE, 0xFF };
        if (1 != fwrite(flag, sizeof(flag), 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        // 文件内容
        size_t nTextSize = wtext.length() * sizeof(wchar_t);
        if (1 != fwrite(wtext.c_str(), nTextSize, 1, hFile))
        {
            fclose(hFile);
            return false;
        }

        fclose(hFile);

        return true;
    }

    /**
    * \brief 读取一行数据
    * \param line 数据
    * \return =0空行 >0成功 <0失败
    */
    int32 FileLoader::ReadLine(std::string &line)
    {
        // Unicode编码格式无法读取，要读取Unicode编码格式，请用ReadLineW接口
        if (m_nEncoding == FILE_ENCODING_UNICODE_BE || m_nEncoding == FILE_ENCODING_UNICODE_LE)
        {
            return -1;
        }

        // 第一次读取，抛掉文件编码头
        if (m_nOffset == 0)
        {
            EncodingOffset();
        }

        line.clear();

        // 已经读取完
        if (IsEOF())
        {
            return -1;
        }

        char * pos = m_pBuffer + m_nOffset;     // 读取位置
        size_t len = 0;                         // 读取长度
        bool flag = false;                      // 引号标志

        while(1)
        {
            if (*pos == '\"')       // 引号
            {
                flag = !flag;
            }

            if ( (*pos == '\r' || *pos == '\n') && !flag )      // 不是引号内部的换行
            {
                break;
            }

            if (*pos == '\0' || m_nOffset + len >= m_nFileSize) // 结束
            {
                break;
            }

            pos++;
            len++;
        }

        int32 result = 0;

        if (len != 0)
        {
            line.append(m_pBuffer + m_nOffset, len);
            result = 1;
        }

        len += 2;       // '\r''\n'

        m_nOffset += len * sizeof(char);

        return result;
    }

    /**
    * \brief 读取一行数据
    * \param wline 数据
    * \return =0空行 >0成功 <0失败
    */
    int32 FileLoader::ReadLineW(std::wstring &wline)
    {
        // 非Unicode编码格式无法读取，要读取非Unicode编码格式，请用ReadLine接口
        if (m_nEncoding != FILE_ENCODING_UNICODE_BE && m_nEncoding != FILE_ENCODING_UNICODE_LE)
        {
            return -1;
        }

        // 第一次读取，抛掉文件编码头
        if (m_nOffset == 0)
        {
            EncodingOffset();
        }

        wline.clear();

        // 已经读取完
        if (IsEOF())
        {
            return -1;
        }

        wchar_t *wpos = (wchar_t *)(m_pBuffer + m_nOffset);     // 读取位置
        size_t len = 0;                                         // 读取长度
        bool flag = false;                                      // 引号标志

        while(1)
        {
            if (*wpos == L'\"')         // 引号
            {
                flag = !flag;
            }

            if ( (*wpos == L'\r' || *wpos == L'\n') && !flag )      // 不是引号内部的换行
            {
                break;
            }

            if (*wpos == '\0' || m_nOffset + len * sizeof(wchar_t) >= m_nFileSize)  // 结束
            {
                break;
            }

            wpos++;
            len++;
        }

        int32 result = 0;

        if (len != 0)
        {
            wline.append((wchar_t *)(m_pBuffer + m_nOffset), len);
            result = 1;
        }

        len += 2;       // L'\r'L'\n'

        m_nOffset += len * sizeof(wchar_t);

        return result;
    }

    /**
    * \brief 编码格式修正
    * 根据文件编码格式，修正文件内容初始buffer位置
    */
    void FileLoader::EncodingOffset()
    {
        switch (m_nEncoding)
        {
        case FILE_ENCODING_UTF8_BOM:
            m_nOffset = 3;
            break;
        case FILE_ENCODING_UNICODE_BE:
        case FILE_ENCODING_UNICODE_LE:
            m_nOffset = 2;
            break;
        case FILE_ENCODING_UNKNOW:
        case FILE_ENCODING_UTF8:
        case FILE_ENCODING_GBK:
        default:
            break;
        }
    }
}