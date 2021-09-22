/**
* \file csv_adapter.h
* \brief csv脚本适配器类头文件
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __CSV_ADAPTER_H__
#define __CSV_ADAPTER_H__

#include "csv_variant.h"
#include "csv_row.h"
#include <vector>
#include "fileloader.h"
#include "strutil.h"
using namespace dtutil;

namespace dtscript
{
    //class FileLoader;

    /**
    * \brief csv脚本适配器
    * \ingroup script_group
    */
    class CsvAdapter : public ICsvIterator
    {
        enum
        {
            CSV_TITLE_ROW_COUNT = 3,      ///< csv标题行数量，第一行为中文字段，第二行为英文字段，第三行为protobuf字段
        };
    public:
        /**
        * \brief 构造函数
        */
        CsvAdapter(void);

        /**
        * \brief 析构函数
        */
        virtual ~CsvAdapter(void);

    public:
        /**
        * \brief 获得文档内容的行数
        * \return 文档内容的行数
        */
        virtual size_t Size();

        /**
        * \brief 根据指定行数获得数据
        * \param nIndex 行数索引
        * \return 指定行数据
        */
        virtual CsvVariant & operator [] (size_t nIndex);

        /**
        * \brief 根据指定名称获得数据
        * \param szName 指定名称
        * \return 指定名称的数据
        */
        virtual CsvVariant & operator [] (const char *szName);

    public:
        /**
        * \brief 装载脚本
        * \param szFileName 脚本文件
        * \param bEncrypt 是否加密
        * \return 装载成功返回true，否则返回false
        */
        bool LoadScript(const char *szFileName, bool bEncrypt);

        /**
        * \brief 装载脚本数据
        * \param pData 脚本数据
        * \param nLen 数据大小
        * \return 装载成功返回true，否则返回false
        */
        bool LoadScript(LPCSTR pData, size_t nLen);

        /**
        * \brief 根据参数名称查找参数在第几列
        * \param szName 列名称
        * \return 参数所在列索引，非列表参数返回-1
        */
        int32 FindPropName(const char *szName);

    private:
        /**
        * \brief 装载csv脚本
        * \param szFileName 脚本文件
        * \return 装载成功返回true，否则返回false
        */
        bool LoadScriptCsv(const char *szFileName);

        /**
        * \brief 装载cse脚本(加密csv脚本)
        * \param szFileName 脚本文件
        * \return 装载成功返回true，否则返回false
        */
        bool LoadScriptCse(const char *szFileName);

        /**
        * \brief 装载脚本文件
        * \param file 加载好的脚本文件
        * \return 装载成功返回true，否则返回false
        */
        bool LoadScriptFile(FileLoader *file);

        /**
        * \brief 根据指定行数获得数据
        * \param nIndex 行数索引
        * \return 指定行数据
        */
        CsvVariant & GetData(size_t nIndex);

        /**
        * \brief 根据指定名称获得数据
        * \param szName 指定名称
        * \return 指定名称的数据
        */
        CsvVariant & GetData(const char *szName);

    private:
        CsvRow                      m_Title[CSV_TITLE_ROW_COUNT];       ///< 标题，第一行为中文字段，第二行为英文字段，第三行为protobuf字段
        std::vector<CsvVariant>     m_RowTable;                         ///< 行数据列表，从第四行开始到文件尾
    };
}

#endif // __CSV_ADAPTER_H__