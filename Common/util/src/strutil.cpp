/**
* \file strutil.cpp
* \brief 字符串辅助类实现代码
*/

#include "pch.h"
#include "strutil.h"
#include "timeutil.h"
#include "encoding.h"

#ifdef WIN32
#include <regex>
#else
#include <regex.h>
#endif


namespace dtutil
{
#ifdef WIN32
    /**
    * \brief 解析字符串，将日期信息保存到结构体中
    * \param str 待解析的字符串
    * \param sm 用于保存字符串匹配的结果
    * \param tim 保存解析后的日期信息
    * \return 成功解析返回true，否则返回false
    */
    bool strdatetotm(const std::string &str, std::smatch &sm, struct tm &tim)
    {
        // Perl(PCRE)规范
        static std::regex date_reg = std::regex("(\\d{4})(?:\\s*(-)\\s*)(\\d{2})(?:\\s*(-)\\s*)(\\d{2})|(\\d{2})(?:\\s*(/)\\s*)(\\d{2})(?:\\s*(/)\\s*)(\\d{4})");

        // 判断是否能匹配日期格式
        if (!std::regex_search(str, sm, date_reg, std::regex_constants::match_default))
        {
            return false;
        }

        std::string sep(sm[2].first, sm[2].second);
        std::string sep1(sm[7].first, sm[7].second);

        if ("-" == sep)     // 以“-”分隔的字符串，日期格式为“年-月-日”
        {
            std::string yearstr(sm[1].first, sm[1].second);
            std::string monstr(sm[3].first, sm[3].second);
            std::string mdaystr(sm[5].first, sm[5].second);
            tim.tm_year = StrUtil::strtoi32(yearstr);
            tim.tm_mon = StrUtil::strtoi32(monstr);
            tim.tm_mday = StrUtil::strtoi32(mdaystr);
        }
        else if ("/" == sep1)   // 以“/”分隔的字符串，日期格式为“月/日/年”
        {
            std::string monstr(sm[6].first, sm[6].second);
            std::string mdaystr(sm[8].first, sm[8].second);
            std::string yearstr(sm[10].first, sm[10].second);
            tim.tm_year = StrUtil::strtoi32(yearstr);
            tim.tm_mon = StrUtil::strtoi32(monstr);
            tim.tm_mday = StrUtil::strtoi32(mdaystr);
        }

        // 修正捕获的字段
        if (tim.tm_year > 1900)
            tim.tm_year -= 1900;
        else if (tim.tm_year < 70)
            tim.tm_year += 100;
        tim.tm_mon -= 1;

        return true;
    }

    /**
    * \brief 解析字符串，将日期信息保存到结构体中
    * \param str 待解析的字符串
    * \param sm 用于保存字符串匹配的结果
    * \param tim 保存解析后的日期信息
    * \return 成功解析返回true，否则返回false
    */
    bool wstrdatetotm(const std::wstring &wstr, std::wsmatch &wsm, struct tm &tim)
    {
        // Perl(PCRE)规范
        static std::wregex date_reg = std::wregex(L"(\\d{4})(?:\\s*(-)\\s*)(\\d{2})(?:\\s*(-)\\s*)(\\d{2})|(\\d{2})(?:\\s*(/)\\s*)(\\d{2})(?:\\s*(/)\\s*)(\\d{4})");

        // 判断是否能匹配日期格式
        if (!std::regex_search(wstr, wsm, date_reg, std::regex_constants::match_default))
        {
            return false;
        }

        std::wstring sep(wsm[2].first, wsm[2].second);
        std::wstring sep1(wsm[7].first, wsm[7].second);

        if (L"-" == sep)     // 以“-”分隔的字符串，日期格式为“年-月-日”
        {
            std::wstring yearstr(wsm[1].first, wsm[1].second);
            std::wstring monstr(wsm[3].first, wsm[3].second);
            std::wstring mdaystr(wsm[5].first, wsm[5].second);
            tim.tm_year = StrUtil::wstrtoi32(yearstr);
            tim.tm_mon = StrUtil::wstrtoi32(monstr);
            tim.tm_mday = StrUtil::wstrtoi32(mdaystr);
        }
        else if (L"/" == sep1)   // 以“/”分隔的字符串，日期格式为“月/日/年”
        {
            std::wstring monstr(wsm[6].first, wsm[6].second);
            std::wstring mdaystr(wsm[8].first, wsm[8].second);
            std::wstring yearstr(wsm[10].first, wsm[10].second);
            tim.tm_year = StrUtil::wstrtoi32(yearstr);
            tim.tm_mon = StrUtil::wstrtoi32(monstr);
            tim.tm_mday = StrUtil::wstrtoi32(mdaystr);
        }

        // 修正捕获的字段
        if (tim.tm_year > 1900)
            tim.tm_year -= 1900;
        else if (tim.tm_year < 70)
            tim.tm_year += 100;
        tim.tm_mon -= 1;

        return true;
    }

    /**
    * \brief 解析字符串，将时间信息保存到结构体中
    * \param str 待解析的字符串
    * \param sm 用于保存字符串匹配的结果
    * \param tim 保存解析后的时间信息
    * \return 成功解析返回true，否则返回false
    */
    bool strtimetotm(const std::string &str, std::smatch &sm, struct tm &tim)
    {
        // Perl(PCRE)规范
        static std::regex time_reg = std::regex("(\\d{1,2}):(\\d{1,2}):(\\d{1,2})(?:\\s*)(AM|PM)?");

        // 判断是否能匹配时间格式
        if (!std::regex_search(str, sm, time_reg, std::regex_constants::match_default))
        {
            return false;
        }

        // 将匹配的时间保存到结构体中去
        std::string hourstr(sm[1].first, sm[1].second);
        std::string minstr(sm[2].first, sm[2].second);
        std::string secstr(sm[3].first, sm[3].second);
        tim.tm_hour = StrUtil::strtoi32(hourstr);
        tim.tm_min = StrUtil::strtoi32(minstr);
        tim.tm_sec = StrUtil::strtoi32(secstr);

        std::string apstr(sm[4].first, sm[4].second);
        // 12小时制式的下午时，要对小时字段加12
        if ("PM" == apstr)
        {
            tim.tm_hour += 12;
        }

        return true;
    }

