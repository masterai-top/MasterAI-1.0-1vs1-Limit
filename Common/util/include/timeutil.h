/**
* \file timeutil.h
* \brief 时间辅助类
*
* 提供时间相关的函数。
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __TIME_UTIL_H__
#define __TIME_UTIL_H__

#include "utildef.h"
#include "typedef.h"
#include "pthread.h"

#ifdef WIN32
#   if !defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
        struct timeval
        {
            long tv_sec;        /*秒*/
            long tv_usec;       /*微妙*/
        };
        struct timezone
        {
            int32 tz_minuteswest; /*格林威治时间往西方的时差*/
            int32 tz_dsttime;     /*DST时间的修正方式*/
        };
#   endif
#else
#   include<sys/time.h>
#endif

namespace dtutil
{
    /**
    * \degroup time_group 时间
    * time 提供了一组时间相关函数
    */	
	
    /**
    * \brief 时间辅助类
    * \ingroup time_group
    */
    class UTIL_EXPORT TimeUtil
    {
    public:
        /**
        * \brief 获得当前时间(秒)
        * \return 当前时间
        */
        static time_t Now();

        /**
        * \brief 获得当前时间(毫秒)
        * \return 当前时间
        */
        static time_t NowMs();

        /**
        * \brief 获得当前时间(微秒)
        * \return 当前时间
        */
        static time_t NowUs();

        /**
        * \brief 获得运行时间(毫秒)
        * \return 运行时间
        */
        static uint32 Tick();

        /**
        * \brief 获得当前时间(微秒)
        * \param tv 保存获取时间结果的结构体
        * \param tz 用于保存时区结果 一般都用NULL(linux下无效)
        * \return 获取结果 =0(成功) <0(失败)
        */
        static int32 GetTimeVal(struct timeval *tv, struct timezone *tz);

        /**
        * \brief 计算延迟时间
        * \param secs 秒
        * \param msecs 毫秒
        * \param nsecs 纳秒
        * \return 从当前时间按参数延迟后的时间
        * 主要用于为一些现成阻塞函数产生等待超时参数
        */
        static timespec Delay(int32 secs, int32 msecs = 0, int32 nsecs = 0);

        /**
        * \brief 时间转换
        * \param time 指定时间，以1970年以来的秒数
        * \return 当地时间
        */
        static struct tm TimeToTm(time_t time);

        /**
        * \brief 获得月初时间
        * \param time 指定时间，以1970年以来的秒数
        * \return 月初时间，以1970年以来的秒数
        */
        static time_t MonthStartTime(time_t time);

        /**
        * \brief 获得周开始时间
        * \param time 指定时间，以1970年以来的秒数
        * \param week 周几，0-6(周日-周六)
        * \return 周开始时间，以1970年以来的秒数
        */
        static time_t WeekStartTime(time_t time, int32 week = 0);

        /**
        * \brief 获得日开始时间
        * \param time 指定时间，以1970年以来的秒数
        * \return 日开始时间，以1970年以来的秒数
        */
        static time_t DayStartTime(time_t time);

        /**
        * \brief 获得指定时间 以1970年1月1日0时0分0秒以来 的天数 
        * \param time 指定时间，以1970年以来的秒数
        * \return 指定时间计算得到的总天数
        */
        static uint32 GetDays(time_t time);

        /**
        * \brief 获得指定时间 以1970年1月1日0时0分0秒以来 的小时数 
        * \param time 指定时间，以1970年以来的秒数
        * \return 指定时间计算得到的总小时数
        */
        static uint32 GetHours(time_t time);

        /**
        * \brief 获得两个时间的相隔天数
        * \param time1 时间1，以1970年以来的秒数
        * \param time2 时间2，以1970年以来的秒数
        * \return 两个时间相隔的天数
        */
        static uint32 GetIntervalDay( time_t time1, time_t time2 );

        /**
        * \brief 获得指定时间之前的指定小时时间点 
        * \param time 指定时间，以1970年以来的秒数
        * \param hour 指定小时
        * \return 指定时间之前的小时时间点，以1970年以来的秒数
        *
        * 若指定时间的小时时间小于指定hour，则返回前一天的整点hour时间
        * 若指定时间的小时时间大于指定hour，则返回当天的整点hour时间
        */
        static time_t GetLastHour(time_t time, int32 hour);

        /**
        * \brief 获得指定时间之后的指定小时时间点 
        * \param time 指定时间，以1970年以来的秒数
        * \param hour 指定小时
        * \return 指定时间之后的小时时间点，以1970年以来的秒数
        *
        * 若指定时间的小时时间小于指定hour，则返回当天的整点hour时间
        * 若指定时间的小时时间大于指定hour，则返回下一天的整点hour时间
        */
        static time_t GetNextHour(time_t time, int32 hour);

        /**
        * \brief 设置时间偏差
        * \param offset 时间偏差
        */
        static void SetTimeOffset(int32 offset);

        /**
        * \brief 获得时间偏差
        * \return 时间偏差
        */
        static int32 GetTimeOffset();

        /**
        * \brief 设置时间偏差地址
        * \param offset 时间偏差地址
        * 进程初始化时，需要指定时间偏差地址；
        * 如果没有指定时间偏差地址，则无法改变时间偏差。
        */
        static void SetTimeOffsetPtr(int32 *offset);

    private:

        static int32        *m_offset;      ///< 时间偏差地址，GM调整时间[时间偏差值存在上层应用中，这里记录上层应用中的位置，修改的时候直接修改上层应用中的值]
    };
}

#endif // __TIME_UTIL_H__