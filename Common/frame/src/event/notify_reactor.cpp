/**
* \file notify_reactor.cpp
* \brief 通知响应器类函数的实现
*/

#include "pch.h"
#include "event/notify_reactor.h"
#include "event/event_reactor.h"
#include "event2/event.h"


namespace frame
{
    /**
    * \brief 构造函数
    */
    NotifyReactor::NotifyReactor(void)
    {
        m_pReactor = NULL;
    }

    /**
    * \brief 析构函数
    */
    NotifyReactor::~NotifyReactor(void)
    {
    }

    /**
    * \brief 初始化
    * \param pReactor 事件响应器
    * \return 初始化成功返回true，否则返回false
    */
    bool NotifyReactor::Init(EventReactor *pReactor)
    {
        if (NULL == pReactor)
        {
            return false;
        }

        event_base *pEventBase = pReactor->GetEventBase();
        if (NULL == pEventBase)
        {
            return false;
        }

        if ( bufferevent_pair_new(pEventBase, BEV_OPT_THREADSAFE, m_evPair) < 0 )
        {
            return false;
        }

        bufferevent_setcb(m_evPair[0], NotifyReactor::NotifyCallback, NULL, NotifyReactor::EventCallback, this);

        /* We have to enable it before our callbacks will be
        * called. */
        bufferevent_enable(m_evPair[0], EV_READ);

        m_pReactor = pReactor;

        return true;
    }

    /**
    * \brief 释放
    */
    void NotifyReactor::Restore()
    {
        if (NULL != m_evPair[0])
        {
            bufferevent_free(m_evPair[0]);
        }

        if (NULL != m_evPair[1])
        {
            bufferevent_free(m_evPair[1]);
        }

        m_pReactor = NULL;
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
    bool NotifyReactor::AttachEvent(INotifyHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID, const char *szDesc)
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

        REACTOR_EVENT::NOTIFY_INFO &stInfo = pEvent->m_NotifyInfo;
        pEvent->m_nType = EVENT_NOTIFY;
        pEvent->m_strDesc = szDesc;
        stInfo.m_pHandler = pHandler;

        EVENT_KEY key(EVENT_NOTIFY, nEventID, nSrcType, nSrcID);

        MAP_NOTIFY_INFO::iterator it = m_mapNotifys.find(key);

        // 加入到订阅列表
        if (it != m_mapNotifys.end())
        {
            LIST_NOTIFY_INFO &lstSubscribers = it->second;
            lstSubscribers.push_back(pEvent);
        }
        else
        {
            LIST_NOTIFY_INFO lstSubscribers;
            lstSubscribers.push_back(pEvent);

            MAP_NOTIFY_INFO::value_type value(key, lstSubscribers);
            m_mapNotifys.insert(value);
        }

        return true;
    }

