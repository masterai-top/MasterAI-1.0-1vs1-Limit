#include "run_game.h"
#include "shstd.h"
#include "apgi_conf.h"
#include "param_handle.h"
#include "game_manager.h"
#include "matchstate_convertor.h"
#include "cfr_svr_client.h"
#include "dt_robot.h"
#include "json/json.h"
#include <atomic>

#define REDIS_HISTORY_CNT (100000)
const string g_strHistoryKey = "lstGameHistory";

std::atomic_int         g_nLocalID(rand());

CRunGame::CRunGame(void)
{
    Reset();
}

CRunGame::~CRunGame(void)
{
    Reset();
}

void CRunGame::Reset()
{
    m_nRobotID      = 0;
    m_nGameType     = 0;
    m_nPlayerID     = 0;
    m_nPlayerScore  = 0;
    
    m_nCfrServerID  = 0;
    m_nSock         = -1;
    m_nUniqueID     = 0;
    m_nGameType     = 0;
    m_nLastTime     = 0;

    memset(&m_oMatchState, 0, sizeof(m_oMatchState));
    memset(m_szRoomID, 0, sizeof(m_szRoomID));
    memset(m_szCfrModel, 0, sizeof(m_szCfrModel));
}

void CRunGame::SetPlayerID(int64 v) 
{ 
    if(v != m_nPlayerID) 
    {   
        uint32 nLocalID = ++g_nLocalID;
        
        m_nPlayerID     = v; 
        m_nRobotScore   = 0;

        m_nLocalRoleID  = nLocalID;
        m_nLocalRoomID  = (time(NULL) << 32) + nLocalID;
    }
}

int64 CRunGame::GetBankerID() const
{
    if(1 == m_oMatchState.viewingPlayer) {
        return m_nRobotID;
    }
    else {
        return m_nPlayerID;
    }
}

int64 CRunGame::GetCurrentPlayerID(const char *szTransID) const 
{
    if(stateFinished(&m_oMatchState.state))
    {
        return 0;
    }

    Game *pGame = CGameManager::Instance()->GetGame(m_nGameType);
    if (NULL == pGame)
    {
        LOG(LT_ERROR_TRANS, szTransID, "Get game failed| game_type=%d| robot_id=%lld", m_nGameType, m_nRobotID);
        return 0;
    }
    
    //获取当前玩家
    uint8 player = currentPlayer(pGame, &m_oMatchState.state);
    if(player >= pGame->numPlayers) {
        LOG(LT_ERROR_TRANS, szTransID, "Get current player failed| player=%d| robot_id=%lld", player, m_nRobotID);
        return 0;
    }

    if(player == m_oMatchState.viewingPlayer) {
        return  m_nRobotID;
    }
    else {
        return m_nPlayerID;
    }
}

int CRunGame::GetAllowRaises(const char *szTransID, std::vector<int32> &vRaises)
{
    vRaises.clear();
    
    ActionInfo action_info;
    int flag = MatchStateConverotr::GetActionInfo(m_oMatchState, action_info);
    if (flag != 0)
    {
        LOG(LT_ERROR_TRANS, szTransID, "Get allow raises| Get action failed err=%d", flag);
        return -101;
    }

    NodeInfo node_info;
    flag = CBettingTree::Instance()->GetNode(szTransID, action_info.action_seq, node_info);
    if (flag != 0)
    {
        LOG(LT_ERROR_TRANS, szTransID, "Get allow raises| Get node failed err=%d", flag);
        return -102;
    }

    char  szTemp[256] = {0};
    int32 len = 0;
    std::vector<std::string> vAllows = node_info.GetChildNode();
    for(uint32 i = 0; i < vAllows.size(); i++)
    {
        if(vAllows[i].length() > 1 && vAllows[i].at(0) == 'r') 
        {
            std::string v = vAllows[i].substr(1);
            int32       n = std::stoi(v);
            vRaises.push_back(n);

            if(strlen(szTemp) < 200) {
                len += snprintf(szTemp + len, 32, "%s[%d] ", vAllows[i].c_str(), n);
            }
        }
    }

    LOG(LT_DEBUG_TRANS, szTransID, "Get allow raises| allows=%s| size=%d| src_size=%d", szTemp, vRaises.size(), vAllows.size());
    
    return 0;
}


