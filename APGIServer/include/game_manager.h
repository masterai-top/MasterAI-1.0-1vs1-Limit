/**
* \file game_manager.h
* \brief 游戏配置
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include "typedef.h"
#include <map>
extern "C" {
    #include "game.h"
};


class CGameManager
{
private:
    /**
    * \brief 构造函数
    */
    CGameManager(void);

public:    
    static CGameManager* Instance()
    {
        static CGameManager m_instance;
        return &m_instance;
    }

    /**
    * \brief 析构函数
    */
    ~CGameManager(void);

    /**
    * \brief 加载配置
    */
    bool LoadConfig();

    /**
    * \brief 获得游戏配置
    * \param 游戏类型
    */
    Game * GetGame(uint32 nGameType);

private:
    std::map<uint32, Game *>        m_mapGame;  ///< 游戏配置表
};

#endif // __GAME_MANAGER_H__

