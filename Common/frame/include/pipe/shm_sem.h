/**
* \file shm_sem.h
* \brief 共享内存信号量
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __SHM_SEM_H__
#define __SHM_SEM_H__

#include "framedef.h"
#include "typedef.h"

namespace frame
{
    /**
    * \brief 共享内存信号信号量
    * \ingroup pipe_group
    */
    class FRAME_EXPORT ShmSem
    {
        enum
        {
            SYSVSEM_SEM = 0,        ///< 进程等待信号量
            SYSVSEM_USAGE = 1,      ///< 进程数量统计信号量
            SYSVSEM_SETVAL = 2,     ///< 进程初始化信号量
            SYSVSEM_READ = 3,       ///< 共享内存读信号量
            SYSVSEM_WRITE = 4,      ///< 共享内存写信号量
            SYSVSEM_WAIT = 5,       ///< 等待统计信号量
        };
    public:
        /**
        * \brief 构造函数
        */
        ShmSem(void);

        /**
        * \brief 析构函数
        */
        ~ShmSem(void);

        /**
        * \brief 初始化共享内存信号量
        * \param nKey 信号主键
        * \return 初始化信号集时的进程数量，失败返回-1
        */ 
        int32 Init(int32 nKey);

        /**
        * \brief 销毁共享内存信号量
        * \return 成功返回true，否则返回false
        * 无需主动销毁，所有进程销毁后该信号集会自动销毁
        */ 
        bool Destroy();

        /**
        * \brief 等待信号通知
        */
        void Wait();

        /**
        * \brief 发送信号通知
        */
        void Post();

        /**
        * \brief 广播信号通知
        */
        void Broadcast();

        /**
        * \brief 共享内存上锁
        */
        void Lock();

        /**
        * \brief 共享内存解锁
        */
        void Unlock();

        /**
        * \brief 共享内存读上锁
        */
        void ReadLock();

        /**
        * \brief 共享内存读解锁
        */
        void ReadUnlock();

        /**
        * \brief 共享内存写上锁
        */
        void WriteLock();

        /**
        * \brief 共享内存写解锁
        */
        void WriteUnlock();

    private:
        int32   m_key;              ///< 信号主键
        int32   m_sem;              ///< 信号量集
                                    ///< 包含6个信号量，信号0[SYSVSEM_SEM], 信号1[SYSVSEM_USAGE], 信号2[SYSVSEM_SETVAL], 信号3[SYSVSEM_READ], 信号4[SYSVSEM_WRITE]，信号5[SYSVSEM_WAIT]
                                    ///< SYSVSEM_SEM 是等待通知信号量
                                    ///< SYSVSEM_USAGE 是使用该信号集的进程数量统计信号量，若进程数量为1，需要初始化SYSVSEM_SHM/SYSVSEM_WAIT信号量
                                    ///< SYSVSEM_SETVAL 是进程初始化信号量，进程初始化的时候需先取得该信号量，以免多个进程同时初始化SYSVSEM_SHM/SYSVSEM_WAIT信号量
                                    ///< 所以通过该信号量保证只允许同时初始化的进程数量
                                    ///< SYSVSEM_READ 是共享内存读信号量，保证同时读的进程数为1
                                    ///< SYSVSEM_WRITE 是共享内存写信号量，保证同时写的进程数为1
                                    ///< SYSVSEM_WAIT 是等待统计信号量，信号广播的时候，根据该值设置信号量
    };
}

#endif // __SHM_SEM_H__