    /**
    * \brief 解析字符串，将时间信息保存到结构体中
    * \param wstr 待解析的字符串
    * \param wsm 用于保存字符串匹配的结果
    * \param tim 保存解析后的时间信息
    * \return 成功解析返回true，否则返回false
    */
    bool wstrtimetotm(const std::wstring &wstr, std::wsmatch &wsm, struct tm &tim)
    {
        // Perl(PCRE)规范
        static std::wregex time_reg = std::wregex(L"(\\d{1,2}):(\\d{1,2}):(\\d{1,2})(?:\\s*)(AM|PM)?");

        // 判断是否能匹配时间格式
        if (!std::regex_search(wstr, wsm, time_reg, std::regex_constants::match_default))
        {
            return false;
        }

        // 将匹配的时间保存到结构体中去
        std::wstring hourstr(wsm[1].first, wsm[1].second);
        std::wstring minstr(wsm[2].first, wsm[2].second);
        std::wstring secstr(wsm[3].first, wsm[3].second);
        tim.tm_hour = StrUtil::wstrtoi32(hourstr);
        tim.tm_min = StrUtil::wstrtoi32(minstr);
        tim.tm_sec = StrUtil::wstrtoi32(secstr);

        std::wstring apstr(wsm[4].first, wsm[4].second);
        // 12小时制式的下午时，要对小时字段加12
        if (L"PM" == apstr)
        {
            tim.tm_hour += 12;
        }

        return true;
    }
#else
    /**
    * \brief 解析字符串，将日期信息保存到结构体中
    * \param str 待解析的字符串
    * \param sm 用于保存字符串匹配的结果
    * \param tim 保存解析后的日期信息
    * \return 成功解析返回true，否则返回false
    */
    bool strdatetotm(const std::string &str, std::string &sm, struct tm &tim)
    {
        // POSIX(ERE)规范，(这里没有处理非捕获分组)
        static std::string date_reg = "([[:digit:]]{4})([[:space:]]*(-)[[:space:]]*)([[:digit:]]{2})([[:space:]]*(-)[[:space:]]*)([[:digit:]]{2})|([[:digit:]]{2})([[:space:]]*(/)[[:space:]]*)([[:digit:]]{2})([[:space:]]*(/)[[:space:]]*)([[:digit:]]{4})";

        // 判断是否能匹配日期格式
        regex_t reg;
        regmatch_t pm[15];
        char ebuf[128];
        memset(ebuf, 0, sizeof(ebuf));
        const size_t TIME_REG_MAX = 15;

        int32 cflag = REG_EXTENDED;

        // 编译正则表达式
        int32 res = regcomp(&reg, date_reg.c_str(), cflag);
        if (0 != res)
        {
            regerror(res, &reg, ebuf, sizeof(ebuf));
            return false;
        }

        // 匹配正则表达式
        res = regexec(&reg, str.c_str(), TIME_REG_MAX, pm, 0);
        if (0 != res)
        {
            regerror(res, &reg, ebuf, sizeof(ebuf));
            return false;
        }

        if ( -1 != pm[3].rm_so && -1 != pm[3].rm_eo)
        {
            std::string sep = str.substr(pm[3].rm_so, pm[3].rm_eo - pm[3].rm_so);
            if ("-" == sep)     // 以“-”分隔的字符串，日期格式为“年-月-日”
            {
                std::string yearstr = str.substr(pm[1].rm_so, pm[1].rm_eo - pm[1].rm_so);
                std::string monstr = str.substr(pm[4].rm_so, pm[4].rm_eo - pm[4].rm_so);
                std::string mdaystr = str.substr(pm[7].rm_so, pm[7].rm_eo - pm[7].rm_so);
                tim.tm_year = StrUtil::strtoi32(yearstr);
                tim.tm_mon = StrUtil::strtoi32(monstr);
                tim.tm_mday = StrUtil::strtoi32(mdaystr);
                sm = str.substr(0, pm[7].rm_eo);
            }
        }

        if ( -1 != pm[10].rm_so && -1 != pm[10].rm_eo)
        {
            std::string sep = str.substr(pm[10].rm_so, pm[10].rm_eo - pm[10].rm_so);
            if ("/" == sep)     // 以“/”分隔的字符串，日期格式为“月/日/年”
            {
                std::string monstr = str.substr(pm[8].rm_so, pm[8].rm_eo - pm[8].rm_so);
                std::string mdaystr = str.substr(pm[11].rm_so, pm[11].rm_eo - pm[11].rm_so);
                std::string yearstr = str.substr(pm[14].rm_so, pm[14].rm_eo - pm[14].rm_so);
                tim.tm_year = StrUtil::strtoi32(yearstr);
                tim.tm_mon = StrUtil::strtoi32(monstr);
                tim.tm_mday = StrUtil::strtoi32(mdaystr);
                sm = str.substr(0, pm[14].rm_eo);
            }
        }

        // 释放正则表达式
        regfree(&reg);

        // 修正捕获的字段
        if (tim.tm_year > 1900)
            tim.tm_year -= 1900;
        else if (tim.tm_year < 70)
            tim.tm_year += 100;
        tim.tm_mon -= 1;

        return true;
    }

    /**
    * \brief 解析字符串，将日期信息保存到结构体中
    * \param wstr 待解析的字符串
    * \param wsm 用于保存字符串匹配的结果
    * \param tim 保存解析后的日期信息
    * \return 成功解析返回true，否则返回false
    */
    bool wstrdatetotm(const std::wstring &wstr, std::wstring &wsm, struct tm &tim)
    {
        // POSIX(ERE)规范，(这里没有处理非捕获分组)
        static std::string date_reg = "([[:digit:]]{4})([[:space:]]*(-)[[:space:]]*)([[:digit:]]{2})([[:space:]]*(-)[[:space:]]*)([[:digit:]]{2})|([[:digit:]]{2})([[:space:]]*(/)[[:space:]]*)([[:digit:]]{2})([[:space:]]*(/)[[:space:]]*)([[:digit:]]{4})";

        std::string str = Encoding::UTF8.Transcode(wstr);

        // 判断是否能匹配日期格式
        regex_t reg;
        regmatch_t pm[15];
        char ebuf[128];
        memset(ebuf, 0, sizeof(ebuf));
        const size_t TIME_REG_MAX = 15;

        int32 cflag = REG_EXTENDED;

        // 编译正则表达式
        int32 res = regcomp(&reg, date_reg.c_str(), cflag);
        if (0 != res)
        {
            regerror(res, &reg, ebuf, sizeof(ebuf));
            return false;
        }

        // 匹配正则表达式
        res = regexec(&reg, str.c_str(), TIME_REG_MAX, pm, 0);
        if (0 != res)
        {
            regerror(res, &reg, ebuf, sizeof(ebuf));
            return false;
        }

        if ( -1 != pm[3].rm_so && -1 != pm[3].rm_eo)
        {
            std::string sep = str.substr(pm[3].rm_so, pm[3].rm_eo - pm[3].rm_so);
            if ("-" == sep)     // 以“-”分隔的字符串，日期格式为“年-月-日”
            {
                std::string yearstr = str.substr(pm[1].rm_so, pm[1].rm_eo - pm[1].rm_so);
                std::string monstr = str.substr(pm[4].rm_so, pm[4].rm_eo - pm[4].rm_so);
                std::string mdaystr = str.substr(pm[7].rm_so, pm[7].rm_eo - pm[7].rm_so);
                tim.tm_year = StrUtil::strtoi32(yearstr);
                tim.tm_mon = StrUtil::strtoi32(monstr);
                tim.tm_mday = StrUtil::strtoi32(mdaystr);
                wsm = Encoding::UTF8.Transcode(str.substr(0, pm[7].rm_eo));
            }
        }

        if ( -1 != pm[10].rm_so && -1 != pm[10].rm_eo)
        {
            std::string sep = str.substr(pm[10].rm_so, pm[10].rm_eo - pm[10].rm_so);
            if ("/" == sep)     // 以“/”分隔的字符串，日期格式为“月/日/年”
            {
                std::string monstr = str.substr(pm[8].rm_so, pm[8].rm_eo - pm[8].rm_so);
                std::string mdaystr = str.substr(pm[11].rm_so, pm[11].rm_eo - pm[11].rm_so);
                std::string yearstr = str.substr(pm[14].rm_so, pm[14].rm_eo - pm[14].rm_so);
                tim.tm_year = StrUtil::strtoi32(yearstr);
                tim.tm_mon = StrUtil::strtoi32(monstr);
                tim.tm_mday = StrUtil::strtoi32(mdaystr);
                wsm = Encoding::UTF8.Transcode(str.substr(0, pm[14].rm_eo));
            }
        }

        // 释放正则表达式
        regfree(&reg);

        // 修正捕获的字段
        if (tim.tm_year > 1900)
            tim.tm_year -= 1900;
        else if (tim.tm_year < 70)
            tim.tm_year += 100;
        tim.tm_mon -= 1;

        return true;
    }

