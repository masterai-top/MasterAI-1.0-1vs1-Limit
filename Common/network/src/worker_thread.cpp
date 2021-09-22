#include "worker_thread.h"
#include "socket_manager.h"



namespace network
{
    CWorkerThread::CWorkerThread()
    {
        m_nNotifyReadFd = INVALID_SOCKET;
        m_nNotifySendFd = INVALID_SOCKET;
        m_base          = NULL;
    }

    CWorkerThread::~CWorkerThread()
    {
        if(INVALID_SOCKET != m_nNotifyReadFd) 
        {
            close(m_nNotifyReadFd);
            m_nNotifyReadFd = INVALID_SOCKET;
        }

        if(INVALID_SOCKET != m_nNotifySendFd) 
        {
            close(m_nNotifySendFd);
            m_nNotifySendFd = INVALID_SOCKET;
        }
        
        if(NULL != m_base) 
        {
            event_del(&m_oNotifyEvent);
            event_base_free(m_base);
            m_base = NULL;
        }
    }

    
    bool CWorkerThread::Init()
    {
        int fds[2];
        if (pipe(fds)) 
        {
            fprintf(stderr, "Worker thread init| Can't create notify pipe\n");
            LOG(LT_ERROR, "Worker thread init| Can't create notify pipe");
            exit(1);
        }

        m_nNotifyReadFd = fds[0];
        m_nNotifySendFd = fds[1];

        m_base = event_base_new();
        if(NULL == m_base) 
        {
            LOG(LT_ERROR, "Worker thread init| new base failed");
            exit(2);
        }

        event_set(&m_oNotifyEvent, m_nNotifyReadFd, EV_READ | EV_PERSIST, NotifyCallback, (void*)this);
        event_base_set(m_base, &m_oNotifyEvent);

        if (-1 == event_add(&m_oNotifyEvent, 0)) 
        {
            fprintf(stderr, "Worker thread init| Can't monitor libevent notify pipe\n");
            LOG(LT_ERROR, "Worker thread init| Can't monitor libevent notify pipe");
            exit(3);
        }

        
        pthread_t tid = 0;
        if(0 != pthread_create( &tid, NULL, &CWorkerThread::Run, (void*)this))
        {
            LOG(LT_ERROR, "Worker thread init| create failed");
            return false;
        }
        
        return true;
    }

    void* CWorkerThread::Run(void *arg) 
    {   
        pthread_detach( pthread_self() );	
        LOG(LT_INFO, "Worker thread run ...| pid=%llu", pthread_self());
        
        CWorkerThread *pThis = (CWorkerThread *)arg;
        pThis->m_nThreadID  = pthread_self();
        
        event_base_dispatch(pThis->m_base);
        LOG(LT_INFO, "Worker thread Exit ...| pid=%llu", pthread_self());
        _exit(0);
        
        return NULL;
    }
    
    bool CWorkerThread::PushNotifyFd(const CNotifyFd &oNotify)
    {   
        {
            Lock lock(&m_oLock);
            m_lstNotifyFd.push_back(oNotify);
        }

        char buf[2] = {'c', '0'};
        if (1 != write(m_nNotifySendFd, buf, 1)) 
        {
            LOG(LT_ERROR, "Worker thread| push notify fd failed| thread_id=%llu| type=%d| fd=%d| local_addr=%s:%d| remote_addr=%s:%d", 
                m_nThreadID, oNotify.m_nType, oNotify.m_nFd, oNotify.m_szLocalAddr, oNotify.m_nLocalPort, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort
                ); 
            _exit(0);
        }

        return true;
    }