int CRunGame::CheckAction(const char *szTransID, const Game *pGame, Action *pAction)
{
    if(NULL == pGame || NULL == pAction) {
        return -1;
    }
    
    if(a_call == pAction->type) {
        return 0;
    }

    if(a_fold == pAction->type) {
        return 0;
    }
    
    if(a_raise != pAction->type) {
        return -2;
    }

    
    if(!isValidAction(pGame, &m_oMatchState.state, 0, pAction))
    {
        return -12;
    }

    std::vector<int32> vRaises;
    if(0 != GetAllowRaises(szTransID, vRaises)) {
        return -13;
    }

    char  szTemp[256] = {0};
    int   len = 0;
    for(uint32 i = 0; i < vRaises.size(); i++)
    {
        if(vRaises[i] == pAction->size) 
        {
            return 0;
        }

        if(strlen(szTemp) < 200) {
            len += snprintf(szTemp + len, 32, "r%d ", vRaises[i]);
        }
    }

    LOG(LT_ERROR_TRANS, szTransID, "Check action| invalid| action=%d:%d| allows=%s", pAction->type, pAction->size, szTemp);
    
    return -99;
}

void CRunGame::DebugPrintf(const char *szTransID, const std::string &strSymbol)
{
    if(CAPGIConf::Instance()->m_nLimitedLogEnable) {
        return;
    }
    
    char sTemp[1024] = {0};
    Game *pGame = CGameManager::Instance()->GetGame(m_nGameType);
    if (NULL != pGame)
    {
        printStateDetailed(pGame, &m_oMatchState.state, sizeof(sTemp) - 1, sTemp);
    }
    
    LOG(LT_INFO_TRANS, szTransID, 
        "Game_message| %s| robot_id=%lld| robot_position=%d| player_id=%lld| player_score=%u| room_id=%s| game_type=%d| banker_id=%lld| hand_id=%u| round=%d| finished=%d| "
        "cfr=0x%x(%s)| game_begin_times=%u| current_player=%lld| robot_result=%d| already_save_history=%d| last_action=%d:%d:%d| matchState=%s",
        strSymbol.c_str(), m_nRobotID, m_oMatchState.viewingPlayer, m_nPlayerID, m_nPlayerScore, m_szRoomID, m_nGameType, GetBankerID(), GetHandID(), GetRound(), m_oMatchState.state.finished,
        m_nCfrServerID, m_szCfrModel, m_nGameBeginTimes, GetCurrentPlayerID(szTransID), m_nRobotResult, m_nAlreadySaveHistory, m_oDLastAction.nType, m_oDLastAction.nBetNum, m_oDLastAction.nBetSize, sTemp
        );
}

int CRunGame::SetCfrModel(const char *szTransID, int32 nAiMode) 
{
    bool bAgain = true;

    std::string strModel = CAPGIConf::Instance()->SelectCfrModel(nAiMode, m_nGameBeginTimes);

INVALID_AGIAN:    
    if(0 == strcmp(strModel.c_str(), m_szCfrModel) && m_nCfrServerID > 0) 
    {
        return 0;  
    }

    if(strModel.length() > (sizeof(m_szCfrModel) - 4))
    {
        LOG(LT_ERROR_TRANS, szTransID, "Check cfr model lenth failed| len=%d", strModel.length());
        return -100;
    }

    CFRServer *pCfr = CAPGIConf::Instance()->GetCfrServer(strModel);
    if(NULL == pCfr) 
    {
        LOG(LT_ERROR_TRANS, szTransID, "Get cfr failed| model=%s", strModel.c_str());
        return -101;
    }

    if(!g_oCfrConnector.IsConnected(pCfr->nServerID))
    {
        LOG(LT_ERROR_TRANS, szTransID, "Cfr not connected| model=%s| server_id=0x%x", strModel.c_str(), pCfr->nServerID);
        if(bAgain) 
        {
            //连接异常使用其他算法重试一次
            bAgain = false;
            strModel = CAPGIConf::Instance()->SelectValidModel(strModel);
            goto INVALID_AGIAN;
        }
        
        return -105;
    }

    
    int nErrCode = ParamHandle::SetActionParam(pCfr->oModelParam.cfr_param, m_oActionParam);
    if(0 != nErrCode)
    {
        LOG(LT_ERROR_TRANS, szTransID, "Player Set action param failed| rc=%d| model=%s| param=%s", nErrCode, strModel.c_str(), pCfr->oModelParam.cfr_param.c_str());
        return -102;
    }

    
    memset(m_szCfrModel, 0, sizeof(m_szCfrModel));
    snprintf(m_szCfrModel, sizeof(m_szCfrModel) - 1, "%s", strModel.c_str());
    SetCfrServerID(pCfr->nServerID);
        
    std::string sOutParam;
    ParamHandle::GetActionParamStr(GetActionParam(), sOutParam);
    LOG(LT_INFO_TRANS, szTransID, "RUN_GAME_SET_CFR_MODEL| model=%s| cfr_server_id=0x%x| action_param=%s(%s)", 
        m_szCfrModel, m_nCfrServerID, pCfr->oModelParam.cfr_param.c_str(), sOutParam.c_str()
        );
    
    return 0;
}

