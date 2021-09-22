#include "pch.h"
#include "apg_task.h"
#include "apgi_svr.h"
#include "comm.pb.h"
#include "net_msg.h"
#include "comm_define.h"
#include <string.h>
#include "google/protobuf/text_format.h"
#include "apgi_timer.h"
#include "define.h"
#include "matchstate_convertor.h"
#include "cfr_svr_client.h"
#include "dt_robot.h"
#include "Def.h"
#include "run_game_hashmap.h"

using namespace network;
using namespace std;



CAPGTask::CAPGTask(void)
{
}

CAPGTask::~CAPGTask(void)
{
}

bool CAPGTask::Init()
{
    return true;
}

void CAPGTask::Restore()
{
}

void CAPGTask::RegisterReq(LISTSMG *const p)
{
    string szMsg;
    int rc = Pb::ERR_COMM_OTHER; //, nErrCode = 0;
    Pb::ThirdLoginAiSvrReq pbReq;
    Pb::ThirdLoginAiSvrRsp pbRsp;
    APGIClient oClient;

    if (!pbReq.ParseFromArray(p->cPacketBuffer + APGI_HEAD_LEN, p->len - APGI_HEAD_LEN))
    {
        szMsg   = "parse failed";
        rc      = Pb::ERR_COMM_PROTO_PARSE;
        CSocketManager::Instance()->CloseConnect(p->connfd);
    }
    else
    {
        oClient.SetConnID(p->nUniqueID, p->connfd);
        oClient.SetLastTime(time(NULL));
        oClient.SetDesc(p->sAddr);
        /*if(0 != (nErrCode = CAPGIClientHashMap::Instance()->Add(p->szTransID, oClient)))
        {
            CSocketManager::Instance()->CloseConnect(p->connfd);
            szMsg   = "add failed(" + std::to_string(nErrCode) + ")";
            rc      = Pb::ERR_COMM_INTERNAL;
        }
        else */
        {
            szMsg = "Succ";
            rc    = Pb::OK;
        }
    }

    LOG(LT_INFO_TRANS, p->szTransID, "DONE_APG_REGISTER_REQ| conn_id=%llu| remote_server_id=0x%x| token=%s| client_cnt=%u| rc=%d| msg=%s",
        oClient.GetConnID(), p->nRemoteServerID, pbReq.token().c_str(), CAPGIClientHashMap::Instance()->Size(), rc, szMsg.c_str());

    pbRsp.set_resultcode(rc);
    DoApgiComNetPacket(p, Pb::THIRD_CMD_LOGIN_AISVR_RSP, 0, p->szTransID, 0, &pbRsp);
    CSocketManager::Instance()->Send(p);
}

void CAPGTask::KeepAliveReq(LISTSMG *const p)
{
    int rc = Pb::ERR_COMM_OTHER;
    string szMsg;
    Pb::ThirdKeepAliveReq pbReq;
    Pb::ThirdKeepAliveRsp pbRsp;
    APGIClient *pClient = NULL;

    if (!pbReq.ParseFromArray(p->cPacketBuffer + APGI_HEAD_LEN, p->len - APGI_HEAD_LEN))
    {
        szMsg   = "parse failed";
        rc      = Pb::ERR_COMM_PROTO_PARSE;
        pbRsp.set_timestamp(time(NULL));

    }
    /*else if(NULL == (pClient = CAPGIClientHashMap::Instance()->Get(p->szTransID, GToConnID(p->nUniqueID, p->connfd))))
    {
        szMsg   = "Get failed";
        rc      = Pb::ERR_COMM_INTERNAL;
        CSocketManager::Instance()->CloseConnect(p->connfd);
    }*/
    else
    {
        pbRsp.set_timestamp(pbReq.timestamp());
        szMsg = "Succ";
        rc    = Pb::OK;
    }

    LOG(LT_INFO_TRANS, p->szTransID, "DONE_APG_KEEP_ALIVE_REQ| conn_id=%llu| remote_server_id=0x%x| timestamp=%u| rc=%d| msg=%s",
        (NULL == pClient) ? 0 : pClient->GetConnID(), p->nRemoteServerID, pbReq.timestamp(), rc, szMsg.c_str());

    DoApgiComNetPacket(p, Pb::THIRD_CMD_KEEPALIVE_RSP, 0, p->szTransID, 0, &pbRsp);
    CSocketManager::Instance()->Send(p);
}