    void CWorkerThread::NotifyCallback(evutil_socket_t fd, short events, void *arg) 
    {
        char buf[2];
        CWorkerThread *pThis = (CWorkerThread *)arg;
        CNotifyFd oNotify;
        
        //fd就是m_nNotifyReadFd
        if (1 != read(fd, buf, 1)) 
        {
            LOG(LT_ERROR, "Worker notify callback| read pipe failed| thread_id=%llu| msg=%s", pThis->m_nThreadID, strerror(errno)); 
            _exit(0);
        }

        //从队列取出通知消息
        {
            Lock lock(&pThis->m_oLock);
            if(pThis->m_lstNotifyFd.empty())
            {
                LOG(LT_ERROR, "Worker notify callback| queue is empty| thread_id=%llu", pThis->m_nThreadID); 
                return;
            }

            oNotify = pThis->m_lstNotifyFd.front();
            pThis->m_lstNotifyFd.pop_front();
        }

        switch (oNotify.m_nType)
        {
        case NOTIFY_TYPE_ACCEPT:
            pThis->DoAccept(oNotify);
            break;
            
        case NOTIFY_TYPE_CONNECT:
            pThis->DoConnect(oNotify);
            break;
            
        case NOTIFY_TYPE_LISTEN:
            pThis->DoListen(oNotify);
            break;

        case NOTIFY_TYPE_CLOSE:
            pThis->DoClose(oNotify);
            break;

        default:
            LOG(LT_ERROR, "Worker notify callback| unknow type| thread_id=%llu| type=%d| fd=%d| local_addr=%s:%d| remote_addr=%s:%d", 
                pThis->m_nThreadID, oNotify.m_nType, oNotify.m_nFd, oNotify.m_szLocalAddr, oNotify.m_nLocalPort, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort);
            break;
        }        
    }

    //Connect的通知处理
    void CWorkerThread::DoAccept(const CNotifyFd &oNotify) 
    {
        TcpSocket *pSocket = CSocketManager::Instance()->GetTcpSocket(oNotify.m_nFd);
        if(NULL == pSocket) 
        {
            LOG(LT_ERROR, 
                "Worker thread| do accept notify get socket failed| thread_id=%llu| type=%d| fd=%d| local_addr=%s:%d| remote_addr=%s:%d", 
                m_nThreadID, oNotify.m_nType, oNotify.m_nFd, oNotify.m_szLocalAddr, oNotify.m_nLocalPort, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort
                ); 
            close(oNotify.m_nFd);            
            return;
        }

        if(Socket::SOCK_STATE_INIT != pSocket->GetState())
        {                
            LOG(LT_WARN, "Worker thread| do accept socket not release| fd=%d| state=%d| unique_id=0x%x| addr_msg=%s", 
                oNotify.m_nFd, pSocket->GetState(), pSocket->GetUniqueID(), pSocket->GetAddrMsg().c_str()
                ); 
            pSocket->Release(false);
        }
        
        //设置参数
        SERVERKEY sk(CSocketManager::Instance()->GetLocalServerID());
        pSocket->SetUniqueID(CSocketManager::Instance()->GetSequence());
        pSocket->SetTransID(sk.nType, sk.nInstID); 
        pSocket->SetIsAcceptFd(true);
        pSocket->SetSocketHandler(NULL, oNotify.m_bBinaryMode);
        
        
        std::string sMsg;
        bool bSuccess = false;
        if(!pSocket->Create(oNotify.m_nFd))
        {
            sMsg = "create failed";
        }
        else if (!pSocket->SetKeepAlive(true))   // 保持连接
        {
            sMsg = "keep alive failed";
        }
        else if (!pSocket->SetNoDelay(true))
        {
            sMsg = "no delay failed";
        }
        else if (!pSocket->SetNonBlock())
        {
            sMsg = "not block failed";
        }
        else if (!pSocket->SetLinger())
        {
            sMsg = "linger failed";
        }
        else if (!pSocket->SetReuse(true))
        {
            sMsg = "reuse failed";
        }
        else 
        {
            pSocket->SetLocalAddr(oNotify.m_szLocalAddr, oNotify.m_nLocalPort);
            pSocket->SetRemoteAddr(oNotify.m_szRemoteAddr, oNotify.m_nRemotePort);
            pSocket->SetState(Socket::SOCK_STATE::SOCK_STATE_NORMAL);
            if(!pSocket->RegisterEvent(m_base))
            {
                sMsg = "register failed";
            }
            else 
            {
                bSuccess = true;
            }
        }
        
        //注册事件
        if(bSuccess)
        {
            LOG(LT_INFO_TRANS, pSocket->NextTransID().c_str(),
                "Worker thread| do accept notify succ| thread_id=%llu| type=%d| fd=%d| unique_id=0x%x| local_addr=%s:%d| remote_addr=%s:%d| binary_mode=%d", 
                m_nThreadID, oNotify.m_nType, oNotify.m_nFd, pSocket->GetUniqueID(), oNotify.m_szLocalAddr, oNotify.m_nLocalPort, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort, oNotify.m_bBinaryMode
                ); 
        }
        else 
        {
            LOG(LT_ERROR, 
                "Worker thread| do accept failed| thread_id=%llu| type=%d| fd=%d| local_addr=%s:%d| remote_addr=%s:%d| msg=%s", 
                m_nThreadID, oNotify.m_nType, oNotify.m_nFd, oNotify.m_szLocalAddr, oNotify.m_nLocalPort, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort, sMsg.c_str()
                ); 
            pSocket->Release(true);
        }        
    }
    
