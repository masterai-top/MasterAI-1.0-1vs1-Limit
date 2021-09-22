/**
* \file pipe_reactor.cpp
* \brief 管道响应器类函数的实现
*/

#include "pch.h"
#include "event/pipe_reactor.h"
#include "event/event_reactor.h"
#include "event2/event.h"


namespace frame
{
    /**
    * \brief 构造函数
    */
    PipeReactor::PipeReactor(void)
    {
        m_pReactor = NULL;
    }

    /**
    * \brief 析构函数
    */
    PipeReactor::~PipeReactor(void)
    {
    }

    /**
    * \brief 初始化
    * \param pReactor 事件响应器
    * \return 初始化成功返回true，否则返回false
    */
    bool PipeReactor::Init(EventReactor *pReactor)
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
    void PipeReactor::Restore()
    {
        m_pReactor = NULL;
    }

    /**
    * \brief 安装管道事件
    * \param pHandler 事件处理回调
    * \param nReadFd 管道读fd
    * \param szDesc 事件描述
    * \return 安装成功返回true，否则返回false
    */
    bool PipeReactor::AttachEvent(IPipeHandler *pHandler, int32 nReadFd, const char *szDesc)
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

        REACTOR_EVENT *pEvent = m_pReactor->PopEvent();
        if (NULL == pEvent)
        {
            return false;
        }

        REACTOR_EVENT::PIPE_INFO &stInfo = pEvent->m_PipeInfo;
        pEvent->m_nType = EVENT_PIPE;
        pEvent->m_strDesc = szDesc;
        stInfo.m_pHandler = pHandler;
        stInfo.m_nReadFd = nReadFd;

        EVENT_KEY key(EVENT_PIPE, nReadFd, 0, 0);

        MAP_PIPE_INFO::iterator it = m_mapPipes.find(key);

        // 加入到订阅列表
        if (it != m_mapPipes.end())
        {
            LIST_PIPE_INFO &lstSubscribers = it->second;
            lstSubscribers.push_back(pEvent);
        }
        else
        {
            LIST_PIPE_INFO lstSubscribers;
            lstSubscribers.push_back(pEvent);

            MAP_PIPE_INFO::value_type value(key, lstSubscribers);
            m_mapPipes.insert(value);
        }

        // 注册管道事件
        if (!RegisterEvent(nReadFd))
        {
            DetachEvent(pHandler, nReadFd);

            return false;
        }

        return true;
    }

    /**
    * \brief 卸载管道事件
    * \param pHandler 事件处理回调
    * \param nReadFd 管道读fd
    * \return 卸载成功返回true，否则返回false
    */
    bool PipeReactor::DetachEvent(IPipeHandler *pHandler, int32 nReadFd)
    {
        if (NULL == m_pReactor)
        {
            return false;
        }

        EVENT_KEY key(EVENT_PIPE, nReadFd, 0, 0);

        MAP_PIPE_INFO::iterator it = m_mapPipes.find(key);

        // 从订阅列表移除
        if (it != m_mapPipes.end())
        {
            LIST_PIPE_INFO &lstSubscribers = it->second;

            LIST_PIPE_INFO::iterator iter = lstSubscribers.begin();
            for(; iter != lstSubscribers.end();)
            {
                REACTOR_EVENT *pEvent = *iter;
                if (NULL == pEvent)
                {
                    iter = lstSubscribers.erase(iter);
                    continue;
                }

                REACTOR_EVENT::PIPE_INFO &stInfo = pEvent->m_PipeInfo;

                if (stInfo.m_pHandler == pHandler)
                {
                    iter = lstSubscribers.erase(iter);
                    m_pReactor->PushEvent(pEvent);
                }
                else
                {
                    ++iter;
                }
            }

            if (lstSubscribers.empty())
            {
                m_mapPipes.erase(it);
            }
        }

        // 无订阅对象，注销管道事件
        if (m_mapPipes.empty())
        {
            UnRegisterEvent(nReadFd);
        }

        return true;
    }

    /**
    * \brief 管道事件回调处理
    * \param fd An fd or signal
    * \param events One or more EV_* flags
    * \param arg A user-supplied argument.
    */
    void PipeReactor::EventCallback(evutil_socket_t fd, short events, void *arg)
    {
        if (NULL == arg)
        {
            return;
        }

        PipeReactor *pPipeReactor = (PipeReactor *)(arg);
        if (NULL == pPipeReactor)
        {
            return;
        }

        pPipeReactor->OnNotify((int32)(fd));
    }

    /**
    * \brief 注册管道事件
    * \param nReadFd 读fd
    * \return 成功返回true，否则返回false
    */
    bool PipeReactor::RegisterEvent(int32 nReadFd)
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

        event *pEvent = event_new(pEventBase, nReadFd, EV_READ | EV_PERSIST, PipeReactor::EventCallback, (void *)(this));
        if (NULL == pEvent)
        {
            return false;
        }

        MAP_PIPE_EVENT::value_type value(nReadFd, pEvent);
        m_mapEvents.insert(value);

        // 注册管道事件
        event_add(pEvent, NULL);

        return true;
    }

    /**
    * \brief 注销管道事件
    * \param nReadFd 读fd
    * \return 成功返回true，否则返回false
    */
    bool PipeReactor::UnRegisterEvent(int32 nReadFd)
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

        MAP_PIPE_EVENT::iterator it = m_mapEvents.find(nReadFd);
        if (it != m_mapEvents.end())
        {
            // 删除管道事件
            event *pEvent = it->second;
            if (NULL != pEvent)
            {
                event_del(pEvent);
            }
            m_mapEvents.erase(it);
        }

        return true;
    }

    /**
    * \brief 信号事件处理
    * \param nSignalID 信号ID
    */
    void PipeReactor::OnNotify(int32 nReadFd)
    {
        // 读数据长度
        uint32 nLen = 0;
        if ( read(nReadFd, (void *)(&nLen), sizeof(nLen)) < 0 )
        {
            return;
        }

        // 读数据
        if (nLen > 0)
        {
            char *pBuffer = new char[nLen];
            if ( read(nReadFd, pBuffer, nLen) < 0 )
            {
                delete [] pBuffer;
                return;
            }

            EVENT_KEY key(EVENT_PIPE, nReadFd, 0, 0);

            // 取出所有订阅者，遍历执行
            MAP_PIPE_INFO::iterator it = m_mapPipes.find(key);
            if (it != m_mapPipes.end())
            {
                LIST_PIPE_INFO &lstSubscribers = it->second;

                LIST_PIPE_INFO::iterator iter = lstSubscribers.begin();
                for(; iter != lstSubscribers.end(); )
                {
                    REACTOR_EVENT *pEvent = *iter;

                    // 有无效订阅者
                    if (NULL == pEvent)
                    {
                        iter = lstSubscribers.erase(iter);
                        continue;
                    }

                    REACTOR_EVENT::PIPE_INFO &stInfo = pEvent->m_PipeInfo;

                    if (NULL != stInfo.m_pHandler)
                    {
                        stInfo.m_pHandler->OnNotify(nReadFd, pBuffer, nLen);
                    }

                    ++iter;
                }
            }

            delete [] pBuffer;
        }
    }
}