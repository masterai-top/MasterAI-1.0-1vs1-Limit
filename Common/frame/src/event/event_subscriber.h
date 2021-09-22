/**
* \file event_subscriber.h
* \brief 事件订阅者
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __EVENT_SUBSCRIBER_H__
#define __EVENT_SUBSCRIBER_H__

#include "typedef.h"
#include <string>

namespace frame
{
    class ISubHandler;

    /**
    * \brief 事件订阅者
    * \ingroup event_group
    */
    class EventSubscriber
    {
    public:
        /**
        * \brief 构造函数
        */
        EventSubscriber();

        /**
        * \brief 析构函数
        */
        virtual ~EventSubscriber(void);

        /**
        * \brief 初始化
        * \param pHandler 事件处理回调
        * \param szDesc 事件描述
        * \return 成功返回true，否则返回fasle
        */
        bool Init(ISubHandler *pHandler, const char *szDesc);

        /**
        * \brief 清除数据
        */
        virtual void Restore();

        /**
        * \brief 得到事件处理回调
        * \return 事件处理回调
        */
        ISubHandler * GetHandler();

        /**
        * \brief 增加引用次数
        */
        void AddRefCount();

        /**
        * \brief 减少引用次数
        */
        void SubRefCount();

        /**
        * \brief 得到引用次数
        * \return 引用次数
        */
        uint32 GetRefCount();

        /**
        * \brief 判断删除标志是否设置
        * \return 设置了删除标志返回true，否则返回false
        */
        bool IsRemove();

        /**
        * \brief 设置删除标志
        * \param bRemove 删除标志
        */
        void SetRemove(bool bRemove);

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
        virtual bool FireEvent(uint8 nEventType, uint32 nEventID, uint32 nSrcType, uint64 nSrcID, LPCSTR lpContext, size_t nLen);

    private:
        ISubHandler     *m_pHandler;        ///< 事件处理回调
        std::string     m_strDesc;          ///< 事件描述

        uint32          m_nRefCount;        ///< 引用次数
        bool            m_bRemove;          ///< 删除标志
    };
}

#endif // __EVENT_SUBSCRIBER_H__
