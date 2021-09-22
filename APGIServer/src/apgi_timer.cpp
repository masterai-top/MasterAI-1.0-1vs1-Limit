/**
* \file room_recycle_timer.cpp
* \brief 房间定时任务处理类实现函数
*/

#include "pch.h"
#include "apgi_timer.h"
#include "comm_define.h"
#include "comm.pb.h"
#include "net_msg.h"
#include "acpc.pb.h"
#include "brain.pb.h"
#include "apgi_conf.h"

void CAPGIClientTimer::OnTimer(uint64 nTimerID)
{        
    int num = 0;
    LISTSMG *p = CQUEUE_List::Instance()->GetNoBlockNode(QUEUE_FREE, &num);
    if(NULL == p) {
        LOG(LT_INFO_TRANS, "", "Robot timer| Get node failed| ConnID=%llu", nTimerID);
    }
    else
    {
        static uint16 nSeq1 = (uint16)rand();
        static uint8  nSeq2 = 1;
        uint32  nUniqueID   = nTimerID >> 32;
        int qIndex = (nUniqueID)%CAPGIConf::Instance()->m_nTaskThreadCnt;
        
        p->recv_time.Restart();
        p->nCliConnID   = nTimerID;
        p->nCmd         = TIMER_CMD_CHECK_CLIENT_TIMEOUT;
        snprintf(p->szTransID, sizeof(p->szTransID) - 1, "tmr.%.08x-%.02x-%.04x", nUniqueID, nSeq2++, nSeq1++);
        
        CQUEUE_List::Instance()->SetNode(p, QUEUE_TASK + qIndex);
    }
}


void CTimerRunGame::OnTimer(uint64 nTimerID)
{        
    int num = 0;
    LISTSMG *p = CQUEUE_List::Instance()->GetNoBlockNode(QUEUE_FREE, &num);
    if(NULL == p) {
        LOG(LT_INFO_TRANS, "", "Run game timer| Get node failed| TimerID=%llu", nTimerID);
    }
    else
    {
        static uint16 nSeq1 = (uint16)rand();
        static uint8  nSeq2 = 1;
        uint32  nTempID     = (nTimerID & 0xFFFFFFFF);
        int qIndex = (nTimerID)%CAPGIConf::Instance()->m_nTaskThreadCnt;
        
        p->recv_time.Restart();
        p->nCliConnID   = nTimerID;
        p->nCmd         = TIMER_CMD_CHECK_RUNGAME_TIMEOUT;
        snprintf(p->szTransID, sizeof(p->szTransID) - 1, "tmr.%.08x-%.02x-%.04x", nTempID, nSeq2++, nSeq1++);
        
        CQUEUE_List::Instance()->SetNode(p, QUEUE_TASK + qIndex);
    }
}