    /**
    * \brief 解析字符串，将时间信息保存到结构体中
    * \param str 待解析的字符串
    * \param sm 用于保存字符串匹配的结果
    * \param tim 保存解析后的时间信息
    * \return 成功解析返回true，否则返回false
    */
    bool strtimetotm(const std::string &str, std::string &sm, struct tm &tim)
    {
        // POSIX(ERE)规范，(这里没有处理非捕获分组)
        static std::string time_reg = "([[:digit:]]{1,2}):([[:digit:]]{1,2}):([[:digit:]]{1,2})([[:space:]]*)(AM|PM)?";

        // 判断是否能匹配时间格式
        regex_t reg;
        regmatch_t pm[15];
        char ebuf[128];
        memset(ebuf, 0, sizeof(ebuf));
        const size_t TIME_REG_MAX = 15;

        int32 cflag = REG_EXTENDED;

        // 编译正则表达式
        int32 res = regcomp(&reg, time_reg.c_str(), cflag);
        if (0 != res)
        {
            regerror(res, &reg, ebuf, sizeof(ebuf));
            return false;
        }

        // 匹配正则表达式
        res = regexec(&reg, str.c_str(), TIME_REG_MAX, pm, 0);
        if (0 != res)
        {
            regerror(res, &reg, ebuf, sizeof(ebuf));
            return false;
        }

        std::string hourstr = str.substr(pm[1].rm_so, pm[1].rm_eo - pm[1].rm_so);
        std::string minstr = str.substr(pm[2].rm_so, pm[2].rm_eo - pm[2].rm_so);
        std::string secstr = str.substr(pm[3].rm_so, pm[3].rm_eo - pm[3].rm_so);
        tim.tm_hour = StrUtil::strtoi32(hourstr);
        tim.tm_min = StrUtil::strtoi32(minstr);
        tim.tm_sec = StrUtil::strtoi32(secstr);
        sm = str.substr(0, pm[3].rm_eo);

        if ( -1 != pm[5].rm_so && -1 != pm[5].rm_eo)
        {
            std::string apstr = str.substr(pm[5].rm_so, pm[5].rm_eo - pm[5].rm_so);
            if ("PM" == apstr)
            {
                tim.tm_hour += 12;
            }

            sm = str.substr(0, pm[5].rm_eo);
        }

        // 释放正则表达式
        regfree(&reg);

        return true;
    }
    
    /**
    * \brief 解析字符串，将时间信息保存到结构体中
    * \param wstr 待解析的字符串
    * \param wsm 用于保存字符串匹配的结果
    * \param tim 保存解析后的时间信息
    * \return 成功解析返回true，否则返回false
    */
    bool wstrtimetotm(const std::wstring &wstr, std::wstring &wsm, struct tm &tim)
    {
        // POSIX(ERE)规范，(这里没有处理非捕获分组)
        static std::string time_reg = "([[:digit:]]{1,2}):([[:digit:]]{1,2}):([[:digit:]]{1,2})([[:space:]]*)(AM|PM)?";
        std::string str = Encoding::UTF8.Transcode(wstr);

        // 判断是否能匹配时间格式
        regex_t reg;
        regmatch_t pm[15];
        char ebuf[128];
        memset(ebuf, 0, sizeof(ebuf));
        const size_t TIME_REG_MAX = 15;

        int32 cflag = REG_EXTENDED;

        // 编译正则表达式
        int32 res = regcomp(&reg, time_reg.c_str(), cflag);
        if (0 != res)
        {
            regerror(res, &reg, ebuf, sizeof(ebuf));
            return false;
        }

        // 匹配正则表达式
        res = regexec(&reg, str.c_str(), TIME_REG_MAX, pm, 0);
        if (0 != res)
        {
            regerror(res, &reg, ebuf, sizeof(ebuf));
            return false;
        }

        std::string hourstr = str.substr(pm[1].rm_so, pm[1].rm_eo - pm[1].rm_so);
        std::string minstr = str.substr(pm[2].rm_so, pm[2].rm_eo - pm[2].rm_so);
        std::string secstr = str.substr(pm[3].rm_so, pm[3].rm_eo - pm[3].rm_so);
        tim.tm_hour = StrUtil::strtoi32(hourstr);
        tim.tm_min = StrUtil::strtoi32(minstr);
        tim.tm_sec = StrUtil::strtoi32(secstr);
        wsm = Encoding::UTF8.Transcode(str.substr(0, pm[3].rm_eo));

        if ( -1 != pm[5].rm_so && -1 != pm[5].rm_eo)
        {
            std::string apstr = str.substr(pm[5].rm_so, pm[5].rm_eo - pm[5].rm_so);
            if ("PM" == apstr)
            {
                tim.tm_hour += 12;
            }

            wsm = Encoding::UTF8.Transcode(str.substr(0, pm[5].rm_eo));
        }

        // 释放正则表达式
        regfree(&reg);

        return true;
    }
#endif

    /**
    * \brief 字符串格式化
    * \param str 输出字符串
    * \param format 格式
    * \param ... 格式化参数
    * \return 格式化后字符串长度，若失败返回-1
    */
    int32 StrUtil::strformat(std::string &str, const char *format, ...)
    {
        va_list ap;
        va_start(ap, format);

        int32 size = strformat(str, format, ap);

        va_end(ap);

        return size;
    }

    /**
    * \brief 字符串格式化
    * \param src 输出字符串
    * \param len 字符串最大长度
    * \param format 格式
    * \param ... 格式化参数
    * \return 格式化后字符串长度，若失败返回-1
    */
    int32 StrUtil::strformat(char *src, int32 len, const char *format, ...)
    {
        va_list ap;
        va_start(ap, format);

        int32 size = strformat(src, len, format, ap);

        va_end(ap);

        return size;
    }

    /**
    * \brief 字符串格式化
    * \param str 输出字符串
    * \param format 格式
    * \param ap 格式化参数列表
    * \return 格式化后字符串长度，若失败返回-1
    */
    int32 StrUtil::strformat(std::string &str, const char *format, va_list ap)
    {
        va_list tap;
        va_copy(tap, ap);

        // 计算需要的内存空间
#ifdef WIN32
        int32 size = ::_vsnprintf(NULL, 0, format, tap);
#else
        int32 size = ::vsnprintf(NULL, 0, format, tap);
#endif

        va_end(tap);

        if (size <= 0)
        {
            return size;
        }

        char *buf = new char[++size];
        if (!buf)
        {
            return -1;
        }

        va_copy(tap, ap);

#ifdef WIN32
        int32 len = ::_vsnprintf(buf, size, format, tap);
#else
        int32 len = ::vsnprintf(buf, size, format, tap);
#endif

        va_end(tap);

        if (len < 0 || len >= size)
        {
            delete [] buf;
            return -1;
        }

        buf[len] = 0;
        str.assign(buf, len);

        delete [] buf;

        return len;
    }

    /**
    * \brief 字符串格式化
    * \param src 输出字符串
    * \param len 字符串最大长度
    * \param format 格式
    * \param ap 格式化参数列表
    * \return 格式化后字符串长度，若失败返回-1
    */
    int32 StrUtil::strformat(char *src, int32 len, const char *format, va_list ap)
    {
        va_list tap;
        va_copy(tap, ap);

        // 计算需要的内存空间
#ifdef WIN32
        int32 size = ::_vsnprintf(NULL, 0, format, tap);
#else
        int32 size = ::vsnprintf(NULL, 0, format, tap);
#endif

        va_end(tap);

        if (size <= 0)
        {
            return size;
        }

        if (NULL == src || len < ++size)
        {
            return -1;
        }

        va_copy(tap, ap);

#ifdef WIN32
        len = ::_vsnprintf(src, size, format, tap);
#else
        len = ::vsnprintf(src, size, format, tap);
#endif

        va_end(tap);

        if (len < 0 || len >= size)
        {
            return -1;
        }

        src[len] = 0;

        return len;
    }