void CAPGTask::CheckClientTimeout(LISTSMG *const p)
{
    string szMsg;
    int rc = Pb::OK;

    uint64 nConnID      = p->nCliConnID;

    if(CAPGIClientHashMap::Instance()->IsTimeOut(p->szTransID, nConnID))
    {
        //不用关闭链接. 因为可能已被复用
        CAPGIClientHashMap::Instance()->Del(p->szTransID, nConnID);
        szMsg = "Succ(Timeout)";
    }
    else
    {
        szMsg = "Succ";
    }

    LOG(LT_INFO_TRANS, p->szTransID, "DO_CHECK_CLIENT_TIMEOUT| conn_id=%llu| client_cnt=%u| rc=%d| msg=%s",
        nConnID, CAPGIClientHashMap::Instance()->Size(), rc, szMsg.c_str());
    CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
}


void CAPGTask::CheckRunGameTimeout(LISTSMG *const p)
{
    string szMsg;
    int rc = Pb::OK;
    int64 nRobotID     = p->nCliConnID;
    CRunGame *pRunGame = NULL;
    Pb::ThirdRobotActionRsp pbRsp;

    CRobotIDLock l(p->szTransID, nRobotID, g_oRobotIDLocks);
    if(CRunGameHashMap::Instance()->IsTimeOut(p->szTransID, nRobotID))
    {
        //不可关闭链接. 因为可能已被复用
        CRunGameHashMap::Instance()->Del(p->szTransID, nRobotID);
        szMsg = "Succ(Timeout)";
    }
    else if(NULL != (pRunGame = CRunGameHashMap::Instance()->Get(p->szTransID, nRobotID, false)))
    {
        if(GRS_WAIT_ROBOT_ACTION == pRunGame->GetRunState() && (time(NULL) - pRunGame->GetRequestActionTime()) > CAPGIConf::Instance()->m_nRobotTimeout)
        {
            pRunGame->SetRunState(GRS_ERR_ROBOT_ACTION);
            pbRsp.set_resultcode(Pb::THIRD_SUCC);
            pbRsp.set_err_msg("Timeout");
            pbRsp.set_actiontype(ActionType::a_fold);
            p->connfd    = pRunGame->GetConnSock();
            p->nUniqueID = pRunGame->GetConnUniqueID();
            rc      = Pb::ERR_COMM_SYSTEM_BUSY;
            szMsg   = "robot_action_delay";
        }
        else
        {
            szMsg = "Succ";
        }
    }
    else
    {
        szMsg = "Succ(nil)";
    }

    LOG(LT_INFO_TRANS, p->szTransID, "DO_CHECK_RUNGAME_TIMEOUT| robot_id=%llu| run_game_cnt=%u| rc=%d| msg=%s",
        nRobotID, CRunGameHashMap::Instance()->Size(), rc, szMsg.c_str()
       );
    if(Pb::ERR_COMM_SYSTEM_BUSY == rc)
    {
        DoApgiComNetPacket(p, Pb::THIRD_CMD_ROBOT_ACTION_RSP, nRobotID, p->szTransID, Pb::THIRD_SUCC, &pbRsp);
        CSocketManager::Instance()->Send(p);
    }
    else
    {
        CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
    }
}


