/**
* \file strutil.h
* \brief 字符串辅助类
*
* 提供字符串的转换和格式化功能。
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __STR_UTIL_H__
#define __STR_UTIL_H__

#include "utildef.h"
#include "typedef.h"
#include <string>
#include <vector>

namespace dtutil
{
    /**
    * \defgroup string_group 实用函数
    * string 提供了一组字符串操作函数，
    * 包括字符串的转换与格式化等。
    */

    /**
    * \brief 字符串辅助类
    * \ingroup string_group
    */
    class UTIL_EXPORT StrUtil
    {
    public:
        /**
        * \brief 字符串格式化
        * \param str 输出字符串
        * \param format 格式
        * \param ... 格式化参数
        * \return 格式化后字符串长度，若失败返回-1
        */
        static int32 strformat(std::string &str, const char *format, ...);

        /**
        * \brief 字符串格式化
        * \param src 输出字符串
        * \param len 字符串最大长度
        * \param format 格式
        * \param ... 格式化参数
        * \return 格式化后字符串长度，若失败返回-1
        */
        static int32 strformat(char *src, int32 len, const char *format, ...);

        /**
        * \brief 字符串格式化
        * \param str 输出字符串
        * \param format 格式
        * \param ap 格式化参数列表
        * \return 格式化后字符串长度，若失败返回-1
        */
        static int32 strformat(std::string &str, const char *format, va_list ap);

        /**
        * \brief 字符串格式化
        * \param src 输出字符串
        * \param len 字符串最大长度
        * \param format 格式
        * \param ap 格式化参数列表
        * \return 格式化后字符串长度，若失败返回-1
        */
        static int32 strformat(char *src, int32 len, const char *format, va_list ap);

        /**
        * \brief 字符串比较 区分大小写
        * \param src 源字符串
        * \param dest 目标字符串
        * \param len 比较长度
        * \return =0(src=dest),>0(src>dest),<0(src<dest)
        */
        static int32 strncmp(const char *src, const char *dest, size_t len);

        /**
        * \brief 字符串比较 不区分大小写
        * \param src 源字符串
        * \param dest 目标字符串
        * \param len 比较长度
        * \return =0(src=dest),>0(src>dest),<0(src<dest)
        */
        static int32 strnicmp(const char *src, const char *dest, size_t len);

        /**
        * \brief 字符串比较 区分大小写
        * \param src 源字符串
        * \param dest 目标字符串
        * \param len 比较长度
        * \return =0(src=dest),>0(src>dest),<0(src<dest)
        */
        static int32 wstrncmp(const wchar_t *src, const wchar_t *dest, size_t len);

        /**
        * \brief 字符串比较 不区分大小写
        * \param src 源字符串
        * \param dest 目标字符串
        * \param len 比较长度
        * \return =0(src=dest),>0(src>dest),<0(src<dest)
        */
        static int32 wstrnicmp(const wchar_t *src, const wchar_t *dest, size_t len);

        /**
        * \brief 字符串分割
        * \param str 源字符串 分割后返回分割后的字符
        * \param delim 分割符
        * \return 分割的第一个字符串
        *
        * 这个函数会修改回来字符串的内容
        * \par 范例
        * \code
        * char *tmps = new char[len+1];
        * memset(tmps, 0, sizeof(char)*(len+1));
        * std::strncpy(tmps,str,len);
        * tmps[len] = 0;
        *
        * char *tmp = tmps;
        *
        * char *token = strtoken(&tmp, &step);
        * while (NULL != token)
        * {
        *   vec.push(token);
        *   token = strtoken(&tmp, &step);
        * }
        * \endcode
        */
        static char * strtoken(char **str, const char *delim);

        /**
        * \brief 字符串分割
        * \param wstr 源字符串 分割后返回分割后的字符
        * \param delim 分割符
        * \return 分割的第一个字符串
        */
        static wchar_t * wstrtoken(wchar_t **wstr, const wchar_t *delim);

        /**
        * \brief 字符串是否为空
        * \param str 字符串
        * \return 空返回true，非空返回false
        */
        static bool strempty(const std::string &str);

        /**
        * \brief 字符串是否为空
        * \param str 字符串
        * \return 空返回true，非空返回false
        */
        static bool strempty(const char *str);

        /**
        * \brief 字符串是否为空
        * \param wstr 字符串
        * \return 空返回true，非空返回false
        */
        static bool wstrempty(const std::wstring &wstr);

        /**
        * \brief 字符串是否为空
        * \param wstr 字符串
        * \return 空返回true，非空返回false
        */
        static bool wstrempty(const wchar_t *wstr);

        /**
        * \brief 字符串安全拷贝
        * \param dest 目标字符串
        * \param size 目标字符串长度
        * \param src 源字符串
        * \return 拷贝结果
        */
        static char * strncpy(char *dest, size_t size, std::string &src);

        /**
        * \brief 字符串安全拷贝
        * \param dest 目标字符串
        * \param size 目标字符串长度
        * \param src 源字符串
        * \return 拷贝结果
        */
        static char * strncpy(char *dest, size_t size, const char *src);

        /**
        * \brief string 转换成 int32
        * \param str 字符串
        * \param value 默认返回值
        * \return 转换值
        */
        static int32 strtoi32(const std::string &str, int32 value = 0);

        /**
        * \brief string 转换成 int32
        * \param str 字符串
        * \param value 默认返回值
        * \return 转换值
        */
        static int32 strtoi32(const char *str, int32 value = 0);

        /**
        * \brief wstring 转换成 int32
        * \param wstr 字符串
        * \param value 默认返回值
        * \return 转换值
        */
        static int32 wstrtoi32(const std::wstring &wstr, int32 value = 0);

        /**
        * \brief wstring 转换成 int32
        * \param wstr 字符串
        * \param value 默认返回值
        * \return 转换值
        */
        static int32 wstrtoi32(const wchar_t *wstr, int32 value = 0);

        /**
        * \brief int32 转换成 string
        * \param value long值
        * \return 转换值
        */
        static std::string i32tostr(int32 value);

        /**
        * \brief int32 转换成 wstring
        * \param value long值
        * \return 转换值
        */
        static std::wstring i32towstr(int32 value);

        /**
        * \brief string 转换成 int64
        * \param str 字符串
        * \param value 默认返回值
        * \return 转换值
        */
        static int64 strtoi64(const std::string &str, int64 value = 0);

        /**
        * \brief string 转换成 int64
        * \param str 字符串
        * \param value 默认返回值
        * \return 转换值
        */
        static int64 strtoi64(const char *str, int64 value = 0);

        /**
        * \brief wstring 转换成 int64
        * \param wstr 字符串
        * \param value 默认返回值
        * \return 转换值
        */
        static int64 wstrtoi64(const std::wstring &wstr, int64 value = 0);

        /**
        * \brief wstring 转换成 int64
        * \param wstr 字符串
        * \param value 默认返回值
        * \return 转换值
        */
        static int64 wstrtoi64(const wchar_t *wstr, int64 value = 0);

        /**
        * \brief int64 转换成 string
        * \param value long值
        * \return 转换值
        */
        static std::string i64tostr(int64 value);

        /**
        * \brief int64 转换成 wstring
        * \param value long值
        * \return 转换值
        */
        static std::wstring i64towstr(int64 value);

        /**
        * \brief string 转换成 double
        * \param str 字符串
        * \param value 默认返回值
        * \return 转换值
        */
        static double strtodouble(const std::string &str, double value = 0);

        /**
        * \brief string 转换成 double
        * \param str 字符串
        * \param value 默认返回值
        * \return 转换值
        */
        static double strtodouble(const char *str, double value = 0);

        /**
        * \brief wstring 转换成 double
        * \param wstr 字符串
        * \param value 默认返回值
        * \return 转换值
        */
        static double wstrtodouble(const std::wstring &wstr, double value = 0);

        /**
        * \brief wstring 转换成 double
        * \param wstr 字符串
        * \param value 默认返回值
        * \return 转换值
        */
        static double wstrtodouble(const wchar_t *wstr, double value = 0);

        /**
        * \brief double 转换成 string
        * \param value double值
        * \return 转换值
        */
        static std::string doubletostr(double value);

        /**
        * \brief double 转换成 wstring
        * \param value double值
        * \return 转换值
        */
        static std::wstring doubletowstr(double value);

        /**
        * \brief string 转换成 bool
        * \param str 字符串
        * \param value 默认返回值
        * \return 转换值
        */
        static bool strtobool(const std::string &str, bool value = false);

        /**
        * \brief string 转换成 bool
        * \param str 字符串
        * \param value 默认返回值
        * \return 转换值
        */
        static bool strtobool(const char *str, bool value = false);

        /**
        * \brief wstring 转换成 bool
        * \param wstr 字符串
        * \param value 默认返回值
        * \return 转换值
        */
        static bool wstrtobool(const std::wstring &wstr, bool value = false);

        /**
        * \brief wstring 转换成 bool
        * \param wstr 字符串
        * \param value 默认返回值
        * \return 转换值
        */
        static bool wstrtobool(const wchar_t *wstr, bool value = false);

        /**
        * \brief bool 转换成 string
        * \param value bool值
        * \return 转换值
        */
        static std::string booltostr(bool value);

        /**
        * \brief bool 转换成 wstring
        * \param value bool值
        * \return 转换值
        */
        static std::wstring booltowstr(bool value);

        /**
        * \brief string 转换成 time_t
        * \param str 字符串
        * \param value 默认返回值
        * \return 转换值，失败返回0
        */
        static time_t strtotime(const std::string &str, time_t value = 0);

        /**
        * \brief string 转换成 time_t
        * \param str 字符串
        * \param value 默认返回值
        * \return 转换值，失败返回0
        */
        static time_t strtotime(const char *str, time_t value = 0);

        /**
        * \brief wstring 转换成 time_t
        * \param wstr 字符串
        * \param value 默认返回值
        * \return 转换值，失败返回0
        */
        static time_t wstrtotime(const std::wstring &wstr, time_t value = 0);

        /**
        * \brief wstring 转换成 time_t
        * \param wstr 字符串
        * \param value 默认返回值
        * \return 转换值，失败返回0
        */
        static time_t wstrtotime(const wchar_t *wstr, time_t value = 0);

        /**
        * \brief time_t 转换成 string
        * \param value time_t值
        * \return 转换值
        */
        static std::string timetostr(time_t value);

        /**
        * \brief time_t 转换成 wstring
        * \param value time_t值
        * \return 转换值
        */
        static std::wstring timetowstr(time_t value);

        /**
        * \brief 删除 string 左空格
        * \param str 字符串
        * \return 删除左空格后的字符串
        */
        static std::string ltrim(const std::string &str);

        /**
        * \brief 删除 string 右空格
        * \param str 字符串
        * \return 删除右空格后的字符串
        */
        static std::string rtrim(const std::string &str);

        /**
        * \brief 删除 string 左右空格
        * \param str 字符串
        * \return 删除左右空格后的字符串
        */
        static std::string trim(const std::string &str);

        /**
        * \brief 删除 wstring 左空格
        * \param wstr 字符串
        * \return 删除左空格后的字符串
        */
        static std::wstring wltrim(const std::wstring &wstr);

        /**
        * \brief 删除 wstring 右空格
        * \param wstr 字符串
        * \return 删除右空格后的字符串
        */
        static std::wstring wrtrim(const std::wstring &wstr);

        /**
        * \brief 删除 wstring 左右空格
        * \param wstr 字符串
        * \return 删除左右空格后的字符串
        */
        static std::wstring wtrim(const std::wstring &wstr);

        /**
        * \brief 删除 string 指定字符
        * \param str 字符串
        * \param ext 删除字符串
        */
        static void trimex(std::string &str, std::string ext = std::string());

        /**
        * \brief 删除 wstring 指定字符
        * \param wstr 字符串
        * \param wext 删除字符串
        */
        static void wtrimex(std::wstring &wstr, std::wstring wext = std::wstring());

        /**
        * \brief string 转换成 int32 数组
        * \param str 字符串
        * \param vec 转换后的int32数组
        * \param step 字符串分割符
        * \return 转换后的int32数组大小
        */
        static size_t strtoi32arr(const std::string &str, std::vector<int32> &vec, const char step = ' ');

        /**
        * \brief string 转换成 int32 数组
        * \param str 字符串
        * \param vec 转换后的int32数组
        * \param step 字符串分割符
        * \return 转换后的int32数组大小
        */
        static size_t strtoi32arr(const char *str, std::vector<int32> &vec, const char step = ' ');

        /**
        * \brief wstring 转换成 int32 数组
        * \param wstr 字符串
        * \param vec 转换后的int32数组
        * \param step 字符串分割符
        * \return 转换后的int32数组大小
        */
        static size_t wstrtoi32arr(const std::wstring &wstr, std::vector<int32> &vec, const wchar_t step = L' ');

        /**
        * \brief wstring 转换成 int32 数组
        * \param wstr 字符串
        * \param vec 转换后的int32数组
        * \param step 字符串分割符
        * \return 转换后的int32数组大小
        */
        static size_t wstrtoi32arr(const wchar_t *wstr, std::vector<int32> &vec, const wchar_t step = L' ');

        /**
        * \brief string 转换成 int64 数组
        * \param str 字符串
        * \param vec 转换后的int64数组
        * \param step 字符串分割符
        * \return 转换后的int64数组大小
        */
        static size_t strtoi64arr(const std::string &str, std::vector<int64> &vec, const char step = ' ');

        /**
        * \brief string 转换成 int64 数组
        * \param str 字符串
        * \param vec 转换后的int64数组
        * \param step 字符串分割符
        * \return 转换后的int64数组大小
        */
        static size_t strtoi64arr(const char *str, std::vector<int64> &vec, const char step = ' ');

        /**
        * \brief wstring 转换成 int64 数组
        * \param wstr 字符串
        * \param vec 转换后的int64数组
        * \param step 字符串分割符
        * \return 转换后的int64数组大小
        */
        static size_t wstrtoi64arr(const std::wstring &wstr, std::vector<int64> &vec, const wchar_t step = L' ');

        /**
        * \brief wstring 转换成 int64 数组
        * \param wstr 字符串
        * \param vec 转换后的int64数组
        * \param step 字符串分割符
        * \return 转换后的int64数组大小
        */
        static size_t wstrtoi64arr(const wchar_t *wstr, std::vector<int64> &vec, const wchar_t step = L' ');

        /**
        * \brief string 转换成 string 数组
        * \param str 字符串
        * \param vec 转换后的string数组
        * \param step 字符串分割符
        * \return 转换后的string数组大小
        */
        static size_t strtostrarr(const std::string &str, std::vector<std::string> &vec, const char step = ' ');

        /**
        * \brief string 转换成 string 数组
        * \param str 字符串
        * \param vec 转换后的string数组
        * \param step 字符串分割符
        * \return 转换后的string数组大小
        */
        static size_t strtostrarr(const char *str, std::vector<std::string> &vec, const char step = ' ');

        /**
        * \brief wstring 转换成 wstring 数组
        * \param wstr 字符串
        * \param vec 转换后的wstring数组
        * \param step 字符串分割符
        * \return 转换后的wstring数组大小
        */
        static size_t wstrtowstrarr(const std::wstring &wstr, std::vector<std::wstring> &vec, const wchar_t step = L' ');

        /**
        * \brief wstring 转换成 wstring 数组
        * \param wstr 字符串
        * \param vec 转换后的wstring数组
        * \param step 字符串分割符
        * \return 转换后的wstring数组大小
        */
        static size_t wstrtowstrarr(const wchar_t *wstr, std::vector<std::wstring> &vec, const wchar_t step = L' ');

        /**
        * \brief 字符串替换
        * \param str 输出字符串
        * \param format 格式，参数格式{0}{1}
        * \param vec 替换字符串组
        * \return 成功返回true，否则返回false
        */
        static bool strreplace(std::string &str, const char *format, std::vector<std::string> &vec);

        /**
        * \brief 字符串替换
        * \param str 输出字符串
        * \param format 格式，参数格式{0}{1}
        * \param vec 替换字符串组
        * \return 成功返回true，否则返回false
        */
        static bool strreplace(char *str, const char *format, std::vector<std::string> &vec);
    };
}

#endif // __STR_UTIL_H__