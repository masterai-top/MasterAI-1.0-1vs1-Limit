/**
* \file worker_thread.cpp
* \brief 工作线程类函数的实现
*/

#include "pch.h"
#include "app/worker_thread.h"
#include "app/thread_task_queue.h"
#include "app/thread_pool.h"


namespace frame
{
    /**
    * \brief 构造函数
    * \param pThreadPool 所属线程池
    */
    WorkerThread::WorkerThread(ThreadPool *pThreadPool)
        : m_pThreadPool(pThreadPool)
    {
        m_nIndex = -1;
        m_nMaxIndex = -1;
    }

    /**
    * \brief 析构函数
    */
    WorkerThread::~WorkerThread(void)
    {
        m_nIndex = -1;
        m_nMaxIndex = -1;
    }

    /**
    * \brief 设置线程在线程池中的位置
    * \param nIndex 线程池中的位置
    */
    void WorkerThread::SetIndex(uint32 nIndex)
    {
        m_nIndex = nIndex;
    }

    /**
    * \brief 得到线程池中的位置
    * \return 线程池中的位置
    */
    uint32 WorkerThread::GetIndex()
    {
        return m_nIndex;
    }

    /**
    * \brief 设置线程池线程最大数
    * \param nIndex 线程最大数
    * 用于任务判断当前线程是否可执行该任务
    */
    void WorkerThread::SetMaxIndex(uint32 nIndex)
    {
        m_nMaxIndex = nIndex;
    }

    /**
    * \brief 得到线程池线程最大数
    * \return 线程最大数
    */
    uint32 WorkerThread::GetMaxIndex()
    {
        return m_nMaxIndex;
    }

    /**
    * \brief 设置线程等待时间
    * \param nIdleTime 线程等待时间(秒),-1=永远等待
    */
    void WorkerThread::SetIdleTime(int32 nIdleTime)
    {
        m_nIdleTime = nIdleTime;
    }

    /**
    * \brief 线程执行函数
    * \return 线程继续执行返回true，线程终止返回false
    * 
    * 此函数被私有的线程入口函数InternalThreadProc()调用，
    * 重载此虚函数以实现不同的线程执行处理
    */
    bool WorkerThread::OnThreadProc()
    {
        if (NULL == m_pThreadPool)
        {
            return false;
        }

        ThreadTaskQueue *pQueue = m_pThreadPool->GetTaskQueue(m_nIndex);
        if (pQueue == NULL)
        {
            return false;
        }

        bool bWait = pQueue->Wait(m_nIdleTime);
        if (bWait)
        {
            // 正常收到信号，执行线程任务
            pQueue->OnRun(m_nIndex, m_nMaxIndex);

            return true;
        }

        // 线程等待超时
        return m_pThreadPool->ThreadTimeout(this);
    }

    /**
    * \brief 线程停止函数
    * 
    * 此函数被线程停止函数Stop()调用，以免重载的线程对象OnThreadProc()是阻塞等待的，
    * 重载此虚函数以实现重载对象通知退出阻塞等待
    */
    void WorkerThread::OnThreadStop()
    {
        if (NULL == m_pThreadPool)
        {
            return;
        }

        ThreadTaskQueue *pQueue = m_pThreadPool->GetTaskQueue(m_nIndex);
        if (pQueue == NULL)
        {
            return;
        }

        pQueue->Stop();
    }
}