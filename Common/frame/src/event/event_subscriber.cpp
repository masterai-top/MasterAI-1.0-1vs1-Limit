/**
* \file event_observer.cpp
* \brief 事件观察者类函数的实现
*/

#include "pch.h"
#include "event/event_subscriber.h"
#include "event/event_engine.h"
#include "event/eventdef.h"


namespace frame
{
    /**
    * \brief 构造函数
    */
    EventSubscriber::EventSubscriber()
    {
        m_pHandler = NULL;
        m_strDesc = "";
    }

    /**
    * \brief 析构函数
    */
    EventSubscriber::~EventSubscriber(void)
    {
    }

    /**
    * \brief 初始化
    * \param pHandler 事件处理回调
    * \param szDesc 事件描述
    * \return 成功返回true，否则返回fasle
    */
    bool EventSubscriber::Init(ISubHandler *pHandler, const char *szDesc)
    {
        m_pHandler = pHandler;
        m_strDesc = szDesc;
        return true;
    }

    /**
    * \brief 清除数据
    */
    void EventSubscriber::Restore()
    {
        m_pHandler = NULL;
        m_strDesc = "";
    }

    /**
    * \brief 得到事件处理回调
    * \return 事件处理回调
    */
    ISubHandler * EventSubscriber::GetHandler()
    {
        return m_pHandler;
    }

    /**
    * \brief 增加引用次数
    */
    void EventSubscriber::AddRefCount()
    {
        m_nRefCount++;
    }

    /**
    * \brief 减少引用次数
    */
    void EventSubscriber::SubRefCount()
    {
        if (m_nRefCount > 0)
        {
            m_nRefCount--;
        }
    }

    /**
    * \brief 得到引用次数
    * \return 引用次数
    */
    uint32 EventSubscriber::GetRefCount()
    {
        return m_nRefCount;
    }

    /**
    * \brief 判断删除标志是否设置
    * \return 设置了删除标志返回true，否则返回false
    */
    bool EventSubscriber::IsRemove()
    {
        return m_bRemove;
    }

    /**
    * \brief 设置删除标志
    * \param bRemove 删除标志
    */
    void EventSubscriber::SetRemove(bool bRemove)
    {
        m_bRemove = bRemove;
    }

    /**
    * \brief 发起事件
    * \param nEventType 事件类型
    * \param nEventID 事件ID
    * \param nSrcType 发送源类型
    * \param nSrcID 发送源标识
    * \param lpContext 上下文数据
    * \param nLen 上下文数据长度
    * \return 处理结果
    */
    bool EventSubscriber::FireEvent(uint8 nEventType, uint32 nEventID, uint32 nSrcType, uint64 nSrcID, LPCSTR lpContext, size_t nLen)
    {
        if (NULL == m_pHandler)
        {
            return false;
        }

        bool bResult = true;

        switch (nSrcType)
        {
        case EVENT_EXECUTE:
            m_pHandler->OnExecute(nEventID, nSrcType, nSrcID, lpContext, nLen);
            break;
        case EVENT_VOTE:
            bResult = m_pHandler->OnVote(nEventID, nSrcType, nSrcID, lpContext, nLen);
        default:
            return false;
            break;
        }

        return bResult;
    }
}