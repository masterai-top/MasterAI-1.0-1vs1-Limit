/**
* \file event_engine.cpp
* \brief 事件引擎的实现
*/

#include "pch.h"
#include "event/event_engine.h"
#include "event/event_dispatcher.h"
#include "event/event_reactor.h"


namespace frame
{
    /**
    * \brief 创建事件调度中心
    * \return 事件调度中心
    */
    IEventDispatcher * EventEngine::CreateEventDispatcher()
    {
        EventDispatcher *pDispatcher = new EventDispatcher();
        if (NULL == pDispatcher)
        {
            return NULL;
        }

        if ( !pDispatcher->Create() )
        {
            delete pDispatcher;
            return NULL;
        }

        return (IEventDispatcher *)(pDispatcher);
    }

    /**
    * \brief 释放事件调度中心
    * \param pEventDispatcher 事件调度中心
    * \return 释放成功返回true，否则返回false
    */
    bool EventEngine::ReleaseEventDispatcher(IEventDispatcher *pEventDispatcher)
    {
        if (NULL == pEventDispatcher)
        {
            return false;
        }

        pEventDispatcher->Release();

        return true;
    }

    /**
    * \brief 创建事件响应器
    * \param pEventBase 事件根基
    * \return 事件响应器
    */
    IEventReactor * EventEngine::CreateEventReactor(event_base *pEventBase)
    {
        if (NULL == pEventBase)
        {
            return NULL;
        }

        EventReactor *pReactor = new EventReactor();
        if ( NULL == pReactor )
        {
            return NULL;
        }

        if ( !pReactor->Create(pEventBase) )
        {
            delete pReactor;
            return NULL;
        }

        return (IEventReactor *)(pReactor);
    }

    /**
    * \brief 释放事件响应器
    * \param pEventReactor 事件响应器
    * \return 释放成功返回true，否则返回false
    */
    bool EventEngine::ReleaseEventReactor(IEventReactor *pEventReactor)
    {
        if (NULL == pEventReactor)
        {
            return false;
        }

        pEventReactor->Release();

        return true;
    }
}