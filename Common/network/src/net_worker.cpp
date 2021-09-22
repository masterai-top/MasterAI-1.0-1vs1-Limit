#include "net_worker.h"
#include "pch.h"
#include "event2/thread.h"
#include "comm_define.h"

namespace network
{
    CNetWorker::CNetWorker()
    {
        m_pDataThread       = NULL;
        m_nDataThreadCnt    = 0;
    }

    CNetWorker::~CNetWorker()
    {
        if(NULL != m_pDataThread) {
            delete []m_pDataThread;
        }
    }


    bool CNetWorker::Init(uint32 nDataThreadCnt)
    {
        evthread_use_pthreads();
        m_nDataThreadCnt = nDataThreadCnt;
        if(m_nDataThreadCnt <= 0 || m_nDataThreadCnt > MAX_NET_WORKER_THREADS) 
        {
            LOG(LT_ERROR, "Net worker init| invalid param| cnt=%d", m_nDataThreadCnt);
            return false;
        }
        
        m_pDataThread = new CWorkerThread[m_nDataThreadCnt];
        if(NULL == m_pDataThread) 
        {
            LOG(LT_ERROR, "Net worker init| new thread failed| cnt=%d", m_nDataThreadCnt);
            return false;
        }

        for(uint32 i = 0; i < m_nDataThreadCnt; i++)
        {
            if(!m_pDataThread[i].Init())
            {
                LOG(LT_ERROR, "Net worker init| data thread init failed| index=%d", i);
                return false;
            }
        }

        if(!m_oAcceptThread.Init())
        {
            LOG(LT_ERROR, "Net worker init| accept thread init failed");
            return false;
        }
        
        LOG(LT_INFO, "Net worker init succ| data_thread_cnt=%d", m_nDataThreadCnt);
        
        return true;
    }

    
    CWorkerThread* CNetWorker::GetDataThread(sockid fd)
    {
        if(fd < 0) {
            return NULL;
        }

        //线程的分配,必须按fd进行分配,避免可能导致多线程同时使用一个event_base ??
        return &m_pDataThread[fd%m_nDataThreadCnt];
    }

    CWorkerThread* CNetWorker::GetAcceptThread()
    {
        return &m_oAcceptThread;
    }
    

}