int CRunGame::DoRunMatch(const char * szTransID, uint32 nHandID, int64 nBankerID)
{
    SetGameType(GAME_TYPE_NOLIMIT_2P);
    Game *pGame = CGameManager::Instance()->GetGame(m_nGameType);
    if (NULL == pGame)
    {
        return -1;
    }

    if(!stateFinished(&m_oMatchState.state))
    {
        LOG(LT_WARN_TRANS, szTransID, "RUN_MATCH| but running| robot_id=%llu| room_id=%s| hand_id=%d| player_id=%llu", 
            m_nRobotID, m_szRoomID, GetHandID(), m_nPlayerID
            );
    }

    memset(&m_oMatchState, 0, sizeof(m_oMatchState));
    if(nBankerID == GetRobotID()) {
        m_oMatchState.viewingPlayer = 1;
    }
    else {
        m_oMatchState.viewingPlayer = 0;
    }
    
    m_nActionSeq    = 0;
    m_nRunState     = GRS_OK;
    m_nRobotResult  = 0;
    m_nGameTime     = time(NULL);

    m_nGameBeginTimes++;
    m_nAlreadySaveHistory       = 0;
    m_oMatchState.state.handId  = nHandID;
    initState(pGame,  m_oMatchState.state.handId, &m_oMatchState.state );

    return 0;
}


int CRunGame::DoSaveCards(const char *szTransID, const Pb::ThirdGameSendCardsNotify &pbCards, std::string &strOut)
{
    char sTemp[256] = {0};
    if(pbCards.round() != GetRound()) {
        snprintf(sTemp, sizeof(sTemp) - 1, "invalid_round=%d:%d", pbCards.round(), GetRound());
        strOut = sTemp;
        return -101;
    }

    //发牌置0
    m_nTRobotRoundSeq = 0;
    
    if(0 == pbCards.round() && 2 == pbCards.robot_cards_size()) 
    {
        for(int j = 0; j < 2; j++)
        {
            if(0 != ApgCardToLocal(pbCards.robot_cards(j), m_oMatchState.state.holeCards[m_oMatchState.viewingPlayer][j])) 
            {
                snprintf(sTemp, sizeof(sTemp) - 1, "invalid_robot_card=%d:%d", j, pbCards.robot_cards(j));
                strOut = sTemp;
                return -102;
            }
        }
    }
    else if(1 == pbCards.round() && 3 == pbCards.board_cards_size()) 
    {
        for(int j = 0; j < 3; j++)
        {
            if(0 != ApgCardToLocal(pbCards.board_cards(j), m_oMatchState.state.boardCards[j])) 
            {
                snprintf(sTemp, sizeof(sTemp) - 1, "invalid_flop_card=%d:%d", j, pbCards.board_cards(j));
                strOut = sTemp;
                return -103;
            }
        }
    }
    else if(2 == pbCards.round() && 1 == pbCards.board_cards_size()) 
    {
        if(0 != ApgCardToLocal(pbCards.board_cards(0), m_oMatchState.state.boardCards[3])) 
        {
            snprintf(sTemp, sizeof(sTemp) - 1, "invalid_turn_card=%d", pbCards.board_cards(0));
            strOut = sTemp;
            return -104;
        }
    }
    else if(3 == pbCards.round() && 1 == pbCards.board_cards_size()) 
    {
        if(0 != ApgCardToLocal(pbCards.board_cards(0), m_oMatchState.state.boardCards[4])) 
        {
            snprintf(sTemp, sizeof(sTemp) - 1, "invalid_river_card=%d", pbCards.board_cards(0));
            strOut = sTemp;
            return -105;
        }
    }
    else 
    {        
        snprintf(sTemp, sizeof(sTemp) - 1, "invalid_card_cnt=%d:%d:%d", pbCards.round(), pbCards.robot_cards_size(), pbCards.board_cards_size());
        strOut = sTemp;
        return -110;    
    }

    Game *pGame = CGameManager::Instance()->GetGame(m_nGameType);
    if (NULL == pGame)
    {
        strOut = "invalid game type(" + std::to_string(m_nGameType) + ")";
        return -1;
    }

    printStateDetailed(pGame, &m_oMatchState.state, sizeof(sTemp) - 1, sTemp);
    strOut = sTemp;
    
    return 0;
}