void CAPGTask::GameBeginNotify(LISTSMG *const p)
{
    string szMsg;
    int rc = Pb::ERR_COMM_OTHER, nErrCode = 0;
    Pb::ThirdGameBeginNotify pbReq;
    CRunGame *pRunGame = NULL;
    int64 nRobotID     = p->nCliConnID;
    int64 nPlayerID    = 0;
    uint32 nLocalRoleID = 0;
    uint64 nLocalRoomID = 0;
    CTimePiece utp;
    string strCfrModel;

    if (!pbReq.ParseFromArray(p->cPacketBuffer + APGI_HEAD_LEN, p->len - APGI_HEAD_LEN))
    {
        szMsg   = "parse failed";
        rc      = Pb::ERR_COMM_PROTO_PARSE;
    }
    else if(0 == nRobotID)
    {
        szMsg   = "invalid robotid";
        rc      = Pb::ERR_COMM_INVALID_PARAM;
    }
    else
    {
        CRobotIDLock l(p->szTransID, nRobotID, g_oRobotIDLocks);
        if(NULL == (pRunGame = CRunGameHashMap::Instance()->Get(p->szTransID, nRobotID)))
        {
            //仅设置Key [即RobotID]
            CRunGame oRunGame;
            oRunGame.SetRobotID(nRobotID);
            if(0 != (nErrCode = CRunGameHashMap::Instance()->Add(p->szTransID, oRunGame)))
            {
                szMsg = "add runGame failed";
                rc    = Pb::ERR_COMM_INTERNAL;
                goto NOTIFY_END;
            }

            //再次查询
            pRunGame = CRunGameHashMap::Instance()->Get(p->szTransID, nRobotID);
        }

        if(NULL == pRunGame)
        {
            szMsg = "Get runGame failed";
            rc    = Pb::ERR_COMM_INTERNAL;
        }
        else if(0 != (nErrCode = pRunGame->SetCfrModel(p->szTransID, pbReq.ai_mode())))
        {
            szMsg = "Set model failed";
            rc    = Pb::ERR_COMM_INTERNAL;
            pRunGame->DebugPrintf(p->szTransID, "NOTIFY_BEGIN");
        }
        else if(0 != pRunGame->DoRunMatch(p->szTransID, pbReq.hand(), pbReq.bankerid()))
        {
            szMsg = "Run match failed";
            rc    = Pb::ERR_COMM_INTERNAL;
            pRunGame->DebugPrintf(p->szTransID, "NOTIFY_BEGIN");
        }
        else
        {
            pRunGame->SetLastTime(time(NULL));
            pRunGame->SetRoomID(pbReq.roomid().c_str());
            pRunGame->SetPlayerID(pbReq.player_id());
            pRunGame->SetPlayerScore(pbReq.player_score());
            pRunGame->DebugPrintf(p->szTransID, "NOTIFY_BEGIN");

            szMsg       = "Succ";
            rc          = Pb::OK;
            nLocalRoleID = pRunGame->GetLocalRoleID();
            nLocalRoomID = pRunGame->GetLocalRoomID();
            strCfrModel = pRunGame->GetCfrModel();
            nPlayerID   = pRunGame->GetPlayerID();
        }
    }

NOTIFY_END:
    LOG(LT_INFO_TRANS, p->szTransID, "DO_APG_GAME_BEGIN_NOTIFY| robot_id=%lld| player_id=%lld| room_id=%s| local_room_id=%llu| role_id=%u| banker_id=%lld| "
        "hand_id=%u| player_id=%lld| player_score=%d| ai_mode=%d:%s| game_size=%u| time=%d| utime=%u| rc=%d| msg=%s",
        nRobotID, nPlayerID, pbReq.roomid().c_str(), nLocalRoomID, nLocalRoleID, pbReq.bankerid(),
        pbReq.hand(), pbReq.player_id(), pbReq.player_score(), pbReq.ai_mode(), strCfrModel.c_str(), CRunGameHashMap::Instance()->Size(),
        p->recv_time.ToNow(), utp.uToNow(), rc, szMsg.c_str()
       );

    CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);

}


