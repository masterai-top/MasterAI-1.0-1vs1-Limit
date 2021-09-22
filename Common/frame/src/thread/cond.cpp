/**
* \file cond.cpp
* \brief 条件变量类实现代码
*/

#include "pch.h"
#include "thread/cond.h"


namespace frame
{
    /**
    * \brief 构造函数
    */
    Condition::Condition(void)
    {
        pthread_cond_init(&m_cond, NULL);
    }


    /**
    * \brief 析构函数
    */
    Condition::~Condition(void)
    {
        pthread_cond_destroy(&m_cond);
    }

    /**
    * \brief 等待
    * \param lock 线程锁，通过该锁使线程阻塞
    */
    void Condition::Wait(Lock &lock)
    {
        pthread_cond_wait(&m_cond, lock.GetLock());
    }

    /**
    * \brief 非阻塞等待
    * \param lock 线程锁，通过该锁使线程阻塞
    * \param timeout 等待时间，指定时间内线程阻塞，直到时间超时
    * \return 有信号返回true，等待超时返还false
    */
    bool Condition::TimeWait(Lock &lock, timespec timeout)
    {
        int ret = pthread_cond_timedwait(&m_cond, lock.GetLock(), &timeout);
        return ret == 0 ? true : false;
    }

    /**
    * \brief 发出信号
    */
    void Condition::Signal()
    {
        pthread_cond_signal(&m_cond);
    }

    /**
    * \brief 广播信号
    */
    void Condition::Broadcast()
    {
        pthread_cond_broadcast(&m_cond);
    }
}
