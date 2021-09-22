/**
* \file thread_pool.cpp
* \brief 线程池封装类函数的实现
*/

#include "pch.h"
#include "app/thread_pool.h"
#include "app/worker_thread.h"
#include "app/thread_task_queue.h"
#include "random.h"
#include <vector>
using namespace dtutil;


namespace frame
{
    /**
    * \brief 构造函数
    */
    ThreadPool::ThreadPool(void)
    {
        m_nType = 0;
        m_nInitNum = 0;
        m_nMaxNum = 0;
        m_nIdleTime = -1;
    }

    /**
    * \brief 析构函数
    */
    ThreadPool::~ThreadPool(void)
    {
    }

    /**
    * \brief 创建线程池
    * \param nType 线程池类型 THREAD_POOL_TYPE
    * \param nInitNum 初始线程数量
    * \param nMaxNum 最大线程数量
    * \param nIdleTime 线程等待时间(秒),-1=永远等待
    * \return 创建成功返回true，否则返回false
    */
    bool ThreadPool::Create(uint32 nType, uint32 nInitNum, uint32 nMaxNum, int32 nIdleTime)
    {
        // 最大线程数量必须大于0
        if (nMaxNum == 0)
        {
            return false;
        }

        // 初始线程数需小于等于最大线程数
        if (nMaxNum < nInitNum)
        {
            nMaxNum = nInitNum;
        }

        m_nType = nType;
        m_nInitNum = nInitNum;
        m_nMaxNum = nMaxNum;
        m_nIdleTime = nIdleTime;

        if (m_nType == THREAD_POOL_TYPE_SINGLE_QUEUE)
        {
            m_pTaskQueue = new ThreadTaskQueue[1];
            m_pTaskQueue[0].SetMultiThread(true);
        }
        else
        {
            m_pTaskQueue = new ThreadTaskQueue[m_nMaxNum];
        }

        return true;
    }

    /**
    * \brief 释放
    */
    void ThreadPool::Release()
    {
        Lock lock(&m_ThreadLock);

        //// 停止所有线程 delete已经阻塞停止了，这里无需再主动停止
        //for(std::map<uint32, WorkerThread *>::iterator it = m_mapThreads.begin(); it != m_mapThreads.end(); ++it)
        //{
        //    WorkerThread *pThread = it->second;
        //    if (NULL != pThread)
        //    {
        //        pThread->Stop(true);
        //    }
        //}

        // 删除所有线程
        for(std::map<uint32, WorkerThread *>::iterator it = m_mapThreads.begin(); it != m_mapThreads.end(); ++it)
        {
            WorkerThread *pThread = it->second;
            if (NULL != pThread)
            {
                delete pThread;
            }
        }
        m_mapThreads.clear();

        for(std::list<WorkerThread *>::iterator it = m_lstIdleThreads.begin(); it != m_lstIdleThreads.end(); ++it)
        {
            WorkerThread *pThread = *it;
            if (NULL != pThread)
            {
                delete pThread;
            }
        }
        m_lstIdleThreads.clear();

        // 删除任务队列
        if (NULL != m_pTaskQueue)
        {
            delete [] m_pTaskQueue;
        }

        m_nType = 0;
        m_nInitNum = 0;
        m_nMaxNum = 0;
        m_nIdleTime = -1;

        delete this;
    }

    /**
    * \brief 线程池执行任务
    * \param pTask 线程任务处理回调
    * \param nPriority 线程任务优先级，值越小优先级越高
    * \return 成功返回true，否则返回false
    */
    bool ThreadPool::Run(IThreadTask *pTask, uint32 nPriority /*= 0*/)
    {
        if (NULL == pTask)
        {
            return false;
        }

        uint32 nIndex = pTask->GetThreadIndex();

        // 所有激活线程是否正在繁忙，若需要则创建新的工作线程
        m_ThreadLock.Lock();

        // 是否需要新建线程
        bool bNeedNewThread = NeedNewThread(nIndex);

        // 获得使用的线程任务队列
        uint32 nQueueIndex = GetQueueIndex(nIndex, bNeedNewThread);

        // 需要新建线程，则创建
        if (bNeedNewThread)
        {
            CreateWorkerThread(nQueueIndex);
        }

        m_ThreadLock.Unlock();

        // 取一个工作线程，若不存在，则自动创建线程；
        // 若返回NULL，则表示工作线程已经饱和
        ThreadTaskQueue *pQueue = GetTaskQueue(nQueueIndex);
        if (NULL == pQueue)
        {
            return false;
        }

        // 将线程任务加入任务队列中
        pQueue->AddTask(pTask, nPriority);

        return true;
    }