void CAPGTask::GameSendCardsNotify(LISTSMG *const p)
{
    string szMsg;
    int rc = Pb::ERR_COMM_OTHER, nErrCode = 0;
    Pb::ThirdGameSendCardsNotify pbReq;
    CRunGame *pRunGame = NULL;
    int64 nRobotID     = p->nCliConnID;
    uint32 nHandID     = 0;
    uint32 nRound      = 0;
    string strOut;
    CTimePiece utp;
    int64 nPlayerID    = 0;

    if (!pbReq.ParseFromArray(p->cPacketBuffer + APGI_HEAD_LEN, p->len - APGI_HEAD_LEN))
    {
        szMsg   = "parse failed";
        rc      = Pb::ERR_COMM_PROTO_PARSE;
    }
    else if(0 == nRobotID)
    {
        szMsg   = "invalid robotid";
        rc      = Pb::ERR_COMM_INVALID_PARAM;
    }
    else
    {
        CRobotIDLock l(p->szTransID, nRobotID, g_oRobotIDLocks);
        pRunGame = CRunGameHashMap::Instance()->Get(p->szTransID, nRobotID);
        if(NULL != pRunGame)
        {
            nHandID     = pRunGame->GetHandID();
            nRound      = pRunGame->GetRound();
            nPlayerID   = pRunGame->GetPlayerID();
        }

        if(NULL == pRunGame)
        {
            szMsg = "Get runGame failed";
            rc    = Pb::ERR_COMM_INTERNAL;
        }
        else if(0 != (nErrCode = pRunGame->DoSaveCards(p->szTransID, pbReq, strOut)))
        {
            pRunGame->SetRunState(GRS_ERR_CARDS);
            szMsg = "Save cards failed(" + std::to_string(nErrCode) + ")";
            rc    = Pb::ERR_COMM_INTERNAL;
            pRunGame->DebugPrintf(p->szTransID, "NOTIFY_CARDS");
        }
        else
        {
            pRunGame->DebugPrintf(p->szTransID, "NOTIFY_CARDS");

            szMsg       = "Succ";
            rc          = Pb::OK;
            nHandID     = pRunGame->GetHandID();
        }
    }

    LOG(LT_INFO_TRANS, p->szTransID, "DO_APG_SEND_CARDS_NOTIFY| robot_id=%lld| player_id=%lld| hand_id=%u| round=%d:%d| room_id=%s| out=%s| time=%d| utime=%u| rc=%d| msg=%s",
        nRobotID, nPlayerID, nHandID, pbReq.round(), nRound, pbReq.roomid().c_str(), strOut.c_str(), p->recv_time.ToNow(), utp.uToNow(), rc, szMsg.c_str()
       );
    CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
}

//不需要...
/*
void CAPGTask::GameTokenNotify(LISTSMG * const p)
{
    string szMsg;
    int rc = Pb::ERR_COMM_OTHER, nErrCode = 0;
    Pb::ThirdGameTokenToNotify pbReq;
    CRunGame *pRunGame = NULL;
    uint64 nConnID     = 0;
    int64 nRobotID     = p->nCliConnID;
    uint32 nHandID     = 0;
    uint32 nRound      = 0;
    CTimePiece utp;

    if (!pbReq.ParseFromArray(p->cPacketBuffer + APGI_HEAD_LEN, p->len - APGI_HEAD_LEN))
    {
        szMsg   = "parse failed";
        rc      = Pb::ERR_COMM_PROTO_PARSE;
    }
    else if(0 == nRobotID)
    {
        szMsg   = "invalid robotid";
        rc      = Pb::ERR_COMM_INVALID_PARAM;
    }
    else
    {
        CRobotIDLock l(p->szTransID, nRobotID, g_oRobotIDLocks);
        pRunGame = CRunGameHashMap::Instance()->Get(p->szTransID, nRobotID);
        if(NULL != pRunGame) {
            nRound      = pRunGame->GetRound();
            nHandID     = pRunGame->GetHandID();
        }

        if(NULL == pRunGame)
        {
            szMsg = "Get runGame failed";
            rc    = Pb::ERR_COMM_INTERNAL;
        }
        else if(0 != (nErrCode = pRunGame->RequestActionFromCfr(p->szTransID)))
        {
            szMsg = "Query robot action failed(" + std::to_string(nErrCode) + ")";
            rc    = Pb::ERR_COMM_INTERNAL;
            pRunGame->DebugPrintf(p->szTransID, "NOTIFY_TOKEN");
        }
        else
        {
            pRunGame->SetAPGConnID(GToConnID(p->nUniqueID, p->connfd));
            pRunGame->DebugPrintf(p->szTransID, "NOTIFY_TOKEN");

            szMsg       = "Succ";
            rc          = Pb::OK;
            nConnID     = pRunGame->GetAPGConnID();
            nRound      = pRunGame->GetRound();
        }
    }

    LOG(LT_INFO_TRANS, p->szTransID, "DO_APG_TOKEN_NOTIFY| robot_id=%lld| hand_id=%u| round=%d| room_id=%s| conn_id=%llu| time=%d| utime=%u| rc=%d| msg=%s",
        nRobotID, nHandID, nRound, pbReq.roomid().c_str(), nConnID, p->recv_time.ToNow(), utp.uToNow(), rc, szMsg.c_str());
    CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
}
*/