    //Connect的通知处理
    void CWorkerThread::DoConnect(const CNotifyFd &oNotify) 
    {
        TcpSocket *pSocket = CSocketManager::Instance()->GetTcpSocket(oNotify.m_nFd);
        if(NULL == pSocket) 
        {
            LOG(LT_ERROR, 
                "Worker thread| do connect notify get socket failed| thread_id=%llu| type=%d| fd=%d| local_addr=%s:%d| remote_addr=%s:%d", 
                m_nThreadID, oNotify.m_nType, oNotify.m_nFd, oNotify.m_szLocalAddr, oNotify.m_nLocalPort, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort
                );  
            close(oNotify.m_nFd);            
            return;
        }

        if(Socket::SOCK_STATE_INIT != pSocket->GetState())
        {                
            LOG(LT_WARN, "Worker thread| do connect socket not release| fd=%d| state=%d| unique_id=0x%x| addr_msg=%s", 
                oNotify.m_nFd, pSocket->GetState(), pSocket->GetUniqueID(), pSocket->GetAddrMsg().c_str()
                ); 
            pSocket->Release(false);
        }
        
        //设置参数
        SERVERKEY sk(CSocketManager::Instance()->GetLocalServerID());
        pSocket->SetUniqueID(CSocketManager::Instance()->GetSequence());
        pSocket->SetTransID(sk.nType, sk.nInstID); 
        pSocket->SetIsAcceptFd(false);
        pSocket->SetSocketHandler(oNotify.m_pHandler, oNotify.m_bBinaryMode);

        std::string sMsg;
        bool bSuccess = false;
        if(!pSocket->Create(oNotify.m_nFd))
        {
            sMsg = "create failed";
        }
        else if (!pSocket->SetKeepAlive(true))   // 保持连接
        {
            sMsg = "keep alive failed";
        }
        else if (!pSocket->SetNoDelay(true))
        {
            sMsg = "no delay failed";
        }
        else if (!pSocket->SetNonBlock())
        {
            sMsg = "not block failed";
        }
        else if (!pSocket->SetLinger())
        {
            sMsg = "linger failed";
        }
        else if (!pSocket->SetReuse(true))
        {
            sMsg = "reuse failed";
        }
        else if (pSocket->Connect(oNotify.m_szRemoteAddr, oNotify.m_nRemotePort) < 0)
        {
            sMsg = "connect failed";
        }
        else if(!pSocket->RegisterEvent(m_base))
        {   
            sMsg = "register failed";
        }
        else {
            bSuccess = true;
        }

        //注册事件
        if(bSuccess)
        {
            LOG(LT_INFO_TRANS, pSocket->NextTransID().c_str(), 
                "Worker thread| do connect notify succ| thread_id=%llu| type=%d| fd=%d| unique_id=0x%x| local_addr=%s:%d| remote_addr=%s:%d| binary_mode=%d", 
                m_nThreadID, oNotify.m_nType, oNotify.m_nFd, pSocket->GetUniqueID(), oNotify.m_szLocalAddr, oNotify.m_nLocalPort, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort, oNotify.m_bBinaryMode
                ); 
        }
        else 
        {
            LOG(LT_ERROR, 
                "Worker thread| do connect failed| thread_id=%llu| type=%d| fd=%d| local_addr=%s:%d| remote_addr=%s:%d| state=%d| msg=%s", 
                m_nThreadID, oNotify.m_nType, oNotify.m_nFd, oNotify.m_szLocalAddr, oNotify.m_nLocalPort, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort, pSocket->GetState(), sMsg.c_str()
                ); 
            pSocket->Release(true);

            if(NULL == oNotify.m_pHandler) 
            {
                oNotify.m_pHandler->OnConnect(oNotify.m_nFd, false, 0, oNotify.m_bBinaryMode);
            }
        } 
         
    }