int CRunGame::DoAction(const char *szTransID, int64 nActionUid, int32 nActionType, int32 nBetNum)
{   
    m_oDLastAction.nType    = nActionType;
    m_oDLastAction.nBetNum  = 0;
    m_oDLastAction.nBetSize = 0;
    if(a_raise == nActionType) 
    {   
        //机器人是加注了, 玩家时加注到
        if(nActionUid == m_nRobotID) 
        {
            m_oDLastAction.nBetNum  = nBetNum - m_oMatchState.state.spent[m_oMatchState.viewingPlayer];
            m_oDLastAction.nBetSize = nBetNum;
        }
        else
        {
            m_oDLastAction.nBetNum  = nBetNum;
            m_oDLastAction.nBetSize = nBetNum + m_oMatchState.state.spent[(m_oMatchState.viewingPlayer + 1)%2];
        }
    }

    Action oAction;
    oAction.type = (ActionType)(nActionType);
    oAction.size = m_oDLastAction.nBetSize;
     
    Game *pGame = CGameManager::Instance()->GetGame(m_nGameType);
    if (NULL == pGame)
    {
        return -1000;
    }
    
    if(GetCurrentPlayerID(szTransID) != nActionUid) {
        return -1001;
    }

    if(0 != CheckAction(szTransID, pGame, &oAction))
    {
        return -1002;
    }

    doAction(pGame, &oAction, &m_oMatchState.state);    
    m_nActionSeq++;
            
    return 0;
}

int CRunGame::DoFinished(const char *szTransID, const Pb::ThirdGameEndNotify &pbEnd, std::string &strOut)
{
    if(1 == m_nAlreadySaveHistory) {
        strOut = "Already save history";
        return 0;
    }
    
    if(!stateFinished(&m_oMatchState.state))
    {
        strOut = "game not finished";
        return -99;
    }
    
    Game *pGame = CGameManager::Instance()->GetGame(m_nGameType);
    if (NULL == pGame)
    {
        strOut = "invalid game type(" + std::to_string(m_nGameType) + ")";
        return -98;
    }

    char sTemp[256] = {0};
    uint8 nPlayer = (m_oMatchState.viewingPlayer + 1)%2;
    for(int j = 0; j < pbEnd.player_cards_size() && j < 2; j++)
    {
        if(0 != ApgCardToLocal(pbEnd.player_cards(j), m_oMatchState.state.holeCards[nPlayer][j])) 
        {
            snprintf(sTemp, sizeof(sTemp) - 1, "invalid_player_card=%d:%d", j, pbEnd.player_cards(j));
            strOut = sTemp;
            return -102;
        }
    }

    for(int j = 0; j < 5 && j < pbEnd.board_cards_size(); j++)
    {
        if(0 != ApgCardToLocal(pbEnd.board_cards(j), m_oMatchState.state.boardCards[j])) 
        {
            snprintf(sTemp, sizeof(sTemp) - 1, "invalid_board_card=%d:%d", j, pbEnd.board_cards(j));
            strOut = sTemp;
            return -103;
        }
    }

    printStateDetailed(pGame, &m_oMatchState.state, sizeof(sTemp) - 1, sTemp);
    strOut = sTemp;

    //计算机器人得分
    m_nRobotResult  = valueOfState(pGame, &m_oMatchState.state, m_oMatchState.viewingPlayer);
    m_nRobotScore  += m_nRobotResult;
    m_nPlayerScore -= m_nRobotResult; 

    if(m_nRobotResult != pbEnd.robotresult()) 
    {
        LOG(LT_ERROR_TRANS, szTransID, "DO_FINISHED_FAILED| robot_id=%llu| hand_id=%u| robot_result=%d| notify_result=%d", m_nRobotID, GetHandID(), m_nRobotResult, pbEnd.robotresult());
        strOut = "invalid_result";
    }
    
    //保存历史记录
    if(!SaveHistory(szTransID, pGame))
    {
        strOut = "to_history_failed";
        return -112;
    }

    m_nAlreadySaveHistory = 1;
    return 0;
}


