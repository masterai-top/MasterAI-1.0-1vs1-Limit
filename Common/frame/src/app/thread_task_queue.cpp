/**
* \file thread_task_queue.cpp
* \brief 线程池任务队列类函数的实现
*/

#include "pch.h"
#include "app/thread_task_queue.h"
#include "app/app.h"


namespace frame
{
    /**
    * \brief 构造函数
    */
    ThreadTaskQueue::ThreadTaskQueue(void)
        : m_NewTaskEvent(false, true)
    {
        m_bMultiThread = false;
    }

    /**
    * \brief 析构函数
    */
    ThreadTaskQueue::~ThreadTaskQueue(void)
    {
        m_bMultiThread = false;
    }

    /**
    * \brief 添加线程任务
    * \param pTask 线程任务处理回调
    * \param nPriority 线程任务优先级，值越小优先级越高
    */
    void ThreadTaskQueue::AddTask(IThreadTask* pTask, uint32 nPriority /*= 0*/)
    {
        Lock lock(&m_TaskLock);

        if (NULL == pTask)
        {
            return;
        }

        std::map<uint32, LIST_THREAD_TASK>::iterator it = m_mapTasks.find(nPriority);
        if (it != m_mapTasks.end())
        {
            LIST_THREAD_TASK &lstTasks = it->second;
            lstTasks.push_back(pTask);
        }
        else
        {
            LIST_THREAD_TASK lstTasks;
            lstTasks.push_back(pTask);

            std::map<uint32, LIST_THREAD_TASK>::value_type value(nPriority, lstTasks);
            m_mapTasks.insert(value);
        }

        // 通知工作线程，有新任务加入
        m_NewTaskEvent.Set();
    }

    /**
    * \brief 等待线程任务
    * \param nWaitTime 等待时间s
    * \return 有信号返回true，否则返回false
    */
    bool ThreadTaskQueue::Wait(int32 nWaitTime)
    {
        return m_NewTaskEvent.Wait(nWaitTime);
    }

    /**
    * \brief 停止等待线程任务
    */
    void ThreadTaskQueue::Stop()
    {
        m_NewTaskEvent.Set();
    }

    /**
    * \brief 设置是否多线程使用同一队列
    * \param bMultiThread 是否多线程使用同一队列
    */
    void ThreadTaskQueue::SetMultiThread(bool bMultiThread)
    {
        m_bMultiThread = bMultiThread;
    }

    /**
    * \brief 执行线程任务
    * \param nIndex 线程索引
    * \param nMaxIndex 线程池中线程最大数
    */
    void ThreadTaskQueue::OnRun(uint32 nIndex, uint32 nMaxIndex)
    {
        m_TaskLock.Lock();

        if (m_mapTasks.empty())
        {
            m_TaskLock.Unlock();
            return;
        }

        std::list<IThreadTask *> lstRunTasks;      // 可执行的任务

        std::map<uint32, LIST_THREAD_TASK>::iterator it = m_mapTasks.begin();
        for( ; it != m_mapTasks.end(); )
        {
            LIST_THREAD_TASK &lstTasks = it->second;

            // 该优先级已经没任务了
            if (lstTasks.empty())
            {
                it = m_mapTasks.erase(it);
                continue;
            }

            LIST_THREAD_TASK::iterator iter = lstTasks.begin();
            for ( ; iter != lstTasks.end(); )
            {
                IThreadTask *pCurTask = *iter;
                if (NULL == pCurTask)
                {
                    iter = lstTasks.erase(iter);
                    continue;
                }

                // 线程任务设置了执行线程
                uint32 nTaskIndex = pCurTask->GetThreadIndex();

                // 任务的执行线程不是当前线程，则不执行
                // 单队列线程池不能根据这个逻辑判断
                if (!m_bMultiThread && nTaskIndex < nMaxIndex && nTaskIndex != nIndex)
                {
                    ++iter;
                    continue;
                }

                // nTaskIndex >= nMaxIndex 没指定执行线程，所有线程都可以执行
                // nTaskIndex == nIndex 是当前线程可执行的任务
                lstRunTasks.push_back(pCurTask);

                // 删除该线程任务
                iter = lstTasks.erase(iter);
            }

            // 该优先级最后一个任务被删除了
            if (lstTasks.empty())
            {
                it = m_mapTasks.erase(it);
            }
            else
            {
                ++it;
            }
        }

        m_TaskLock.Unlock();

        for (std::list<IThreadTask *>::iterator it = lstRunTasks.begin(); it != lstRunTasks.end(); ++it)
        {
            IThreadTask *pTask = *it;
            if (NULL != pTask)
            {
                pTask->OnRun(nIndex);
            }
        }
    }

    /**
    * \brief 是否没有任务
    * \return 队列没有任务返回true，否则返回false
    */
    bool ThreadTaskQueue::TaskEmpty()
    {
        Lock lock(&m_TaskLock);

        return m_mapTasks.empty();
    }
}
