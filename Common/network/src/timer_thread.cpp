#include "timer_thread.h"
#include "socket.h"
#include "timer_manager.h"
#include <assert.h>
#include "timeutil.h"

using namespace dtutil;

namespace network
{
    CTimerThread::CTimerThread()
    {
        m_nPipeReadFd = INVALID_SOCKET;
        m_nPipeSendFd = INVALID_SOCKET;
        m_base        = NULL;
    }

    CTimerThread::~CTimerThread()
    {
        if(INVALID_SOCKET != m_nPipeReadFd) 
        {
            close(m_nPipeReadFd);
            m_nPipeReadFd = INVALID_SOCKET;
        }

        if(INVALID_SOCKET != m_nPipeSendFd) 
        {
            close(m_nPipeSendFd);
            m_nPipeSendFd = INVALID_SOCKET;
        }
        
        if(NULL != m_base) 
        {
            event_del(&m_oPipeEvent);
            event_base_free(m_base);
            m_base = NULL;
        }
    }

    
    bool CTimerThread::Init()
    {
        int fds[2];
        if (pipe(fds)) 
        {
            fprintf(stderr, "Timer thread init| Can't create notify pipe\n");
            LOG(LT_ERROR, "Timer thread init| Can't create notify pipe");
            exit(1);
            return false;
        }

        m_nUnProcCnt  = 0;
        m_nPipeReadFd = fds[0];
        m_nPipeSendFd = fds[1];

        m_base = event_base_new();
        if(NULL == m_base) 
        {
            LOG(LT_ERROR, "Timer thread init| new base failed");
            exit(2);
            return false;
        }

        event_set(&m_oPipeEvent, m_nPipeReadFd, EV_READ | EV_PERSIST, NotifyCallback, (void*)this);
        event_base_set(m_base, &m_oPipeEvent);

        if (-1 == event_add(&m_oPipeEvent, 0)) 
        {
            fprintf(stderr, "Timer thread init| Can't monitor libevent notify pipe\n");
            LOG(LT_ERROR, "Timer thread init| Can't monitor libevent notify pipe");
            exit(3);
            return false;
        }

        
        pthread_t tid = 0;
        if(0 != pthread_create( &tid, NULL, &CTimerThread::Run, (void*)this))
        {
            LOG(LT_ERROR, "Timer thread init| create failed");
            return false;
        }

        return true;
    }

    void* CTimerThread::Run(void *arg) 
    {   
        pthread_detach( pthread_self() );	
        LOG(LT_INFO, "Timer thread run ...| pid=%llu", pthread_self());
        
        CTimerThread *pThis = (CTimerThread *)arg;
        pThis->m_nThreadID  = pthread_self();
        
        event_base_dispatch(pThis->m_base);
        LOG(LT_INFO, "Timer thread Exit ...| pid=%llu", pthread_self());
        _exit(0);
        
        return NULL;
    }
    
    bool CTimerThread::PushTimerNotify(const CTimerNotify &oNotify)
    {   
        {
            Lock lock(&m_oLock);
            m_lstTimerNotify.push_back(oNotify);
            ++m_nUnProcCnt;
        }

        char buf[2] = {'c', '0'};
        if (1 != write(m_nPipeSendFd, buf, 1)) 
        {
            LOG(LT_ERROR_TRANS, oNotify.m_szTransID, "Timer thread| push notify failed| thread_id=%llu| type=%d| timer_id=%llu", 
                m_nThreadID, oNotify.m_nType, oNotify.GetTimerID()
                ); 
            sleep(10);
            _exit(0);
        }

        return true;
    }

    void CTimerThread::NotifyCallback(evutil_socket_t fd, short events, void *arg) 
    {
        if(NULL == arg) {
            LOG(LT_ERROR, "Timer notify callback| arg is null");
            return;
        }
        
        char buf[2];
        CTimerThread *pThis = (CTimerThread *)arg;
        CTimerNotify oNotify;
        
        //fd就是m_nNotifyReadFd
        if (1 != read(fd, buf, 1)) 
        {
            LOG(LT_ERROR, "Timer notify callback| read pipe failed| thread_id=%llu| msg=%s", pThis->m_nThreadID, strerror(errno)); 
            sleep(10);
            _exit(0);
        }

        //从队列取出通知消息
        {
            Lock lock(&pThis->m_oLock);
            if(pThis->m_lstTimerNotify.empty())
            {
                LOG(LT_ERROR, "Timer notify callback| queue is empty| thread_id=%llu", pThis->m_nThreadID); 
                return;
            }

            oNotify = pThis->m_lstTimerNotify.front();
            pThis->m_lstTimerNotify.pop_front();
            --pThis->m_nUnProcCnt;
            
            if(NULL != oNotify.m_pTimerValue) {
                oNotify.m_pTimerValue->m_pTimerThread = pThis;
            }
        }

        

        switch (oNotify.m_nType)
        {
        case TIMER_NOTIFY_TYPE_ATTACH:
            pThis->DoAttack(oNotify);
            break;
            
        case TIMER_NOTIFY_TYPE_DETACH:
            pThis->DoDetach(oNotify, "external");
            break;

        default:
            LOG(LT_ERROR_TRANS, oNotify.m_szTransID, "Timer notify callback| unknow type| type=%d| timer_id=%llu| un_proc_cnt=%d", 
                oNotify.m_nType, oNotify.GetTimerID(), pThis->m_nUnProcCnt
                );
            CTimerManager::Instance()->PushTimerValue(oNotify.m_pTimerValue);
            break;
        }        
    }

