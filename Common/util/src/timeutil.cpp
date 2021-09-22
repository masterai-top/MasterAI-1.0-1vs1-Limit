/**
* \file timeutil.cpp
* \brief 时间辅助类实现代码
*/

#include "pch.h"
#include "timeutil.h"


namespace dtutil
{
    int32 * TimeUtil::m_offset = NULL;       ///< 时间偏差地址，GM调整时间[时间偏差值存在上层应用中，这里记录上层应用中的位置，修改的时候直接修改上层应用中的值]

    /**
    * \brief 获得当前时间(秒)
    * \return 当前时间
    */
    time_t TimeUtil::Now()
    {
        time_t tt = ::time(0);  // 获取1970年以来的秒数

        if (m_offset != NULL)
        {
            int32 val = *m_offset;
            if (val > 0)
            {
                tt += val;
            }
            else if (val < 0)
            {
                tt -= (time_t)(-1 * val);
            }
        }

        return tt;
    }

    /**
    * \brief 获得当前时间(毫秒)
    * \return 当前时间
    */
    time_t TimeUtil::NowMs()
    {
        // 获取当前时间
        timeval now;
        GetTimeVal(&now, NULL);

        if (m_offset != NULL)
        {
            int32 val = *m_offset;
            if (val > 0)
            {
                now.tv_sec += val;
            }
            else if (val < 0)
            {
                now.tv_sec -= (time_t)(-1 * val);
            }
        }

        return (now.tv_sec * 1000 + now.tv_usec / 1000);
    }

    /**
    * \brief 获得当前时间(微秒)
    * \return 当前时间
    */
    time_t TimeUtil::NowUs()
    {
        // 获取当前时间
        timeval now;
        GetTimeVal(&now, NULL);

        if (m_offset != NULL)
        {
            int32 val = *m_offset;
            if (val > 0)
            {
                now.tv_sec += val;
            }
            else if (val < 0)
            {
                now.tv_sec -= (time_t)(-1 * val);
            }
        }

        return (now.tv_sec * 1000 * 1000 + now.tv_usec);
    }