    /**
    * \brief 字符串比较 区分大小写
    * \param src 源字符串
    * \param dest 目标字符串
    * \param len 比较长度
    * \return =0(src=dest),>0(src>dest),<0(src<dest)
    */
    int32 StrUtil::strncmp(const char *src, const char *dest, size_t len)
    {
        unsigned char c1, c2;
        c1 = 0; c2 = 0;

        if (0 != len)
        {
            do 
            {
                c1 = *src; c2 = *dest;
                src++; dest++;

                // 是否已到字符串的末尾或两字符串是否有空串，如果到了末尾或有空串，则比较结束
                if (!c1) break;
                if (!c2) break;

                // 如果没有，且字符串相等，则继续比较下个字符
                if (c1 == c2)
                    continue;
                // 如果不相同，则比较完毕，否则继续
                else
                    break;
            } while (--len);
        }

        return (int32)(c1) - (int32)(c2);
    }

    /**
    * \brief 字符串比较 不区分大小写
    * \param src 源字符串
    * \param dest 目标字符串
    * \param len 比较长度
    * \return =0(src=dest),>0(src>dest),<0(src<dest)
    */
    int32 StrUtil::strnicmp(const char *src, const char *dest, size_t len)
    {
        unsigned char c1, c2;
        c1 = 0; c2 = 0;

        if (0 != len)
        {
            do 
            {
                c1 = *src; c2 = *dest;
                src++; dest++;

                // 是否已到字符串的末尾或两字符串是否有空串，如果到了末尾或有空串，则比较结束
                if (!c1) break;
                if (!c2) break;

                // 如果没有，且字符串相等，则继续比较下个字符
                if (c1 == c2)
                    continue;

                // 如果不相同，则同时转换成小写字符再进行比较
                c1 = tolower(c1);
                c2 = tolower(c2);

                // 如果不相同，则比较完毕，否则继续
                if (c1 != c2)
                    break;
            } while (--len);
        }

        return (int32)(c1) - (int32)(c2);
    }
    
    /**
    * \brief 字符串比较 区分大小写
    * \param src 源字符串
    * \param dest 目标字符串
    * \param len 比较长度
    * \return =0(src=dest),>0(src>dest),<0(src<dest)
    */
    int32 StrUtil::wstrncmp(const wchar_t *src, const wchar_t *dest, size_t len)
    {
        wchar_t c1, c2;
        c1 = 0; c2 = 0;

        if (0 != len)
        {
            do 
            {
                c1 = *src; c2 = *dest;
                src++; dest++;

                // 是否已到字符串的末尾或两字符串是否有空串，如果到了末尾或有空串，则比较结束
                if (!c1) break;
                if (!c2) break;

                // 如果没有，且字符串相等，则继续比较下个字符
                if (c1 == c2)
                    continue;
                // 如果不相同，则比较完毕，否则继续
                else
                    break;
            } while (--len);
        }

        return (int32)(c1) - (int32)(c2);
    }

    /**
    * \brief 字符串比较 不区分大小写
    * \param src 源字符串
    * \param dest 目标字符串
    * \param len 比较长度
    * \return =0(src=dest),>0(src>dest),<0(src<dest)
    */
    int32 StrUtil::wstrnicmp(const wchar_t *src, const wchar_t *dest, size_t len)
    {
        wchar_t c1, c2;
        c1 = 0; c2 = 0;

        if (0 != len)
        {
            do 
            {
                c1 = *src; c2 = *dest;
                src++; dest++;

                // 是否已到字符串的末尾或两字符串是否有空串，如果到了末尾或有空串，则比较结束
                if (!c1) break;
                if (!c2) break;

                // 如果没有，且字符串相等，则继续比较下个字符
                if (c1 == c2)
                    continue;

                // 如果不相同，则同时转换成小写字符再进行比较
                c1 = towlower(c1);
                c2 = towlower(c2);

                // 如果不相同，则比较完毕，否则继续
                if (c1 != c2)
                    break;
            } while (--len);
        }

        return (int32)(c1) - (int32)(c2);
    }

    /**
    * \brief 字符串分割
    * \param str 源字符串 分割后返回分割后的字符
    * \param delim 分割符
    * \return 分割的第一个字符串
    */
    char * StrUtil::strtoken(char **str, const char *delim)
    {
        char *begin = *str;
        if (NULL == begin)
        {
            return NULL;
        }

        char *end = strpbrk(begin, delim);
        if (end)
        {
            *end++ = '\0';
        }

        *str = end;

        return begin;
    }

    /**
    * \brief 字符串分割
    * \param wstr 源字符串 分割后返回分割后的字符
    * \param delim 分割符
    * \return 分割的第一个字符串
    */
    wchar_t * StrUtil::wstrtoken(wchar_t **wstr, const wchar_t *delim)
    {
        wchar_t *begin = *wstr;
        if (NULL == begin)
        {
            return NULL;
        }

        wchar_t *end = wcspbrk(begin, delim);
        if (end)
        {
            *end++ = '\0';
        }

        *wstr = end;

        return begin;
    }

    /**
    * \brief 字符串是否为空
    * \param str 字符串
    * \return 空返回true，非空返回false
    */
    bool StrUtil::strempty(const std::string &str)
    {
        if (0 == strcmp(str.c_str(), ""))
        {
            return true;
        }

        return false;
    }

    /**
    * \brief 字符串是否为空
    * \param str 字符串
    * \return 空返回true，非空返回false
    */
    bool StrUtil::strempty(const char *str)
    {
        if (0 == strcmp(str, ""))
        {
            return true;
        }

        return false;
    }

    /**
    * \brief 字符串是否为空
    * \param wstr 字符串
    * \return 空返回true，非空返回false
    */
    bool StrUtil::wstrempty(const std::wstring &wstr)
    {
        if (0 == wcscmp(wstr.c_str(), L""))
        {
            return true;
        }

        return false;
    }

    /**
    * \brief 字符串是否为空
    * \param wstr 字符串
    * \return 空返回true，非空返回false
    */
    bool StrUtil::wstrempty(const wchar_t *wstr)
    {
        if (0 == wcscmp(wstr, L""))
        {
            return true;
        }

        return false;
    }

    /**
    * \brief 字符串安全拷贝
    * \param dest 目标字符串
    * \param size 目标字符串长度
    * \param src 源字符串
    * \return 拷贝结果
    */
    char * StrUtil::strncpy(char *dest, size_t size, std::string &src)
    {
        if (0 == size)
        {
            dest[0] = '\0';
            return NULL;
        }

#ifdef WIN32
        size_t len = min(size-1, src.length());
#else
        size_t len = std::min(size-1, src.length());
#endif

        if (0 == len)
        {
            dest[0] = '\0';
            return NULL;
        }

        std::strncpy(dest, src.c_str(), len);

        dest[len] = '\0';

        return dest;
    }

    /**
    * \brief 字符串安全拷贝
    * \param dest 目标字符串
    * \param size 目标字符串长度
    * \param src 源字符串
    * \return 拷贝结果
    */
    char * StrUtil::strncpy(char *dest, size_t size, const char *src)
    {
        if (0 == size)
        {
            dest[0] = '\0';
            return NULL;
        }

#ifdef WIN32
        size_t len = min(size-1, strlen(src));
#else
        size_t len = std::min(size-1, strlen(src));
#endif

        if (0 == len)
        {
            dest[0] = '\0';
            return NULL;
        }

        std::strncpy(dest, src, len);

        dest[len] = '\0';

        return dest;
    }

    /**
    * \brief string 转换成 int32
    * \param str 字符串
    * \param value 默认返回值
    * \return 转换值
    */
    int32 StrUtil::strtoi32(const std::string &str, int32 value /*= 0*/)
    {
        return strtoi32(str.c_str(), value);
    }

