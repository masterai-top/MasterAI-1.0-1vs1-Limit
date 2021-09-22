/**
* \file sem.cpp
* \brief 信号灯类实现代码
*/

#include "pch.h"
#include "thread/sem.h"


namespace frame
{
    /**
    * \brief 构造函数
    */
    Semaphore::Semaphore(void)
    {
        sem_init(&m_sem, 0, 0);
    }

    /**
    * \brief 构造函数
    * \param pshared 是否进程间共享
    * 一般若是进程间共享，需定义在共享内存中
    */
    Semaphore::Semaphore(bool pshared)
    {
        if (pshared)
        {
            sem_init(&m_sem, 1, 0);
        }
        else
        {
            sem_init(&m_sem, 0, 0);
        }
    }

    /**
    * \brief 析构函数
    */
    Semaphore::~Semaphore(void)
    {
        sem_destroy(&m_sem);
    }

    /**
    * \brief 等待
    * 线程阻塞，直到有信号为止
    * \return 成功返回0，否则返回-1
    */
    int32 Semaphore::Wait()
    {
        return sem_wait(&m_sem);
    }

    /**
    * \brief 非阻塞等待
    * 线程不阻塞，若有信号不执行返回成功，若无信号返回错误
    * \return 成功返回0，否则返回-1
    */
    int32 Semaphore::TryWait()
    {
        return sem_trywait(&m_sem);
    }

    /**
    * \brief 非阻塞等待
    * \param timeout 等待时间，指定时间内线程阻塞，直到时间超时
    * \return 有信号返回true，否则返回fasle
    */
    bool Semaphore::TimeWait(timespec timeout)
    {
        return sem_timedwait(&m_sem, &timeout) > 0 ? true : false;
    }

    /**
    * \brief 发出信号
    * \return 成功返回0，否则返回-1
    */
    int32 Semaphore::Post()
    {
        return sem_post(&m_sem);
    }
}
