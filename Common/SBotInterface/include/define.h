/**
* \file define.h
* \brief AI服定义
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __BRAIN_DEFINE_H__
#define __BRAIN_DEFINE_H__

#include "server_define.h"
#include "system.pb.h"


enum
{
    TIMER_ID_KEEPALIVE = 0,            ///< ping
    TIMER_ID_RECONNECT,                ///< 重连
};



#endif // __BRAIN_DEFINE_H__