int CRunGame::DoQueryRobotAction(const char *szTransID, const Pb::ThirdRobotActionReq &pbReq, Action &oAction, std::string &strOut)
{
    if(GetCurrentPlayerID(szTransID) != m_nRobotID) 
    {
        std::vector<int32> vRaises;
        if(0 != GetAllowRaises(szTransID, vRaises)) {
            return -1900;
        }
    
        char  szTemp[256] = {0};
        int   len = 0;
        for(uint32 i = 0; i < vRaises.size(); i++)
        {        
            if(strlen(szTemp) < 200) {
                len += snprintf(szTemp + len, 32, "r%d ", vRaises[i]);
            }
        }

        strOut = "Player_allow_raise=" + std::string(szTemp);

        return -1901;
    }

    //请求机器人出牌
    int nRet = RequestActionFromCfr(szTransID);
    if(0 != nRet) 
    {
        return (-20000 + nRet);
    }

    m_nRequestActionTime = time(NULL);
    SetRunState(GRS_WAIT_ROBOT_ACTION);
    return 0;

    /* 以下是旧版本的实现
    if(GRS_OK != m_nRunState) 
    {
        if(GetCurrentPlayerID(szTransID) == m_nPlayerID) 
        {
            std::vector<int32> vRaises;
            if(0 != GetAllowRaises(szTransID, vRaises)) {
                return -1900;
            }
        
            char  szTemp[256] = {0};
            int   len = 0;
            for(uint32 i = 0; i < vRaises.size(); i++)
            {        
                if(strlen(szTemp) < 200) {
                    len += snprintf(szTemp + len, 32, "r%d ", vRaises[i]);
                }
            }

            strOut = "Player_allow_raise=" + std::string(szTemp);
        }
        else {
            strOut = "invalid runState(" + std::to_string(m_nRunState) + ")";
        }
        
        return -1901;
    }
    
    //1.轮数相同
    //2.请求轮数+1=当前轮 [机器人的action导致进入下一轮的情况]
    //3.游戏结束 [机器人的action导致finished的情况]
    if(pbReq.round() == GetRound() || (pbReq.round() + 1) == GetRound() || stateFinished(&m_oMatchState.state))
    {   
        ;
    }
    else 
    {
        DebugPrintf(szTransID, "QUERY_ROBOT_ACTION_FAILED1");
        strOut = "invalid round";
        return -99;
    }

    uint8   nLastPlayer = 0xFF;
    Game *pGame = CGameManager::Instance()->GetGame(m_nGameType);
    if (NULL == pGame)
    {
        strOut = "invalid game type(" + std::to_string(m_nGameType) + ")";
        return -98;
    }

    int nErrCode = GetRoundLastAction(pGame, &m_oMatchState.state, pbReq.round(), &nLastPlayer, &oAction);
    if(ROUND_NOT_ACTION == nErrCode)
    {
        ; //当前轮还没有动作
    }
    else if(0 != nErrCode) 
    {
        strOut = "Get last action failed(" + std::to_string(nErrCode) + ")";
        return -97;
    }
    else 
    {   
        //最后一个动作时机器人
        if(nLastPlayer == m_oMatchState.viewingPlayer)
        {
            if(a_raise == oAction.type && (oAction.size != m_oDLastAction.nBetSize || m_oDLastAction.nBetNum < 0))
            {
                strOut = "Invalid bet param";
                DebugPrintf(szTransID, "QUERY_ROBOT_ACTION_FAILED3");
                return -96;
            }

            oAction.size = m_oDLastAction.nBetNum;
            return 0;
        }
    }

    //执行至这里的条件: 1.当前轮还没有动作 2.最后不是机器人的动作 
    //此时如果轮到机器人出牌,表示机器人还没出牌
    if(GetCurrentPlayerID(szTransID) == m_nRobotID)
    {
        return WAITING_ROBOT_ACTION;
    }

    DebugPrintf(szTransID, "QUERY_ROBOT_ACTION_FAILED2");
    strOut = "Data failed";
    
    return -91;*/
}