    //Listen的通知处理
    void CWorkerThread::DoListen(const CNotifyFd &oNotify) 
    {
        TcpSocket *pSocket = CSocketManager::Instance()->GetTcpSocket(oNotify.m_nFd);
        if(NULL == pSocket) 
        {
            LOG(LT_ERROR, 
                "Worker thread| do listen notify get socket failed| thread_id=%llu| type=%d| fd=%d| local_addr=%s:%d| remote_addr=%s:%d", 
                m_nThreadID, oNotify.m_nType, oNotify.m_nFd, oNotify.m_szLocalAddr, oNotify.m_nLocalPort, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort
                ); 
            close(oNotify.m_nFd);
            _exit(0);
        }

        pSocket->SetLocalAddr(oNotify.m_szLocalAddr, oNotify.m_nLocalPort);

        //注册listen事件
        if(!pSocket->RegisterListen(m_base))
        {
            LOG(LT_ERROR, 
                "Worker thread| do listen notify register failed| thread_id=%llu| type=%d| fd=%d| local_addr=%s:%d| remote_addr=%s:%d", 
                m_nThreadID, oNotify.m_nType, oNotify.m_nFd, oNotify.m_szLocalAddr, oNotify.m_nLocalPort, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort
                ); 
                
            pSocket->Release(true);
            _exit(0);
        }

        LOG(LT_INFO, "Worker thread| do listen notify succ| thread_id=%llu| type=%d| fd=%d| local_addr=%s:%d| remote_addr=%s:%d", 
                m_nThreadID, oNotify.m_nType, oNotify.m_nFd, oNotify.m_szLocalAddr, oNotify.m_nLocalPort, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort
                );  
    }


    
    void CWorkerThread::DoClose(const CNotifyFd &oNotify) 
        {
            TcpSocket *pSocket = CSocketManager::Instance()->GetTcpSocket(oNotify.m_nFd);
            if(NULL == pSocket) 
            {
                LOG(LT_ERROR, 
                    "Worker thread| do connect notify get socket failed| thread_id=%llu| type=%d| fd=%d| local_addr=%s:%d| remote_addr=%s:%d", 
                    m_nThreadID, oNotify.m_nType, oNotify.m_nFd, oNotify.m_szLocalAddr, oNotify.m_nLocalPort, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort
                    );  
                close(oNotify.m_nFd);            
                return;
            }

            LOG(LT_INFO_TRANS, pSocket->NextTransID().c_str(), 
                "Worker thread| do close notify succ| thread_id=%llu| type=%d| fd=%d| unique_id=0x%x| local_addr=%s:%d| remote_addr=%s:%d| binary_mode=%d", 
                m_nThreadID, oNotify.m_nType, oNotify.m_nFd, pSocket->GetUniqueID(), oNotify.m_szLocalAddr, oNotify.m_nLocalPort, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort, oNotify.m_bBinaryMode
                ); 
            
            pSocket->ActiveClose();
        }
}

