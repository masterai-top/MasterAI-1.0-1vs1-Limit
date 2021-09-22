/**
* \file appdef.h
* \brief 应用的基础定义
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __APP_DEF_H__
#define __APP_DEF_H__

namespace frame
{
#ifdef WIN32
#   define APP_PID_FILE "C:\\%s.pid"        /// 保存正在运行的进程ID文件名
#else
#   define APP_PID_FILE "/tmp/%s.pid"       /// 保存正在运行的进程ID文件名
#endif

    /// 应用启动命令
    enum RUN_COMMAND
    {
        RUN_CMD_START               = 1,    // 启动命令
        RUN_CMD_RESTART             = 2,    // 重启命令
        RUN_CMD_STOP                = 3,    // 停止命令
        RUN_CMD_RELOAD              = 4,    // 重载命令
    };
}

#endif // __APP_DEF_H__