//向cfrsvr请求机器人动作
int CRunGame::RequestActionFromCfr(const char *szTransID)
{
    if(GRS_OK != m_nRunState) 
    {
        LOG(LT_ERROR_TRANS, szTransID, "REQUEST_ROBOT_ACTION| invalid runState=%d", m_nRunState);
        return -1000;
    }

    //不是机器人出牌
    if(GetCurrentPlayerID(szTransID) != m_nRobotID)
    {   
        LOG(LT_ERROR_TRANS, szTransID, "REQUEST_ROBOT_ACTION| robot turn failed");
        return -1001;
    }

    if(!g_oCfrConnector.IsConnected(GetCfrServerID()))
    {
        LOG(LT_ERROR_TRANS, szTransID, "REQUEST_ROBOT_ACTION| cfr connect  failed");
        return -1002;
    }
    
    /*if(GetRobotID() == (int64)CAPGIConf::Instance()->m_oTRobotAction.nRobotID) 
    {
        uint8 nRound = GetRound();
        Action oa;
        int nnRet = -999;
        if(m_nTRobotRoundSeq < MAX_ROBOT_ACTION_CNT) 
        {
            if(0 == GetRound()) {
                oa.size = CAPGIConf::Instance()->m_oTRobotAction.oAction0[m_nTRobotRoundSeq].size;
                oa.type = CAPGIConf::Instance()->m_oTRobotAction.oAction0[m_nTRobotRoundSeq].type;
            }
            else if(1 == GetRound()) {
                oa.size = CAPGIConf::Instance()->m_oTRobotAction.oAction1[m_nTRobotRoundSeq].size;
                oa.type = CAPGIConf::Instance()->m_oTRobotAction.oAction1[m_nTRobotRoundSeq].type;
            }
            else if(2 == GetRound()) {
                oa.size = CAPGIConf::Instance()->m_oTRobotAction.oAction2[m_nTRobotRoundSeq].size;
                oa.type = CAPGIConf::Instance()->m_oTRobotAction.oAction2[m_nTRobotRoundSeq].type;
            }
            else {
                oa.size = CAPGIConf::Instance()->m_oTRobotAction.oAction3[m_nTRobotRoundSeq].size;
                oa.type = CAPGIConf::Instance()->m_oTRobotAction.oAction3[m_nTRobotRoundSeq].type;
            }

            //成功与否都要+1
            m_nTRobotRoundSeq++;
            nnRet = DoAction(szTransID, GetRobotID(),  oa.type, oa.size);
            if(0 == nnRet)
            {
                LOG(LT_INFO_TRANS,szTransID, "DO_APG_ACTION_ROBOT_TEST| succ| robot_id=%lld| hand_id=%u| round=%u| finished=%d| local_room_id=%llu| room_id=%s| player_id=%lld| action=%d:%d:%d| TRobot_action_seq=%d", 
                    GetRobotID(), GetHandID(), nRound, GetFinished(), m_nLocalRoomID, m_szRoomID, m_nPlayerID, oa.type, m_oDLastAction.nBetNum, oa.size, m_nTRobotRoundSeq
                );

                //以后完善:需要发送响应
                return 0;
            }
        }
        
        //失败则按算法正常流程处理
        LOG(LT_WARN_TRANS, szTransID, "DO_APG_ACTION_ROBOT_TEST| failed| robot_id=%lld| hand_id=%u| round=%u| finished=%d| local_room_id=%llu| room_id=%s| player_id=%lld| action=%d:%d:%d| TRobot_action_seq=%d| rc=%d", 
            GetRobotID(), GetHandID(), nRound, GetFinished(), m_nLocalRoomID, m_szRoomID, m_nPlayerID, oa.type, m_oDLastAction.nBetNum, oa.size, m_nTRobotRoundSeq, nnRet
            );
    }*/
    
    Game *pGame = CGameManager::Instance()->GetGame(m_nGameType);
    if (NULL == pGame)
    {
        LOG(LT_ERROR_TRANS, szTransID, "REQUEST_ROBOT_ACTION| Get game failed| game_type=%d", m_nGameType);
        return -1002;
    }

    char szAcpcStr[512] = {0};
    int nLen = 500;
    int len = printMatchState(pGame, &m_oMatchState, nLen, szAcpcStr);
    if (len < 0)
    {
        LOG(LT_ERROR_TRANS, szTransID, "REQUEST_ROBOT_ACTION| To acpc failed| rc=%d", len);
        return -3;
    }
    
    //根据前端字符串及从CFR读取查询获取参数
    ActionInfo action_info;
    int flag = MatchStateConverotr::GetActionInfo(m_oMatchState, action_info);
    if (flag != 0)
    {
        LOG(LT_ERROR_TRANS, szTransID, "REQUEST_ROBOT_ACTION| Get Action Info Failed err=%d",flag);
        return -101;
    }

    NodeInfo node_info;
    flag = CBettingTree::Instance()->GetNode(szTransID, action_info.action_seq, node_info);
    if (flag != 0)
    {
        LOG(LT_ERROR_TRANS, szTransID, "REQUEST_ROBOT_ACTION| Get Node Info Failed err=%d",flag);
        return -102;
    }

    CFRServer *pCfrServer = CAPGIConf::Instance()->GetCfrServer(m_szCfrModel);
    if(NULL == pCfrServer) {
        LOG(LT_ERROR_TRANS, szTransID, "REQUEST_ROBOT_ACTION| Get Cfr server Failed model=%s", m_szCfrModel);
        return -112;
    }

    CFRKey cfr_key;
    flag = DTRobot::GetKey(action_info, pCfrServer->oModelParam, node_info, cfr_key);
    if (flag != 0)
    {
        LOG(LT_ERROR_TRANS, szTransID, "REQUEST_ROBOT_ACTION| Get CFR KEY Failed err=%d", flag);
        return -103;
    }

    std::string strCfrRequest;
    flag = CfrSvrClient::ConvertKeyToRequestStr(cfr_key, strCfrRequest, pCfrServer->GetVersion());
    if (flag != 0)
    {
        LOG(LT_ERROR_TRANS, szTransID, "REQUEST_ROBOT_ACTION| ConvertKeyToRequestStr err=%d", flag);
        return -104;
    }
   
    strCfrRequest.append("#ActionInfo=");
    strCfrRequest.append(action_info.ToString().c_str());

    int num = 0;
    LISTSMG *p = CQUEUE_List::Instance()->GetNoBlockNode(QUEUE_FREE, &num);
    if(NULL == p) {
        LOG(LT_ERROR_TRANS, szTransID, "REQUEST_ROBOT_ACTION| Get node failed| robot_id=%lld", m_nRobotID);
        return -105;
    }

    char szBuf[1024];
    std::string sTid(szTransID);
    snprintf(szBuf, sizeof(szBuf) - 1, "QUERY_ACTION#TransID=%s#KEY=%s#RoleID=%u#RoomID=%llu#RobotID=%lld#HandID=%u#Round=%u#ActionSeq=%d#\r\n", 
        sTid.c_str(), strCfrRequest.c_str(), m_nLocalRoleID, m_nLocalRoomID, GetRobotID(), GetHandID(), GetRound(), m_nActionSeq
        );
    g_oCfrConnector.DoRequest(GetCfrServerID(), p, sTid, 0, szBuf, GetRobotID());
         
    return 0;
}


