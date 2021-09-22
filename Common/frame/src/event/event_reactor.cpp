/**
* \file event_reactor.cpp
* \brief 事件反应堆类函数的实现
*/

#include "pch.h"
#include "event_reactor.h"


namespace frame
{
    /**
    * \brief 构造函数
    */
    EventReactor::EventReactor()
    {
        m_pEventBase = NULL;
    }

    /**
    * \brief 析构函数
    */
    EventReactor::~EventReactor(void)
    {
        m_pEventBase = NULL;
    }

    /**
    * \brief 创建
    * \param pEventBase 事件根基
    * \return 创建成功返回true，否则返回false
    */
    bool EventReactor::Create(event_base *pEventBase)
    {
        if (NULL == pEventBase)
        {
            return false;
        }

        m_pEventBase = pEventBase;

        m_SignalReactor.Init(this);
        m_TimerReactor.Init(this);
        m_NotifyReactor.Init(this);
        m_PipeReactor.Init(this);

        return true;
    }

    /**
    * \brief 释放
    */
    void EventReactor::Release()
    {
        m_SignalReactor.Restore();
        m_TimerReactor.Restore();
        m_NotifyReactor.Restore();
        m_PipeReactor.Restore();

        delete this;
    }

    /**
    * \brief 安装信号
    * \param pHandler 事件处理回调
    * \param nSignalID 信号ID
    * \param szDesc 事件描述
    * \return 安装成功返回true，否则返回false
    */
    bool EventReactor::AttachSignal(ISignalHandler *pHandler, int32 nSignalID, const char *szDesc)
    {
        return m_SignalReactor.AttachEvent(pHandler, nSignalID, szDesc);
    }

    /**
    * \brief 卸载信号
    * \param pHandler 事件处理回调
    * \param nSignalID 信号ID
    * \return 卸载成功返回true，否则返回false
    */
    bool EventReactor::DetachSignal(ISignalHandler *pHandler, int32 nSignalID)
    {
        return m_SignalReactor.DetachEvent(pHandler, nSignalID);
    }

    /**
    * \brief 安装定时器
    * \param pHandler 事件处理回调
    * \param nTimerID 定时器ID
    * \param nInterval 定时器调用间隔时间ms
    * \param nCallTimes 调用次数，默认调用无限次
    * \param szDesc 事件描述
    * \return 安装成功返回true，否则返回false
    */
    bool EventReactor::AttachTimer(ITimerHandler *pHandler, uint64 nTimerID, uint32 nInterval, uint32 nCallTimes /*= 0xFFFFFFFF*/, const char *szDesc /*= NULL*/)
    {
        return m_TimerReactor.AttachEvent(pHandler, nTimerID, nInterval, nCallTimes, szDesc);
    }

    /**
    * \brief 卸载定时器
    * \param pHandler 事件处理回调
    * \param nTimerID 定时器ID
    * \return 卸载成功返回true，否则返回false
    */
    bool EventReactor::DetachTimer(ITimerHandler *pHandler, uint64 nTimerID)
    {
        return m_TimerReactor.DetachEvent(pHandler, nTimerID);
    }

    /**
    * \brief 订阅通知事件
    * \param pHandler 事件处理回调
    * \param nEventID 事件ID
    * \param nSrcType 发送源类型
    * \param nSrcID 发送源标识
    * \param szDesc 事件描述
    * \return 订阅成功返回true，否则返回false
    */
    bool EventReactor::AttachNotify(INotifyHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID, const char *szDesc)
    {
        return m_NotifyReactor.AttachEvent(pHandler, nEventID, nSrcType, nSrcID, szDesc);
    }

    /**
    * \brief 取消订阅通知事件
    * \param pHandler 事件处理回调
    * \param nEventID 事件ID
    * \param nSrcType 发送源类型
    * \param nSrcID 发送源标识
    * \return 取消订阅成功返回true，否则返回false
    */
    bool EventReactor::DetachNotify(INotifyHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID)
    {
        return m_NotifyReactor.DetachEvent(pHandler, nEventID, nSrcType, nSrcID);
    }

    /**
    * \brief 发起通知事件
    * \param nEventID 事件ID
    * \param nSrcType 发送源类型
    * \param nSrcID 发送源标识
    * \param lpContext 上下文数据
    * \param nLen 上下文数据长度
    * \return 发起成功返回true，否则返回false
    */
    bool EventReactor::PostNotify(uint32 nEventID, uint32 nSrcType, uint64 nSrcID, LPCSTR lpContext, size_t nLen)
    {
        return m_NotifyReactor.PostEvent(nEventID, nSrcType, nSrcID, lpContext, nLen);
    }

    /**
    * \brief 安装管道事件
    * \param pHandler 事件处理回调
    * \param nReadFd 管道读fd
    * \param szDesc 事件描述
    * \return 安装成功返回true，否则返回false
    */
    bool EventReactor::AttachPipe(IPipeHandler *pHandler, int32 nReadFd, const char *szDesc)
    {
        return m_PipeReactor.AttachEvent(pHandler, nReadFd, szDesc);
    }

    /**
    * \brief 卸载管道事件
    * \param pHandler 事件处理回调
    * \param nReadFd 管道读fd
    * \return 卸载成功返回true，否则返回false
    */
    bool EventReactor::DetachPipe(IPipeHandler *pHandler, int32 nReadFd)
    {
        return m_PipeReactor.DetachEvent(pHandler, nReadFd);
    }

    /**
    * \brief 获得事件根基
    * \return 事件根基
    */
    event_base * EventReactor::GetEventBase()
    {
        return m_pEventBase;
    }

    /**
    * \brief 获取一个空闲的事件结构
    * \return 空闲的网络事件结构
    */    
    REACTOR_EVENT * EventReactor::PopEvent()
    {
        return m_EventPool.Pop();
    }

    /**
    * \brief 回收事件结构
    * \param pEvent 要回收的事件结构
    */
    void EventReactor::PushEvent(REACTOR_EVENT *pEvent)
    {
        m_EventPool.Push(pEvent);
    }
}