void CAPGTask::GameActionNotify(LISTSMG *const p)
{
    string szMsg;
    int rc = Pb::ERR_COMM_OTHER, nErrCode = 0;
    Pb::ThirdGameActionNotify pbReq;
    CRunGame *pRunGame  = NULL;
    int64 nRobotID      = p->nCliConnID;
    uint64 nLocalRoomID = 0;
    uint32  nRound      = 0;
    uint32  nHandID     = 0;
    uint8   nFinished   = 0;
    int32   nBetSize    = 0;
    CTimePiece utp;

    if (!pbReq.ParseFromArray(p->cPacketBuffer + APGI_HEAD_LEN, p->len - APGI_HEAD_LEN))
    {
        szMsg   = "parse failed";
        rc      = Pb::ERR_COMM_PROTO_PARSE;
    }
    else if(0 == nRobotID)
    {
        szMsg   = "invalid robotid";
        rc      = Pb::ERR_COMM_INVALID_PARAM;
    }
    else
    {
        CRobotIDLock l(p->szTransID, nRobotID, g_oRobotIDLocks);
        pRunGame = CRunGameHashMap::Instance()->Get(p->szTransID, nRobotID);
        if(NULL != pRunGame)
        {
            nLocalRoomID = pRunGame->GetLocalRoomID();
            nRound      = pRunGame->GetRound();
            nHandID     = pRunGame->GetHandID();
            nFinished   = pRunGame->GetFinished();
        }

        if(NULL == pRunGame)
        {
            szMsg = "Get runGame failed";
            rc    = Pb::ERR_COMM_INTERNAL;
        }
        else if(nRobotID == pbReq.player_id())
        {
            szMsg   = "Succ(RobotAction)";
            rc      = Pb::OK;
        }
        else if(0 != (nErrCode = pRunGame->DoAction(p->szTransID, pbReq.player_id(), pbReq.actiontype(), pbReq.betnum())))
        {
            pRunGame->SetRunState(GRS_ERR_PLAYER_ACTION);
            szMsg = "Do action failed(" + std::to_string(nErrCode) + ")";
            rc    = Pb::ERR_COMM_INTERNAL;
            pRunGame->DebugPrintf(p->szTransID, "NOTIFY_ACTION");
        }
        else
        {
            pRunGame->DebugPrintf(p->szTransID, "NOTIFY_ACTION");
            nFinished   = pRunGame->GetFinished();
            nBetSize    = pRunGame->GetLastBetSize();
            szMsg       = "Succ";
            rc          = Pb::OK;

            //1. 进入下一轮则不请求[由发牌请求]
            //if(pRunGame->GetRound() == nRound)
            //{
            //    pRunGame->RequestActionFromCfr(p->szTransID);
            //}
        }
    }

    LOG(LT_INFO_TRANS, p->szTransID, "DO_APG_ACTION_NOTIFY| robot_id=%lld| player_id=%lld| hand_id=%u| round=%u| finished=%d| local_room_id=%llu| room_id=%s| action=%d:%d:%d| time=%d utime=%u| rc=%d| msg=%s",
        nRobotID, pbReq.player_id(), nHandID, nRound, nFinished, nLocalRoomID, pbReq.roomid().c_str(), pbReq.actiontype(), pbReq.betnum(), nBetSize, p->recv_time.ToNow(), utp.uToNow(), rc, szMsg.c_str()
       );

    CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
}