    /**
    * \brief string 转换成 int32
    * \param str 字符串
    * \param value 默认返回值
    * \return 转换值
    */
    int32 StrUtil::strtoi32(const char *str, int32 value /*= 0*/)
    {
        if (NULL == str)
        {
            return value;
        }

        long res = 0;
        res = strtol(str, NULL, 10);
        if (0 == res || '0' == str[0])
        {
            return value;
        }

        return (int32)(res);     // 实现strtoi32
    }

    /**
    * \brief wstring 转换成 int32
    * \param wstr 字符串
    * \param value 默认返回值
    * \return 转换值
    */
    int32 StrUtil::wstrtoi32(const std::wstring &wstr, int32 value /*= 0*/)
    {
        return wstrtoi32(wstr.c_str(), value);
    }

    /**
    * \brief wstring 转换成 int32
    * \param wstr 字符串
    * \param value 默认返回值
    * \return 转换值
    */
    int32 StrUtil::wstrtoi32(const wchar_t *wstr, int32 value /*= 0*/)
    {
        if (NULL == wstr)
        {
            return value;
        }

        long res = 0;
        res = wcstol(wstr, NULL, 10);
        if (0 == res || L'0' == wstr[0])
        {
            return value;
        }

        return (int32)(res);     // 实现wstrtoi32
    }

    /**
    * \brief int32 转换成 string
    * \param value long值
    * \return 转换值
    */
    std::string StrUtil::i32tostr(int32 value)
    {
        char buf[20];
        sprintf(buf, "%d", value);
        return buf;     // 实现i32tostr
    }

    /**
    * \brief int32 转换成 wstring
    * \param value long值
    * \return 转换值
    */
    std::wstring StrUtil::i32towstr(int32 value)
    {
        wchar_t buf[20];
        swprintf(buf, sizeof(buf), L"%d", value);
        return buf;     // 实现i32towstr
    }

    /**
    * \brief string 转换成 int64
    * \param str 字符串
    * \param value 默认返回值
    * \return 转换值
    */
    int64 StrUtil::strtoi64(const std::string &str, int64 value /*= 0*/)
    {
        return strtoi64(str.c_str(), value);
    }

    /**
    * \brief string 转换成 int64
    * \param str 字符串
    * \param value 默认返回值
    * \return 转换值
    */
    int64 StrUtil::strtoi64(const char *str, int64 value /*= 0*/)
    {
        if (NULL == str)
        {
            return value;
        }

        int64 res = 0;
        res = strtoll(str, NULL, 10);
        if (0 == res || '0' == str[0])
        {
            return value;
        }

        return res;     // 实现strtoi64
    }

    /**
    * \brief wstring 转换成 int64
    * \param wstr 字符串
    * \param value 默认返回值
    * \return 转换值
    */
    int64 StrUtil::wstrtoi64(const std::wstring &wstr, int64 value /*= 0*/)
    {
        return wstrtoi64(wstr.c_str(), value);
    }

    /**
    * \brief wstring 转换成 int64
    * \param wstr 字符串
    * \param value 默认返回值
    * \return 转换值
    */
    int64 StrUtil::wstrtoi64(const wchar_t *wstr, int64 value /*= 0*/)
    {
        if (NULL == wstr)
        {
            return value;
        }

        int64 res = 0;
        res = wcstoll(wstr, NULL, 10);
        if (0 == res || L'0' == wstr[0])
        {
            return value;
        }

        return res;     // 实现wstrtoi64
    }

    /**
    * \brief int64 转换成 string
    * \param value long值
    * \return 转换值
    */
    std::string StrUtil::i64tostr(int64 value)
    {
        char buf[20];
        sprintf(buf, "%lld", value);
        return buf;     // 实现longtostr
    }

    /**
    * \brief int64 转换成 wstring
    * \param value long值
    * \return 转换值
    */
    std::wstring StrUtil::i64towstr(int64 value)
    {
        wchar_t buf[20];
        swprintf(buf, sizeof(buf), L"%lld", value);
        return buf;     // 实现longtowstr
    }

    /**
    * \brief string 转换成 double
    * \param str 字符串
    * \param value 默认返回值
    * \return 转换值
    */
    double StrUtil::strtodouble(const std::string &str, double value /*= 0*/)
    {
        return strtodouble(str.c_str(), value);
    }

    /**
    * \brief string 转换成 double
    * \param str 字符串
    * \param value 默认返回值
    * \return 转换值
    */
    double StrUtil::strtodouble(const char *str, double value /*= 0*/)
    {
        if (NULL == str)
        {
            return value;
        }

        double res = 0;
        size_t len = strlen(str);
        if (0 == len)
        {
            return value;
        }

        char *end = (char*)&str[len-1];
        res = strtod(str, &end);
        if (0 == res || '0' == str[0])
        {
            return value;
        }

        return res;     // 实现strtodouble
    }

    /**
    * \brief wstring 转换成 double
    * \param wstr 字符串
    * \param value 默认返回值
    * \return 转换值
    */
    double StrUtil::wstrtodouble(const std::wstring &wstr, double value /*= 0*/)
    {
        return wstrtodouble(wstr.c_str(), value);
    }

    /**
    * \brief wstring 转换成 long
    * \param wstr 字符串
    * \param value 默认返回值
    * \return 转换值
    */
    double StrUtil::wstrtodouble(const wchar_t *wstr, double value /*= 0*/)
    {
        if (NULL == wstr)
        {
            return value;
        }

        double res = 0;
        size_t len = wcslen(wstr);
        if (0 == len)
        {
            return value;
        }

        wchar_t *end = (wchar_t*)&wstr[len-1];
        res = wcstod(wstr, &end);
        if (0 == res || L'0' == wstr[0])
        {
            return value;
        }

        return res;     // 实现wstrtodouble
    }

    /**
    * \brief double 转换成 string
    * \param value double值
    * \return 转换值
    */
    std::string StrUtil::doubletostr(double value)
    {
        char buf[25];
        sprintf(buf, "%f", value);
        return buf;     // 实现doubletostr
    }

    /**
    * \brief double 转换成 wstring
    * \param value double值
    * \return 转换值
    */
    std::wstring StrUtil::doubletowstr(double value)
    {
        wchar_t buf[20];
        swprintf(buf, sizeof(buf), L"%f", value);
        return buf;     // 实现doubletowstr
    }

    /**
    * \brief string 转换成 bool
    * \param str 字符串
    * \param value 默认返回值
    * \return 转换值
    */
    bool StrUtil::strtobool(const std::string &str, bool value /*= false*/)
    {
        return strtobool(str.c_str(), value);
    }

    /**
    * \brief string 转换成 bool
    * \param str 字符串
    * \param value 默认返回值
    * \return 转换值
    */
    bool StrUtil::strtobool(const char *str, bool value /*= false*/)
    {
        size_t len = strlen(str);
        if (0 == strnicmp("true", str, len))
        {
            return true;
        }

        if (0 == strnicmp("false", str, len))
        {
            return false;
        }

        return value;
    }

    /**
    * \brief wstring 转换成 double
    * \param wstr 字符串
    * \param value 默认返回值
    * \return 转换值
    */
    bool StrUtil::wstrtobool(const std::wstring &wstr, bool value /*= false*/)
    {
        return wstrtobool(wstr.c_str(), value);
    }

    /**
    * \brief wstring 转换成 long
    * \param wstr 字符串
    * \param value 默认返回值
    * \return 转换值
    */
    bool StrUtil::wstrtobool(const wchar_t *wstr, bool value /*= false*/)
    {
        size_t len = wcslen(wstr);
        if (0 == wstrnicmp(L"true", wstr, len))
        {
            return true;
        }

        if (0 == wstrnicmp(L"false", wstr, len))
        {
            return false;
        }

        return value;
    }

