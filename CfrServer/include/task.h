/**
* \file task.h
* \brief 任务处理
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
#include <string>

/**
* \brief 任务处理
*/
class CTask
{
private:
    CTask(void);
    
public:
    static CTask* Instance()
    {
        static CTask m_instance;
        return &m_instance;
    }
    
    ~CTask(void);

    bool Init();
    void Restore();

public:
    void ErrorResponse(LISTSMG * const p, uint32 errCode, const std::string &errMsg);
    void QueryAction(LISTSMG * const p);
    void KeeyAlive(LISTSMG * const p);
    void QueryVersion(LISTSMG * const p);
};

#endif // __CFR_TASK_H__