void CAPGTask::GameResultNotify(LISTSMG *const p)
{
    string szMsg;
    int rc = Pb::ERR_COMM_OTHER, nErrCode = 0;
    Pb::ThirdGameEndNotify pbReq;
    CRunGame *pRunGame = NULL;
    int64 nRobotID     = p->nCliConnID;
    int64 nPlayerID    = 0;
    uint32 nHandID     = 0;
    string strOut;
    CTimePiece utp;

    if (!pbReq.ParseFromArray(p->cPacketBuffer + APGI_HEAD_LEN, p->len - APGI_HEAD_LEN))
    {
        szMsg   = "parse failed";
        rc      = Pb::ERR_COMM_PROTO_PARSE;
    }
    else if(0 == nRobotID)
    {
        szMsg   = "invalid robotid";
        rc      = Pb::ERR_COMM_INVALID_PARAM;
    }
    else
    {
        CRobotIDLock l(p->szTransID, nRobotID, g_oRobotIDLocks);
        pRunGame = CRunGameHashMap::Instance()->Get(p->szTransID, nRobotID);
        if(NULL != pRunGame)
        {
            nHandID     = pRunGame->GetHandID();
            nPlayerID   = pRunGame->GetPlayerID();
        }

        if(NULL == pRunGame)
        {
            szMsg = "Get runGame failed";
            rc    = Pb::ERR_COMM_INTERNAL;
        }
        else if(0 != (nErrCode = pRunGame->DoFinished(p->szTransID, pbReq, strOut)))
        {
            szMsg = "Do finished failed(" + std::to_string(nErrCode) + ")";
            rc    = Pb::ERR_COMM_INTERNAL;
            pRunGame->DebugPrintf(p->szTransID, "NOTIFY_RESULT");
        }
        else
        {
            pRunGame->DebugPrintf(p->szTransID, "NOTIFY_RESULT");
            szMsg       = "Succ";
            rc          = Pb::OK;
        }
    }


    LOG(LT_INFO_TRANS, p->szTransID, "DO_APG_RESULT_NOTIFY| robot_id=%lld| player_id=%lld| room_id=%s| hand_id=%d:%d| robot_score=%d| out_msg=%s| time=%d| utime=%u| rc=%d| msg=%s",
        nRobotID, nPlayerID, pbReq.roomid().c_str(), pbReq.hand(), nHandID, pbReq.robotresult(), strOut.c_str(), p->recv_time.ToNow(), utp.uToNow(), rc, szMsg.c_str()
       );

    CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
}



