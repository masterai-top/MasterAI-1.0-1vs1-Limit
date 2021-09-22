/**
* \file pipe_reactor.h
* \brief 管道响应器
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __PIPE_REACTOR_H__
#define __PIPE_REACTOR_H__

#include "typedef.h"
#include "event/eventdef.h"
#include "event/reactor_event.h"
#include <map>
#include <list>

namespace frame
{
    class EventReactor;

    /**
    * \brief 管道响应器
    * \ingroup event_group
    */
    class PipeReactor
    {
    public:
        /**
        * \brief 构造函数
        */
        PipeReactor();

        /**
        * \brief 析构函数
        */
        ~PipeReactor(void);

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
        * \brief 安装管道事件
        * \param pHandler 事件处理回调
        * \param szDesc 事件描述
        * \return 安装成功返回true，否则返回false
        */
        bool AttachEvent(IPipeHandler *pHandler, int32 nReadFd, const char *szDesc);

        /**
        * \brief 卸载管道事件
        * \param pHandler 事件处理回调
        * \param nReadFd 管道读fd
        * \return 卸载成功返回true，否则返回false
        */
        bool DetachEvent(IPipeHandler *pHandler, int32 nReadFd);

        /**
        * \brief 发起通知事件
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        * \param lpContext 上下文数据
        * \param nLen 上下文数据长度
        * \return 发起成功返回true，否则返回false
        */
        bool PostNotify(uint32 nEventID, uint32 nSrcType, uint64 nSrcID, LPCSTR lpContext, size_t nLen);

    private:
        /**
        * \brief 管道事件回调处理
        * \param fd An fd or signal
        * \param events One or more EV_* flags
        * \param arg A user-supplied argument.
        */
        static void EventCallback(evutil_socket_t fd, short events, void *arg);

        /**
        * \brief 注册管道事件
        * \param nReadFd 读fd
        * \return 成功返回true，否则返回false
        */
        bool RegisterEvent(int32 nReadFd);

        /**
        * \brief 注销管道事件
        * \param nReadFd 读fd
        * \return 成功返回true，否则返回false
        */
        bool UnRegisterEvent(int32 nReadFd);

        /**
        * \brief 管道事件处理
        * \param nReadFd 读fd
        */
        void OnNotify(int32 nReadFd);

    private: 
        typedef std::list< REACTOR_EVENT * >            LIST_PIPE_INFO;     ///< 管道回调列表
        typedef std::map<EVENT_KEY, LIST_PIPE_INFO>     MAP_PIPE_INFO;      ///< 管道回调列表
        typedef std::map<int32, event *>                MAP_PIPE_EVENT;     ///< 管道事件列表

        EventReactor                    *m_pReactor;        ///< 事件响应器
        MAP_PIPE_INFO                   m_mapPipes;         ///< 管道回调列表

        MAP_PIPE_EVENT                  m_mapEvents;        ///< 管道事件列表
    };
}

#endif // __PIPE_REACTOR_H__