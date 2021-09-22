/**
* \file thread_task_queue.h
* \brief 线程池任务队列类
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __THREAD_TASK_QUEUE_H__
#define __THREAD_TASK_QUEUE_H__

#include "typedef.h"
#include "thread/lock.h"
#include "thread/tevent.h"
#include <list>
#include <map>

namespace frame
{
    class IThreadTask;

    /**
    * \brief 线程池任务队列类
    * \ingroup thread_group
    */
    class ThreadTaskQueue
    {
    public:
        /**
        * \brief 构造函数
        */
        ThreadTaskQueue();

        /**
        * \brief 析构函数
        */
        ~ThreadTaskQueue(void);

        /**
        * \brief 添加线程任务
        * \param pTask 线程任务处理回调
        * \param nPriority 线程任务优先级，值越小优先级越高
        */
        void AddTask(IThreadTask* pTask, uint32 nPriority = 0);

        /**
        * \brief 等待线程任务
        * \param nWaitTime 等待时间s
        * \return 有信号返回true，否则返回false
        */
        bool Wait(int32 nWaitTime);

        /**
        * \brief 停止等待线程任务
        */
        void Stop();

        /**
        * \brief 设置是否多线程使用同一队列
        * \param bMultiThread 是否多线程使用同一队列
        */
        void SetMultiThread(bool bMultiThread);

        /**
        * \brief 执行线程任务
        * \param nIndex 执行线程索引
        * \param nMaxIndex 线程池中线程最大数
        */
        void OnRun(uint32 nIndex, uint32 nMaxIndex);

        /**
        * \brief 是否没有任务
        * \return 队列没有任务返回true，否则返回false
        */
        bool TaskEmpty();

    private:
        /**
        * \brief 得到一个线程任务
        * \return 线程任务
        */
        IThreadTask* PopThreadTask();

        /**
        * \brief 放入一个线程任务
        * \param pTask 线程任务
        * \param bPriority 是否优先执行 是放入队列前面，否放入队列后面
        * \return 放入线程任务是否成功
        */
        bool PushThreadTask(IThreadTask* pTask, bool bPriority);

    private:
        typedef std::list<IThreadTask *> LIST_THREAD_TASK;      ///< 线程任务队列

        std::map<uint32, LIST_THREAD_TASK>  m_mapTasks;         ///< 优先级线程任务列表

        LockObject                          m_TaskLock;         ///< 任务列表互斥锁

        TEvent                              m_NewTaskEvent;     ///< 线程等待事件

        bool                                m_bMultiThread;     ///< 是否多线程使用同一队列
    };
}

#endif // __THREAD_TASK_QUEUE_H__
