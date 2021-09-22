/**
* \file brain_task.cpp
* \brief AI任务处理类实现函数
*/
#include "GameCfg.h"

GameCfg::GameCfg()
{
    Reset();
}

GameCfg& GameCfg::operator=(const GameCfg &ths)
{
    m_strBettingCtrl = ths.m_strBettingCtrl;
    memcpy(&m_oGame, &ths.m_oGame, sizeof(Game));
    
    return *this;
}

bool GameCfg::operator==(const GameCfg &ths) const
{
    //相同
    if(m_strBettingCtrl == ths.m_strBettingCtrl && 0 == memcmp(&m_oGame, &ths.m_oGame, sizeof(Game)))
    {
        return true;
    }
    
    return false;
}

void GameCfg::Reset()
{
    memset(&m_oGame, 0, sizeof(Game));
    m_strBettingCtrl.clear();
    m_strFileName.clear();
    m_strFilePath.clear();
}

//从protobuf协议的内容转换
int GameCfg::FromProtoBuf(const Pb::QueryGameConfig &pbGameCfg)
{
    Reset();
    
    if(1 != pbGameCfg.game_config_size())
    {
        return -1;
    }
    
    const Pb::GameDef &pbDef = pbGameCfg.game_config(0);
    if((int)pbDef.numplayers() != pbDef.stack_size())
    {
        return -2;
    }

    if((int)pbDef.numplayers() != pbDef.blind_size())
    {
        return -3;
    }

    if((int)pbDef.numrounds() != pbDef.firstplayer_size())
    {
        return -4;
    }

    if((int)pbDef.numrounds() != pbDef.numboardcards_size())
    {
        return -5;
    }

    if((int)pbDef.numrounds() != pbDef.raisesize_size())
    {
        return -6;
    }

    if((int)pbDef.numrounds() != pbDef.maxraises_size())
    {
        return -7;
    }

    m_strBettingCtrl         = pbGameCfg.betting_control();
    
    m_oGame.bettingType      = (BettingType)pbDef.betting_type();
    m_oGame.numPlayers       = (uint8_t)pbDef.numplayers();
    m_oGame.numRounds        = (uint8_t)pbDef.numrounds();
    m_oGame.numSuits         = (uint8_t)pbDef.numsuits();
    m_oGame.numRanks         = (uint8_t)pbDef.numranks();
    m_oGame.numHoleCards     = (uint8_t)pbDef.numholecards();

    for (int i = 0; i < pbDef.blind_size(); ++i)
    {
        m_oGame.blind[i] = pbDef.blind(i);
    }

    for (int i = 0; i < pbDef.stack_size(); ++i)
    {
        m_oGame.stack[i] = pbDef.stack(i);
    }

    for (int i = 0; i < pbDef.firstplayer_size(); ++i)
    {
        m_oGame.firstPlayer[i] = pbDef.firstplayer(i);
    }

    for (int i = 0; i < pbDef.numboardcards_size(); ++i)
    {
        m_oGame.numBoardCards[i] = pbDef.numboardcards(i);
    }

    for (int i = 0; i < pbDef.raisesize_size(); ++i)
    {
        m_oGame.raiseSize[i] = pbDef.raisesize(i);
    }

    for (int i = 0; i < pbDef.maxraises_size(); ++i)
    {
        m_oGame.maxRaises[i] = pbDef.maxraises(i);
    }

    return 0;    
}

