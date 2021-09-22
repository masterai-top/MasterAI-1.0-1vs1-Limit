/**
* \file reactor_event_pool.h
* \brief 响应器事件池
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __REACTOR_EVENT_POOL_H__
#define __REACTOR_EVENT_POOL_H__

#include "memory/array_pool.h"
#include "event/reactor_event.h"
#include "event/eventdef.h"

namespace frame
{
    /**
    * \brief 响应器事件池
    * \ingroup event_group
    */
    class ReactorEventPool : public ArrayPool<REACTOR_EVENT, 80000>
    {
    public:
        /**
        * \brief 构造函数
        */
        ReactorEventPool(void) { };

        /**
        * \brief 析构函数
        */
        ~ReactorEventPool(void) { };
    };
}

#endif // __REACTOR_EVENT_POOL_H__