/**
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
*/

#ifndef __NW_NET_WORKER_H__
#define __NW_NET_WORKER_H__
#include "worker_thread.h"

namespace network
{
    
    class CNetWorker
    {
    private:
        CNetWorker();
        
    public:
        ~CNetWorker();
        static CNetWorker* Instance()
        {
            static CNetWorker m_instance;
            return &m_instance;
        }

        bool Init(uint32 nDataThreadCnt);
        CWorkerThread* GetDataThread(sockid fd);
        CWorkerThread* GetAcceptThread();
        
    private:
        uint32              m_nDataThreadCnt;
        CWorkerThread       m_oAcceptThread;
        CWorkerThread      *m_pDataThread;
    };
}

#endif // __NW_NET_WORKER_H__


