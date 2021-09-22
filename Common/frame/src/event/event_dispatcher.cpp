/**
* \file event_dispatcher.cpp
* \brief 事件调度中心类函数的实现
*/

#include "pch.h"
#include "event/event_dispatcher.h"


namespace frame
{
    /**
    * \brief 构造函数
    */
    EventDispatcher::EventDispatcher()
    {
        m_nHandleCount = 0;
    }

    /**
    * \brief 析构函数
    */
    EventDispatcher::~EventDispatcher(void)
    {
        m_nHandleCount = 0;
    }

    /**
    * \brief 创建
    * \return 创建成功返回true，否则返回false
    */
    bool EventDispatcher::Create()
    {
        return true;
    }

    /**
    * \brief 释放
    */
    void EventDispatcher::Release()
    {
        delete this;
    }

    /**
    * \brief 订阅执行事件
    * \param pHandler 事件处理回调
    * \param nEventID 事件ID
    * \param nSrcType 发送源类型
    * \param nSrcID 发送源标识
    * \param szDesc 事件描述
    * \return 订阅成功返回true，否则返回false
    */
    bool EventDispatcher::AttachExecute(ISubHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID, const char *szDesc)
    {
        if (NULL == pHandler)
        {
            return false;
        }

        EVENT_KEY key(EVENT_EXECUTE, nEventID, nSrcType, nSrcID);
        EventSubscriber *pSubscriber = m_SubscriberPool.Pop();
        if (NULL == pSubscriber)
        {
            return false;
        }
        pSubscriber->Init(pHandler, szDesc);

        MAP_EVENT_INFO::iterator it = m_mapEvents.find(key);

        // 加入到订阅列表
        if (it != m_mapEvents.end())
        {
            LIST_EVENT_INFO &lstSubscribers = it->second;
            lstSubscribers.push_back(pSubscriber);
        }
        else
        {
            LIST_EVENT_INFO lstSubscribers;
            lstSubscribers.push_back(pSubscriber);

            MAP_EVENT_INFO::value_type value(key, lstSubscribers);
            m_mapEvents.insert(value);
        }

        return true;
    }

    /**
    * \brief 取消订阅执行事件
    * \param pHandler 事件处理回调
    * \param nEventID 事件ID
    * \param nSrcType 发送源类型
    * \param nSrcID 发送源标识
    * \return 取消订阅成功返回true，否则返回false
    */
    bool EventDispatcher::DetachExecute(ISubHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID)
    {
        if (NULL == pHandler)
        {
            return false;
        }

        EVENT_KEY key(EVENT_EXECUTE, nEventID, nSrcType, nSrcID);

        MAP_EVENT_INFO::iterator it = m_mapEvents.find(key);

        // 从订阅列表移除
        if (it != m_mapEvents.end())
        {
            LIST_EVENT_INFO &lstSubscribers = it->second;

            LIST_EVENT_INFO::iterator iter = lstSubscribers.begin();
            for(; iter != lstSubscribers.end();)
            {
                EventSubscriber *pSubscriber = *iter;
                if (NULL == pSubscriber)
                {
                    iter = lstSubscribers.erase(iter);
                    continue;
                }

                if (pSubscriber->GetHandler() == pHandler)
                {
                    if (pSubscriber->GetRefCount() == 0)
                    {
                        iter = lstSubscribers.erase(iter);
                        pSubscriber->Restore();
                        m_SubscriberPool.Push(pSubscriber);
                    }
                    else
                    {
                        pSubscriber->SetRemove(true);
                    }
                }
                else
                {
                    ++iter;
                }
            }

            if (lstSubscribers.empty())
            {
                m_mapEvents.erase(it);
            }
        }

        return true;
    }

    /**
    * \brief 发起执行事件
    * \param nEventID 事件ID
    * \param nSrcType 发送源类型
    * \param nSrcID 发送源标识
    * \param lpContext 上下文数据
    * \param nLen 上下文数据长度
    * \return 发起成功即返回true，否则返回false
    */
    bool EventDispatcher::FireExecute(uint32 nEventID, uint32 nSrcType, uint64 nSrcID, LPCSTR lpContext, size_t nLen)
    {
        EVENT_KEY key(EVENT_EXECUTE, nEventID, nSrcType, nSrcID);

        // 先发送有源ID的
        if (key.m_nSrcID != 0 && !FireExecute(key, nSrcID, lpContext, nLen))
        {
            return false;
        }

        // 然后发送无源ID的
        key.m_nSrcID = 0;
        if (!FireExecute(key, nSrcID, lpContext, nLen))
        {
            return false;
        }

        return true;
    }

