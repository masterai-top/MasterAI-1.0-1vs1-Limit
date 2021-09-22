/**
* \file define.cpp
* \brief 定义全局变量&函数
*/
#include "define.h"
extern "C" {
#include "game.h"
};

CServerConnector        g_oDataConnector;       //数据统计分析服务连接器
CServerConnector        g_oCfrConnector;        //cfr服务连接器
CIDLocks                g_oRobotIDLocks;        //机器人ID全局锁
CRedisConnector         g_oHistoryRedis;        //历史记录的redis服务


int ApgCardToLocal(const nndef::nncard::card_t &card, uint8 &nLocalCard)
{
    /*本地牌的定义. 见Game.c
    *   static char suitChars[ MAX_SUITS + 1 ] = "cdhs";            //值:0-3  花色: c:club(梅花)(0) d:diamond(方块)(1) h:heart(红桃)(2) s:spade(黑桃)(3)
    *   static char rankChars[ MAX_RANKS + 1 ] = "23456789TJQKA";   //值:0-12 (牌:2-A)
    */
    nndef::nncard::card_t apgsuit = nndef::nncard::getNNType(card);
    nndef::nncard::card_t apgrank = nndef::nncard::getNNNum(card);  //值范围: 2-14

    uint8 nSuit = 0;
    uint8 nRank = 0;
    if(nndef::nncard::NN_CARD_DIAMOND == apgsuit)      //= 0x0000, //方
    {
        nSuit = 1;
    }
    else if(nndef::nncard::NN_CARD_CLUB  == apgsuit)   //= 0x0010, //梅
    {
        nSuit = 0;
    }
    else if(nndef::nncard::NN_CARD_HEART == apgsuit)   //= 0x0020, //红
    {
        nSuit = 2;
    }
    else if(nndef::nncard::NN_CARD_SPADE == apgsuit)   //= 0x0030, //黑
    {
        nSuit = 3;
    }
    else
    {
        return -1;
    }

    if(apgrank < 2 || apgrank > 14)
    {
        return -2;
    }

    nRank = apgrank - 2;
    nLocalCard = makeCard(nRank, nSuit);

    return 0;
}

