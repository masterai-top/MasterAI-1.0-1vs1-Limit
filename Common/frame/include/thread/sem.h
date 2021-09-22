/**
* \file sem.h
* \brief 信号灯
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include "framedef.h"
#include "typedef.h"
#include "pthread.h"
#include "semaphore.h"

namespace frame
{
    /**
    * \brief 信号灯
    * \ingroup thread_group
    */
    class FRAME_EXPORT Semaphore
    {
    public:
        /**
        * \brief 构造函数
        */
        Semaphore(void);

        /**
        * \brief 构造函数
        * \param pshared 是否进程间共享
        * 一般若是进程间共享，需定义在共享内存中
        */
        Semaphore(bool pshared);

        /**
        * \brief 析构函数
        */
        virtual ~Semaphore(void);

        /**
        * \brief 等待
        * 线程阻塞，直到有信号为止
        * \return 成功返回0，否则返回-1
        */
        int32 Wait();

        /**
        * \brief 非阻塞等待
        * 线程不阻塞，若有信号不执行返回成功，若无信号返回错误
        * \return 成功返回0，否则返回-1
        */
        int32 TryWait();

        /**
        * \brief 非阻塞等待
        * \param timeout 等待时间，指定时间内线程阻塞，直到时间超时
        * \return 有信号返回true，否则返回fasle
        */
        bool TimeWait(timespec timeout);

        /**
        * \brief 发出信号
        * \return 成功返回0，否则返回-1
        */
        int32 Post();

    private:
        sem_t   m_sem;      ///< 信号灯对象
    };
}


#endif // __SEMAPHORE_H__