    /**
    * \brief 取消订阅通知事件
    * \param pHandler 事件处理回调
    * \param nEventID 事件ID
    * \param nSrcType 发送源类型
    * \param nSrcID 发送源标识
    * \return 取消订阅成功返回true，否则返回false
    */
    bool NotifyReactor::DetachEvent(INotifyHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID)
    {
        if (NULL == m_pReactor)
        {
            return false;
        }

        EVENT_KEY key(EVENT_NOTIFY, nEventID, nSrcType, nSrcID);

        MAP_NOTIFY_INFO::iterator it = m_mapNotifys.find(key);

        // 从订阅列表移除
        if (it != m_mapNotifys.end())
        {
            LIST_NOTIFY_INFO &lstSubscribers = it->second;

            LIST_NOTIFY_INFO::iterator iter = lstSubscribers.begin();
            for(; iter != lstSubscribers.end();)
            {
                REACTOR_EVENT *pEvent = *iter;
                if (NULL == pEvent)
                {
                    iter = lstSubscribers.erase(iter);
                    continue;
                }

                REACTOR_EVENT::NOTIFY_INFO &stInfo = pEvent->m_NotifyInfo;

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
                m_mapNotifys.erase(it);
            }
        }

        return true;
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
    bool NotifyReactor::PostEvent(uint32 nEventID, uint32 nSrcType, uint64 nSrcID, LPCSTR lpContext, size_t nLen)
    {
        EVENT_KEY key(EVENT_NOTIFY, nEventID, nSrcType, nSrcID);

        uint32 nTotalLen = (uint32)(sizeof(key) + nLen);

        //初始化一个缓冲区
        evbuffer *pBuffer = evbuffer_new();

        //写入总长度
        evbuffer_add(pBuffer, (const char *)(&nTotalLen), sizeof(nTotalLen));

        //写入事件key
        evbuffer_add(pBuffer, (const char *)(&key), sizeof(key));

        //写入待发送数据
        if (nLen > 0)
        {
            evbuffer_add(pBuffer, lpContext, nLen);
        }

        //将待发送数据写入发送缓冲区，即bufferevent->output
        bufferevent_write_buffer(m_evPair[1], pBuffer);

        //释放缓冲区内存
        evbuffer_drain(pBuffer, nTotalLen + sizeof(nTotalLen));

        return true;
    }

    /**
    * \brief 通知回调处理
    * \param bev the bufferevent that triggered the callback
    * \param ctx the user-specified context for this bufferevent
    */
    void NotifyReactor::NotifyCallback(struct bufferevent *bev, void *arg)
    {
        if (NULL == arg)
        {
            return;
        }

        NotifyReactor *pNotifyReactor = (NotifyReactor *)(arg);
        if (NULL == pNotifyReactor)
        {
            return;
        }

        while (1)
        {
            //获取输入缓存
            struct evbuffer * pInput = bufferevent_get_input(pNotifyReactor->m_evPair[0]);
            //获取输入缓存数据的长度
            size_t nLen = evbuffer_get_length(pInput);

            if (nLen < sizeof(uint32) + sizeof(EVENT_KEY))
            {
                break;
            }

            //有数据，处理数据
            if (nLen >= sizeof(uint32) + sizeof(EVENT_KEY))      // 数据长度 + 事件key + 上下文数据
            {
                //获取数据的地址
                unsigned char *pContext = evbuffer_pullup(pInput, nLen);

                pContext[nLen] = 0;

                uint32 nTotalLen = *(uint32 *)(pContext);

                EVENT_KEY key = *(EVENT_KEY *)(pContext + sizeof(nTotalLen));

                LPCSTR lpContext = (LPCSTR)(pContext + sizeof(nTotalLen) + sizeof(key));

                size_t nLength = nTotalLen - sizeof(key);

                // 先发送有源ID的
                uint64 nSrcID = key.m_nSrcID;
                if (nSrcID != 0 && !pNotifyReactor->OnNotify(key, nSrcID, lpContext, nLength))
                {
                    return;
                }

                // 然后发送无源ID的
                key.m_nSrcID = 0;
                if (!pNotifyReactor->OnNotify(key, nSrcID, lpContext, nLength))
                {
                    return;
                }

                //删除数据
                evbuffer_drain(pInput, nLen);
            }
        }
    }

    /**
    * \brief 网络事件回调处理
    * \param bev the bufferevent for which the error condition was reached
    * \param what a conjunction of flags: BEV_EVENT_READING or BEV_EVENT_WRITING
                to indicate if the error was encountered on the read or write path,
                and one of the following flags: BEV_EVENT_EOF, BEV_EVENT_ERROR,
                BEV_EVENT_TIMEOUT, BEV_EVENT_CONNECTED.
    * \param ctx the user-specified context for this bufferevent
    */
    void NotifyReactor::EventCallback(struct bufferevent *bev, short what, void *ctx)
    {
        std::string strError = evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR());
    }

    /**
    * \brief 通知事件处理
    * \param key 事件key
    * \param nSrcID 发送源标识
    * \param lpContext 上下文数据
    * \param nLen 上下文数据长度
    * \return 成功返回true，否则返回false
    */
    bool NotifyReactor::OnNotify(EVENT_KEY key, uint64 nSrcID, LPCSTR lpContext, size_t nLen)
    {
        // 取出所有订阅者，遍历执行
        MAP_NOTIFY_INFO::iterator it = m_mapNotifys.find(key);
        if (it != m_mapNotifys.end())
        {
            LIST_NOTIFY_INFO &lstSubscribers = it->second;

            LIST_NOTIFY_INFO::iterator iter = lstSubscribers.begin();
            for(; iter != lstSubscribers.end(); )
            {
                REACTOR_EVENT *pEvent = *iter;

                // 有无效订阅者
                if (NULL == pEvent)
                {
                    iter = lstSubscribers.erase(iter);
                    continue;
                }

                REACTOR_EVENT::NOTIFY_INFO &stInfo = pEvent->m_NotifyInfo;

                if (NULL != stInfo.m_pHandler)
                {
                    stInfo.m_pHandler->OnNotify(key.m_nEventID, key.m_nSrcType, nSrcID, lpContext, nLen);
                }

                ++iter;
            }
        }

        return true;
    }
}
