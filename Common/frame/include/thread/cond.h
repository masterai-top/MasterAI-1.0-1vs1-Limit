/**
* \file cond.h
* \brief 条件变量
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __CONDITION_H__
#define __CONDITION_H__

#include "framedef.h"
#include "thread/lock.h"
#include "pthread.h"

namespace frame
{
    /**
    * \brief 条件变量对象
    * \ingroup thread_group
    */
    class FRAME_EXPORT Condition
    {
    public:
        /**
        * \brief 构造函数
        */
        Condition(void);

        /**
        * \brief 析构函数
        */
        virtual ~Condition(void);

        /**
        * \brief 等待
        * \param lock 线程锁，通过该锁使线程阻塞
        */
        void Wait(Lock &lock);

        /**
        * \brief 非阻塞等待
        * \param lock 线程锁，通过该锁使线程阻塞
        * \param timeout 等待时间，指定时间内线程阻塞，直到时间超时
        * \return 有信号返回true，等待超时返还false
        */
        bool TimeWait(Lock &lock, timespec timeout);

        /**
        * \brief 发出信号
        */
        void Signal();

        /**
        * \brief 广播信号
        */
        void Broadcast();

    private:
        pthread_cond_t  m_cond;     ///< 条件对象
    };
}

/**
* \brief 条件变量定义宏
* \ingroup thread_group
* \def CONDITION_DEF(COND)
* \param COND 条件变量名称
*
* \par 范例:
* \code
*   CONDITION_DEF(ThreadCond);  // 线程条件变量
*   CONDITION_DEF(ConstCond);   // 常量条件变量
* \endcode
*/
#define CONDITION_DEF(COND) jjserver::Condition COND

#endif // __CONDITION_H__