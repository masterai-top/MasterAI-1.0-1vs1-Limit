/**
* \file game_cfg.cpp
* \brief 游戏配置类实现函数
*/

#include "pch.h"
#include "game_manager.h"
#include "define.h"
#include "comm_define.h"

typedef struct _GAME_CONFIG
{
    uint32      m_nGameType;
    char        m_szFileName[256];
} GAME_CONFIG;

static GAME_CONFIG g_szFileName[GAME_TYPE_MAX] =
{
    { GAME_TYPE_LEDUC, "../data/leduc.game" },
    { GAME_TYPE_LIMIT_2P, "../data/holdem.limit.2p.reverse_blinds.game" },
    { GAME_TYPE_NOLIMIT_2P, "../data/holdem.nolimit.2p.reverse_blinds.game" },
};

/**
* \brief 构造函数
*/
CGameManager::CGameManager(void)
{

}

/**
* \brief 析构函数
*/
CGameManager::~CGameManager(void)
{
    std::map<uint32, Game *>::iterator it = m_mapGame.begin();
    for ( ; it != m_mapGame.end(); ++it)
    {
        Game *pGame = it->second;
        if (NULL != pGame)
        {
            delete pGame;
        }
    }
    m_mapGame.clear();
}

/**
* \brief 加载配置
*/
bool CGameManager::LoadConfig()
{
    for(uint32 i = GAME_TYPE_LEDUC; i < GAME_TYPE_MAX; ++i)
    {
        FILE *pFile = fopen( g_szFileName[i].m_szFileName, "r" );
        if (NULL == pFile)
        {
            LOG(LT_ERROR, "ERROR: could not open game file %s", g_szFileName[i].m_szFileName );
            return false;
        }

        Game *pGame = readGame( pFile );
        if (NULL == pGame)
        {
            LOG(LT_ERROR, "ERROR: read game file %s", g_szFileName[i].m_szFileName );
            return false;
        }

        m_mapGame[g_szFileName[i].m_nGameType] = pGame;
    }

    return true;
}

/**
* \brief 获得游戏配置
* \param 游戏类型
*/
Game *CGameManager::GetGame(uint32 nGameType)
{
    std::map<uint32, Game *>::iterator it = m_mapGame.find(nGameType);
    if (it == m_mapGame.end())
    {
        return NULL;
    }

    return it->second;
}




