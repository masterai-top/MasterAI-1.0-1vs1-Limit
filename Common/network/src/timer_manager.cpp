
#include "pch.h"
#include "timer_manager.h"
#include "timeutil.h"

using namespace frame;
using namespace dtutil;

namespace network
{
    CTimerManager::CTimerManager() 
    {
    }

    CTimerManager::~CTimerManager() 
    {
    }

    bool  CTimerManager::Init(uint32 nPreNodeCnt)
    {
        if(!m_oTimerThread.Init()) {
            return false;
        }

        int nErr = m_oPreTimerValue.Init("TIMER_VALUE", nPreNodeCnt);
        if(0 != nErr) {
            LOG(LT_ERROR, "TIMER_MANAGER init value failed| err=%d", nErr);
            return false;
        }

        return true;
    }

    bool  CTimerManager::Attack(ITimerHandler *pHandler, uint64 nTimerID, uint32 nInterval, uint32 nCallTimes, const char *szDesc, const char *szTransID)
    {
        if (NULL == pHandler)
        {
            LOG(LT_ERROR_TRANS, szTransID, "Timer manager attack handler is null");
            return false;
        }

        if(0 == nInterval || 0 == nCallTimes) {
            LOG(LT_ERROR_TRANS, szTransID, "Timer manager attack invalid internal=%d| call_times=%d", nInterval, nCallTimes);
            return false;
        }

        TimerValue *ptv = PopTimerValue();
        if (NULL == ptv)
        {
            LOG(LT_ERROR_TRANS, szTransID, "Timer manager attack no memory| timer_id=%llu", nTimerID);
            return false;
        }

        time_t tCurrent = TimeUtil::NowMs();
        ptv->m_nTimerID         = nTimerID;     //ID
        ptv->m_nCallTimes       = nCallTimes;   //调用次数
        ptv->m_nInterval        = nInterval;    //间隔        
        ptv->m_pHandler         = pHandler;     //回调函数
        ptv->m_nInitTime        = tCurrent;     //初始化时间
        ptv->m_nLastCallTime    = tCurrent;     //最后一次调用的时间
        ptv->m_event.ev_base    = NULL;
        snprintf(ptv->m_szDesc, sizeof(ptv->m_szDesc) - 1, "%s", szDesc);

        CTimerNotify oTNotify(TIMER_NOTIFY_TYPE_ATTACH, nTimerID, (uint64)(pHandler), ptv, szTransID);
        m_oTimerThread.PushTimerNotify(oTNotify);
        
        return true;
    }
    
    void  CTimerManager::Detach(ITimerHandler *pHandler, uint64 nTimerID, const char *szTransID)
    {
        if (NULL == pHandler)
        {
            LOG(LT_ERROR_TRANS, szTransID, "Timer manager detach handler is null");
            return ;
        }

       
        CTimerNotify oTNotify(TIMER_NOTIFY_TYPE_DETACH, nTimerID, (uint64)(pHandler), NULL, szTransID);
        m_oTimerThread.PushTimerNotify(oTNotify);
    }

    TimerValue *CTimerManager::PopTimerValue()
    {
        return m_oPreTimerValue.Pop();
    }

    void CTimerManager::PushTimerValue(TimerValue *pTimerValue)
    {
        if(NULL == pTimerValue) {
            return;
        }

        pTimerValue->m_pHandler      = NULL;
        pTimerValue->m_pTimerThread  = NULL;
        pTimerValue->m_event.ev_base = NULL;
        pTimerValue->m_nTimerID      = 0;
        pTimerValue->m_nCallTimes    = 0;
        pTimerValue->m_nInterval     = 0;
        
        m_oPreTimerValue.Push(pTimerValue);

        //LOG(LT_WARN, "PRE_TIMER_VALUE| init_size=%u| free_size=%u| dynamic_size=%u", m_oPreTimerValue.init_size(), m_oPreTimerValue.size(), m_oPreTimerValue.dynamic_size());
    }
}