void CAPGTask::GameRobotActionReq(LISTSMG *const p, const int &qIndex)
{
    string szMsg;
    int rc = Pb::ERR_COMM_OTHER, nErrCode = 0;
    Pb::ThirdRobotActionReq pbReq;
    Pb::ThirdRobotActionRsp pbRsp;

    CRunGame *pRunGame = NULL;
    int64 nRobotID     = p->nCliConnID;
    int64 nPlayerID    = 0;
    uint32 nHandID     = 0;
    uint32 nRound      = 0;
    uint8  nFinished   = 0;

    string strOut;
    Action oAction;
    CTimePiece utp;

    if (!pbReq.ParseFromArray(p->cPacketBuffer + APGI_HEAD_LEN, p->len - APGI_HEAD_LEN))
    {
        szMsg   = "parse failed";
        rc      = Pb::ERR_COMM_PROTO_PARSE;
    }
    else if(0 == nRobotID)
    {
        szMsg   = "invalid robotid";
        rc      = Pb::ERR_COMM_INVALID_PARAM;
    }
    else
    {
        CRobotIDLock l(p->szTransID, nRobotID, g_oRobotIDLocks);
        pRunGame = CRunGameHashMap::Instance()->Get(p->szTransID, nRobotID);
        if(NULL != pRunGame)
        {
            nHandID     = pRunGame->GetHandID();
            nRound      = pRunGame->GetRound();
            nPlayerID   = pRunGame->GetPlayerID();
        }

        if(NULL == pRunGame)
        {
            szMsg = "Get runGame failed";
            rc    = Pb::ERR_COMM_INTERNAL;
        }
        else if(0 != (nErrCode = pRunGame->DoQueryRobotAction(p->szTransID, pbReq, oAction, strOut)))
        {
            //等待机器人出牌
            /*uint32 nPTime = p->recv_time.ToNow();
            if(WAITING_ROBOT_ACTION == nErrCode && (0 == p->nDelayTimes || nPTime < p->nDelayTime))
            {
                if(0 == p->nDelayTimes)
                {
                    p->nDelayTime = nPTime + CAPGIConf::Instance()->m_nRobotDelayTime;   //等待配置时长
                    LOG(LT_WARN_TRANS, p->szTransID, "NEED_WAIT_ROBOT_ACTION| robot_id=%lld| hand_id=%u| round=%d| delay_time=%u:%u",
                        nRobotID, pRunGame->GetHandID(), pRunGame->GetRound(), nPTime, p->nDelayTime
                        );
                }

                p->nDelayTimes++;
                p->cPacketType = PKT_TYPE_DELAY_DONE;
                CQUEUE_List::Instance()->SetNode(p, QUEUE_TASK + qIndex);
                return ;
            }

            if(p->nDelayTimes > 0)
            {
                if(nPTime >= p->nDelayTime)
                {
                    nTDelayTime = nPTime - p->nDelayTime + 10;
                }
                else
                {
                    nTDelayTime = 10 - (p->nDelayTime - nPTime);
                }
            }*/

            szMsg = "done failed(" + std::to_string(nErrCode) + ")[" + strOut + "]";
            rc    = Pb::ERR_COMM_INTERNAL;
            pRunGame->DebugPrintf(p->szTransID, "QUERY_ACTION");
        }
        else
        {
            pRunGame->SetAPGConnID(p->connfd, p->nUniqueID);
            pRunGame->DebugPrintf(p->szTransID, "QUERY_ACTION");

            szMsg       = "Succ(to cfrsvr)";
            rc          = Pb::OK;
            nHandID     = pRunGame->GetHandID();
            nRound      = pRunGame->GetRound();
            nFinished   = pRunGame->GetFinished();
        }
    }

    LOG(LT_INFO_TRANS, p->szTransID, "DO_APG_QUERY_ROBOT_ACTION| robot_id=%lld| player_id=%llu| hand_id=%u| round=%d| finished=%d| room_id=%s| time=%d| sock=%d| unique_id=0x%x| utime=%u| rc=%d| msg=%s[%s]",
        nRobotID, nPlayerID, nHandID, nRound, nFinished, pbReq.roomid().c_str(), p->recv_time.ToNow(), p->connfd, p->nUniqueID, utp.uToNow(), rc, szMsg.c_str(), strOut.c_str()
       );

    if(Pb::OK == rc)
    {
        //pbRsp.set_resultcode(Pb::THIRD_SUCC);
        //pbRsp.set_actiontype(oAction.type);
        //pbRsp.set_betnum(oAction.size);
        //pbRsp.set_err_msg("Succ");
        //DoApgiComNetPacket(p, Pb::THIRD_CMD_ROBOT_ACTION_RSP, nRobotID, p->szTransID, Pb::THIRD_SUCC, &pbRsp);
        CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
    }
    else
    {
        pbRsp.set_err_msg(szMsg);
        pbRsp.set_actiontype(ActionType::a_fold);
        pbRsp.set_resultcode(Pb::THIRD_SUCC);
        DoApgiComNetPacket(p, Pb::THIRD_CMD_ROBOT_ACTION_RSP, nRobotID, p->szTransID, Pb::THIRD_SUCC, &pbRsp);
        CSocketManager::Instance()->Send(p);
    }
}


