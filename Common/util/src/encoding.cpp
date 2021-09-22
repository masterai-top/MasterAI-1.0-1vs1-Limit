/**
* \file encoding.cpp
* \brief 字符串编码转换实现代码
*/

#include "pch.h"
#include "encoding.h"
#include "encoding_ansi.h"
#include "encoding_gbk.h"
#include "encoding_utf8.h"


namespace dtutil
{
    /**
    * \brief 取 ANSI 编码实例
    */
    Encoding & Encoding::ANSI = ANSIEncoding::GetInstance();

    /**
    * \brief 取 UTF8 编码实例
    */
    Encoding & Encoding::UTF8 = UTF8Encoding::GetInstance();

    /**
    * \brief 取 GBK 编码实例
    */
    Encoding & Encoding::GBK = GBKEncoding::GetInstance();
}