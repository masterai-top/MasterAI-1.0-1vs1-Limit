/**
* \file event_dispatcher.h
* \brief 事件调度中心
*
* 观察者模式（有时又被称为发布-订阅Subscribe>模式、模型-视图View>模式、
* 源-收听者Listener>模式或从属者模式）是软件设计模式的一种。
* 在此种模式中，一个目标物件(Subject)管理所有相依于它的观察者物件(Observer)，
* 并且在它本身的状态改变时主动发出通知。
* 这通常透过呼叫各观察者所提供的方法来实现。
* 此种模式通常被用来实作事件处理系统。

* 发布/订阅模式，订阅者把自己想订阅的事件注册到调度中心，当该事件触发时候，
* 发布者发布该事件到调度中心（顺带上下文），由调度中心统一调度订阅者注册到调度中心的处理代码。
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __EVENT_DISPATCHER_H__
#define __EVENT_DISPATCHER_H__

#include "typedef.h"
#include "event/eventdef.h"
#include "event/event_engine.h"
#include "event/event_subscriber_pool.h"
#include <map>
#include <list>

namespace frame
{
    /**
    * \brief 事件调度中心
    * \ingroup event_group
    */
    class EventDispatcher : public IEventDispatcher
    {
    public:
        /**
        * \brief 构造函数
        */
        EventDispatcher();

        /**
        * \brief 析构函数
        */
        virtual ~EventDispatcher(void);

        /**
        * \brief 创建
        * \return 创建成功返回true，否则返回false
        */
        bool Create();

        /**
        * \brief 释放
        */
        virtual void Release();

        /**
        * \brief 订阅执行事件
        * \param pHandler 事件处理回调
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param szDesc 事件描述
        * \return 订阅成功返回true，否则返回false
        */
        virtual bool AttachExecute(ISubHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID, const char *szDesc);

        /**
        * \brief 取消订阅执行事件
        * \param pHandler 事件处理回调
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \return 取消订阅成功返回true，否则返回false
        */
        virtual bool DetachExecute(ISubHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID);

        /**
        * \brief 发起执行事件
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param lpContext 上下文数据
        * \param nLen 上下文数据长度
        * \return 发起成功即返回true，否则返回false
        */
        virtual bool FireExecute(uint32 nEventID, uint32 nSrcType, uint64 nSrcID, LPCSTR lpContext, size_t nLen);

        /**
        * \brief 订阅票决事件
        * \param pHandler 事件处理回调
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param szDesc 事件描述
        * \return 订阅成功返回true，否则返回false
        */
        virtual bool AttachVote(ISubHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID, const char *szDesc);

        /**
        * \brief 取消订阅票决事件
        * \param pHandler 事件处理回调
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \return 取消订阅成功返回true，否则返回false
        */
        virtual bool DetachVote(ISubHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID);

        /**
        * \brief 发起票决事件
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param lpContext 上下文数据
        * \param nLen 上下文数据长度
        * \return 有订阅者否决了返回false，全都支持返回true
        */
        virtual bool FireVote(uint32 nEventID, uint32 nSrcType, uint64 nSrcID, LPCSTR lpContext, size_t nLen);

    private:
        /**
        * \brief 发起执行事件
        * \param key 事件key
        * \param nSrcID 发送源ID
        * \param lpContext 上下文数据
        * \param nLen 上下文数据长度
        * \return 执行事件发起成功即返回true
        */
        bool FireExecute(EVENT_KEY key, uint64 nSrcID, LPCSTR lpContext, size_t nLen);

        /**
        * \brief 发起票决事件
        * \param key 事件key
        * \param nSrcID 发送源ID
        * \param lpContext 上下文数据
        * \param nLen 上下文数据长度
        * \return 有观察者否决了返回false，全都支持返回true
        */
        bool FireVote(EVENT_KEY key, uint64 nSrcID, LPCSTR lpContext, size_t nLen);

    private:
        typedef std::list< EventSubscriber * >          LIST_EVENT_INFO;    ///< 订阅者列表
        typedef std::map<EVENT_KEY, LIST_EVENT_INFO>    MAP_EVENT_INFO;     ///< 订阅者列表

        MAP_EVENT_INFO                  m_mapEvents;        ///< 订阅事件列表

        EventSubscriberPool             m_SubscriberPool;   ///< 订阅者池

        uint32                          m_nHandleCount;     ///< 当前处理层数[由于可能在事件FireEvent()中再次Attach()，导致进入层数过多，这里限制事件处理层数]
    };
}

#endif // __EVENT_DISPATCHER_H__