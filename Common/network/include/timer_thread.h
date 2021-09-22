/**
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
*/

#ifndef __NW_TIMER_THREAD_H__
#define __NW_TIMER_THREAD_H__
#include "shstd.h"
#include "typedef.h"
#include "list"
#include "thread/lock.h"
#include "event.h"
#include <map>
#include "event/event_engine.h"

using namespace frame;


namespace network
{
    class CTimerThread;
    
    enum {
        TIMER_NOTIFY_TYPE_DEFAULT     = (0), 
        TIMER_NOTIFY_TYPE_ATTACH      = (1), 
        TIMER_NOTIFY_TYPE_DETACH      = (2), 
    };

    struct TimerKey
    {
        uint64  m_nTimerID;         //定时器ID
        uint64  m_nHandlerID;       //回调ID

        TimerKey(uint64 nTimerID, uint64 nHandlerID):m_nTimerID(nTimerID), m_nHandlerID(nHandlerID) {}
        bool operator == (const TimerKey &o)
        {
            return ((m_nTimerID == o.m_nTimerID) && (m_nHandlerID == o.m_nHandlerID));
        }

        bool operator < (const TimerKey &o) const
        {
            if (m_nTimerID < o.m_nTimerID)
            {
                return true;
            }
            else if (m_nTimerID > o.m_nTimerID)
            {
                return false;
            }

            if (m_nHandlerID < o.m_nHandlerID)
            {
                return true;
            }
            else if (m_nHandlerID > o.m_nHandlerID)
            {
                return false;
            }

            // 都相等
            return false;
        }
    };
    
    struct TimerValue
    {
        uint64  m_nTimerID;             //ID
        uint32  m_nCallTimes;           //调用次数
        uint32  m_nInterval;            //间隔
        ITimerHandler *m_pHandler;      //回调函数
        CTimerThread  *m_pTimerThread;  //对象指针
        event   m_event;                //事件
        char    m_szDesc[32];           //描述

        time_t  m_nInitTime;            //初始化时间
        time_t  m_nLastCallTime;        //最后一次调用的时间

        TimerValue():m_nTimerID(0), m_nCallTimes(0), m_nInterval(0), m_pHandler(NULL), m_pTimerThread(NULL) {
            m_event.ev_base = NULL;
        }
    } ;
    
    struct CTimerNotify
    {
        int         m_nType;            //类型
        TimerKey    m_oTimerKey;        //定时器Key
        TimerValue *m_pTimerValue;      //定时器对象指针
        char        m_szTransID[23];    //任务ID

        CTimerNotify(): m_nType(TIMER_NOTIFY_TYPE_DEFAULT), m_oTimerKey(0, 0), m_pTimerValue(NULL) {  }
        CTimerNotify(int type,    uint64 nTimerID, uint64 nHandlerID, TimerValue *pTimerValue, const char *szTransID)
            : m_nType(type), m_oTimerKey(nTimerID, nHandlerID), m_pTimerValue(pTimerValue)
        {
            snprintf(m_szTransID, sizeof(m_szTransID) - 1, "%s", szTransID);
        }

        uint64 GetTimerID() const {    return m_oTimerKey.m_nTimerID ; }
    };

    
   
    class CTimerThread
    {
    public:
        CTimerThread();
        ~CTimerThread();
        
        bool Init();
        int  GetNotifyFd() { return m_nPipeSendFd; } 
        bool PushTimerNotify(const CTimerNotify &oNotify);
        
        static void  NotifyCallback(evutil_socket_t fd, short events, void *arg);
        static void TimerCallback(evutil_socket_t fd, short events, void *arg);
        static void* Run(void *);
            
    private:
        void    DoAttack(const CTimerNotify &oNotify);
        bool    DoDetach(const CTimerNotify &oNotify, const string &sReason);
        void    RegisterTimerEvent(TimerValue *pTimerValue);
            
    private:
        pthread_t           m_nThreadID;            //线程ID
        int                 m_nPipeReadFd;          //Pipe读的fd
        int                 m_nPipeSendFd;          //Pipe发送的fd[写]
        struct event        m_oPipeEvent;           //通知Pipe的事件
        struct event_base  *m_base;                 //事件根基
        int                 m_nUnProcCnt;           //未处理数量
        
        LockObject                      m_oLock;            //锁
        std::list<CTimerNotify>         m_lstTimerNotify;   //定时器通知
        std::map<TimerKey, TimerValue*> m_mapTimerTask;     //定时任务map(都在一个base的回调处理,不需要加锁) -- 以后可使用unordered_map提升效率
    };
}

#endif // __NW_TIMER_THREAD_H__


