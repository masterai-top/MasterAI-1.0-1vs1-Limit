#pragma once

#include<vector>

namespace nndef
{
    namespace nnuser
    {
        typedef short cid_t;
        typedef long guid_t;
    };

    namespace nncard
    {
        typedef short card_t;
        typedef std::vector<card_t> vecc_t;

        //花色类型 不同的牌值用个位标识
        enum E_NN_CARD
        {
            NN_CARD_DIAMOND = 0x0000, //方
            NN_CARD_CLUB    = 0x0010, //梅
            NN_CARD_HEART   = 0x0020, //红
            NN_CARD_SPADE   = 0x0030, //黑
            NN_CARD_KING    = 0x0040, //大小王
        };

        //获取花色
        card_t getNNType(card_t card);

        //获取计算牛值
        card_t getNNValue(card_t card);

        //获取数值
        card_t getNNNum(card_t card);
    };

    //牌型
    enum E_NN_TYPE
    {
        NN_TYPE_NN_N0     = 0x0000, //高牌
        NN_TYPE_NN_1P,              //一对
        NN_TYPE_NN_2P,              //两对
        NN_TYPE_NN_BOMB3,           //三条
        NN_TYPE_NN_STRAIGHT,        //顺子
        NN_TYPE_NN_SAMEKIND,        //同花
        NN_TYPE_NN_HULU,            //葫芦
        NN_TYPE_NN_BOMB4,           //四条
        NN_TYPE_NN_FLUSH,           //同花顺
        NN_TYPE_NN_KING,            //皇家同花顺
    };

    //下注类型
    enum E_NN_ACT
    {
        NN_ACT_UNKNOWN   = 0x0001,  //未知
        NN_ACT_FOLD     = 0x0010,   //弃牌
        NN_ACT_PASS     = 0x0020,   //过
        NN_ACT_FOLLOW   = 0x0040,   //跟注
        NN_ACT_RAISE    = 0x0080,   //加注
        NN_ACT_ALLIN    = 0x0100,   //全下
        NN_ACT_SMALL_BLIND = 0x0200, //小盲
        NN_ACT_BIG_BLIND   = 0x0400, //大盲
    };

    //牌型
    enum E_NN_GUESS
    {
        NN_GUESS_1A = 0x0001,
        NN_GUESS_1P,            //对子
        NN_GUESS_AA,            //对A
    };

    enum E_NN_STATE
    {
        NN_STATE_GAME_BEGIN     = 0x0001,
        NN_STATE_FRONT_BET      = 0x0002,
        NN_STATE_GAME_BANKER    = 0x0003,
        NN_STATE_HD_CARD        = 0x0004,
        NN_STATE_COMMON_CARD    = 0x0005,
        NN_STATE_TURN_CARD      = 0x0006,
        NN_STATE_RIVER_CARD     = 0x0007,
        NN_STATE_GAME_END       = 0x0008,
    };

    enum E_NN_XTIME
    {
        NN_XTIME_GAME_BEGIN = 100,
        NN_XTIME_CHOOSE_BANKER,
        NN_XTIME_SEND_CARD4,
        NN_XTIME_CHOOSE_BET,
        NN_XTIME_SEND_CARD5,
        NN_XTIME_OPEN_CARD,
        NN_XTIME_GAME_END,
        NN_XTIME_WAIT_SPACE,
        NN_XTIME_WAIT_GAME,
        NN_XTIME_KILL_ALL,
    };

    enum E_NN_USER_STATE
    {
        NN_USER_STATE_NORMAL = 1,
        NN_USER_STATE_WIN,
        NN_USER_STATE_LOSE,
    };

    namespace nninvalid
    {
        using namespace nnuser;
        using namespace nncard;

        extern guid_t       nil_uid;
        extern cid_t        nil_cid;
        extern card_t       nil_card;

        extern E_NN_ACT     nil_act;
        extern E_NN_TYPE    nil_nntype;
        extern E_NN_STATE   nil_nnstate;
        extern E_NN_GUESS   nil_guess;
    };
};

