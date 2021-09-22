/**
* \file lock.h
* \brief 线程锁
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __LOCK_H__
#define __LOCK_H__

#include "framedef.h"
#include "pthread.h"

namespace frame
{
#if defined(USE_WIN32_CS) && defined(WIN32) && defined(_MSC_VER)
    /**
    * \brief 单个锁对象
    * \ingroup thread_group
    */
    class FRAME_EXPORT LockObject
    {
    public:
        /**
        * \brief 构造函数
        */
        LockObject()
        {
            InitializeCriticalSection(&m_lock);
            m_flag = false;
        }

        /**
        * \brief 析构函数
        */
        ~LockObject()
        {
            DeleteCriticalSection(&m_lock);
            m_flag = false;
        }

        /**
        * \brief 上锁
        */
        void Lock()
        {
            if (!m_flag)
            {
                EnterCriticalSection(&m_lock);
            }
            m_flag = true;
        }

        /**
        * \brief 解锁
        */
        void Unlock()
        {
            if (m_flag)
            {
                LeaveCriticalSection(&m_lock);
            }
            m_flag = false;
        }

        /**
        * \brief 是否上锁
        * \return 是返回true，否则返回false
        */
        bool Islock()
        {
            return m_flag;
        }

    private:
        CRITICAL_SECTION	m_lock;     ///< 锁
        bool                m_flag;     ///< 是否上锁标志
    };
#endif

    /**
    * \brief 单个锁对象
    * \ingroup thread_group
    */
    class FRAME_EXPORT LockObject
    {
    public:
        /**
        * \brief 构造函数
        */
        LockObject()
        {
            pthread_mutex_init(&m_lock, NULL);
        }

        /**
        * \brief 构造函数
        * \param pshared 是否进程锁
        * 进程锁一般定义在共享内存中
        */
        LockObject(bool pshared)
        {
            //初始化互斥对象属性
            pthread_mutexattr_init(&m_attr);
            if (pshared)
            {
                //设置互斥对象为PTHREAD_PROCESS_SHARED共享，即可以在多个进程的线程访问,PTHREAD_PROCESS_PRIVATE为同一进程的线程共享
                pthread_mutexattr_setpshared(&m_attr,PTHREAD_PROCESS_SHARED);
            }
            else
            {
                pthread_mutexattr_setpshared(&m_attr,PTHREAD_PROCESS_PRIVATE);
            }

            //初始化互斥对象
            pthread_mutex_init(&m_lock, &m_attr);
        }

        /**
        * \brief 析构函数
        */
        ~LockObject()
        {
            pthread_mutex_destroy(&m_lock);
        }

        /**
        * \brief 上锁
        */
        void Lock()
        {
            pthread_mutex_lock(&m_lock);
        }

        /**
        * \brief 尝试上锁
        * \return 可以锁返回true，否则返回false
        */
        bool TryLock()
        {
            return pthread_mutex_trylock(&m_lock) == 0 ? true : false;
        }

        /**
        * \brief 解锁
        */
        void Unlock()
        {
            pthread_mutex_unlock(&m_lock);
        }

        /**
        * \brief 获得锁
        * \return 锁
        */
        pthread_mutex_t* GetLock()
        {
            return &m_lock;
        }

    private:
        pthread_mutex_t	m_lock;         ///< 锁变量
        pthread_mutexattr_t m_attr;     ///< 锁属性 
    };

    /**
    * \brief 锁定义宏
    * \ingroup thread_group
    * \def LOCK_DEF(LOCK)
    * \param LOCK 新锁名称
    *
    * \par 范例
    * \code
    *   LOCK_DEF(MapLock);    ///< map锁
    *   LOCK_DEF(ConstLock);  ///< 常量锁
    * \endcode
    */
    #define LOCK_DEF(LOCK) frame::LockObject LOCK

    /**
    * \brief 锁应用类
    * \ingroup thread_group
    *
    * \par 范例
    * \code
    *   LOCK_DEF(MapLock);                    ///< map锁
    *   #define LOCKMAP() Lock lock(MapLock)  ///< 应用锁
    *   void functionTest()
    *   {
    *       LOCKMAP();      // 加锁
            DoSomthing();
    *   }
    * \endcode
    */
    class FRAME_EXPORT Lock
    {
    public:
        /**
        * \brief 构造函数
        * \param lock 锁对象
        */
        Lock(LockObject *lock) : m_lock(lock)
        {
            if (m_lock)
            {
                m_lock->Lock();
            }
        }

        /**
        * \brief 析构函数
        */
        ~Lock()
        {
            if (m_lock)
            {
                m_lock->Unlock();
            }
        }

        /**
        * \brief 获得锁
        * \return 锁
        */
        pthread_mutex_t * GetLock()
        {
            if (m_lock)
            {
                return m_lock->GetLock();
            }

            return NULL;
        }
    private:
        LockObject      *m_lock;    ///< 锁对象
    };
}

#endif // __LOCK_H__