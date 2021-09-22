/**
* \file notify_reactor.h
* \brief 通知响应器
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __NOTIFY_REACTOR_H__
#define __NOTIFY_REACTOR_H__

#include "typedef.h"
#include "event/eventdef.h"
#include "event/reactor_event.h"
#include <map>
#include <list>

namespace frame
{
    class EventReactor;

    /**
    * \brief 通知响应器
    * \ingroup event_group
    */
    class NotifyReactor
    {
    public:
        /**
        * \brief 构造函数
        */
        NotifyReactor();

        /**
        * \brief 析构函数
        */
        ~NotifyReactor(void);

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
        * \brief 订阅通知事件
        * \param pHandler 事件处理回调
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param szDesc 事件描述
        * \return 订阅成功返回true，否则返回false
        */
        bool AttachEvent(INotifyHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID, const char *szDesc);

        /**
        * \brief 取消订阅通知事件
        * \param pHandler 事件处理回调
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \return 取消订阅成功返回true，否则返回false
        */
        bool DetachEvent(INotifyHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID);

        /**
        * \brief 发起通知事件
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param lpContext 上下文数据
        * \param nLen 上下文数据长度
        * \return 发起成功返回true，否则返回false
        */
        bool PostEvent(uint32 nEventID, uint32 nSrcType, uint64 nSrcID, LPCSTR lpContext, size_t nLen);

    private:
        /**
        * \brief 通知回调处理
        * \param bev the bufferevent that triggered the callback
        * \param ctx the user-specified context for this bufferevent
        */
        static void NotifyCallback(struct bufferevent *bev, void *arg);

        /**
        * \brief 网络事件回调处理
        * \param bev the bufferevent for which the error condition was reached
        * \param what a conjunction of flags: BEV_EVENT_READING or BEV_EVENT_WRITING
                    to indicate if the error was encountered on the read or write path,
                    and one of the following flags: BEV_EVENT_EOF, BEV_EVENT_ERROR,
                    BEV_EVENT_TIMEOUT, BEV_EVENT_CONNECTED.
        * \param ctx the user-specified context for this bufferevent
        */
        static void EventCallback(struct bufferevent *bev, short what, void *ctx);

        /**
        * \brief 通知事件处理
        * \param key 事件key
        * \param nSrcID 发送源标识
        * \param lpContext 上下文数据
        * \param nLen 上下文数据长度
        * \return 成功返回true，否则返回false
        */
        bool OnNotify(EVENT_KEY key, uint64 nSrcID, LPCSTR lpContext, size_t nLen);

    private: 
        typedef std::list< REACTOR_EVENT * >            LIST_NOTIFY_INFO;   ///< 通知事件列表
        typedef std::map<EVENT_KEY, LIST_NOTIFY_INFO>   MAP_NOTIFY_INFO;    ///< 通知事件列表

        EventReactor                    *m_pReactor;        ///< 事件响应器
        MAP_NOTIFY_INFO                 m_mapNotifys;       ///< 通知事件列表

        bufferevent                     *m_evPair[2];       ///< 成对的bufferevent，m_evPair[0]读，m_evPair[1]写
    };
}

#endif // __NOTIFY_REACTOR_H__