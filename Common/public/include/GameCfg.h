/**
* \file game_cfg.h
* \brief 
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __DGAME_CFG__
#define __DGAME_CFG__
#include "pch.h"
#include "typedef.h"
#include <string>
#include "comm.pb.h"
using namespace std;

extern "C" {
    #include "game.h"
};

//所有的游戏配置(Game对象 + 算法的特定配置)
class GameCfg
{
public:
    GameCfg();
    GameCfg& operator=(const GameCfg &ths);
    bool operator==(const GameCfg &ths) const;

    void Reset();    
    int  FromProtoBuf(const Pb::QueryGameConfig &pbGameCfg);
    void ToProtoBuf(uint8 nMatchEnable, uint32 nMatchHands, Pb::QueryGameConfig &pbGameCfg);

    string  OutPrintf();
    string  OutPrintf() const;
    
    
public:
    Game    m_oGame;              //游戏对象
    string  m_strBettingCtrl;     //算法betting控制串        

    string  m_strFileName;
    string  m_strFilePath;
};


#endif 


