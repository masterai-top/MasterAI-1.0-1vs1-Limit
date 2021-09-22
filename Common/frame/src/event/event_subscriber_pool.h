/**
* \file event_subscriber_pool.h
* \brief 事件订阅者池
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __EVENT_SUBSCRIBER_POOL_H__
#define __EVENT_SUBSCRIBER_POOL_H__

#include "memory/array_pool.h"
#include "event/event_subscriber.h"
#include "event/eventdef.h"

namespace frame
{
    /**
    * \brief 事件订阅者池
    * \ingroup event_group
    */
    class EventSubscriberPool : public ArrayPool<EventSubscriber, SUBSCRIBER_POOL_INIT_NUM>
    {
    public:
        /**
        * \brief 构造函数
        */
        EventSubscriberPool(void) { };

        /**
        * \brief 析构函数
        */
        ~EventSubscriberPool(void) { };
    };
}

#endif // __EVENT_SUBSCRIBER_POOL_H__