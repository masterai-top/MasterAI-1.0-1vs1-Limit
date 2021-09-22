
#ifndef _ERROR_CODE_H_
#define _ERROR_CODE_H_


const int ERR_OK = 0;

const int ERR_CONFIG_INVALID_GAME                   = -1;
const int ERR_CONFIG_INVALID_BETTING_PARAMS         = -2;

const int ERR_COMM_INVALID_MSG                      = -101;
const int ERR_COMM_CONNECT_FAILED                   = -102;
const int ERR_COMM_SEND_VERSION_FAILED              = -103;
const int ERR_COMM_SEND_ACTION_FAILED               = -104;
const int ERR_COMM_SEND_HEARTBEAT_FAILED            = -105;

const int ERR_BDTREE_NULL                           = -301;
const int ERR_BDTREE_INVALID_CARD                   = -302;

const int ERR_BETTINGTREE_MISMATCH_STREET           = -401;
const int ERR_BETTINGTREE_MISMATCH_PLAYER           = -402;
const int ERR_BETTINGTREE_INVALID_NODE              = -403;

const int ERR_GAME_PARAM_ZERO_RANKS                 = -501;
const int ERR_GAME_PARAM_ZERO_SUITS                 = -502;
const int ERR_GAME_PARAM_INVALID_HOLE_CARD_NUM      = -503;
const int ERR_GAME_PARAM_NO_ENOUGH_FIRST_PLAYERS    = -504;
const int ERR_GAME_PARAM_NOT_SET_FLOP_CARDS         = -505;

const int ERR_INVALID_ACTIONSEQ                     = -601;

const int ERR_LOADING_CONFIG_FAILED                 = -701;


const int ERR_TOO_MANY_CARDS                        = -901;
const int ERR_INVALID_CARD                          = -902;
const int ERR_REPEAT_CARDS                          = -903;
const int ERR_WRONG_CARD_STR                        = -904;
const int ERR_WRONG_CARD_CNT                        = -905;
const int ERR_WRONG_VAL_CNT                         = -906;
const int ERR_INVALID_VAL                           = -907;
const int ERR_WRONG_PARAM_CNT                       = -908;
const int ERR_INVALID_PARAM_VAL                     = -909;
const int ERR_MISMATCH_ACTION_SEQ_AND_CARDS         = -910;
const int ERR_NULL_RESPONES                         = -914;


#endif