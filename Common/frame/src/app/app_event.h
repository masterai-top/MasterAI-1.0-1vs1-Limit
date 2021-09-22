/**
* \file app_event.h
* \brief 应用事件类
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __APP_EVENT_H__
#define __APP_EVENT_H__

#include "event/event_engine.h"
#include "app/app_signal.h"
#include "event2/event.h"

namespace frame
{
    /**
    * \brief 应用事件类
    * \ingroup app_event
    */
    class AppEvent
    {
    public:
        /**
        * \brief 构造函数
        */
        AppEvent(void);

        /**
        * \brief 析构函数
        */
        ~AppEvent(void);

        /**
        * \brief 初始化
        * \return 初始化成功返回true，否则返回false
        */
        bool Init();

        /**
        * \brief 释放
        */
        void Restore();

        /**
        * \brief 获得事件根基
        * \return 事件根基
        */
        event_base * GetEventBase();

        /**
        * \brief 获得事件响应器
        * \return 事件响应器
        */
        IEventReactor * GetEventReactor();

        /**
        * \brief 获得事件调度器
        * \return 事件调度器
        */
        IEventDispatcher * GetEventDispatcher();

        /**
        * \brief 获得信号中心
        * \return 信号中心
        */
        AppSignal * GetSignalCenter();

        /**
        * \brief 派发事件
        * 调用该接口后，线程阻塞循环派发事件
        * 哪个线程调用该接口，回调即在哪个线程
        */
        virtual void Dispatch();

        /**
        * \brief 强制退出循环
        */
        virtual void Break();

        /**
        * \brief 等待所有事件处理完退出循环
        */
        virtual void Exit();

        
        /**
        * \brief 设置libevent日志函数
        */
        virtual void SetLogCallBack(event_log_cb cb);

    private:
        event_base          *m_pEventBase;      ///< 事件根基

        IEventReactor       *m_pReactor;        ///< 事件响应器
        IEventDispatcher    *m_pDispatcher;     ///< 事件调度器

        AppSignal           m_SignalCenter;     ///< 信号中心
    };
}

#endif // __APP_EVENT_H__