/**
* \file app_signal.h
* \brief 应用信号事件类
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __APP_SIGNAL_H__
#define __APP_SIGNAL_H__

#include "event/event_engine.h"

namespace frame
{
    /**
    * \brief 应用信号事件类
    * \ingroup app_event
    */
    class AppSignal : public frame::ISignalHandler
    {
    public:
        /**
        * \brief 构造函数
        */
        AppSignal(void);

        /**
        * \brief 析构函数
        */
        ~AppSignal(void);

        /**
        * \brief 初始化
        * \return 初始化成功返回true，否则返回false
        */
        bool Init();

        /**
        * \brief 释放
        */
        void Restore();

        /**
        * \brief 发送信号事件
        * \param nSignalID 信号ID
        * \param nPID 进程ID，给本进程发送信号，nPID=0
        * \return 成功返回true，否则返回false
        */
        void SendSignal(int32 nSignalID, uint32 nPID);

    public:
        /**
        * \brief 信号事件处理
        * \param nSignalID 信号ID
        */
        virtual void OnSignal(int32 nSignalID);
    };
}

#endif // __APP_SIGNAL_H__