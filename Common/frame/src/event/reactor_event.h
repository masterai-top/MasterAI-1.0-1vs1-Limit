/**
* \file reactor_event.h
* \brief 响应器事件
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __REACTOR_EVENT_H__
#define __REACTOR_EVENT_H__

#include "typedef.h"
#include "event.h"
#include <string>


namespace frame
{
    class ISignalHandler;
    class ITimerHandler;
    class INotifyHandler;
    class IPipeHandler;

    /**
    * \brief 响应器事件
    * \ingroup event_group
    */
    typedef struct _REACTOR_EVENT
    {
        /**
        * \brief 信号事件数据
        */
        typedef struct _SIGNAL_INFO
        {
            ISignalHandler  *m_pHandler;        ///< 事件处理回调

            int32           m_nSignalID;        ///< 信号ID
        } SIGNAL_INFO;

        /**
        * \brief 超时事件数据
        */
        typedef struct _TIMER_INFO
        {
            ITimerHandler   *m_pHandler;        ///< 事件处理回调

            uint64          m_nTimerID;         ///< 定时器ID
            uint32          m_nInterval;        ///< 定时器调用间隔时间ms
            uint32          m_nCallTimes;       ///< 定时器调用次数
            void            *m_pManager;        ///< 定时器管理器

            time_t          m_nInitTime;        ///< 初始化时间
            time_t          m_nLastCallTime;    ///< 最后一次调用的时间
        } TIMER_INFO;

        /**
        * \brief 通知事件数据
        */
        typedef struct _NOTIFY_INFO
        {
            INotifyHandler  *m_pHandler;        ///< 事件处理回调
        } NOTIFY_INFO;

        /**
        * \brief 管道事件数据
        */
        typedef struct _PIPE_INFO
        {
            IPipeHandler    *m_pHandler;        ///< 事件处理回调

            int32           m_nReadFd;          ///< 读fd
        } PIPE_INFO;

        union
        {
            SIGNAL_INFO     m_SignalInfo;
            TIMER_INFO      m_TimerInfo;
            NOTIFY_INFO     m_NotifyInfo;
            PIPE_INFO       m_PipeInfo;
        };                                      ///< 事件数据
        uint8               m_nType;            ///< 事件类型
        std::string         m_strDesc;          ///< 事件描述

    } REACTOR_EVENT;
} 

#endif // __REACTOR_EVENT_H__