    /**
    * \brief bool 转换成 string
    * \param value bool值
    * \return 转换值
    */
    std::string StrUtil::booltostr(bool value)
    {
        return value ? "true" : "false";
    }

    /**
    * \brief bool 转换成 wstring
    * \param value bool值
    * \return 转换值
    */
    std::wstring StrUtil::booltowstr(bool value)
    {
        return value ? L"true" : L"false";
    }

    /**
    * \brief string 转换成 time_t
    * \param str 字符串
    * \param value 默认返回值
    * \return 转换值，失败返回0
    */
    time_t StrUtil::strtotime(const std::string &str, time_t value /*= 0*/)
    {
#ifdef WIN32
        struct tm stm;
        bool getdate = false;   //匹配日期标志
        std::smatch wsm;
        std::string timestr;

        // 获取日期结构
        if (!strdatetotm(str, wsm, stm))
        {
            // 没有日期结构，在这里处理日期，使用当前日期赋值
            struct tm tdate = TimeUtil::TimeToTm(TimeUtil::Now());
            stm.tm_year = tdate.tm_year;
            stm.tm_mon = tdate.tm_mon;
            stm.tm_mday = tdate.tm_mday;
            timestr = str;
        }
        else
        {
            getdate = true;
            size_t sublen = wsm.length();
            if (sublen < str.length())
                timestr = str.substr(sublen + 1);
            else
                timestr = "";
        }

        // 获取时间结构
        if (!strtimetotm(timestr, wsm, stm))
        {
            if (getdate)
            {
                // 没有时间结构，在这里处理，使用0处理
                stm.tm_hour = 0;
                stm.tm_min = 0;
                stm.tm_sec = 0;
            }
            else
            {
                // 日期和时间都没匹配成功，直接返回0或抛出异常
                return value;
            }
        }

        return mktime(&stm);    // 实现strtotime
#else
        // linux下已经实现字符串转时间函数
        struct tm tt;
        if (NULL == strptime(str.c_str(), "%Y-%m-%d %H:%M:%S", &tt)
            && NULL == strptime(str.c_str(), "%m//%d//%Y %H:%M:%S", &tt))
        {
            return value;
        }

        //struct tm stm;
        //bool getdate = false;   //匹配日期标志
        //std::string wsm;
        //std::string timestr;

        //// 获取日期结构
        //if (!strdatetotm(str, wsm, stm))
        //{
        //    // 没有日期结构，在这里处理日期，使用当前日期赋值
        //    time_t now = time(NULL);
        //    struct tm tdate = *localtime(&now);
        //    stm.tm_year = tdate.tm_year;
        //    stm.tm_mon = tdate.tm_mon;
        //    stm.tm_mday = tdate.tm_mday;
        //    timestr = str;
        //}
        //else
        //{
        //    getdate = true;
        //    size_t sublen = wsm.length();
        //    if (sublen < str.length())
        //        timestr = str.substr(sublen + 1);
        //    else
        //        timestr = "";
        //}

        //// 获取时间结构
        //if (!strtimetotm(timestr, wsm, stm))
        //{
        //    if (getdate)
        //    {
        //        // 没有时间结构，在这里处理，使用0处理
        //        stm.tm_hour = 0;
        //        stm.tm_min = 0;
        //        stm.tm_sec = 0;
        //    }
        //    else
        //    {
        //        // 日期和时间都没匹配成功，直接返回0或抛出异常
        //        return value;
        //    }
        //}

        return mktime(&tt);    // 实现strtotime
#endif
    }

    /**
    * \brief string 转换成 time_t
    * \param str 字符串
    * \param value 默认返回值
    * \return 转换值，失败返回0
    */
    time_t StrUtil::strtotime(const char *str, time_t value /*= 0*/)
    {
        if (NULL == str)
        {
            return value;
        }

        std::string timestr(str);
        return strtotime(timestr, value);
    }

    /**
    * \brief wstring 转换成 time_t
    * \param wstr 字符串
    * \param value 默认返回值
    * \return 转换值，失败返回0
    */
    time_t StrUtil::wstrtotime(const std::wstring &wstr, time_t value /*= 0*/)
    {
#ifdef WIN32
        struct tm stm;
        bool getdate = false;   //匹配日期标志
        std::wsmatch wsm;
        std::wstring timewstr;

        // 获取日期结构
        if (!wstrdatetotm(wstr, wsm, stm))
        {
            // 没有日期结构，在这里处理日期，使用当前日期赋值
            time_t now = time(NULL);
            struct tm tdate = *localtime(&now);
            stm.tm_year = tdate.tm_year;
            stm.tm_mon = tdate.tm_mon;
            stm.tm_mday = tdate.tm_mday;
            timewstr = wstr;
        }
        else
        {
            getdate = true;
            size_t sublen = wsm.length();
            if (sublen < wstr.length())
                timewstr = wstr.substr(sublen + 1);
            else
                timewstr = L"";
        }

        // 获取时间结构
        if (!wstrtimetotm(timewstr, wsm, stm))
        {
            if (getdate)
            {
                // 没有时间结构，在这里处理，使用0处理
                stm.tm_hour = 0;
                stm.tm_min = 0;
                stm.tm_sec = 0;
            }
            else
            {
                // 日期和时间都没匹配成功，直接返回0或抛出异常
                return value;
            }
        }

        return mktime(&stm);    // 实现wstrtotime
#else
        struct tm stm;
        bool getdate = false;   //匹配日期标志
        std::wstring wsm;
        std::wstring timewstr;

        // 获取日期结构
        if (!wstrdatetotm(wstr, wsm, stm))
        {
            // 没有日期结构，在这里处理日期，使用当前日期赋值
            time_t now = time(NULL);
            struct tm tdate = *localtime(&now);
            stm.tm_year = tdate.tm_year;
            stm.tm_mon = tdate.tm_mon;
            stm.tm_mday = tdate.tm_mday;
            timewstr = wstr;
        }
        else
        {
            getdate = true;
            size_t sublen = wsm.length();
            if (sublen < wstr.length())
                timewstr = wstr.substr(sublen + 1);
            else
                timewstr = L"";
        }

        // 获取时间结构
        if (!wstrtimetotm(timewstr, wsm, stm))
        {
            if (getdate)
            {
                // 没有时间结构，在这里处理，使用0处理
                stm.tm_hour = 0;
                stm.tm_min = 0;
                stm.tm_sec = 0;
            }
            else
            {
                // 日期和时间都没匹配成功，直接返回0或抛出异常
                return value;
            }
        }

        return mktime(&stm);    // 实现wstrtotime
#endif
    }

    /**
    * \brief wstring 转换成 time_t
    * \param wstr 字符串
    * \param value 默认返回值
    * \return 转换值，失败返回0
    */
    time_t StrUtil::wstrtotime(const wchar_t *wstr, time_t value /*= 0*/)
    {
        if (NULL == wstr)
        {
            return value;
        }

        std::wstring wtimestr(wstr);
        return wstrtotime(wtimestr, value);
    }

    /**
    * \brief time_t 转换成 string
    * \param value time_t值
    * \return 转换值
    */
    std::string StrUtil::timetostr(time_t value)
    {
        const size_t TIME_STR_LEN = 25;
        char buf[TIME_STR_LEN];

        // 获取本地时间，并保存到ttime中，减少localtime的线程风险
        struct tm ttime = *localtime(&value);

        // 使用自己定义的格式输出
        strftime(buf, TIME_STR_LEN, "%Y-%m-%d %H:%M:%S", &ttime);
        return buf;     //实现timetostr
    }