int CRunGame::SerializeQueryState(const char *szTransID, Pb::ThirdGameQueryStateRsp &pbRsp)
{
    if(stateFinished(&m_oMatchState.state)) 
    {
        pbRsp.set_finished(1);
        return 0;
    }
    
    std::vector<int32> vRaises;
    if(0 != GetAllowRaises(szTransID, vRaises))
    {
        return -901;
    }

    int64 nNextTurnID = GetCurrentPlayerID(szTransID);
    if(nNextTurnID <= 0) {
        return -902;
    }
    
    pbRsp.set_finished(0);
    pbRsp.set_next_turn_id(nNextTurnID);
    pbRsp.set_round(GetRound());
    for(uint32 i = 0; i < vRaises.size(); i++) {
        pbRsp.add_allow_raise(vRaises[i]);
    }
    
    return 0;
}


bool CRunGame::SaveHistory(const char *szTransID, const Game *pGame)
{
    if(1 == CAPGIConf::Instance()->m_nUnsaveHistory) {
        return true;
    }
    
    if (NULL == pGame)
    {
        return false;
    }

    Json::Value oJGame;
    Json::Value oJPlayers;
    Json::Value oJRoundActions;
    Json::Value oJHoleCards;
    Json::Value oJBoardCards;

    //玩家信息 
    {
        Json::Value oJPlayer;
        oJPlayer["role_id"]     = std::to_string(m_nPlayerID);
        oJPlayer["score"]       = -1*m_nRobotResult;
        oJPlayer["position"]    = (m_oMatchState.viewingPlayer+1)%2;
        oJPlayer["money"]       = m_nPlayerScore;
        oJPlayer["is_robot"]    = 0;
        oJPlayer["role_name"]   = "Player_" + std::to_string(m_nPlayerID);
        oJPlayers.append(oJPlayer);
    }

    //机器人信息
    {
        Json::Value oJPlayer;
        oJPlayer["role_id"]     = std::to_string(m_nRobotID);  //不能用nLocalRoleID, 可能与m_nPlayerID相同
        oJPlayer["score"]       = m_nRobotResult;
        oJPlayer["position"]    = m_oMatchState.viewingPlayer;
        oJPlayer["money"]       = m_nRobotScore;
        oJPlayer["is_robot"]    = 1;
        oJPlayer["role_name"]   = "Robot_" + std::to_string(m_nRobotID);
        oJPlayers.append(oJPlayer);
    }
        
    //Action信息
    for (int i=0; i<=m_oMatchState.state.round; ++i)
    {   
        if(0 == m_oMatchState.state.numActions[i])
        {
            break;
        }
        
        Json::Value oJRound;
        Json::Value oJActions;
        for (int j=0; j < m_oMatchState.state.numActions[i]; ++j)
        {
            Json::Value oJAction;
            oJAction["position"] = m_oMatchState.state.actingPlayer[i][j];
			oJAction["action"]   = (int)(m_oMatchState.state.action[i][j].type);
			oJAction["size"]     = m_oMatchState.state.action[i][j].size;
			oJActions.append(oJAction);
        }

        oJRound["round_id"]  = i;
        oJRound["actions"]   = oJActions;
        oJRoundActions.append(oJRound);
    }

    //私牌
    for(int i=0; i<pGame->numPlayers; ++i)
    {
        Json::Value oJHoleCard;
        Json::Value oJCards;
        for (int j=0; j<pGame->numHoleCards; ++j)
        {
            unsigned char nCard = m_oMatchState.state.holeCards[i][j];
            int nRank = rankOfCard(nCard);
            int nSuit = suitOfCard(nCard);
            
            Json::Value oJCard;
            oJCard["suit"] = nSuit;
            oJCard["rank"] = nRank;

            oJCards.append(oJCard);
        }
        oJHoleCard["cards"] = oJCards;
        oJHoleCards.append(oJHoleCard);
    }    

    //公牌, 存所有公牌
    int nBoardCardCount = sumBoardCards(pGame, pGame->numRounds);
    for (int i=0; i<nBoardCardCount; ++i)
    {
        unsigned char nCard = m_oMatchState.state.boardCards[i];
        int nRank = rankOfCard(nCard);
        int nSuit = suitOfCard(nCard);

        Json::Value oJCard;
        oJCard["suit"] = nSuit;
        oJCard["rank"] = nRank;

        oJBoardCards.append(oJCard);
    }

    oJGame["room_id"]        = std::to_string(m_nLocalRoomID);
    oJGame["room_name"]      = m_szRoomID;
    oJGame["game_type"]      = m_nGameType;
    oJGame["player_numbers"] = pGame->numPlayers;
    oJGame["hand_id"]        = m_oMatchState.state.handId;
    oJGame["players"]        = oJPlayers;
    oJGame["round_actions"]  = oJRoundActions;
    oJGame["board_cards"]    = oJBoardCards;
    oJGame["hole_cards"]     = oJHoleCards;
    oJGame["game_time"]      = (uint32)(time(NULL) - m_nGameTime);
    oJGame["trans_id"]       = szTransID;
    oJGame["cfr_model"]      = m_szCfrModel;
    
    std::string sJson = oJGame.toStyledString();

    
    if(m_nHistoryCnt > REDIS_HISTORY_CNT) 
    {
        m_nHistoryCnt = g_oHistoryRedis.LLen(szTransID, g_strHistoryKey);
    }

    if(m_nHistoryCnt > REDIS_HISTORY_CNT) 
    {
        LOG(LT_ERROR_TRANS, szTransID, 
            "History to redis limited| robot_id=%llu| player_id=%llu| hand_id=%u| history_cnt=%d| {%s}", 
            m_nRobotID, m_nPlayerID, GetHandID(), m_nHistoryCnt, sJson.c_str()
            );
        return false;
    }

    m_nHistoryCnt = g_oHistoryRedis.LPush(szTransID, g_strHistoryKey, sJson);
    if(m_nHistoryCnt <= 0) 
    {
        LOG(LT_ERROR_TRANS, szTransID, 
            "History to redis failed| robot_id=%llu| player_id=%llu| hand_id=%u| history_cnt=%d| {%s}", 
            m_nRobotID, m_nPlayerID, GetHandID(), m_nHistoryCnt, sJson.c_str()
            );
    }
    
    return true;
}


