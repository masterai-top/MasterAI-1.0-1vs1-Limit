/**
* \file  define.h
* \brief 定义全局变量&函数
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
*/

#ifndef __B_DEFINE_H__
#define __B_DEFINE_H__

#include "server_define.h"
#include "system.pb.h"
#include "redis_connector.h"
#include "shstd.h"
#include "frame.h"
#include "net_engine.h"
#include "socket_manager.h"
#include "server_connector.h"
#include "CQUEUE_List.h"
#include "BettingTree.h"
#include "client_hashmap.h"
#include "id_locks.h"
#include "apg_nndef.h"
#include <atomic>

//定时器ID
enum
{
    TIMER_ID_DATA_KEEPALIVE = 0,            // ping
    TIMER_ID_DATA_RECONNECT,                // 重连
    
    TIMER_ID_CFR_KEEPALIVE = 100,           // ping
    TIMER_ID_CFR_RECONNECT = 101,           // 重连
};

enum 
{
    TIMER_CMD_CHECK_CLIENT_TIMEOUT      = Pb::CMD_COMM_TIMER_CALLBACK_MIN + 1, 
    TIMER_CMD_CHECK_RUNGAME_TIMEOUT     = Pb::CMD_COMM_TIMER_CALLBACK_MIN + 2,
};

//全局变量定义
extern CServerConnector         g_oDataConnector;       //数据统计分析服务连接器
extern CServerConnector         g_oCfrConnector;        //cfr服务连接器
extern CIDLocks                 g_oRobotIDLocks;        //机器人ID全局锁
extern CRedisConnector          g_oHistoryRedis;        //历史记录的redis服务


extern int ApgCardToLocal(const nndef::nncard::card_t &card, uint8 &nLocalCard);

#endif // __B_DEFINE_H__