    //定时器回调
    void CTimerThread::TimerCallback(evutil_socket_t fd, short events, void *arg)
    {
        //pTValue与m_mapTimerTask的指针执行同一个地址
        TimerValue *pTValue = (TimerValue*)(arg);
        if(NULL == pTValue) {
            LOG(LT_ERROR, "Timer callback| arg is null");
            return ;
        }

        if(NULL == pTValue->m_pTimerThread) {
            LOG(LT_ERROR, "Timer callback| thread obj is null");
            return ;
        }
        
        CTimerThread *pThis = pTValue->m_pTimerThread;
        if(NULL == pThis)
        {
            LOG(LT_ERROR, "Timer callback| pthis is null");
            return;
        }

        if(NULL == pTValue->m_pHandler) 
        {
            LOG(LT_ERROR, "Timer callback| handler is null");
            return;
        }
        
        //对象处理函数回调
        pTValue->m_pHandler->OnTimer(pTValue->m_nTimerID);

        //参数赋值
        pTValue->m_nLastCallTime = TimeUtil::NowMs();
        if (pTValue->m_nCallTimes > 0)
        {
            pTValue->m_nCallTimes--;
        }

        // 调用次数已经够了
        if (0 == pTValue->m_nCallTimes)
        {
            CTimerNotify oNotify(TIMER_NOTIFY_TYPE_DETACH, pTValue->m_nTimerID, (uint64)pTValue->m_pHandler, NULL, "NormalDetachTid");
            if(!pThis->DoDetach(oNotify, "internal")) 
            {   
                //理论上不应该出现...
                evtimer_del(&pTValue->m_event);
                //CTimerManager::Instance()->PushTimerValue(pTValue); 
                LOG(LT_ERROR, "Timer callback| detach failed| timer_id=%llu| handler_id=%llu", pTValue->m_nTimerID, (uint64)pTValue->m_pHandler);
            }
        }
        else
        {
            // 重新注册超时事件
            pThis->RegisterTimerEvent(pTValue);
            LOG(LT_INFO, "Timer callback| register again| timer_id=%llu| handler_id=%llu| call_times=%u| internal=%u| timer_size=%u",
                pTValue->m_nTimerID, (uint64)pTValue->m_pHandler, pTValue->m_nCallTimes, pTValue->m_nInterval, pThis->m_mapTimerTask.size()
                );
        }
    }
    
