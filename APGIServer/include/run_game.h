//
#ifndef __RUN_GAME_H__
#define __RUN_GAME_H__

#include "typedef.h"
#include <string>
#include "Def.h"
#include "string"
#include "Third.pb.h"


extern "C" {
    #include "game.h"
};

enum GAME_RUN_STATE {
    GRS_OK                      = 0,    //正常状态
    GRS_WAIT_ROBOT_ACTION       = 1,    //等待机器人出牌
    GRS_ERR_CARDS               = 11,   //牌处理错误
    GRS_ERR_PLAYER_ACTION       = 12,   //玩家动作错误
    GRS_ERR_ROBOT_ACTION        = 13,   //机器人动作错误
};

#define WAITING_ROBOT_ACTION   (1001)   //等待机器人动作中...

struct DLastAction
{
    uint8   nType;          // 最后一个动作的类型
    int32   nBetNum;        // 最后一次加注的 “加注了” 值
    int32   nBetSize;       // 最后一次加注的 “加注到” 值
};

class CRunGame
{
public:
    CRunGame(void);
    ~CRunGame(void);
    void Reset();

    int64   GetRobotID() const { return m_nRobotID; }
    void    SetRobotID(int64 v) { m_nRobotID = v; }

    int64   GetPlayerID() const { return m_nPlayerID; }
    void    SetPlayerID(int64 v);

    uint8   GetRunState() const { return m_nRunState; }
    void    SetRunState(GAME_RUN_STATE v) { m_nRunState = v; } 
    
    uint32  GetPlayerScore() const { return m_nPlayerScore; }
    void    SetPlayerScore(uint32 v) { m_nPlayerScore = v; }
    
    uint8   GetGameType() const { return    m_nGameType; }
    void    SetGameType(uint8 nGameType) { m_nGameType = nGameType; }

    void    SetAPGConnID(const int32 &nSock, const uint32 &nUniqueID) { m_nSock = nSock; m_nUniqueID = nUniqueID; }
    int32   GetConnSock() const { return m_nSock; }
    uint32  GetConnUniqueID() const { return m_nUniqueID; }
    time_t  GetRequestActionTime() const { return m_nRequestActionTime; }
    
    time_t  GetLastTime() const { return     m_nLastTime; }
    void    SetLastTime(time_t nTime) { m_nLastTime = nTime; }

    std::string GetRoomID() const { return std::string(m_szRoomID); }
    void        SetRoomID(const char *v) { memset(m_szRoomID, 0, sizeof(m_szRoomID)); snprintf(m_szRoomID, sizeof(m_szRoomID) - 1, "%s", v); }

    int64   GetBankerID() const;
    int64   GetCurrentPlayerID(const char *szTransID) const ;
    uint32  GetRound() const {    return m_oMatchState.state.round; }
    uint32  GetHandID() const {    return m_oMatchState.state.handId; }
    uint8   GetFinished() const { return m_oMatchState.state.finished; }
    
    uint8   GetActionSeq() const { return m_nActionSeq; }
    
    uint32  GetLocalRoleID() const { return m_nLocalRoleID; } 
    uint64  GetLocalRoomID() const { return m_nLocalRoomID; } 
    int32   GetLastBetNum() const { return m_oDLastAction.nBetNum; }
    int32   GetLastBetSize() const { return m_oDLastAction.nBetSize; }
    uint8   GetTRobotRoundSeq() const { return m_nTRobotRoundSeq; }
    
    //cfrsvr的相关配置
    int         SetCfrModel(const char *szTransID, int32 nAiMode);
    std::string GetCfrModel() const { return std::string(m_szCfrModel); }
    uint32      GetCfrServerID() const { return m_nCfrServerID; }
    const ActionParam& GetActionParam() { return m_oActionParam; }
    
    void        DebugPrintf(const char *szTransID, const std::string &strSymbol);
    MatchState* GetMatchState()     { return &m_oMatchState; }
    
    //业务处理
    int     DoRunMatch(const char * szTransID, uint32 nHandID, int64 nBankerID);
    int     DoSaveCards(const char *szTransID, const Pb::ThirdGameSendCardsNotify &pbCards, std::string &strOut);
    int     DoAction(const char *szTransID, int64 nActionUid, int32 nActionType, int32 nBetNum);
    int     DoFinished(const char *szTransID, const Pb::ThirdGameEndNotify &pbEnd, std::string &strOut);
    int     DoQueryRobotAction(const char *szTransID, const Pb::ThirdRobotActionReq &pbReq, Action &oAction, std::string &strOut);

    int     RequestActionFromCfr(const char *szTransID);
    int     GetAllowRaises(const char *szTransID, std::vector<int32> &vRaises);
    int     CheckAction(const char *szTransID, const Game *pGame, Action *pAction);
    int     SerializeQueryState(const char *szTransID, Pb::ThirdGameQueryStateRsp &pbRsp);
    bool    SaveHistory(const char *szTransID, const Game *pGame);
    
private:
    void    SetCfrServerID(uint32 nServerID) { m_nCfrServerID = nServerID; }

private:
    //需要写入到redis的参数
    int64           m_nRobotID;                 // 机器人ID
    uint8           m_nGameType;                // 游戏类型
    char            m_szRoomID[32];             // 房间ID
    int64           m_nPlayerID;                // 玩家ID
    int32           m_nPlayerScore;             // 玩家总积分
    uint8           m_nActionSeq;               // 执行action的序列
    GAME_RUN_STATE  m_nRunState;                // 执行状态
    
    int32           m_nSock;                    // APG客户端的连接fd
    uint32          m_nUniqueID;                // APG客户端的连接唯一ID
    time_t          m_nLastTime;                // 最后操作时间    
    time_t          m_nRequestActionTime;       // 请求机器人决策的时间
    uint32          m_nLocalRoleID;             // 本地的RoleID[算法相关的日志使用] 
    uint64          m_nLocalRoomID;             // 本地的RoomID[算法相关的日志使用] 
    
    //cfrsvr的相关配置
    uint32          m_nCfrServerID;             // cfr服务ID
    char            m_szCfrModel[64];           // cfr算法模型
    ActionParam     m_oActionParam;             // 玩家动作配置
    
    MatchState      m_oMatchState;              // 当前的游戏状态 

    //临时变量
    int32           m_nRobotResult;             // 机器人单局得分
    int32           m_nRobotScore;              // 机器人积分
    time_t          m_nGameTime;                // 游戏开始时间
    int32           m_nHistoryCnt;              // 历史记录数
    uint8           m_nTRobotRoundSeq;          // 测试参数: 
    uint8           m_nAlreadySaveHistory;      // 已经保存了历史记录标记
    uint32          m_nGameBeginTimes;          // 开局次数
    
    DLastAction     m_oDLastAction;             //最后一个动作的信息
};


#endif // __BRAIN_PLAYER_H__


