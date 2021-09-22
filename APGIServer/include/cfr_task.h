/**
* \file cfr_task.h
* \brief 处理来自cfrsvr的任务
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
*/

#ifndef __CFR_TASK_H__
#define __CFR_TASK_H__

#include "typedef.h"
#include "CQUEUE_List.h"

/**
* \brief 处理来自balance的任务
*/
class CCfrTask
{
private:
    CCfrTask(void);
    
public:
    static CCfrTask* Instance()
    {
        static CCfrTask m_instance;
        return &m_instance;
    }
    
    ~CCfrTask(void);

   
    bool Init();
    void Restore();

    void DoProc(LISTSMG * const p);

    void KeepAliveResp(LISTSMG * const p);
    void QueryActionResp(LISTSMG * const p);

};

#endif