    /**
    * \brief time_t 转换成 wstring
    * \param value time_t值
    * \return 转换值
    */
    std::wstring StrUtil::timetowstr(time_t value)
    {
        const size_t TIME_STR_LEN = 25;
        wchar_t buf[TIME_STR_LEN];

        // 获取本地时间，并保存到ttime中，减少localtime的线程风险
        struct tm ttime = *localtime(&value);

        // 使用自己定义的格式输出
        wcsftime(buf, TIME_STR_LEN, L"%Y-%m-%d %H:%M:%S", &ttime);
        return buf;     //实现timetowstr
    }

    /**
    * \brief 删除 string 左空格
    * \param str 字符串
    * \return 删除左空格后的字符串
    */
    std::string StrUtil::ltrim(const std::string &str)
    {
        std::string res;
        if (str.empty())
        {
            return res;
        }

        std::basic_string<char>::const_iterator it = str.begin();
        while (it != str.end())
        {
            if ( ' ' != (*it) )
                break;
            it++;
        }

        res.append(it, str.end());

        return res;
    }

    /**
    * \brief 删除 string 右空格
    * \param str 字符串
    * \return 删除右空格后的字符串
    */
    std::string StrUtil::rtrim(const std::string &str)
    {
        std::string res;
        if (str.empty())
        {
            return res;
        }

        std::basic_string<char>::const_iterator it = str.end();
        it--;

        for (; it>=str.begin(); it--)
        {
            if ( ' ' != (*it) )
                break;
        }
        it++;

        res.append(str.begin(), it);

        return res;
    }

    /**
    * \brief 删除 string 左右空格
    * \param str 字符串
    * \return 删除左右空格后的字符串
    */
    std::string StrUtil::trim(const std::string &str)
    {
        return ltrim(rtrim(str));
    }

    /**
    * \brief 删除 wstring 左空格
    * \param wstr 字符串
    * \return 删除左空格后的字符串
    */
    std::wstring StrUtil::wltrim(const std::wstring &wstr)
    {
        std::wstring res;
        if (wstr.empty())
        {
            return res;
        }

        std::basic_string<wchar_t>::const_iterator it = wstr.begin();
        while (it != wstr.end())
        {
            if ( L' ' != (*it) )
                break;
            it++;
        }

        res.append(it, wstr.end());

        return res;
    }

    /**
    * \brief 删除 wstring 右空格
    * \param wstr 字符串
    * \return 删除右空格后的字符串
    */
    std::wstring StrUtil::wrtrim(const std::wstring &wstr)
    {
        std::wstring res;
        if (wstr.empty())
        {
            return res;
        }

        std::basic_string<wchar_t>::const_iterator it = wstr.end();
        it--;

        for (; it>=wstr.begin(); it--)
        {
            if ( L' ' != (*it) )
                break;
        }
        it++;

        res.append(wstr.begin(), it);

        return res;
    }

    /**
    * \brief 删除 wstring 左右空格
    * \param wstr 字符串
    * \return 删除左右空格后的字符串
    */
    std::wstring StrUtil::wtrim(const std::wstring &wstr)
    {
        return wltrim(wrtrim(wstr));
    }

    /**
    * \brief 删除 string 指定字符
    * \param str 字符串
    * \param ext 删除字符串
    */
    void StrUtil::trimex(std::string &str, std::string ext /*= std::string()*/)
    {
        if (str.empty())
        {
            return;
        }

        size_t len = str.length();
        size_t begin = 0;
        size_t end = 0;

        for (size_t i = 0; i < len; i++)
        {
            bool check = false;
            for (size_t j = 0; j < ext.length(); j++)
            {
                if (str[i] == ext[j])
                {
                    check = true;
                }
            }

            if (check)
            {
                if (i == begin)
                {
                    begin ++;
                }
            }
            else
            {
                end = i + 1;
            }
        }

        if (begin < end)
        {
            str = str.substr(begin, end - begin);
        }
        else
        {
            str.clear();
        }
    }

    /**
    * \brief 删除 wstring 指定字符
    * \param wstr 字符串
    * \param wext 删除字符串
    */
    void StrUtil::wtrimex(std::wstring &wstr, std::wstring wext /*= std::wstring()*/)
    {
        if (wstr.empty())
        {
            return;
        }

        size_t len = wstr.length();
        size_t begin = 0;
        size_t end = 0;

        for (size_t i = 0; i < len; i++)
        {
            bool check = false;
            for (size_t j = 0; j < wext.length(); j++)
            {
                if (wstr[i] == wext[j])
                {
                    check = true;
                }
            }

            if (check)
            {
                if (i == begin)
                {
                    begin ++;
                }
            }
            else
            {
                end = i + 1;
            }
        }

        if (begin < end)
        {
            wstr = wstr.substr(begin, end - begin);
        }
        else
        {
            wstr.clear();
        }
    }

    /**
    * \brief string 转换成 int32 数组
    * \param str 字符串
    * \param vec 转换后的int32数组
    * \param step 字符串分割符
    * \return 转换后的int32数组大小
    */
    size_t StrUtil::strtoi32arr(const std::string &str, std::vector<int32> &vec, const char step /*= ' '*/)
    {
        return strtoi32arr(str.c_str(), vec, step);
    }

    /**
    * \brief string 转换成 int32 数组
    * \param str 字符串
    * \param vec 转换后的int32数组
    * \param step 字符串分割符
    * \return 转换后的int32数组大小
    */
    size_t StrUtil::strtoi32arr(const char *str, std::vector<int32> &vec, const char step /*= ' '*/)
    {
        if (NULL == str)
        {
            return 0;
        }

        vec.clear();

        size_t len = strlen(str);
        if (0 == len)
        {
            return 0;
        }

        // strtoken 这个函数会修改回来字符串的内容
        char *tmps = new char[len + 1];
        memset(tmps, 0, sizeof(char)*(len+1));
        std::strncpy(tmps, str, len);
        tmps[len] = '\0';

        char *tmp = tmps;

        char *token = strtoken(&tmp, &step);
        while (NULL != token)
        {
            vec.push_back( strtoi32(token) );
            token = strtoken(&tmp, &step);
        }

        if (NULL != tmps)
        {
            delete [] tmps;
            tmps = NULL;
        }

        return vec.size();
    }

    /**
    * \brief wstring 转换成 int32 数组
    * \param wstr 字符串
    * \param vec 转换后的int32数组
    * \param step 字符串分割符
    * \return 转换后的int32数组大小
    */
    size_t StrUtil::wstrtoi32arr(const std::wstring &wstr, std::vector<int32> &vec, const wchar_t step /*= L' '*/)
    {
        return wstrtoi32arr(wstr.c_str(), vec, step);
    }

    /**
    * \brief wstring 转换成 int32 数组
    * \param wstr 字符串
    * \param vec 转换后的int32数组
    * \param step 字符串分割符
    * \return 转换后的int32数组大小
    */
    size_t StrUtil::wstrtoi32arr(const wchar_t *wstr, std::vector<int32> &vec, const wchar_t step /*= L' '*/)
    {
        if (NULL == wstr)
        {
            return 0;
        }

        vec.clear();

        size_t len = wcslen(wstr);
        if (0 == len)
        {
            return 0;
        }

        // strtoken 这个函数会修改回来字符串的内容
        wchar_t *tmps = new wchar_t[len + 1];
        memset(tmps, 0, sizeof(wchar_t)*(len+1));
        wcsncpy(tmps, wstr, len);
        tmps[len] = L'\0';

        wchar_t *tmp = tmps;

        wchar_t *token = wstrtoken(&tmp, &step);
        while (NULL != token)
        {
            vec.push_back( wstrtoi32(token) );
            token = wstrtoken(&tmp, &step);
        }

        if (NULL != tmps)
        {
            delete [] tmps;
            tmps = NULL;
        }

        return vec.size();
    }

