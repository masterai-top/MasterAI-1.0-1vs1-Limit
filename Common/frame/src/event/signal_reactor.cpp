/**
* \file signal_reactor.cpp
* \brief 信号响应器类函数的实现
*/

#include "pch.h"
#include "event/signal_reactor.h"
#include "event/event_reactor.h"
#include "event2/event.h"


namespace frame
{
    /**
    * \brief 构造函数
    */
    SignalReactor::SignalReactor()
    {
        m_pReactor = NULL;
    }

    /**
    * \brief 析构函数
    */
    SignalReactor::~SignalReactor(void)
    {
    }

    /**
    * \brief 初始化
    * \param pReactor 事件响应器
    * \return 初始化成功返回true，否则返回false
    */
    bool SignalReactor::Init(EventReactor *pReactor)
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
    void SignalReactor::Restore()
    {
        m_pReactor = NULL;
    }

    /**
    * \brief 安装事件
    * \param pHandler 事件处理回调
    * \param nSignalID 信号ID
    * \param szDesc 事件描述
    * \return 安装成功返回true，否则返回false
    */
    bool SignalReactor::AttachEvent(ISignalHandler *pHandler, int32 nSignalID, const char *szDesc)
    {
        if (NULL == m_pReactor)
        {
            return false;
        }

        REACTOR_EVENT *pEvent = m_pReactor->PopEvent();
        if (NULL == pEvent)
        {
            return false;
        }

        REACTOR_EVENT::SIGNAL_INFO &stInfo = pEvent->m_SignalInfo;
        pEvent->m_nType = EVENT_SIGNAL;
        pEvent->m_strDesc = szDesc;
        stInfo.m_pHandler = pHandler;
        stInfo.m_nSignalID = nSignalID;

        EVENT_KEY key(EVENT_SIGNAL, nSignalID, 0, 0);

        MAP_SIGNAL_INFO::iterator it = m_mapSignals.find(key);

        // 加入到订阅列表
        if (it != m_mapSignals.end())
        {
            LIST_SIGNAL_INFO &lstSubscribers = it->second;
            lstSubscribers.push_back(pEvent);
        }
        else
        {
            LIST_SIGNAL_INFO lstSubscribers;
            lstSubscribers.push_back(pEvent);

            MAP_SIGNAL_INFO::value_type value(key, lstSubscribers);
            m_mapSignals.insert(value);
        }

        // 注册信号事件
        if (!RegisterEvent(nSignalID))
        {
            DetachEvent(pHandler, nSignalID);

            return false;
        }

        return true;
    }

    /**
    * \brief 卸载事件
    * \param pHandler 事件处理回调
    * \param nSignalID 信号ID
    * \return 卸载成功返回true，否则返回false
    */
    bool SignalReactor::DetachEvent(ISignalHandler *pHandler, int32 nSignalID)
    {
        if (NULL == m_pReactor)
        {
            return false;
        }

        EVENT_KEY key(EVENT_SIGNAL, nSignalID, 0, 0);

        MAP_SIGNAL_INFO::iterator it = m_mapSignals.find(key);

        // 从订阅列表移除
        if (it != m_mapSignals.end())
        {
            LIST_SIGNAL_INFO &lstSubscribers = it->second;

            LIST_SIGNAL_INFO::iterator iter = lstSubscribers.begin();
            for(; iter != lstSubscribers.end();)
            {
                REACTOR_EVENT *pEvent = *iter;
                if (NULL == pEvent)
                {
                    iter = lstSubscribers.erase(iter);
                    continue;
                }

                REACTOR_EVENT::SIGNAL_INFO &stInfo = pEvent->m_SignalInfo;

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
                m_mapSignals.erase(it);
            }
        }

        // 无订阅对象，注销信号事件
        if (m_mapSignals.empty())
        {
            UnRegisterEvent(nSignalID);
        }

        return true;
    }

    /**
    * \brief 信号回调处理
    * \param signo 信号ID
    * \param events 事件标识
    * \param arg 安装时的参数
    */
    void SignalReactor::SignalCallback(evutil_socket_t signo, short events, void *arg)
    {
        if (NULL == arg)
        {
            return;
        }

        SignalReactor *pSignalReactor = (SignalReactor *)(arg);
        if (NULL == pSignalReactor)
        {
            return;
        }

        pSignalReactor->OnSignal((int32)(signo));
    }

    /**
    * \brief 注册信号事件
    * \param nSignalID 信号ID
    * \return 成功返回true，否则返回false
    */
    bool SignalReactor::RegisterEvent(int32 nSignalID)
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

        // 已经注册过了
        if (m_mapEvents.find(nSignalID) != m_mapEvents.end())
        {
            return true;
        }

        event *pEvent = evsignal_new(pEventBase, nSignalID, SignalReactor::SignalCallback, (void *)(this));
        if (NULL == pEvent)
        {
            return false;
        }

        MAP_SIGNAL_EVENT::value_type value(nSignalID, pEvent);
        m_mapEvents.insert(value);

        // 注册信号事件
        evsignal_add(pEvent, NULL);

        return true;
    }

    /**
    * \brief 注销信号事件
    * \param nSignalID 信号ID
    * \return 成功返回true，否则返回false
    */
    bool SignalReactor::UnRegisterEvent(int32 nSignalID)
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

        MAP_SIGNAL_EVENT::iterator it = m_mapEvents.find(nSignalID);
        if (it != m_mapEvents.end())
        {
            // 删除信号事件
            event *pEvent = it->second;
            if (NULL != pEvent)
            {
                evsignal_del(pEvent);
            }
            m_mapEvents.erase(it);
        }

        return true;
    }

    /**
    * \brief 信号事件处理
    * \param nSignalID 信号ID
    */
    void SignalReactor::OnSignal(int32 nSignalID)
    {
        EVENT_KEY key(EVENT_SIGNAL, nSignalID, 0, 0);

        // 取出所有订阅者，遍历执行
        MAP_SIGNAL_INFO::iterator it = m_mapSignals.find(key);
        if (it != m_mapSignals.end())
        {
            LIST_SIGNAL_INFO &lstSubscribers = it->second;

            LIST_SIGNAL_INFO::iterator iter = lstSubscribers.begin();
            for(; iter != lstSubscribers.end(); )
            {
                REACTOR_EVENT *pEvent = *iter;

                // 有无效订阅者
                if (NULL == pEvent)
                {
                    iter = lstSubscribers.erase(iter);
                    continue;
                }

                REACTOR_EVENT::SIGNAL_INFO &stInfo = pEvent->m_SignalInfo;

                if (NULL != stInfo.m_pHandler)
                {
                    stInfo.m_pHandler->OnSignal(stInfo.m_nSignalID);
                }

                ++iter;
            }
        }
    }
}