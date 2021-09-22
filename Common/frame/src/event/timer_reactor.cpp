/**
* \file timer_reactor.cpp
* \brief 超时响应器类函数的实现
*/

#include "pch.h"
#include "event/timer_reactor.h"
#include "event/event_reactor.h"
#include "timeutil.h"
#include "event2/event.h"
using namespace dtutil;

namespace frame
{
    /**
    * \brief 构造函数
    */
    TimerReactor::TimerReactor()
    {
        m_pReactor = NULL;
    }

    /**
    * \brief 析构函数
    */
    TimerReactor::~TimerReactor(void)
    {
    }

    /**
    * \brief 初始化
    * \param pReactor 事件响应器
    * \return 初始化成功返回true，否则返回false
    */
    bool TimerReactor::Init(EventReactor *pReactor)
    {
        if (NULL == pReactor)
        {
            return false;
        }

        m_pReactor = pReactor;

        return true;
    }

    /**
    * \brief 释放
    */
    void TimerReactor::Restore()
    {
        m_pReactor = NULL;
    }

    /**
    * \brief 安装事件
    * \param pHandler 事件处理回调
    * \param nTimerID 定时器ID
    * \param nInterval 定时器调用间隔时间ms
    * \param nCallTimes 调用次数，默认调用无限次
    * \param szDesc 事件描述
    * \return 安装成功返回true，否则返回false
    */
    bool TimerReactor::AttachEvent(ITimerHandler *pHandler, uint64 nTimerID, uint32 nInterval, uint32 nCallTimes, const char *szDesc)
    {
        if (NULL == pHandler)
        {
            return false;
        }

        //key
        uint32 nTimerIDLow = nTimerID & 0xFFFFFFFF;
        uint32 nTimerIDHigh = (nTimerID >> 32) & 0xFFFFFFFF;
        EVENT_KEY key(EVENT_TIMER, nTimerIDLow, nTimerIDHigh, (uint64)(pHandler));

        event_base *pEventBase = m_pReactor->GetEventBase();
        if (NULL == pEventBase)
        {
            return false;
        }

        REACTOR_EVENT *pEvent = m_pReactor->PopEvent();
        if (NULL == pEvent)
        {
            return false;
        }

        bool bNeedExit = false;
        if(1) 
        {
            Lock lock(&m_lock);
            
            // 检查是否已经添加了这个Timer
            MAP_TIMER_INFO::iterator it = m_mapTimers.find(key);
            if (it != m_mapTimers.end())
            {
                bNeedExit = true;
            }
            else 
            {
                time_t tNow = TimeUtil::NowMs();
                REACTOR_EVENT::TIMER_INFO &stInfo = pEvent->m_TimerInfo;
                pEvent->m_nType     = EVENT_TIMER;
                pEvent->m_strDesc   = szDesc;
                stInfo.m_pHandler   = pHandler;
                stInfo.m_nTimerID   = nTimerID;
                stInfo.m_nInterval  = nInterval;
                stInfo.m_nCallTimes = nCallTimes;
                stInfo.m_pManager   = (void *)(this);
                stInfo.m_nInitTime  = tNow;
                stInfo.m_nLastCallTime = tNow;

                // 加入到订阅列表
                MAP_TIMER_INFO::value_type value(key, pEvent);
                m_mapTimers.insert(value);
            }
        }

        if(bNeedExit) 
        {            
            m_pReactor->PushEvent(pEvent);
            return false;
        }
        
        if (!RegisterEvent(key))
        {
            DetachEvent(pHandler, nTimerID);
            return false;
        }

        return true;
    }

    /**
    * \brief 卸载事件
    * \param pHandler 事件处理回调
    * \param nTimerID 定时器ID
    * \return 卸载成功返回true，否则返回false
    */
    bool TimerReactor::DetachEvent(ITimerHandler *pHandler, uint64 nTimerID)
    {
        if (NULL == pHandler)
        {
            return false;
        }

        uint32 nTimerIDLow = nTimerID & 0xFFFFFFFF;
        uint32 nTimerIDHigh = (nTimerID >> 32) & 0xFFFFFFFF;

        EVENT_KEY key(EVENT_TIMER, nTimerIDLow, nTimerIDHigh, (uint64)(pHandler));

        REACTOR_EVENT *pEvent = NULL;
        if(1) 
        {
            // 从订阅列表移除
            Lock lock(&m_lock);
            MAP_TIMER_INFO::iterator it = m_mapTimers.find(key);
            if (it != m_mapTimers.end())
            {
                pEvent = it->second;
                m_mapTimers.erase(it);
            }
        }

        //回收
        if(NULL != pEvent) {
            m_pReactor->PushEvent(pEvent);
        }
        
        // 注销信号事件
        UnRegisterEvent(key);

        return true;
    }