    /**
    * \brief string 转换成 int64 数组
    * \param str 字符串
    * \param vec 转换后的int64数组
    * \param step 字符串分割符
    * \return 转换后的int64数组大小
    */
    size_t StrUtil::strtoi64arr(const std::string &str, std::vector<int64> &vec, const char step /*= ' '*/)
    {
        return strtoi64arr(str.c_str(), vec, step);
    }

    /**
    * \brief string 转换成 int64 数组
    * \param str 字符串
    * \param vec 转换后的int64数组
    * \param step 字符串分割符
    * \return 转换后的int64数组大小
    */
    size_t StrUtil::strtoi64arr(const char *str, std::vector<int64> &vec, const char step /*= ' '*/)
    {
        if (NULL == str)
        {
            return 0;
        }

        vec.clear();

        size_t len = strlen(str);
        if (0 == len)
        {
            return 0;
        }

        // strtoken 这个函数会修改回来字符串的内容
        char *tmps = new char[len + 1];
        memset(tmps, 0, sizeof(char)*(len+1));
        std::strncpy(tmps, str, len);
        tmps[len] = '\0';

        char *tmp = tmps;

        char *token = strtoken(&tmp, &step);
        while (NULL != token)
        {
            vec.push_back( strtoi64(token) );
            token = strtoken(&tmp, &step);
        }

        if (NULL != tmps)
        {
            delete [] tmps;
            tmps = NULL;
        }

        return vec.size();
    }

    /**
    * \brief wstring 转换成 int64 数组
    * \param wstr 字符串
    * \param vec 转换后的int64数组
    * \param step 字符串分割符
    * \return 转换后的int64数组大小
    */
    size_t StrUtil::wstrtoi64arr(const std::wstring &wstr, std::vector<int64> &vec, const wchar_t step /*= L' '*/)
    {
        return wstrtoi64arr(wstr.c_str(), vec, step);
    }

    /**
    * \brief wstring 转换成 int64 数组
    * \param wstr 字符串
    * \param vec 转换后的int64数组
    * \param step 字符串分割符
    * \return 转换后的int64数组大小
    */
    size_t StrUtil::wstrtoi64arr(const wchar_t *wstr, std::vector<int64> &vec, const wchar_t step /*= L' '*/)
    {
        if (NULL == wstr)
        {
            return 0;
        }

        vec.clear();

        size_t len = wcslen(wstr);
        if (0 == len)
        {
            return 0;
        }

        // strtoken 这个函数会修改回来字符串的内容
        wchar_t *tmps = new wchar_t[len + 1];
        memset(tmps, 0, sizeof(wchar_t)*(len+1));
        wcsncpy(tmps, wstr, len);
        tmps[len] = L'\0';

        wchar_t *tmp = tmps;

        wchar_t *token = wstrtoken(&tmp, &step);
        while (NULL != token)
        {
            vec.push_back( wstrtoi64(token) );
            token = wstrtoken(&tmp, &step);
        }

        if (NULL != tmps)
        {
            delete [] tmps;
            tmps = NULL;
        }

        return vec.size();
    }

    /**
    * \brief string 转换成 string 数组
    * \param str 字符串
    * \param vec 转换后的string数组
    * \param step 字符串分割符
    * \return 转换后的string数组大小
    */
    size_t StrUtil::strtostrarr(const std::string &str, std::vector<std::string> &vec, const char step /*= ' '*/)
    {
        return strtostrarr(str.c_str(), vec, step);
    }

    /**
    * \brief string 转换成 string 数组
    * \param str 字符串
    * \param vec 转换后的string数组
    * \param step 字符串分割符
    * \return 转换后的string数组大小
    */
    size_t StrUtil::strtostrarr(const char *str, std::vector<std::string> &vec, const char step /*= ' '*/)
    {
        if (NULL == str)
        {
            return 0;
        }

        vec.clear();

        size_t len = strlen(str);
        if (0 == len)
        {
            return 0;
        }

        // strtoken 这个函数会修改回来字符串的内容
        char *tmps = new char[len + 1];
        memset(tmps, 0, sizeof(char)*(len+1));
        std::strncpy(tmps, str, len);
        tmps[len] = '\0';

        char *tmp = tmps;

        char *token = strtoken(&tmp, &step);
        while (NULL != token)
        {
            vec.push_back( token );
            token = strtoken(&tmp, &step);
        }

        if (NULL != tmps)
        {
            delete [] tmps;
            tmps = NULL;
        }

        return vec.size();
    }

    /**
    * \brief wstring 转换成 wstring 数组
    * \param wstr 字符串
    * \param vec 转换后的wstring数组
    * \param step 字符串分割符
    * \return 转换后的wstring数组大小
    */
    size_t StrUtil::wstrtowstrarr(const std::wstring &wstr, std::vector<std::wstring> &vec, const wchar_t step /*= L' '*/)
    {
        return wstrtowstrarr(wstr.c_str(), vec, step);
    }

    /**
    * \brief wstring 转换成 wstring 数组
    * \param wstr 字符串
    * \param vec 转换后的wstring数组
    * \param step 字符串分割符
    * \return 转换后的wstring数组大小
    */
    size_t StrUtil::wstrtowstrarr(const wchar_t *wstr, std::vector<std::wstring> &vec, const wchar_t step /*= L' '*/)
    {
        if (NULL == wstr)
        {
            return 0;
        }

        vec.clear();

        size_t len = wcslen(wstr);
        if (0 == len)
        {
            return 0;
        }

        // strtoken 这个函数会修改回来字符串的内容
        wchar_t *tmps = new wchar_t[len + 1];
        memset(tmps, 0, sizeof(wchar_t)*(len+1));
        wcsncpy(tmps, wstr, len);
        tmps[len] = L'\0';

        wchar_t *tmp = tmps;

        wchar_t *token = wstrtoken(&tmp, &step);
        while (NULL != token)
        {
            vec.push_back( token );
            token = wstrtoken(&tmp, &step);
        }

        if (NULL != tmps)
        {
            delete [] tmps;
            tmps = NULL;
        }

        return vec.size();
    }

    /**
    * \brief 字符串替换
    * \param str 输出字符串
    * \param format 格式，参数格式{0}{1}
    * \param vec 替换字符串组
    * \return 成功返回true，否则返回false
    */
    bool StrUtil::strreplace(std::string &str, const char *format, std::vector<std::string> &vec)
    {
        std::string fstr(format);

        for (size_t i = 0; i < vec.size(); i++)
        {
            std::string key = "{" + i32tostr((int32)(i)) + "}";
            for (std::string::size_type pos(0); pos != std::string::npos; pos += vec[i].length())
            {
                pos = fstr.find(key, pos);
                if (std::string::npos != pos)
                {
                    break;
                }

                fstr.replace(pos, key.length(), vec[i]);
            }
        }

        str = fstr;

        return true;
    }

    /**
    * \brief 字符串替换
    * \param str 输出字符串
    * \param format 格式，参数格式{0}{1}
    * \param vec 替换字符串组
    * \return 成功返回true，否则返回false
    */
    bool StrUtil::strreplace(char *str, const char *format, std::vector<std::string> &vec)
    {
        std::string fstr(format);

        for (size_t i = 0; i < vec.size(); i++)
        {
            std::string key = "{" + i32tostr((int32)(i)) + "}";
            for (std::string::size_type pos(0); pos != std::string::npos; pos += vec[i].length())
            {
                pos = fstr.find(key, pos);
                if (std::string::npos != pos)
                {
                    break;
                }

                fstr.replace(pos, key.length(), vec[i]);
            }
        }

        size_t len = fstr.length();
        std::strncpy(str, fstr.c_str(), len);
        str[len] = '\0';

        return true;
    }
}