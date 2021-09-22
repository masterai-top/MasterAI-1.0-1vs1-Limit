/**
* \file netdef.h
* \brief 网络库的结构定义
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __NET_DEFINE_H__
#define __NET_DEFINE_H__

/**
* \brief 网络相关宏定义
* \ingroup net_group
*/
enum
{
    TID_LEN                 = 22,       // 任务ID长度
    PACKAGE_HEAD_LEN        = 60,       // 包头长度
    PACKAGE_LEN_MAX         = (1024*8), ///< 网络消息缓存最大值
    PACKAGE_POOL_INIT_NUM   = 500,      ///< 网络包池初始数量
    SOCKET_POOL_INIT_NUM    = 500,      ///< 套接字池初始数量
};
    



#endif // __NET_DEF_H__
