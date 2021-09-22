/**
* \file timer_reactor.h
* \brief 超时响应器
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __TIMER_REACTOR_H__
#define __TIMER_REACTOR_H__

#include "typedef.h"
#include "event/eventdef.h"
#include "event/reactor_event.h"
#include <map>
#include <list>
#include "thread/lock.h"

namespace frame
{
    class EventReactor;

    /**
    * \brief 超时响应器
    * \ingroup event_group
    */
    class TimerReactor
    {
    public:
        /**
        * \brief 构造函数
        */
        TimerReactor();

        /**
        * \brief 析构函数
        */
        ~TimerReactor(void);

        /**
        * \brief 初始化
        * \param pReactor 事件响应器
        * \return 初始化成功返回true，否则返回false
        */
        bool Init(EventReactor *pReactor);

        /**
        * \brief 释放
        */
        void Restore();

        /**
        * \brief 安装事件
        * \param pHandler 事件处理回调
        * \param nTimerID 定时器ID
        * \param nInterval 定时器调用间隔时间ms
        * \param nCallTimes 调用次数，默认调用无限次
        * \param szDesc 事件描述
        * \return 安装成功返回true，否则返回false
        */
        bool AttachEvent(ITimerHandler *pHandler, uint64 nTimerID, uint32 nInterval, uint32 nCallTimes, const char *szDesc);

        /**
        * \brief 卸载事件
        * \param pHandler 事件处理回调
        * \param nTimerID 定时器ID
        * \return 卸载成功返回true，否则返回false
        */
        bool DetachEvent(ITimerHandler *pHandler, uint64 nTimerID);

    private:
        /**
        * \brief 定时器回调处理
        * \param fd An fd or signal
        * \param events One or more EV_* flags
        * \param arg A user-supplied argument.
        */
        static void TimerCallback(evutil_socket_t fd, short events, void *arg);

        /**
        * \brief 获得事件的超时时间
        * \param pEvent 超时事件
        * \return 事件的超时时间
        */
        static uint32 GetEventTimeout(REACTOR_EVENT *pEvent);

        /**
        * \brief 注册超时事件
        * \param key 定时器key
        * \return 成功返回true，否则返回false
        */
        bool RegisterEvent(EVENT_KEY key);

        /**
        * \brief 注销超时事件
        * \param key 定时器key
        * \return 成功返回true，否则返回false
        */
        bool UnRegisterEvent(EVENT_KEY key);

        /**
        * \brief 获得超时事件
        * \param key 定时器key
        * \return 超时事件
        */
        event * GetEvent(EVENT_KEY key);

    private:
        typedef std::map<EVENT_KEY, REACTOR_EVENT *>    MAP_TIMER_INFO;     ///< 超时回调列表
        typedef std::map<EVENT_KEY, event *>            MAP_TIMER_EVENT;    ///< 超时事件列表

        EventReactor                    *m_pReactor;        ///< 事件响应器
        MAP_TIMER_INFO                  m_mapTimers;        ///< 超时回调列表

        MAP_TIMER_EVENT                 m_mapEvents;        ///< 信号事件列表
        frame::LockObject               m_lock;             // 锁对象
    };
}

#endif // __TIMER_REACTOR_H__