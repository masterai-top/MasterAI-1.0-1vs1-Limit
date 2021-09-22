/**
* \file apg_task.h
* \brief 处理来自APG平台的任务
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
*/

#ifndef __APG_TASK_H__
#define __APG_TASK_H__

#include "typedef.h"
#include "CQUEUE_List.h"

class CAPGTask
{
private:
    CAPGTask(void);
    
public:
    static CAPGTask* Instance()
    {
        static CAPGTask m_instance;
        return &m_instance;
    }
    
    ~CAPGTask(void);

    bool Init();
    void Restore();

    void KeepAliveReq(LISTSMG * const p);
    void RegisterReq(LISTSMG * const p);
    void CheckClientTimeout(LISTSMG * const p);
    void CheckRunGameTimeout(LISTSMG * const p);
    
    void GameBeginNotify(LISTSMG * const p);
    void GameSendCardsNotify(LISTSMG * const p);
    void GameTokenNotify(LISTSMG * const p);
    void GameActionNotify(LISTSMG * const p);
    void GameResultNotify(LISTSMG * const p);
    void GameRobotActionReq(LISTSMG * const p, const int &qIndex);
    void GameQueryStateReq(LISTSMG * const p);
    
    void UserStatDataReq(LISTSMG * const p);
    
};

#endif


