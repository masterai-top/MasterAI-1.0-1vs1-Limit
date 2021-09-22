/**
* \file signal_reactor.h
* \brief 信号响应器
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __SIGNAL_REACTOR_H__
#define __SIGNAL_REACTOR_H__

#include "typedef.h"
#include "event/eventdef.h"
#include "event/reactor_event.h"
#include <map>
#include <list>

namespace frame
{
    class EventReactor;

    /**
    * \brief 信号响应器
    * \ingroup event_group
    */
    class SignalReactor
    {
    public:
        /**
        * \brief 构造函数
        */
        SignalReactor();

        /**
        * \brief 析构函数
        */
        ~SignalReactor(void);

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
        * \param nSignalID 信号ID
        * \param szDesc 事件描述
        * \return 安装成功返回true，否则返回false
        */
        bool AttachEvent(ISignalHandler *pHandler, int32 nSignalID, const char *szDesc);

        /**
        * \brief 卸载事件
        * \param pHandler 事件处理回调
        * \param nSignalID 信号ID
        * \return 卸载成功返回true，否则返回false
        */
        bool DetachEvent(ISignalHandler *pHandler, int32 nSignalID);

    private:
        /**
        * \brief 信号回调处理
        * \param signo 信号ID
        * \param events 事件标识
        * \param arg 安装时的参数
        */
        static void SignalCallback(evutil_socket_t signo, short events, void *arg);

        /**
        * \brief 注册信号事件
        * \param nSignalID 信号ID
        * \return 成功返回true，否则返回false
        */
        bool RegisterEvent(int32 nSignalID);

        /**
        * \brief 注销信号事件
        * \param nSignalID 信号ID
        * \return 成功返回true，否则返回false
        */
        bool UnRegisterEvent(int32 nSignalID);

        /**
        * \brief 信号事件处理
        * \param nSignalID 信号ID
        */
        void OnSignal(int32 nSignalID);

    private:
        typedef std::list< REACTOR_EVENT * >            LIST_SIGNAL_INFO;   ///< 信号回调列表
        typedef std::map<EVENT_KEY, LIST_SIGNAL_INFO>   MAP_SIGNAL_INFO;    ///< 信号回调列表
        typedef std::map<int32, event *>                MAP_SIGNAL_EVENT;   ///< 信号事件列表

        EventReactor                    *m_pReactor;        ///< 事件响应器
        MAP_SIGNAL_INFO                 m_mapSignals;       ///< 信号回调列表

        MAP_SIGNAL_EVENT                m_mapEvents;        ///< 信号事件列表
    };
}

#endif // __SIGNAL_REACTOR_H__