//内容转换至protobuf协议
void GameCfg::ToProtoBuf(uint8 nMatchEnable, uint32 nMatchHands, Pb::QueryGameConfig &pbGameCfg)
{
    pbGameCfg.set_betting_control(m_strBettingCtrl.c_str());
    Pb::GameDef *pbDef = pbGameCfg.add_game_config();    
    pbDef->set_betting_type(m_oGame.bettingType);
    pbDef->set_numplayers(m_oGame.numPlayers);
    pbDef->set_numrounds(m_oGame.numRounds);
    pbDef->set_numsuits(m_oGame.numSuits);
    pbDef->set_numranks(m_oGame.numRanks);
    pbDef->set_numholecards(m_oGame.numHoleCards);

    for(int i = 0; i < m_oGame.numPlayers; i++)
    {
        pbDef->add_stack(m_oGame.stack[i]);
        pbDef->add_blind(m_oGame.blind[i]);
    }

    for(int i = 0; i < m_oGame.numRounds; i++)
    {
        pbDef->add_firstplayer(m_oGame.firstPlayer[i]);
        pbDef->add_numboardcards(m_oGame.numBoardCards[i]);
        pbDef->add_raisesize(m_oGame.raiseSize[i]);
        pbDef->add_maxraises(m_oGame.maxRaises[i]);
    }

    Pb::MatchDef *pbMatch = pbGameCfg.mutable_match_config();
    pbMatch->set_match_enable(nMatchEnable);
    pbMatch->set_match_hands(nMatchHands);
}


string GameCfg::OutPrintf() 
{
    char szBuf[1024] = {0};

    string sStack = "";
    string sBlind = "";
    for(int i = 0; i < m_oGame.numPlayers; i++)
    {   
        if(i > 0) 
        {
            sStack.append(" ");
            sBlind.append(" ");
        }
        sStack.append(to_string(m_oGame.stack[i]));
        sBlind.append(to_string(m_oGame.blind[i]));
    }
    
    string sFirstPlayer     = "";
    string sNumBoardCards   = "";
    for(int i = 0; i < m_oGame.numRounds; i++)
    {   
        if(i > 0) 
        {
            sFirstPlayer.append(" ");
            sNumBoardCards.append(" ");
        }
        sFirstPlayer.append(to_string(m_oGame.firstPlayer[i]));
        sNumBoardCards.append(to_string(m_oGame.numBoardCards[i]));
    }
    
    snprintf(szBuf, sizeof(szBuf) - 1, 
        "filename=%s| filepath=%s| betting_ctrl=%s, bettingType=%d, numPlayers=%d, numRounds=%d, stack=%s, blind=%s, firstPlayer=%s, numSuits=%d, numRanks=%d, numHoleCards=%d, numBoardCards=%s", 
        m_strFileName.c_str(), m_strFilePath.c_str(), m_strBettingCtrl.c_str(), m_oGame.bettingType, m_oGame.numPlayers, m_oGame.numRounds, sStack.c_str(), sBlind.c_str(), sFirstPlayer.c_str(), m_oGame.numSuits, m_oGame.numRanks, m_oGame.numHoleCards, sNumBoardCards.c_str());
    
    return string(szBuf);
}

string GameCfg::OutPrintf() const
{
    char szBuf[512] = {0};

    string sStack = "";
    string sBlind = "";
    for(int i = 0; i < m_oGame.numPlayers; i++)
    {   
        if(i > 0) 
        {
            sStack.append(" ");
            sBlind.append(" ");
        }
        sStack.append(to_string(m_oGame.stack[i]));
        sBlind.append(to_string(m_oGame.blind[i]));
    }
    
    string sFirstPlayer     = "";
    string sNumBoardCards   = "";
    for(int i = 0; i < m_oGame.numRounds; i++)
    {   
        if(i > 0) 
        {
            sFirstPlayer.append(" ");
            sNumBoardCards.append(" ");
        }
        sFirstPlayer.append(to_string(m_oGame.firstPlayer[i]));
        sNumBoardCards.append(to_string(m_oGame.numBoardCards[i]));
    }
    
    snprintf(szBuf, sizeof(szBuf) - 1, 
        "betting_ctrl=%s, bettingType=%d, numPlayers=%d, numRounds=%d, stack=%s, blind=%s, firstPlayer=%s, numSuits=%d, numRanks=%d, numHoleCards=%d, numBoardCards=%s", 
        m_strBettingCtrl.c_str(), m_oGame.bettingType, m_oGame.numPlayers, m_oGame.numRounds, sStack.c_str(), sBlind.c_str(), sFirstPlayer.c_str(), m_oGame.numSuits, m_oGame.numRanks, m_oGame.numHoleCards, sNumBoardCards.c_str());
    
    return string(szBuf);
}