    /**
    * \brief 订阅票决事件
    * \param pHandler 事件处理回调
    * \param nEventID 事件ID
    * \param nSrcType 发送源类型
    * \param nSrcID 发送源标识
    * \param szDesc 事件描述
    * \return 订阅成功返回true，否则返回false
    */
    bool EventDispatcher::AttachVote(ISubHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID, const char *szDesc)
    {
        if (NULL == pHandler)
        {
            return false;
        }

        EVENT_KEY key(EVENT_VOTE, nEventID, nSrcType, nSrcID);
        EventSubscriber *pSubscriber = m_SubscriberPool.Pop();
        if (NULL == pSubscriber)
        {
            return false;
        }
        pSubscriber->Init(pHandler, szDesc);

        MAP_EVENT_INFO::iterator it = m_mapEvents.find(key);

        // 加入到订阅列表
        if (it != m_mapEvents.end())
        {
            LIST_EVENT_INFO &lstSubscribers = it->second;
            lstSubscribers.push_back(pSubscriber);
        }
        else
        {
            LIST_EVENT_INFO lstSubscribers;
            lstSubscribers.push_back(pSubscriber);

            MAP_EVENT_INFO::value_type value(key, lstSubscribers);
            m_mapEvents.insert(value);
        }

        return true;
    }

    /**
    * \brief 取消订阅票决事件
    * \param pHandler 事件处理回调
    * \param nEventID 事件ID
    * \param nSrcType 发送源类型
    * \param nSrcID 发送源标识
    * \return 取消订阅成功返回true，否则返回false
    */
    bool EventDispatcher::DetachVote(ISubHandler *pHandler, uint32 nEventID, uint32 nSrcType, uint64 nSrcID)
    {
        if (NULL == pHandler)
        {
            return false;
        }

        EVENT_KEY key(EVENT_VOTE, nEventID, nSrcType, nSrcID);

        MAP_EVENT_INFO::iterator it = m_mapEvents.find(key);

        // 从订阅列表移除
        if (it != m_mapEvents.end())
        {
            LIST_EVENT_INFO &lstSubscribers = it->second;

            LIST_EVENT_INFO::iterator iter = lstSubscribers.begin();
            for(; iter != lstSubscribers.end();)
            {
                EventSubscriber *pSubscriber = *iter;
                if (NULL == pSubscriber)
                {
                    iter = lstSubscribers.erase(iter);
                    continue;
                }

                if (pSubscriber->GetHandler() == pHandler)
                {
                    if (pSubscriber->GetRefCount() == 0)
                    {
                        iter = lstSubscribers.erase(iter);
                        pSubscriber->Restore();
                        m_SubscriberPool.Push(pSubscriber);
                    }
                    else
                    {
                        pSubscriber->SetRemove(true);
                    }
                }
                else
                {
                    ++iter;
                }
            }

            if (lstSubscribers.empty())
            {
                m_mapEvents.erase(it);
            }
        }

        return true;
    }

    /**
    * \brief 发起票决事件
    * \param nEventID 事件ID
    * \param nSrcType 发送源类型
    * \param nSrcID 发送源标识
    * \param lpContext 上下文数据
    * \param nLen 上下文数据长度
    * \return 有订阅者否决了返回false，全都支持返回true
    */
    bool EventDispatcher::FireVote(uint32 nEventID, uint32 nSrcType, uint64 nSrcID, LPCSTR lpContext, size_t nLen)
    {
        EVENT_KEY key(EVENT_VOTE, nEventID, nSrcType, nSrcID);

        // 先发送有源ID的
        if (key.m_nSrcID != 0 && !FireVote(key, nSrcID, lpContext, nLen))
        {
            return false;
        }

        // 然后发送无源ID的
        key.m_nSrcID = 0;
        if (!FireVote(key, nSrcID, lpContext, nLen))
        {
            return false;
        }

        return true;
    }

