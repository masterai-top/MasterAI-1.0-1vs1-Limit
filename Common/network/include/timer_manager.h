/**
* \file timer_manager.h
* \brief 定时器管理器
*
* Copyright (c) 2019
* All rights reserved.
*/

#ifndef __TIMER_MANAGER_H__
#define __TIMER_MANAGER_H__
//#include "event/event_engine.h"
//#include "memory/array_pool.h"
#include "shpreobject.h"
#include "timer_thread.h"
using namespace frame;

namespace network
{    
    //定时器管理器
    class CTimerManager
    {
    private:
        CTimerManager();
        
    public:
        ~CTimerManager();
        static CTimerManager* Instance()
        {
            static CTimerManager m_instance;
            return &m_instance;
        }
        
        bool  Init(uint32 nPreNodeCnt);
        bool  Attack(ITimerHandler *pHandler, uint64 nTimerID, uint32 nInterval, uint32 nCallTimes, const char *szDesc, const char *szTransID);
        void  Detach(ITimerHandler *pHandler, uint64 nTimerID, const char *szTransID);

        void  PushTimerValue(TimerValue *pEvtTimer);
        TimerValue *PopTimerValue();
            
    private:
        CTimerThread m_oTimerThread;
        shstd::PreObject<TimerValue> m_oPreTimerValue;      //TimerValue预分配对象
    };
}
#endif // __TIMER_MANAGER_H__