/**
* \file event_engine.h
* \brief 事件引擎
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __EVENT_ENGINE_H__
#define __EVENT_ENGINE_H__

#include "framedef.h"
#include "typedef.h"

struct event_base;

namespace frame
{
    /**
    * \defgroup event_group 事件引擎
    * 提供了一组事件操作的相关函数
    */

    /**
    * \brief 订阅事件回调处理接口
    * \ingroup event_group
    */
    class ISubHandler
    {
    public:
        /**
        * \brief 执行事件处理
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param lpContext 上下文数据
        * \param nLen 上下文数据长度
        */
        virtual void OnExecute(uint32 nEventID, uint32 nSrcType, uint64 nSrcID, LPCSTR lpContext, size_t nLen) = 0;

        /**
        * \brief 票决事件处理
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param lpContext 上下文数据
        * \param nLen 上下文数据长度
        * \return 如果返回false，则中断执行，否则继续向下执行
        */
        virtual bool OnVote(uint32 nEventID, uint32 nSrcType, uint64 nSrcID, LPCSTR lpContext, size_t nLen) = 0;
    };

    /**
    * \brief 信号回调处理接口
    * \ingroup event_group
    */
    class ISignalHandler
    {
    public:
        /**
        * \brief 信号事件处理
        * \param nSignalID 信号ID
        */
        virtual void OnSignal(int32 nSignalID) = 0;
    };

    /**
    * \brief 超时回调处理接口
    * \ingroup event_group
    */
    class ITimerHandler
    {
    public:
        /**
        * \brief 定时器事件处理
        * \param nTimerID 定时器ID
        */
        virtual void OnTimer(uint64 nTimerID) = 0;
    };

    /**
    * \brief 通知回调处理接口
    * \ingroup event_group
    */
    class INotifyHandler
    {
    public:
        /**
        * \brief 通知事件处理
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param lpContext 上下文数据
        * \param nLen 上下文数据长度
        */
        virtual void OnNotify(uint32 nEventID, uint32 nSrcType, uint64 nSrcID, LPCSTR lpContext, size_t nLen) = 0;
    };

    /**
    * \brief 管道回调处理接口
    * \ingroup event_group
    */
    class IPipeHandler
    {
    public:
        /**
        * \brief 通知事件处理
        * \param nReadFd 读fd
        * \param lpContext 上下文数据
        * \param nLen 上下文数据长度
        */
        virtual void OnNotify(int32 nReadFd, LPCSTR lpContext, size_t nLen) = 0;
    };

    /**
    * \brief 事件调度中心接口
    * \ingroup event_group
    * 封装了事件的操作接口，非线程安全
    */
    class IEventDispatcher
    {
    public:
        /**
        * \brief 释放
        */
        virtual void Release() = 0;

        /**
        * \brief 订阅执行事件
        * \param pHandler 事件处理回调
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param szDesc 事件描述
        * \return 订阅成功返回true，否则返回false
        */
        virtual bool AttachExecute(ISubHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID, const char *szDesc) = 0;

        /**
        * \brief 取消订阅执行事件
        * \param pHandler 事件处理回调
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \return 取消订阅成功返回true，否则返回false
        */
        virtual bool DetachExecute(ISubHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID) = 0;

        /**
        * \brief 发起执行事件
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param lpContext 上下文数据
        * \param nLen 上下文数据长度
        * \return 发起成功即返回true，否则返回false
        */
        virtual bool FireExecute(uint32 nEventID, uint32 nSrcType, uint64 nSrcID, LPCSTR lpContext, size_t nLen) = 0;

        /**
        * \brief 订阅票决事件
        * \param pHandler 事件处理回调
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param szDesc 事件描述
        * \return 订阅成功返回true，否则返回false
        */
        virtual bool AttachVote(ISubHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID, const char *szDesc) = 0;

        /**
        * \brief 取消订阅票决事件
        * \param pHandler 事件处理回调
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \return 取消订阅成功返回true，否则返回false
        */
        virtual bool DetachVote(ISubHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID) = 0;

        /**
        * \brief 发起票决事件
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param lpContext 上下文数据
        * \param nLen 上下文数据长度
        * \return 有订阅者否决了返回false，全都支持返回true
        */
        virtual bool FireVote(uint32 nEventID, uint32 nSrcType, uint64 nSrcID, LPCSTR lpContext, size_t nLen) = 0;
    };

    /**
    * \brief 事件响应器接口类
    * \ingroup event_group
    */
    class IEventReactor
    {
    public:
        /**
        * \brief 释放
        */
        virtual void Release() = 0;

        /**
        * \brief 安装信号
        * \param pHandler 事件处理回调
        * \param nSignalID 信号ID
        * \param szDesc 事件描述
        * \return 安装成功返回true，否则返回false
        */
        virtual bool AttachSignal(ISignalHandler *pHandler, int32 nSignalID, const char *szDesc) = 0;

        /**
        * \brief 卸载信号
        * \param pHandler 事件处理回调
        * \param nSignalID 信号ID
        * \return 卸载成功返回true，否则返回false
        */
        virtual bool DetachSignal(ISignalHandler *pHandler, int32 nSignalID) = 0;

        /**
        * \brief 安装定时器
        * \param pHandler 事件处理回调
        * \param nTimerID 定时器ID
        * \param nInterval 定时器调用间隔时间ms
        * \param nCallTimes 调用次数，默认调用无限次
        * \param szDesc 事件描述
        * \return 安装成功返回true，否则返回false
        */
        virtual bool AttachTimer(ITimerHandler *pHandler, uint64 nTimerID, uint32 nInterval, uint32 nCallTimes = 0xFFFFFFFF, const char *szDesc = NULL) = 0;

        /**
        * \brief 卸载定时器
        * \param pHandler 事件处理回调
        * \param nTimerID 定时器ID
        * \return 卸载成功返回true，否则返回false
        */
        virtual bool DetachTimer(ITimerHandler *pHandler, uint64 nTimerID) = 0;

        /**
        * \brief 订阅通知事件
        * \param pHandler 事件处理回调
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param szDesc 事件描述
        * \return 订阅成功返回true，否则返回false
        */
        virtual bool AttachNotify(INotifyHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID, const char *szDesc) = 0;

        /**
        * \brief 取消订阅通知事件
        * \param pHandler 事件处理回调
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \return 取消订阅成功返回true，否则返回false
        */
        virtual bool DetachNotify(INotifyHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID) = 0;

        /**
        * \brief 发起通知事件
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param lpContext 上下文数据
        * \param nLen 上下文数据长度
        * \return 发起成功返回true，否则返回false
        */
        virtual bool PostNotify(uint32 nEventID, uint32 nSrcType, uint64 nSrcID, LPCSTR lpContext, size_t nLen) = 0;

        /**
        * \brief 安装管道事件
        * \param pHandler 事件处理回调
        * \param nReadFd 管道读fd
        * \param szDesc 事件描述
        * \return 安装成功返回true，否则返回false
        */
        virtual bool AttachPipe(IPipeHandler *pHandler, int32 nReadFd, const char *szDesc) = 0;

        /**
        * \brief 卸载管道事件
        * \param pHandler 事件处理回调
        * \param nReadFd 管道读fd
        * \return 卸载成功返回true，否则返回false
        */
        virtual bool DetachPipe(IPipeHandler *pHandler, int32 nReadFd) = 0;
    };

    /**
    * \brief 事件引擎
    * \ingroup event_group
    * 事件调度中心是上层逻辑主动发起事件，直接在发起事件的过程中回调到各个订阅对象中，是一个同步的过程。
    * 而事件响应器则是底层监控事件，将激活的事件放入一个激活列表，再回调到各个订阅对象中，所以即使上层
    * 主动发起事件，也不会立刻执行到各个回调对象中，而是等待下一帧底层监控到事件，再回调上来，是一个异步的过程。
    */
    class FRAME_EXPORT EventEngine
    {
    public:
        /**
        * \brief 创建事件调度中心
        * \return 事件调度中心
        */
        static IEventDispatcher * CreateEventDispatcher();

        /**
        * \brief 释放事件调度中心
        * \param pEventDispatcher 事件调度中心
        * \return 释放成功返回true，否则返回false
        */
        static bool ReleaseEventDispatcher(IEventDispatcher *pEventDispatcher);

        /**
        * \brief 创建事件响应器
        * \param pEventBase 事件根基
        * \return 事件响应器
        */
        static IEventReactor * CreateEventReactor(event_base *pEventBase);

        /**
        * \brief 释放事件响应器
        * \param pEventReactor 事件响应器
        * \return 释放成功返回true，否则返回false
        */
        static bool ReleaseEventReactor(IEventReactor *pEventReactor);
    };
}

#endif // __EVENT_ENGINE_H__
