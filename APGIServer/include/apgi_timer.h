#ifndef __APGI_TIMER_H__
#define __APGI_TIMER_H__

#include "typedef.h"
#include "frame.h"


class CAPGIClientTimer : public frame::ITimerHandler
{
private:
    CAPGIClientTimer(void) {}
    
public:
    static CAPGIClientTimer* Instance()
    {
        static CAPGIClientTimer m_instance;
        return &m_instance;
    }
    
    ~CAPGIClientTimer(void) {}
    
    bool Init() { return true; }
    void Restore() {}

    virtual void OnTimer(uint64 nTimerID);
};


class CTimerRunGame : public frame::ITimerHandler
{
private:
    CTimerRunGame(void) {}
    
public:
    static CTimerRunGame* Instance()
    {
        static CTimerRunGame m_instance;
        return &m_instance;
    }
    
    ~CTimerRunGame(void) {}
    
    bool Init() { return true; }
    void Restore() {}

    virtual void OnTimer(uint64 nTimerID);
};



#endif 