    /**
    * \brief 线程超时
    * \param pThread 超时线程
    * \return 需要停止线程返回false，否则返回true
    */
    bool ThreadPool::ThreadTimeout(WorkerThread *pThread)
    {
        Lock lock(&m_ThreadLock);

        // 当前线程数量比初始数量多，则停掉此线程
        // 重置为空闲状态，并放入空闲队列中
        if ((uint32)(m_mapThreads.size()) > m_nInitNum)
        {
            uint32 nIndex = pThread->GetIndex();
            ThreadTaskQueue *pQueue = GetTaskQueue(nIndex);
            if (NULL != pQueue && !pQueue->TaskEmpty())
            {
                return true;
            }

            m_mapThreads.erase(pThread->GetIndex());
            m_lstIdleThreads.push_back(pThread);
            return false;
        }

        return true;
    }

    /**
    * \brief 根据线程索引获得线程任务队列索引
    * \param nIndex 线程索引
    * \return 线程任务队列索引
    */
    uint32 ThreadPool::GetQueueIndex(uint32 nIndex, bool bNeedNewThread)
    {
        // 线程已经存在，返回线程绑定的队列索引
        std::map<uint32, WorkerThread *>::iterator it = m_mapThreads.find(nIndex);
        if (it != m_mapThreads.end())
        {
            WorkerThread *pThread = it->second;
            if (NULL != pThread)
            {
                return nIndex;
            }
        }

        // 需要创建新线程
        if (bNeedNewThread)
        {
            // 未指定线程，随机选择一个没使用的线程
            if (nIndex >= m_nMaxNum)
            {
                std::set<int32> lstExcludeThreads;
                for(std::map<uint32, WorkerThread *>::iterator it = m_mapThreads.begin(); it != m_mapThreads.end(); ++it)
                {
                    int32 nExcludeIndex = (int32)(it->first);
                    lstExcludeThreads.insert(nExcludeIndex);
                }

                nIndex = Random::RandomIntExclude(0, m_nMaxNum-1, lstExcludeThreads);
            }
        }
        // 不需要创建新线程
        else
        {
            // 未指定线程，随机选择一个使用的线程
            if (nIndex >= m_nMaxNum)
            {
                std::vector<uint32> vecThreads;
                for(std::map<uint32, WorkerThread *>::iterator it = m_mapThreads.begin(); it != m_mapThreads.end(); ++it)
                {
                    int32 nUseIndex = (int32)(it->first);
                    vecThreads.push_back(nUseIndex);
                }

                int32 nCount = (int32)(vecThreads.size());
                if (nCount != 0)
                {
                    uint32 nCntIndex = Random::RandomInt(0, nCount-1);

                    nIndex = vecThreads[nCntIndex];

                    return nIndex;
                }  
            }
        }

        return nIndex;
    }

    /**
    * \brief 判断是否需要新建线程
    * \param nIndex 线程索引
    * \return 需要返回true，否则返回false
    */
    bool ThreadPool::NeedNewThread(uint32 nIndex)
    {
        // 指定线程索引在线程最大数内，判断指定nIndex线程是否存在
        if (nIndex < m_nMaxNum)
        {
            // 线程已经存在，无需新线程
            std::map<uint32, WorkerThread *>::iterator it = m_mapThreads.find(nIndex);
            if (it != m_mapThreads.end())
            {
                return false;
            }

            // 线程不在，需创建新线程
            return true;
        }

        // 线程索引超过线程最大数

        // 当前线程数量已经满了
        if ((uint32)(m_mapThreads.size()) >= m_nMaxNum)
        {
            return false;
        }
        
        return true;
    }

    /**
    * \brief 创建工作线程
    * \param nIndex 绑定的队列索引
    * \return 创建成功返回true，否则返回false
    */
    bool ThreadPool::CreateWorkerThread(uint32 nIndex)
    {
        if (nIndex >= m_nMaxNum)
        {
            return false;
        }

        WorkerThread *pThread = NULL;

        if (m_lstIdleThreads.empty())
        {
            pThread = new WorkerThread(this);
        }
        else
        {
            pThread = m_lstIdleThreads.front();
            m_lstIdleThreads.pop_front();
        }

        pThread->SetIndex(nIndex);
        pThread->SetMaxIndex(m_nMaxNum);
        pThread->SetIdleTime(m_nIdleTime);

        m_mapThreads[nIndex] = pThread;

        pThread->Run("worker_thread");

        return true;
    }

    /**
    * \brief 得到线程任务队列
    * \param nIndex 线程任务队列索引
    * \return 线程任务队列
    */
    ThreadTaskQueue * ThreadPool::GetTaskQueue(uint32 nIndex)
    {
        if (m_nType == THREAD_POOL_TYPE_SINGLE_QUEUE)
        {
            return &m_pTaskQueue[0];
        }

        if (nIndex >= m_nMaxNum)
        {
            return NULL;
        }
    
        return &m_pTaskQueue[nIndex];
    }
}
