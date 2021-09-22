/**
* \file worker_thread.h
* \brief 工作线程类
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __WORKER_THREAD_H__
#define __WORKER_THREAD_H__

#include "typedef.h"
#include "thread/thread.h"

namespace frame
{
    class ThreadPool;

    /**
    * \brief 工作线程类
    * \ingroup app_group
    */
    class WorkerThread : public Thread
    {
    public:
        /**
        * \brief 构造函数
        * \param pThreadPool 所属线程池
        */
        WorkerThread(ThreadPool *pThreadPool);

        /**
        * \brief 析构函数
        */
        ~WorkerThread(void);

        /**
        * \brief 设置线程在线程池中的位置
        * \param nIndex 线程池中的位置
        */
        void SetIndex(uint32 nIndex);

        /**
        * \brief 得到线程池中的位置
        * \return 线程池中的位置
        */
        uint32 GetIndex();

        /**
        * \brief 设置线程池线程最大数
        * \param nIndex 线程最大数
        * 用于任务判断当前线程是否可执行该任务
        */
        void SetMaxIndex(uint32 nIndex);

        /**
        * \brief 得到线程池线程最大数
        * \return 线程最大数
        */
        uint32 GetMaxIndex();

        /**
        * \brief 设置线程等待时间
        * \param nIdleTime 线程等待时间(秒),-1=永远等待
        */
        void SetIdleTime(int32 nIdleTime);

    protected:
        /**
        * \brief 线程执行函数
        * \return 线程继续执行返回true，线程终止返回false
        * 
        * 此函数被私有的线程入口函数InternalThreadProc()调用，
        * 重载此虚函数以实现不同的线程执行处理
        */
        virtual bool OnThreadProc();

        /**
        * \brief 线程停止函数
        * 
        * 此函数被线程停止函数Stop()调用，以免重载的线程对象OnThreadProc()是阻塞等待的，
        * 重载此虚函数以实现重载对象通知退出阻塞等待
        */
        virtual void OnThreadStop();

    private:
        ThreadPool          *m_pThreadPool;         ///< 所属线程池
        int32               m_nIdleTime;            ///< 线程等待时间(秒)
        uint32              m_nIndex;               ///< 在线程池中的位置
        uint32              m_nMaxIndex;            ///< 线程最大数
    };
}

#endif // __WORKER_THREAD_H__