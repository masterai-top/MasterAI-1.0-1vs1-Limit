/**
* \file cfr_task.cpp
* \brief AI任务处理类实现函数
*/

#include "pch.h"
#include "cfr_task.h"
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


/**
* \brief 构造函数
*/
CCfrTask::CCfrTask(void)
{

}

/**
* \brief 析构函数
*/
CCfrTask::~CCfrTask(void)
{

}

/**
* \brief 初始化
* \return 初始化成功返回true，失败返回false
*/
bool CCfrTask::Init()
{
    return true;
}

/**
* \brief 关闭服务
*/
void CCfrTask::Restore()
{
}


void CCfrTask::DoProc(LISTSMG *const p)
{
    if(0 == strncmp(p->cPacketBuffer, "QUERY_ACTION", 12))
    {
        QueryActionResp(p);
    }
    else if(0 == strncmp(p->cPacketBuffer, "KEEP_ALIVE", 10))
    {
        KeepAliveResp(p);
    }
    else
    {
        LOG(LT_WARN_TRANS, p->szTransID,
            "DO_UNKONW_PACKET| cmd=%d| time=%d| buf=%s", p->nCmd, p->recv_time.ToNow(), p->cPacketBuffer
           );
        CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
    }
}

void CCfrTask::KeepAliveResp(LISTSMG *const p)
{
    int rc = Pb::ERR_COMM_OTHER;
    string szMsg;

    if(Pb::OK != p->nErrCode)
    {
        CSocketManager::Instance()->CloseConnect(p->connfd);
        szMsg = "response failed";
        rc = Pb::ERR_COMM_INTERNAL;
    }
    else if(!g_oCfrConnector.SetAliveTime(p->szTransID, p->nRemoteServerID, p->nUniqueID))
    {
        CSocketManager::Instance()->CloseConnect(p->connfd);
        szMsg = "set alive time failed";
        rc    = Pb::ERR_COMM_INTERNAL;
    }
    else
    {
        szMsg = "Succ";
        rc    = Pb::OK;
    }

    LOG(LT_INFO_TRANS, p->szTransID, "DO_CFR_KEEP_ALIVE_RESP| remote_server_id=0x%x| rc=%d| msg=%s",
        p->nRemoteServerID, rc, szMsg.c_str());

    CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
}