    /**
    * \brief 获得运行时间(毫秒)
    * \return 运行时间
    */
    uint32 TimeUtil::Tick()
    {
#ifdef WIN32
        return ::GetTickCount();
#else
        struct timespec ts;

        clock_gettime(CLOCK_MONOTONIC, &ts);    // 此处可以判断下返回值

        return (ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000));
#endif
    }

    /**
    * \brief 获得当前时间(微秒)
    * \param tv 保存获取时间结果的结构体
    * \param tz 用于保存时区结果 一般都用NULL(linux下无效)
    * \return 获取结果 =0(成功) <0(失败)
    */
    int32 TimeUtil::GetTimeVal(struct timeval *tv, struct timezone *tz)
    {
#ifdef WIN32
        union
        {
            int64       ns100;
            FILETIME    ft;
        } now;

        GetSystemTimeAsFileTime(&now.ft);
        tv->tv_usec = (long) ((now.ns100 / 10LL) % 1000000LL);
        tv->tv_sec = (long) ((now.ns100 - 116444736000000000LL) / 10000000LL);

        return 0;
#else
        int32 rs = gettimeofday(tv, tz);
        
        return rs;
#endif
    }

    /**
    * \brief 计算延迟时间
    * \param secs 秒
    * \param msecs 毫秒
    * \param nsecs 纳秒
    * \return 从当前时间按参数延迟后的时间
    * 主要用于为一些现成阻塞函数产生等待超时参数
    */
    timespec TimeUtil::Delay(int32 secs, int32 msecs/* = 0*/, int32 nsecs /*= 0*/)
    {
        const int32 NANOSECS_PER_SEC = 1000 * 1000 * 1000;      // 秒(s) -> 纳秒(ns)
        const int32 NANOSECS_PER_MILLISEC = 1000 * 1000;        // 毫秒(ms) -> 纳秒(ns)
        const int32 NANOSECS_PER_MICROSEC = 1000;               // 微秒(us) -> 纳秒(ns)

        timespec xt;

        // 获取当前时间
        timeval now;
        GetTimeVal(&now, NULL);
        
        // 以当前纳秒时间计算超时纳秒
        nsecs += now.tv_usec * NANOSECS_PER_MICROSEC;
        nsecs += msecs * NANOSECS_PER_MILLISEC;

        // 计算延迟时间
        xt.tv_sec = now.tv_sec + secs + (nsecs / NANOSECS_PER_SEC);
        xt.tv_nsec = nsecs % NANOSECS_PER_SEC;

        return xt;
    }

    /**
    * \brief 时间转换
    * \param time 指定时间，以1970年以来的秒数
    * \return 当地时间
    */
    struct tm TimeUtil::TimeToTm(time_t time)
    {
#ifdef WIN32
#   if _MSC_VER < 1400     // vs2003
        return *localtime(&time);
#   else                   // vs2005 -> vs2013 ->
        struct tm tt = { 0 };
        localtime_s(&tt, &time);
        return tt;
#   endif
#else
        struct tm tt = { 0 };
        localtime_r(&time, &tt);
        return tt;
#endif
    }

    /**
    * \brief 获得月初时间
    * \param time 指定时间，以1970年以来的秒数
    * \return 月初时间，以1970年以来的秒数
    */
    time_t TimeUtil::MonthStartTime(time_t time)
    {
        if (0 == time)
        {
            time = Now();
        }

        struct tm tt = TimeToTm(time);

        tt.tm_mday = 1;
        tt.tm_hour = 0;
        tt.tm_min = 0;
        tt.tm_sec = 0;

        return mktime(&tt);
    }

    /**
    * \brief 获得本周开始时间
    * \param time 指定时间，以1970年以来的秒数
    * \param week 周几，0-6(周日-周六)
    * \return 本周开始时间，以1970年以来的秒数
    */
    time_t TimeUtil::WeekStartTime(time_t time, int32 week /*= 0*/)
    {
        if (0 == time)
        {
            time = Now();
        }

        struct tm tt = TimeToTm(time);

        tt.tm_wday = week;
        tt.tm_hour = 0;
        tt.tm_min = 0;
        tt.tm_sec = 0;

        return mktime(&tt);
    }

    /**
    * \brief 获得日开始时间
    * \param time 指定时间，以1970年以来的秒数
    * \return 日开始时间，以1970年以来的秒数
    */
    time_t TimeUtil::DayStartTime(time_t time)
    {
        if (0 == time)
        {
            time = Now();
        }

        struct tm tt = TimeToTm(time);

        tt.tm_hour = 0;
        tt.tm_min = 0;
        tt.tm_sec = 0;

        return mktime(&tt);
    }
    
    /**
    * \brief 获得指定时间 以1970年1月1日0时0分0秒以来 的天数 
    * \param time 指定时间，以1970年以来的秒数
    * \return 指定时间计算得到的总天数
    */
    uint32 TimeUtil::GetDays(time_t time)
    {
        static uint32 loctz = 0;     // 本地时区秒数
        if (0 == loctz)
        {
            tm tt = TimeToTm(0);
            loctz = tt.tm_hour * 3600;
        }

        if (0 == time)
        {
            time = Now();
        }

        time += loctz;              // 本地时区偏移

        uint32 days = (uint32)(time / 86400);

        return days;
    }

    /**
    * \brief 获得指定时间 以1970年1月1日0时0分0秒以来 的小时数 
    * \param time 指定时间，以1970年以来的秒数
    * \return 指定时间计算得到的总小时数
    */
    uint32 TimeUtil::GetHours(time_t time)
    {
        if (0 == time)
        {
            time = Now();
        }

        uint32 hours = (uint32)(time / 3600);

        return hours;
    }

    /**
    * \brief 获得两个时间的相隔天数
    * \param time1 时间1，以1970年以来的秒数
    * \param time2 时间2，以1970年以来的秒数
    * \return 两个时间相隔的天数
    */
    uint32 TimeUtil::GetIntervalDay( time_t time1, time_t time2 )
    {
        uint32 day1 = GetDays(time1);
        uint32 day2 = GetDays(time2);

        return ( (day1 >= day2) ? (day1 - day2) : (day2 - day1) );
    }

    /**
    * \brief 获得指定时间之前的指定小时时间点 
    * \param time 指定时间，以1970年以来的秒数
    * \param hour 指定小时
    * \return 指定时间之前的小时时间点，以1970年以来的秒数
    *
    * 若指定时间的小时时间小于指定hour，则返回前一天的整点hour时间
    * 若指定时间的小时时间大于指定hour，则返回当天的整点hour时间
    */
    time_t TimeUtil::GetLastHour(time_t time, int32 hour)
    {
        if (0 == time)
        {
            time = Now();
        }

        struct tm when = TimeToTm(time);

        if (when.tm_hour < hour)
        {
            time -= 3600 * hour;
            when = TimeToTm(time);
        }

        when.tm_hour = hour;
        when.tm_min = 0;
        when.tm_sec = 0;

        return mktime(&when);
    }

    /**
    * \brief 获得指定时间之后的指定小时时间点 
    * \param time 指定时间，以1970年以来的秒数
    * \param hour 指定小时
    * \return 指定时间之后的小时时间点，以1970年以来的秒数
    *
    * 若指定时间的小时时间小于指定hour，则返回当天的整点hour时间
    * 若指定时间的小时时间大于指定hour，则返回下一天的整点hour时间
    */
    time_t TimeUtil::GetNextHour(time_t time, int32 hour)
    {
        if (0 == time)
        {
            time = Now();
        }

        struct tm when = TimeToTm(time);

        if (when.tm_hour > hour)
        {
            time += 3600 * hour;
            when = TimeToTm(time);
        }

        when.tm_hour = hour;
        when.tm_min = 0;
        when.tm_sec = 0;

        return mktime(&when);
    }

    /**
    * \brief 设置时间偏差
    * \param offset 时间偏差
    */
    void TimeUtil::SetTimeOffset(int32 offset)
    {
        if (NULL != m_offset)
        {
            *m_offset = offset;
        }
    }

    /**
    * \brief 获得时间偏差
    * \return 时间偏差
    */
    int32 TimeUtil::GetTimeOffset()
    {
        if (NULL != m_offset)
        {
            return *m_offset;
        }
        
        return 0;
    }

    /**
    * \brief 设置时间偏差地址
    * \param offset 时间偏差地址
    * 进程初始化时，需要指定时间偏差地址；
    * 如果没有指定时间偏差地址，则无法改变时间偏差。
    */
    void TimeUtil::SetTimeOffsetPtr(int32* offset)
    {
        m_offset = offset;
    }
}