    /**
    * \brief 发起执行事件
    * \param key 事件key
    * \param nSrcID 发送源ID
    * \param lpContext 上下文数据
    * \param nLen 上下文数据长度
    * \return 成功返回true，否则返回false
    */
    bool EventDispatcher::FireExecute(EVENT_KEY key, uint64 nSrcID, LPCSTR lpContext, size_t nLen)
    {
        m_nHandleCount++;

        if (m_nHandleCount >= HANDLE_COUNT_MAX)
        {
            m_nHandleCount--;
            return false;
        }

        // 取出所有订阅者，遍历执行
        MAP_EVENT_INFO::iterator it = m_mapEvents.find(key);
        if (it != m_mapEvents.end())
        {
            LIST_EVENT_INFO &lstSubscribers = it->second;

            LIST_EVENT_INFO::iterator iter = lstSubscribers.begin();
            for(; iter != lstSubscribers.end(); )
            {
                EventSubscriber *pSubscriber = *iter;

                // 有无效订阅者
                if (NULL == pSubscriber)
                {
                    iter = lstSubscribers.erase(iter);
                    m_nHandleCount--;
                    continue;
                }

                // 订阅者引用次数达到最大层数
                if (pSubscriber->GetRefCount() >= REF_COUNT_MAX)
                {
                    m_nHandleCount--;
                    continue;
                }

                // 订阅者还在观察事件
                if (!pSubscriber->IsRemove())
                {
                    try
                    {
                        pSubscriber->AddRefCount();
                        pSubscriber->FireEvent(key.m_nEventType, key.m_nEventID, key.m_nSrcType, nSrcID, lpContext, nLen);  // 处理事件
                        pSubscriber->SubRefCount();
                    }
                    // 事件处理过程中出错
                    catch(...)
                    {
                        pSubscriber->SubRefCount();
                        continue;
                    }

                    // 在事件处理过程中移除订阅列表
                    if (pSubscriber->IsRemove() && pSubscriber->GetRefCount() == 0)
                    {
                        iter = lstSubscribers.erase(iter);
                        pSubscriber->Restore();
                        m_SubscriberPool.Push(pSubscriber);
                    }
                    else
                    {
                        ++iter;
                    }
                }
                // 订阅者已经不观察事件了
                else
                {
                    if (pSubscriber->GetRefCount() == 0)
                    {
                        iter = lstSubscribers.erase(iter);
                        pSubscriber->Restore();
                        m_SubscriberPool.Push(pSubscriber);
                    }
                    else
                    {
                        ++iter;
                    }
                }

                if (lstSubscribers.empty())
                {
                    m_mapEvents.erase(it);
                }
            }
        }

        m_nHandleCount--;

        return true;
    }

    /**
    * \brief 发起票决事件
    * \param key 事件key
    * \param nSrcID 发送源ID
    * \param lpContext 上下文数据
    * \param nLen 上下文数据长度
    * \return 有观察者否决了返回false，全都支持返回true
    */
    bool EventDispatcher::FireVote(EVENT_KEY key, uint64 nSrcID, LPCSTR lpContext, size_t nLen)
    {
        m_nHandleCount++;

        if (m_nHandleCount >= HANDLE_COUNT_MAX)
        {
            m_nHandleCount--;
            return false;
        }

        // 取出所有订阅者，遍历执行
        MAP_EVENT_INFO::iterator it = m_mapEvents.find(key);
        if (it != m_mapEvents.end())
        {
            LIST_EVENT_INFO &lstSubscribers = it->second;

            LIST_EVENT_INFO::iterator iter = lstSubscribers.begin();
            for(; iter != lstSubscribers.end(); )
            {
                EventSubscriber *pSubscriber = *iter;

                // 有无效订阅者
                if (NULL == pSubscriber)
                {
                    iter = lstSubscribers.erase(iter);
                    m_nHandleCount--;
                    return false;
                }

                // 订阅者引用次数达到最大层数
                if (pSubscriber->GetRefCount() >= REF_COUNT_MAX)
                {
                    m_nHandleCount--;
                    return false;
                }

                // 订阅者还在观察事件
                if (!pSubscriber->IsRemove())
                {
                    bool result = false;
                    try
                    {
                        pSubscriber->AddRefCount();
                        result = pSubscriber->FireEvent(key.m_nEventType, key.m_nEventID, key.m_nSrcType, nSrcID, lpContext, nLen);  // 处理事件
                        pSubscriber->SubRefCount();
                    }
                    // 事件处理过程中出错
                    catch(...)
                    {
                        pSubscriber->SubRefCount();
                        return false;
                    }

                    // 在事件处理过程中移除订阅列表
                    if (pSubscriber->IsRemove() && pSubscriber->GetRefCount() == 0)
                    {
                        iter = lstSubscribers.erase(iter);
                        pSubscriber->Restore();
                        m_SubscriberPool.Push(pSubscriber);
                    }
                    else
                    {
                        ++iter;
                    }

                    if (!result)    // 被订阅者否决了
                    {
                        m_nHandleCount--;
                        return false;
                    }
                }
                // 订阅者已经不观察事件了
                else
                {
                    if (pSubscriber->GetRefCount() == 0)
                    {
                        iter = lstSubscribers.erase(iter);
                        pSubscriber->Restore();
                        m_SubscriberPool.Push(pSubscriber);
                    }
                    else
                    {
                        ++iter;
                    }
                }

                if (lstSubscribers.empty())
                {
                    m_mapEvents.erase(it);
                }
            }
        }

        m_nHandleCount--;

        return true;
    }
}