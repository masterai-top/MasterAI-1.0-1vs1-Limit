/**
* \file thread_pool.h
* \brief 线程池封装类
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include "typedef.h"
#include "thread/lock.h"
#include "app/app.h"
#include <map>
#include <list>

namespace frame
{
    class WorkerThread;
    class ThreadTaskQueue;

    /**
    * \brief 线程池封装类
    * \ingroup app_group
    */
    class ThreadPool : public IThreadPool
    {
    public:
        /**
        * \brief 构造函数
        */
        ThreadPool();

        /**
        * \brief 析构函数
        */
        virtual ~ThreadPool(void);

        /**
        * \brief 创建线程池
        * \param nType 线程池类型 THREAD_POOL_TYPE
        * \param nInitNum 初始线程数量
        * \param nMaxNum 最大线程数量
        * \param nIdleTime 线程等待时间(秒),-1=永远等待
        * \return 创建成功返回true，否则返回false
        */
        bool Create(uint32 nType, uint32 nInitNum, uint32 nMaxNum, int32 nIdleTime);

        /**
        * \brief 释放
        */
        virtual void Release();

        /**
        * \brief 线程池执行任务
        * \param pTask 线程任务处理回调
        * \param nPriority 线程任务优先级，值越小优先级越高
        * \return 成功返回true，否则返回false
        */
        virtual bool Run(IThreadTask *pTask, uint32 nPriority = 0);

    public:
        /**
        * \brief 将空闲线程放入繁忙列表
        * \param pThread 空闲线程
        * \return 成功返回true，否则返回false
        */
        bool MoveToBusyList(WorkerThread *pThread);

        /**
        * \brief 将繁忙线程放入空闲列表
        * \param pThread 繁忙线程
        * \return 成功返回true，否则返回false
        */
        bool MoveToIdleList(WorkerThread *pThread);

        /**
        * \brief 线程超时
        * \param pThread 超时线程
        * \return 需要停止线程返回false，否则返回true
        */
        bool ThreadTimeout(WorkerThread *pThread);

        /**
        * \brief 得到线程任务队列
        * \param nIndex 线程任务队列索引
        * \return 线程任务队列
        */
        ThreadTaskQueue * GetTaskQueue(uint32 nIndex);

    private:
        /**
        * \brief 根据线程索引获得线程任务队列索引
        * \param nIndex 线程索引
        * \return 线程任务队列索引
        */
        uint32 GetQueueIndex(uint32 nIndex, bool bNeedNewThread);

        /**
        * \brief 判断是否需要新建线程
        * \param nIndex 线程索引
        * \return 需要返回true，否则返回false
        */
        bool NeedNewThread(uint32 nIndex);

        /**
        * \brief 创建工作线程
        * \param nIndex 绑定的队列索引
        * \return 创建成功返回true，否则返回false
        */
        bool CreateWorkerThread(uint32 nIndex);

        /**
        * \brief 关闭线程池
        * \param bWait 若任务列表还有任务，是否等待
        * \return 成功返回true，失败返回false
        */
        bool Close(bool bWait = true);

    private:
        uint32                                  m_nType;            ///< 线程池类型 THREAD_POOL_TYPE
        uint32                                  m_nInitNum;         ///< 初始线程数量
        uint32                                  m_nMaxNum;          ///< 最大线程数量
        int32                                   m_nIdleTime;        ///< 线程等待时间(秒)

        ThreadTaskQueue                         *m_pTaskQueue;      ///< 线程任务队列[0~m_nMaxNum]

        std::map<uint32, WorkerThread *>        m_mapThreads;	    ///< 当前线程列表
        std::list<WorkerThread *>               m_lstIdleThreads;   ///< 空闲线程列表

        LockObject                              m_ThreadLock;       ///< 线程列表互斥锁 [m_setThreads,m_lstIdleThreads,m_nBusyThreadNum]
    };
}

#endif // __THREAD_POOL_H__
