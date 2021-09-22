/**
* \file app_event.cpp
* \brief 应用事件实现代码
*/

#include "pch.h"
#include "app/app_event.h"
#include "system.h"
#include "event2/thread.h"
using namespace dtutil;


namespace frame
{
    /**
    * \brief 构造函数
    */
    AppEvent::AppEvent(void)
    {
#ifdef WIN32
        evthread_use_windows_threads();
#else
        evthread_use_pthreads();
#endif
    }

    /**
    * \brief 析构函数
    */
    AppEvent::~AppEvent(void)
    {
    }

    /**
    * \brief 初始化
    * \return 初始化成功返回true，否则返回false
    */
    bool AppEvent::Init()
    {
        m_pEventBase = event_base_new();
        if (NULL == m_pEventBase)
        {
            return false;
        }

        m_pReactor = EventEngine::CreateEventReactor(m_pEventBase);
        if (NULL == m_pReactor)
        {
            return false;
        }

        m_pDispatcher = EventEngine::CreateEventDispatcher();
        if (NULL == m_pDispatcher)
        {
            return false;
        }

        m_SignalCenter.Init();

        return true;
    }

    /**
    * \brief 释放
    */
    void AppEvent::Restore()
    {
        m_SignalCenter.Restore();

        if (NULL != m_pReactor)
        {
            EventEngine::ReleaseEventReactor(m_pReactor);
        }

        if (NULL != m_pDispatcher)
        {
            EventEngine::ReleaseEventDispatcher(m_pDispatcher);
        }

        if (NULL != m_pEventBase)
        {
            event_base_free(m_pEventBase);
        }
    }

    /**
    * \brief 获得事件根基
    * \return 事件根基
    */
    event_base * AppEvent::GetEventBase()
    {
        return m_pEventBase;
    }

    /**
    * \brief 获得事件响应器
    * \return 事件响应器
    */
    IEventReactor * AppEvent::GetEventReactor()
    {
        return m_pReactor;
    }

    /**
    * \brief 获得事件调度器
    * \return 事件调度器
    */
    IEventDispatcher * AppEvent::GetEventDispatcher()
    {
        return m_pDispatcher;
    }

    /**
    * \brief 获得信号中心
    * \return 信号中心
    */
    AppSignal * AppEvent::GetSignalCenter()
    {
        return &m_SignalCenter;
    }

    /**
    * \brief 派发事件
    * 调用该接口后，线程阻塞循环派发事件
    */
    void AppEvent::Dispatch()
    {
        event_base_dispatch(m_pEventBase);
    }

    /**
    * \brief 强制退出循环
    */
    void AppEvent::Break()
    {
        event_base_loopbreak(m_pEventBase);
    }

    /**
    * \brief 等待所有事件处理完退出循环
    */
    void AppEvent::Exit()
    {
        event_base_loopexit(m_pEventBase, NULL);
    }

    /**
    * \brief 设置libevent日志函数
    */
    void AppEvent::SetLogCallBack(event_log_cb cb)
    {
        event_set_log_callback(cb);
    }
}