void CAPGTask::GameQueryStateReq(LISTSMG *const p)
{
    string szMsg;
    int rc = Pb::ERR_COMM_OTHER, nErrCode = 0;
    Pb::ThirdGameQueryStateReq pbReq;
    Pb::ThirdGameQueryStateRsp pbRsp;

    CRunGame *pRunGame = NULL;
    int64 nRobotID     = p->nCliConnID;
    uint32 nHandID     = 0;
    CTimePiece utp;

    if (!pbReq.ParseFromArray(p->cPacketBuffer + APGI_HEAD_LEN, p->len - APGI_HEAD_LEN))
    {
        szMsg   = "parse failed";
        rc      = Pb::ERR_COMM_PROTO_PARSE;
    }
    else if(0 == nRobotID)
    {
        szMsg   = "invalid robotid";
        rc      = Pb::ERR_COMM_INVALID_PARAM;
    }
    else
    {
        CRobotIDLock l(p->szTransID, nRobotID, g_oRobotIDLocks);
        if(NULL == (pRunGame = CRunGameHashMap::Instance()->Get(p->szTransID, nRobotID)))
        {
            szMsg = "Get runGame failed";
            rc    = Pb::ERR_COMM_INTERNAL;
        }
        else if(0 != (nErrCode = pRunGame->SerializeQueryState(p->szTransID, pbRsp)))
        {
            szMsg = "Serialize failed(" + std::to_string(nErrCode) + ")";
            rc    = Pb::ERR_COMM_INTERNAL;
            pRunGame->DebugPrintf(p->szTransID, "QUERY_STATE");
        }
        else
        {
            pRunGame->DebugPrintf(p->szTransID, "QUERY_STATE");
            szMsg       = "Succ";
            rc          = Pb::OK;
            nHandID     = pRunGame->GetHandID();
        }
    }

    LOG(LT_INFO_TRANS, p->szTransID, "DO_APG_QUERY_GAME_STATE| robot_id=%lld| hand_id=%u| round=%d| finished=%d| room_id=%s| next_turn_id=%lld| time=%d| utime=%u| rc=%d| msg=%s",
        nRobotID, nHandID, pbRsp.round(), pbRsp.finished(), pbReq.roomid().c_str(), pbRsp.next_turn_id(), p->recv_time.ToNow(), utp.uToNow(), rc, szMsg.c_str()
       );

    if(Pb::OK == rc)
    {
        DoApgiComNetPacket(p, Pb::THIRD_CMD_GAME_QUERY_STATE_RSP, nRobotID, p->szTransID, Pb::THIRD_SUCC, &pbRsp);
    }
    else
    {
        DoApgiComNetPacket(p, Pb::THIRD_CMD_GAME_QUERY_STATE_RSP, nRobotID, p->szTransID, Pb::THIRD_FAIL, &pbRsp);
    }

    CSocketManager::Instance()->Send(p);
}


void CAPGTask::UserStatDataReq(LISTSMG *const p)
{
    string szMsg;
    int rc = Pb::ERR_COMM_OTHER;
    Pb::ThirdGameStatDataReq pbReq;
    Pb::ThirdGameStatDataRsp pbRsp;


    if (!pbReq.ParseFromArray(p->cPacketBuffer + APGI_HEAD_LEN, p->len - APGI_HEAD_LEN))
    {
        szMsg   = "parse failed";
        rc      = Pb::ERR_COMM_PROTO_PARSE;
    }
    else if(0 == pbReq.uid())// || 0 == pbReq.start().length() || 0 == pbReq.end().length())
    {
        szMsg   = "invalid param";
        rc      = Pb::ERR_COMM_INVALID_PARAM;
    }
    else if(!g_oDataConnector.IsConnected(CAPGIConf::Instance()->m_vDataServer[0].nServerID))
    {
        szMsg   = "datasvr disConnect";
        rc      = Pb::ERR_COMM_INVALID_PARAM;
    }
    else
    {
        uint64 nT64     = p->nUniqueID;
        p->nCliConnID   = (nT64 << 32) + p->connfd;
        rc              = Pb::OK;
        szMsg           = "Succ(to datasvr)";
    }

    LOG(LT_INFO_TRANS, p->szTransID, "DO_APG_QUERY_USER_STATDATA| player_id=%lld| start=%s| end=%s| cli_conn_id=%llx(%x:%d)| time=%d| rc=%d| msg=%s",
        pbReq.uid(), pbReq.start().c_str(), pbReq.end().c_str(), p->nCliConnID, p->nUniqueID, p->connfd, p->recv_time.ToNow(), rc, szMsg.c_str()
       );

    if(Pb::OK == rc)
    {
        std::string sPTid(p->szTransID);
        g_oDataConnector.DoRequest(CAPGIConf::Instance()->m_vDataServer[0].nServerID, p, Pb::CMD_DATA_QUERY_USER_STATDATA, sPTid, p->nCliConnID, &pbReq, 0);
    }
    else
    {
        std::string sTid(p->szTransID);
        pbRsp.set_uid(pbReq.uid());
        pbRsp.set_resultcode(Pb::THIRD_FAIL);
        pbRsp.set_err_msg(szMsg);
        DoApgiComNetPacket(p, Pb::THIRD_CMD_GAME_STATDATA_RSP, pbReq.uid(), sTid.c_str(), Pb::THIRD_FAIL, &pbRsp);
        CSocketManager::Instance()->Send(p);
    }
}