    /**
    * \brief 定时器回调处理
    * \param fd An fd or signal
    * \param events One or more EV_* flags
    * \param arg A user-supplied argument.
    */
    void TimerReactor::TimerCallback(evutil_socket_t fd, short events, void *arg)
    {
        if (NULL == arg)
        {
            return;
        }

        REACTOR_EVENT *pEvent = (REACTOR_EVENT *)(arg);
        if (NULL == pEvent)
        {
            return;
        }

        if (pEvent->m_nType != EVENT_TIMER)
        {
            return;
        }

        REACTOR_EVENT::TIMER_INFO &stInfo = pEvent->m_TimerInfo;
            
        stInfo.m_pHandler->OnTimer(stInfo.m_nTimerID);

        stInfo.m_nLastCallTime = TimeUtil::NowMs();
        if (stInfo.m_nCallTimes > 0)
        {
            stInfo.m_nCallTimes--;
        }

        TimerReactor *pReactor = (TimerReactor *)(stInfo.m_pManager);
        if (NULL == pReactor)
        {
            return;
        }

        // 调用次数已经够了
        if (stInfo.m_nCallTimes == 0)
        {
            pReactor->DetachEvent(stInfo.m_pHandler, stInfo.m_nTimerID);
        }
        else
        {
            uint32 nTimerIDLow = stInfo.m_nTimerID & 0xFFFFFFFF;
            uint32 nTimerIDHigh = (stInfo.m_nTimerID >> 32) & 0xFFFFFFFF;

            EVENT_KEY key(EVENT_TIMER, nTimerIDLow, nTimerIDHigh, (uint64)(stInfo.m_pHandler));

            event *pTimerEvent = pReactor->GetEvent(key);
            if (NULL == pTimerEvent)
            {
                return;
            }

            uint32 nTimeout = GetEventTimeout(pEvent);
            uint32 nTimeoutSec = nTimeout / 1000;
            uint32 nTimeoutUsec = nTimeout % 1000 * 1000;

            struct timeval timeout;
            evutil_timerclear(&timeout);
            timeout.tv_sec = nTimeoutSec;
            timeout.tv_usec = nTimeoutUsec;

            // 重新注册超时事件
            evtimer_add(pTimerEvent, &timeout);
        }
    }

    /**
    * \brief 获得事件的超时时间
    * \param pEvent 超时事件
    * \return 事件的超时时间
    */
    uint32 TimerReactor::GetEventTimeout(REACTOR_EVENT *pEvent)
    {
        if (NULL == pEvent)
        {
            return 0;
        }

        if (pEvent->m_nType != EVENT_TIMER)
        {
            return 0;
        }

        REACTOR_EVENT::TIMER_INFO &stInfo = pEvent->m_TimerInfo;

        return stInfo.m_nInterval - ( stInfo.m_nLastCallTime - stInfo.m_nInitTime) % stInfo.m_nInterval;
    }

    /**
    * \brief 注册超时事件
    * \param key 定时器key
    * \return 成功返回true，否则返回false
    */
    bool TimerReactor::RegisterEvent(EVENT_KEY key)
    {
        if (NULL == m_pReactor)
        {
            return false;
        }

        event_base *pEventBase = m_pReactor->GetEventBase();
        if (NULL == pEventBase)
        {
            return false;
        }

        event *pEvent = NULL;
        REACTOR_EVENT *pTimerInfo = NULL;
        if(1) 
        {
            Lock lock(&m_lock);

            // 已经注册过了
            if (m_mapEvents.find(key) != m_mapEvents.end())
            {
                return false;   
            }

            // 查找定时器信息
            MAP_TIMER_INFO::iterator it = m_mapTimers.find(key);
            if (it == m_mapTimers.end())
            {
                return false;   
            }

            pTimerInfo = it->second;
            if (NULL == pTimerInfo)
            {
                return false;
            }

            pEvent = event_new(pEventBase, -1, EV_PERSIST, TimerReactor::TimerCallback, (void *)(pTimerInfo)); // evtimer_new默认是一次就自动删除的，这里不适合
            if (NULL == pEvent)
            {
                return false;
            }

            MAP_TIMER_EVENT::value_type value(key, pEvent);
            m_mapEvents.insert(value);
        }
        
        uint32 nTimeout = GetEventTimeout(pTimerInfo);
        uint32 nTimeoutSec = nTimeout / 1000;
        uint32 nTimeoutUsec = nTimeout % 1000 * 1000;

        struct timeval timeout;
        evutil_timerclear(&timeout);
        timeout.tv_sec = nTimeoutSec;
        timeout.tv_usec = nTimeoutUsec;

        // 注册超时事件
        evtimer_add(pEvent, &timeout);

        return true;
    }

    /**
    * \brief 注销信号事件
    * \param key 定时器key
    * \return 成功返回true，否则返回false
    */
    bool TimerReactor::UnRegisterEvent(EVENT_KEY key)
    {
        if (NULL == m_pReactor)
        {
            return false;
        }

        event_base *pEventBase = m_pReactor->GetEventBase();
        if (NULL == pEventBase)
        {
            return false;
        }

        event *pEvent = NULL;

        if(1)
        {
            Lock lock(&m_lock);
            MAP_TIMER_EVENT::iterator it = m_mapEvents.find(key);
            if (it != m_mapEvents.end())
            {
                // 删除信号事件
                //event *pEvent = it->second;
                //if (NULL != pEvent)
                //{
                //    evtimer_del(pEvent);
                //}

                pEvent = it->second;
                m_mapEvents.erase(it);
            }
        }

        if(NULL != pEvent) 
        {
            evtimer_del(pEvent);
            event_free(pEvent);
        }

        return true;
    }

    /**
    * \brief 获得超时事件
    * \param key 定时器key
    * \return 超时事件
    */
    event * TimerReactor::GetEvent(EVENT_KEY key)
    {
        Lock lock(&m_lock);
        MAP_TIMER_EVENT::iterator it = m_mapEvents.find(key);
        if (it == m_mapEvents.end())
        {
            return NULL;
        }

        return it->second;
    }
}