    //添加定时器
    void CTimerThread::DoAttack(const CTimerNotify &oNotify) 
    {
        assert(NULL != oNotify.m_pTimerValue);
        if( NULL == oNotify.m_pTimerValue->m_pHandler || oNotify.m_oTimerKey.m_nHandlerID != (uint64)oNotify.m_pTimerValue->m_pHandler 
            || oNotify.m_oTimerKey.m_nTimerID != oNotify.m_pTimerValue->m_nTimerID || this != oNotify.m_pTimerValue->m_pTimerThread 
            || 0 == oNotify.m_pTimerValue->m_nInitTime  || 0 == oNotify.m_pTimerValue->m_nLastCallTime
            || 0 == oNotify.m_pTimerValue->m_nCallTimes || 0 == oNotify.m_pTimerValue->m_nInterval
            )
        {
            LOG(LT_ERROR_TRANS, oNotify.m_szTransID, "Timer attack invalid param| handler_id=%llu(%llu)| timer_id=%llu(%llu)| timer_ptr=%llu(%llu)|"
                "init_time=%u| last_call_time=%u| call_times=%u| internal=%u",
                oNotify.m_oTimerKey.m_nHandlerID, (uint64)oNotify.m_pTimerValue->m_pHandler,
                oNotify.m_oTimerKey.m_nTimerID, oNotify.m_pTimerValue->m_nTimerID,
                (uint64)this, (uint64)oNotify.m_pTimerValue->m_pTimerThread, 
                oNotify.m_pTimerValue->m_nInitTime, oNotify.m_pTimerValue->m_nLastCallTime, oNotify.m_pTimerValue->m_nCallTimes, oNotify.m_pTimerValue->m_nInterval
                );
                
            CTimerManager::Instance()->PushTimerValue(oNotify.m_pTimerValue);
            return;
        }

        //已存在
        std::map<TimerKey, TimerValue*>::iterator it = m_mapTimerTask.find(oNotify.m_oTimerKey);
        if(it != m_mapTimerTask.end()) 
        {
            //更新定时器
            TimerValue *pOld        = it->second;
            pOld->m_nInterval       = oNotify.m_pTimerValue->m_nInterval;
            pOld->m_nCallTimes      = oNotify.m_pTimerValue->m_nCallTimes;
            pOld->m_nLastCallTime   = oNotify.m_pTimerValue->m_nLastCallTime;
            RegisterTimerEvent(pOld);

            LOG(LT_WARN_TRANS, oNotify.m_szTransID, "Timer attack cover| timer_id=%llu| handler_id=%llu| call_times=%u| internal=%u",
                oNotify.m_oTimerKey.m_nTimerID, oNotify.m_oTimerKey.m_nHandlerID, oNotify.m_pTimerValue->m_nCallTimes, oNotify.m_pTimerValue->m_nInterval
                );
                
            //回收资源
            CTimerManager::Instance()->PushTimerValue(oNotify.m_pTimerValue);
            return;
        }

        std::pair<std::map<TimerKey, TimerValue*>::iterator, bool> ret;
        ret = m_mapTimerTask.insert(std::pair<TimerKey, TimerValue*>(oNotify.m_oTimerKey, oNotify.m_pTimerValue));
        if (ret.second==false) 
        {
            LOG(LT_ERROR_TRANS, oNotify.m_szTransID, "Timer attack insert failed| timer_id=%llu| handler_id=%llu",
                oNotify.m_oTimerKey.m_nTimerID, oNotify.m_oTimerKey.m_nHandlerID
                );
            CTimerManager::Instance()->PushTimerValue(oNotify.m_pTimerValue);
            return;
        }

        event_set(&oNotify.m_pTimerValue->m_event, 1, EV_PERSIST, TimerCallback, (void *)(oNotify.m_pTimerValue));
        if(0 != event_base_set(m_base, &oNotify.m_pTimerValue->m_event))
        {
            LOG(LT_ERROR_TRANS, oNotify.m_szTransID, "Timer attack set event base failed| timer_id=%llu| handler_id=%llu",
                oNotify.m_oTimerKey.m_nTimerID, oNotify.m_oTimerKey.m_nHandlerID
                );
            CTimerManager::Instance()->PushTimerValue(oNotify.m_pTimerValue);
            return;
        }
        
        RegisterTimerEvent(oNotify.m_pTimerValue);      
        LOG(LT_INFO_TRANS, oNotify.m_szTransID, "Timer attack succ| timer_id=%llu| handler_id=%llu| call_times=%u| internal=%u| timer_size=%u",
            oNotify.m_oTimerKey.m_nTimerID, oNotify.m_oTimerKey.m_nHandlerID, oNotify.m_pTimerValue->m_nCallTimes, oNotify.m_pTimerValue->m_nInterval, m_mapTimerTask.size()
            );
    }
    
    //删除定时器
    bool CTimerThread::DoDetach(const CTimerNotify &oNotify, const string &sReason) 
    {
        assert(NULL == oNotify.m_pTimerValue);
        bool bRet   = false;
        string sMsg = "UnFind";
        std::map<TimerKey, TimerValue*>::iterator it = m_mapTimerTask.find(oNotify.m_oTimerKey);
        if(it != m_mapTimerTask.end()) 
        {   
            TimerValue *pTimerValue = it->second;
            m_mapTimerTask.erase(it);

            evtimer_del(&pTimerValue->m_event);
            CTimerManager::Instance()->PushTimerValue(pTimerValue);
            sMsg = "Succ";
            bRet = true;
        }

        LOG(LT_INFO_TRANS, oNotify.m_szTransID, 
            "Timer detach %s| type=%d| timer_id=%llu| un_proc_cnt=%d| timer_size=%u| msg=%s", 
            sReason.c_str(), oNotify.m_nType, oNotify.GetTimerID(), m_nUnProcCnt, m_mapTimerTask.size(), sMsg.c_str()
            ); 

        return bRet;
    }

    // 注册定时器超时事件
    void CTimerThread::RegisterTimerEvent(TimerValue *pTimerValue)
    {
        assert(NULL != pTimerValue);
        struct timeval timeout;
        evutil_timerclear(&timeout);
        uint32 nTimeout = pTimerValue->m_nInterval;// - (pTimerValue->m_nLastCallTime - pTimerValue->m_nInitTime)%pTimerValue->m_nInterval;  //直接使用internal?
        timeout.tv_sec  = nTimeout / 1000;
        timeout.tv_usec = nTimeout % 1000 * 1000;
                
        evtimer_add(&pTimerValue->m_event, &timeout);  
    }
}

