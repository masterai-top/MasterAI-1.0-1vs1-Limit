/**
* \file event_reactor.h
* \brief 事件响应器
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __EVENT_REACTOR_H__
#define __EVENT_REACTOR_H__

#include "typedef.h"
#include "event/eventdef.h"
#include "event/event_engine.h"
#include "event/reactor_event_pool.h"
#include "event/signal_reactor.h"
#include "event/timer_reactor.h"
#include "event/notify_reactor.h"
#include "event/pipe_reactor.h"
#include "event.h"

namespace frame
{
    /**
    * \brief 事件响应器
    * \ingroup event_group
    */
    class EventReactor : public IEventReactor
    {
    public:
        /**
        * \brief 构造函数
        */
        EventReactor();

        /**
        * \brief 析构函数
        */
        virtual ~EventReactor(void);

        /**
        * \brief 创建
        * \param pEventBase 事件根基
        * \return 创建成功返回true，否则返回false
        */
        bool Create(event_base *pEventBase);

        /**
        * \brief 释放
        */
        virtual void Release();

        /**
        * \brief 安装信号
        * \param pHandler 事件处理回调
        * \param nSignalID 信号ID
        * \param szDesc 事件描述
        * \return 安装成功返回true，否则返回false
        */
        virtual bool AttachSignal(ISignalHandler *pHandler, int32 nSignalID, const char *szDesc);

        /**
        * \brief 卸载信号
        * \param pHandler 事件处理回调
        * \param nSignalID 信号ID
        * \return 卸载成功返回true，否则返回false
        */
        virtual bool DetachSignal(ISignalHandler *pHandler, int32 nSignalID);

        /**
        * \brief 安装定时器
        * \param pHandler 事件处理回调
        * \param nTimerID 定时器ID
        * \param nInterval 定时器调用间隔时间ms
        * \param nCallTimes 调用次数，默认调用无限次
        * \param szDesc 事件描述
        * \return 安装成功返回true，否则返回false
        */
        virtual bool AttachTimer(ITimerHandler *pHandler, uint64 nTimerID, uint32 nInterval, uint32 nCallTimes = 0xFFFFFFFF, const char *szDesc = NULL);

        /**
        * \brief 卸载定时器
        * \param pHandler 事件处理回调
        * \param nTimerID 定时器ID
        * \return 卸载成功返回true，否则返回false
        */
        virtual bool DetachTimer(ITimerHandler *pHandler, uint64 nTimerID);

        /**
        * \brief 订阅通知事件
        * \param pHandler 事件处理回调
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param szDesc 事件描述
        * \return 订阅成功返回true，否则返回false
        */
        virtual bool AttachNotify(INotifyHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID, const char *szDesc);

        /**
        * \brief 取消订阅通知事件
        * \param pHandler 事件处理回调
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \return 取消订阅成功返回true，否则返回false
        */
        virtual bool DetachNotify(INotifyHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID);

        /**
        * \brief 发起通知事件
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param lpContext 上下文数据
        * \param nLen 上下文数据长度
        * \return 发起成功返回true，否则返回false
        */
        virtual bool PostNotify(uint32 nEventID, uint32 nSrcType, uint64 nSrcID, LPCSTR lpContext, size_t nLen);

        /**
        * \brief 安装管道事件
        * \param pHandler 事件处理回调
        * \param nReadFd 管道读fd
        * \param szDesc 事件描述
        * \return 安装成功返回true，否则返回false
        */
        virtual bool AttachPipe(IPipeHandler *pHandler, int32 nReadFd, const char *szDesc);

        /**
        * \brief 卸载管道事件
        * \param pHandler 事件处理回调
        * \param nReadFd 管道读fd
        * \return 卸载成功返回true，否则返回false
        */
        virtual bool DetachPipe(IPipeHandler *pHandler, int32 nReadFd);

    public:
        /**
        * \brief 获得事件根基
        * \return 事件根基
        */
        event_base * GetEventBase();

        /**
        * \brief 获取一个空闲的事件结构
        * \return 空闲的网络事件结构
        */    
        REACTOR_EVENT * PopEvent();

        /**
        * \brief 回收事件结构
        * \param pEvent 要回收的事件结构
        */
        void PushEvent(REACTOR_EVENT *pEvent);

    private:
        event_base                      *m_pEventBase;      ///< 事件根基
        ReactorEventPool                m_EventPool;        ///< 事件池

        SignalReactor                   m_SignalReactor;    ///< 信号响应器
        TimerReactor                    m_TimerReactor;     ///< 超时响应器
        NotifyReactor                   m_NotifyReactor;    ///< 通知响应器
        PipeReactor                     m_PipeReactor;      ///< 管道响应器
    };
}

#endif // __EVENT_REACTOR_H__