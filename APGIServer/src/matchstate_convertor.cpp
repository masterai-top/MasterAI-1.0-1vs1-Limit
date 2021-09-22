#include "matchstate_convertor.h"
#include "game.h"
int MatchStateConverotr::GetActionInfo(const MatchState &matchstate, ActionInfo &info)
{
    info.st = matchstate.state.round;
    GetActionSeq(matchstate, info);
    GetCards(matchstate, info);
    return 0;//ERR_OK;
}

int MatchStateConverotr::ConvertStrToMatchState(const char *line, MatchState &matchstate, const Game *acpc_game)
{
    /*if (line[0] == '#' || line[0] == ';')
    {
        return -1;//ERR_ROBOT_COMMENT_MSG;
    }
    int len = readMatchState(line,acpc_game,&matchstate);
    if (len < 0)
    {
        return -2;//ERR_ROBOT_INVALID_MATCHSTATE;
    }

    if (stateFinished(&matchstate.state))
    {
        return -3;//ERR_ROBOT_GAME_OVER;
    }

    if (currentPlayer(acpc_game, &matchstate.state) != matchstate.viewingPlayer)
    {
        return -4;//ERR_ROBOT_NOT_OUR_TURN;
    }*/

    return 0;//ERR_OK;
}

void MatchStateConverotr::GetCards(const MatchState &matchstate, ActionInfo &info)
{
    int pos = matchstate.viewingPlayer;
    for (int i = 0; i < NUM_HCP; i++)
    {
        info.hcp_cards[i] = matchstate.state.holeCards[pos][i];
    }

    int street_board_cnt = STREET_BOARD_CNT[matchstate.state.round];
    for (int i = 0; i < street_board_cnt; i++)
    {
        info.board_cards[i] = matchstate.state.boardCards[i];
    }
}

void MatchStateConverotr::GetActionSeq(const MatchState &matchstate, ActionInfo &info)
{
    std::string action_seq = "";

    int end_st = matchstate.state.round;
    int bet_to = 2;
    for (int st = 0; st <= end_st; st++)
    {
        int num_actions = matchstate.state.numActions[st];
        if(num_actions > 0 && st > 0)
        {
            action_seq += " ";
        }
        for (int action_idx = 0; action_idx < num_actions; action_idx++)
        {
            Action action = matchstate.state.action[st][action_idx];
            action_seq += GetSingleActionStr(action);
            if (action.type == ActionType::a_raise)
            {
                if (action.size > bet_to)
                {
                    bet_to = action.size;
                }
            }
        }
    }
    info.action_seq = action_seq;
    info.bet_to = bet_to;
}

std::string MatchStateConverotr::GetSingleActionStr(Action action)
{
    std::string action_str = "";
    switch (action.type)
    {
    case ActionType::a_fold:
        action_str = "f";
        break;
    case ActionType::a_call:
        action_str = "c";
        break;
    case ActionType::a_raise:
        action_str = "r" + std::to_string(action.size);
        break;
    default:
        action_str = "";
        break;
    }

    return action_str;
}