void CCfrTask::QueryActionResp(LISTSMG *const p)
{
    int rc = Pb::ERR_COMM_OTHER;
    string szMsg;
    int nErrCode = -99;
    int32 nLastBetNum = 0;

    ActionInfo oActionInfo;
    NodeInfo oNodeInfo;
    std::vector<unsigned int> vals;
    string strAction;
    string strRate;
    Action oAction;

    string sRobotParam;
    CTimePiece tp;
    bool bToAPG = true;

    Game *pGame           = NULL;
    CRunGame *pRunGame    = NULL;
    CFRServer *pCfrServer = NULL;
    uint8  nFinished      = 0;
    string strRoomID;

    uint64 nRobotID     = (uint64)(StrUtil::strtoi64(shstd::fun::GetValueBykey(p->cPacketBuffer, "RobotID", "#")));
    uint32 nHandID      = (uint32)(StrUtil::strtoi32(shstd::fun::GetValueBykey(p->cPacketBuffer, "HandID", "#")));
    uint32 nRound       = (uint32)(StrUtil::strtoi32(shstd::fun::GetValueBykey(p->cPacketBuffer, "Round", "#")));
    uint8  nActionSeq   = (uint8)(StrUtil::strtoi32(shstd::fun::GetValueBykey(p->cPacketBuffer, "ActionSeq", "#")));
    uint32 nLocalRoleID = (uint32)(StrUtil::strtoi64(shstd::fun::GetValueBykey(p->cPacketBuffer, "RoleID", "#")));
    uint64 nLocalRoomID = (uint64)(StrUtil::strtoi64(shstd::fun::GetValueBykey(p->cPacketBuffer, "RoomID", "#")));

    string strResponse  = shstd::fun::GetValueBykey(p->cPacketBuffer, "Resp", "#");
    string strActionInfo = shstd::fun::GetValueBykey(p->cPacketBuffer, "ActionInfo", "#");

    LogInfo oLogInfo(std::string(p->szTransID), (uint64)nLocalRoleID, nHandID);

    CRobotIDLock l(p->szTransID, nRobotID, g_oRobotIDLocks);
    pRunGame = CRunGameHashMap::Instance()->Get(p->szTransID, nRobotID);
    if(NULL != pRunGame)
    {
        nFinished = pRunGame->GetFinished();
        strRoomID = pRunGame->GetRoomID();
    }

    if(NULL == pRunGame)
    {
        szMsg  = "Get runGame failed";
        rc     = Pb::ERR_COMM_INTERNAL;
    }
    else if(nHandID != pRunGame->GetHandID())
    {
        bToAPG  = false;
        szMsg   = "invalid hand(" + std::to_string(pRunGame->GetHandID()) + ")";
        rc      = Pb::ERR_COMM_INTERNAL;
    }
    else if(nRound != pRunGame->GetRound())
    {
        bToAPG  = false;
        szMsg   = "invalid round(" + std::to_string(pRunGame->GetRound()) + ")";
        rc      = Pb::ERR_COMM_INTERNAL;
    }
    else if(GRS_WAIT_ROBOT_ACTION != pRunGame->GetRunState())
    {
        szMsg  = "Invalid state";
        rc     = Pb::ERR_COMM_INTERNAL;
    }
    else if(Pb::OK != p->nErrCode)
    {
        szMsg = "response failed(" + to_string(p->nErrCode) + ")";
        rc    = Pb::ERR_COMM_INTERNAL;
    }
    else if(0 == strResponse.length() || 0 == strActionInfo.length())
    {
        szMsg = "invalid param";
        rc    = Pb::ERR_COMM_INTERNAL;
    }
    else if(NULL == (pCfrServer = CAPGIConf::Instance()->GetCfrServer(pRunGame->GetCfrModel())))
    {
        szMsg = "Get cfr server failed";
        rc    = Pb::ERR_COMM_INTERNAL;
    }
    else if(0 != (nErrCode = oActionInfo.FromString(strActionInfo.c_str())))
    {
        szMsg = "Form ActionInfo failed(" + to_string(nErrCode) + ")";
        rc    = Pb::ERR_COMM_INTERNAL;
    }
    else if(NULL == (pGame = CGameManager::Instance()->GetGame(pRunGame->GetGameType())))
    {
        rc    = Pb::ERR_ACPC_GAME_TYPE;
        szMsg = "Get game failed";
    }
    else if(0 != (nErrCode = CBettingTree::Instance()->GetNode(p->szTransID, oActionInfo.action_seq, oNodeInfo)))
    {
        rc    = Pb::ERR_COMM_INTERNAL;
        szMsg  = "Get betting tree node failed(" + to_string(nErrCode) + ")";
    }
    else if(0 != (nErrCode = CfrSvrClient::ConvertResponseStrToVals(strResponse, oNodeInfo.GetChildNodeCount(), vals)))
    {
        rc    = Pb::ERR_COMM_INTERNAL;
        szMsg  = "Convert response failed(" + to_string(nErrCode) + ") =" + strResponse;
    }
    else if(0 != (nErrCode = DTRobot::DoAction(pRunGame->GetActionParam(), pCfrServer->oModelParam, oActionInfo, oNodeInfo, vals, oLogInfo, strAction, strRate)))
    {
        rc     = Pb::ERR_COMM_INTERNAL;
        szMsg  = "Get action failed(" + to_string(nErrCode) + ")";
    }
    else if(0 == strAction.length())
    {
        rc    = Pb::ERR_ACPC_GAME_TYPE;
        szMsg = "Action invalid";
    }
    else if(readAction(strAction.c_str(), pGame, &oAction) < 0)
    {
        rc    = Pb::ERR_ACPC_GAME_TYPE;
        szMsg = "To action failed";
    }
    else if(nActionSeq != pRunGame->GetActionSeq())
    {
        rc     = Pb::ERR_COMM_INTERNAL;
        szMsg  = "invalid actionSeq(internal:" + to_string(pRunGame->GetActionSeq()) + ")";
        bToAPG = false;
    }
    else if(0 != (nErrCode = pRunGame->DoAction(p->szTransID, nRobotID, oAction.type, oAction.size)))
    {
        rc     = Pb::ERR_COMM_INTERNAL;
        szMsg  = "Do action failed(" + to_string(nErrCode) + ")";
    }
    else
    {
        sRobotParam = "[ver:" + pRunGame->GetCfrModel() + "][param:" + pCfrServer->oModelParam.cfr_param + "] ";
        szMsg       = "Succ";
        rc          = Pb::OK;

        nFinished   = pRunGame->GetFinished();
        nLastBetNum = pRunGame->GetLastBetNum();
        pRunGame->DebugPrintf(p->szTransID, "ROBOT_ACTION");
    }

    LOG(LT_INFO_TRANS, p->szTransID,
        "DO_APG_ACTION_ROBOT_FROM_CFR| robot_id=%lld| room_id=%s| local_room_id=%llu| role_id=%u| finished=%d| hand_id=%u| round=%d| action=%d:%d:%d| time=%u(%u)| rc=%d| "
        "msg=%s| rate=%s| robot_param=%s| action_info=%s",
        nRobotID, strRoomID.c_str(), nLocalRoomID, nLocalRoleID, nFinished, nHandID, nRound, oAction.type, nLastBetNum, oAction.size, p->recv_time.ToNow(), tp.uToNow(), rc,
        szMsg.c_str(), strRate.c_str(), sRobotParam.c_str(), oActionInfo.ToString().c_str()
       );

    if(bToAPG && NULL != pRunGame && GRS_WAIT_ROBOT_ACTION == pRunGame->GetRunState())
    {
        pRunGame->SetRunState(GRS_OK);
        Pb::ThirdRobotActionRsp pbRsp;
        pbRsp.set_resultcode(Pb::THIRD_SUCC);
        pbRsp.set_err_msg("Succ");
        if(Pb::OK == rc)
        {
            pbRsp.set_actiontype(oAction.type);
            pbRsp.set_betnum(nLastBetNum);
        }
        else
        {
            pbRsp.set_actiontype(ActionType::a_fold);
        }
        p->connfd    = pRunGame->GetConnSock();
        p->nUniqueID = pRunGame->GetConnUniqueID();
        DoApgiComNetPacket(p, Pb::THIRD_CMD_ROBOT_ACTION_RSP, nRobotID, p->szTransID, Pb::THIRD_SUCC, &pbRsp);
        CSocketManager::Instance()->Send(p);
    }
    else
    {
        CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
    }

}


