#ifndef _ERROR_CODE_H
#define _ERROR_CODE_H

const int ERR_OK                                        = 0;
const int ERR_INVALID_KEY                               = -1;

const int ERR_GAME_PARAM_ZERO_RANKS                     = -101;
const int ERR_GAME_PARAM_ZERO_SUITS                     = -102;
const int ERR_GAME_PARAM_INVALID_HOLE_CARD_NUM          = -103;
const int ERR_GAME_PARAM_NO_ENOUGH_FIRST_PLAYERS        = -104;
const int ERR_GAME_PARAM_NOT_SET_FLOP_CARDS             = -105;

const int ERR_BDTREE_NULL                               = -201;
const int ERR_BDTREE_INVALID_CARD                       = -202;

const int ERR_LOADING_CFR_NO_FILE                       = -301;
const int ERR_LOADING_CFR_NOT_FULL_PLAYER               = -302;
const int ERR_LOADING_CFR_NOT_FULL_STREET               = -303;
const int ERR_LOADING_CFR_NOT_THE_END                   = -304;


#endif