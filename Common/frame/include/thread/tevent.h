/**
* \file tevent.h
* \brief 信号事件类
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __TEVENT_H__
#define __TEVENT_H__

#include "framedef.h"
#include "typedef.h"
#include "thread/lock.h"
#include "thread/cond.h"

namespace frame
{
    /**
    * \brief 信号事件类
    * \ingroup thread_group
    * 等效于 Windows Event 的实现
    */
    class FRAME_EXPORT TEvent
    {
    public:
        /**
        * \brief 构造函数
        * \param state 初始状态，默认false
        * \param reset 是否自动复位，默认false
        *
        * 非自动复位模式下，有信号状态一直持续到调用Reset()。
        * 自动复位模式，当有信号状态持续到单个等待线程被释放时，会自动复位为无信号状态
        */
        TEvent(bool state = false, bool reset = false);

        /**
        * \brief 析构函数
        */
        virtual ~TEvent(void);

        /**
        * \brief 设置为有信号状态
        */
        void Set();

        /**
        * \brief 复位为无信号状态
        */
        void Reset();

        /**
        * \brief 等待有信号状态
        * \param secs 超时的秒，默认-1，表示不超时
        * \param msecs 超时的毫秒，默认0
        * \param nsecs 超时的纳秒，默认0
        * \return true表示有信号，false表示等待超时，或者Event被释放
        */
        bool Wait(int32 secs = -1, int32 msecs = 0, int32 nsecs = 0);

    private:
        bool        m_bState;       ///< 信号状态，true表示有信号，false表示无
        bool        m_bReset;       ///< 指明是否自动复位

        LockObject  m_Mutex;        ///< 事件访问锁
        Condition   m_Cond;         ///< 事件条件变量
    };
}

#endif // __TEVENT_H__