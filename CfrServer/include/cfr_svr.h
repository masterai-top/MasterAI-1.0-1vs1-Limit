/**
* \file cfr_svr.h
* \brief 
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __CFR_SVR_H__
#define __CFR_SVR_H__

#include "app/app.h"
#include "memory/singleton.h"
#include "conf.h"
#include "task.h"
#include "define.h"


class CCfrSvr : public frame::App, public frame::Singleton<CCfrSvr>
{
public:
    CCfrSvr(void);
    virtual ~CCfrSvr(void);

    
    virtual bool OnInit();
    virtual void OnRestore();
    virtual void OnSignal(int32 nSignalID);

    static void * TaskIt(void *);
    
private:
};

#endif // __CFR_SVR_H__

