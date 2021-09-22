#include "rule_filter.h"
#include "hand_maker.h"
#include "help_functions.h"
#include "hand_value_tree.h"
#include "action_idx_selector.h"
#include "rng_generator.h"
#include "shstd.h"
#include "Def.h"

int RuleFilter::FilterFlopLargeBetAction(const ActionInfo &action_info,
                                         const NodeInfo &node_info,
                                         const std::vector<unsigned int> &reg_vals,
                                         const LogInfo &log_info,
                                         int action_idx, double pot_th)
{
    std::vector<std::string> child_nodes = node_info.GetChildNode();
    if (child_nodes[action_idx] == "c" || child_nodes[action_idx] == "f")
    {
        return action_idx;
    }

    std::string str_bet_to = child_nodes[action_idx].substr(1);
    int action_bet_to = 0;
    if (sscanf(str_bet_to.c_str(),"%i",&action_bet_to) != 1)
    {
        return action_idx;
    }
    int raise_amount = action_bet_to - action_info.bet_to;
    double pot_frac = double(raise_amount) / double(2 * action_info.bet_to);
    if (pot_frac < pot_th)
    {
        return action_idx;//不符合底池条件，直接返回
    }
    
    const int flop_board_cnt = STREET_BOARD_CNT[1];
    Card board_cards[3];
    for (int i = 0; i < flop_board_cnt; i++)
    {
        board_cards[i] = action_info.board_cards[i];
    }
    SortCards(board_cards, flop_board_cnt);

    int board_type = HandMaker::GetFlopBoardType(board_cards);
    if (board_type == 2)
    {
        LOG(LT_INFO_TRANS, log_info.trans_id.c_str(), "RoleID=%d| handID=%d|"
                                                      "Flop >= %0.2f Pot But Board is ThreeOfAKind",
            log_info.role_id, log_info.hand_id, pot_th);
        return action_idx; //不处理公共牌有一对或者三条的情况
    }
    else if (board_type == 1)
    {
        Card cur_cards[5];
        cur_cards[0] = board_cards[0];
        cur_cards[1] = board_cards[1];
        cur_cards[2] = board_cards[2];
        cur_cards[3] = action_info.hcp_cards[0];
        cur_cards[4] = action_info.hcp_cards[1];
        int hand_type = HandMaker::GetHandType(cur_cards,5);
        if (hand_type < 3 )
        {
            LOG(LT_INFO_TRANS, log_info.trans_id.c_str(), "RoleID=%d| handID=%d|"
                                                          "Flop Not ThreeOfAKind But Raise >= %0.2f pot",
                log_info.role_id, log_info.hand_id, pot_th);
            return action_idx; //非顶对则保持原有动作
        }

    }
    else
    {
        Card top_pair_cards[5];
        HandMaker::MakeTopPairHand(board_cards, 1, top_pair_cards);

        int top_pair_val = HandValueTree::ValByCount(top_pair_cards, 5);
        Card cur_cards[5];
        cur_cards[0] = board_cards[0];
        cur_cards[1] = board_cards[1];
        cur_cards[2] = board_cards[2];
        cur_cards[3] = action_info.hcp_cards[0];
        cur_cards[4] = action_info.hcp_cards[1];
        int cur_val = HandValueTree::ValByCount(cur_cards, 5);
        if (cur_val < top_pair_val)
        {
            LOG(LT_INFO_TRANS, log_info.trans_id.c_str(), "RoleID=%d| handID=%d|"
                                                          "Flop Not Top Pair But Raise >= %0.2f pot",
                log_info.role_id, log_info.hand_id, pot_th);
            return action_idx; //非顶对则保持原有动作
        }
    }

    // 剔除掉fold动作,将符合底池的条件进行完全的随机
    std::vector<std::pair<int, unsigned int>> candidate_action;
    for (int i = 0; i < action_idx; i++)
    {
        double pot_frac = GetPotFrac(child_nodes, i, action_info.bet_to);
        if (child_nodes[i] != "f" && pot_frac < pot_th)
        {
            candidate_action.push_back(std::make_pair(i, reg_vals[i]));
        }
        LOG(LT_INFO_TRANS, log_info.trans_id.c_str(), "RoleID=%d| handID=%d|"
                                                      "Flop Top Pair And Raise >= %0.2f pot",
            log_info.role_id, log_info.hand_id, pot_th);
    }
    return ActionIdxSelector::ChooseIdxByMinReg(candidate_action);

}

int RuleFilter::FilterTurnLargeBetAction(const ActionInfo &action_info, const NodeInfo &node_info,
                                         const std::vector<unsigned int> &reg_vals, const LogInfo &log_info,
                                         int action_idx, double pot_th)
{
    // std::vector<std::string> child_nodes = node_info.GetChildNode();
    // if (child_nodes[action_idx] == "c" || child_nodes[action_idx] == "f")
    // {
    //     return action_idx;
    // }

    // std::string str_bet_to = child_nodes[action_idx].substr(1);
    // int action_bet_to = 0;
    // if (sscanf(str_bet_to.c_str(),"%i",&action_bet_to) != 1)
    // {
    //     return action_idx;
    // }
    // int raise_amount = action_bet_to - action_info.bet_to;
    // double pot_frac = double(raise_amount) / double(2 * action_info.bet_to);
    // if (pot_frac <= pot_th)
    // {
    //     return action_idx;//不符合底池条件，直接返回
    // }
    
    // const int turn_board_cnt = STREET_BOARD_CNT[1];
    // std::vector<Card> board_cards;
    // board_cards.resize(turn_board_cnt);
    // for (int i = 0; i < turn_board_cnt; i++)
    // {
    //     board_cards[i] = action_info.board_cards[i];
    // }
    // std::sort(board_cards.begin(), board_cards.end());
    // int board_type = HandMaker::GetTurnBoardType(board_cards);
    // int high_board_type_val = HandMaker::GetTypeMinVal(board_type + 1);

    // Card hand_cards[6];
    // for (int i = 0; i < turn_board_cnt; i++)
    // {
    //     hand_cards[i] = action_info.board_cards[i];
    // }
    // hand_cards[4] = action_info.hcp_cards[0];
    // hand_cards[5] = action_info.hcp_cards[1];
    // int hand_val = HandValueTree::ValByCount(hand_cards,6);
    
    // if (hand_val < high_board_type_val)
    // {
    //     LOG(LT_INFO_TRANS, log_info.trans_id.c_str(), "RoleID=%d| handID=%d|"
    //                                                   "Turn Not Max But Raise > %0.2f pot",
    //         log_info.role_id, log_info.hand_id, pot_th);
    //     return action_idx; //非顶对则保持原有动作
    // }
    // // 剔除掉fold动作,将符合底池的条件进行完全的随机
    // std::vector<std::pair<int, unsigned int>> candidate_action;
    // for (int i = 0; i < action_idx; i++)
    // {
    //     double pot_frac = GetPotFrac(child_nodes, i, action_info.bet_to);
    //     if (child_nodes[i] != "f" && pot_frac <= pot_th)
    //     {
    //         candidate_action.push_back(std::make_pair(i, reg_vals[i]));
    //     }
    //     LOG(LT_INFO_TRANS, log_info.trans_id.c_str(), "RoleID=%d| handID=%d|"
    //                                                   "turn Max And Raise > %0.2f pot",
    //         log_info.role_id, log_info.hand_id, pot_th);
    // }
    // return ActionIdxSelector::ChooseIdxByMinReg(candidate_action);
    return 1;
}

int RuleFilter::FilterAllinAction(const ActionInfo &action_info,
                                  const NodeInfo &node_info, 
                                  const std::vector<unsigned int> &reg_vals,
                                  const LogInfo &log_info,
                                  int action_idx, double filter_th,bool handle_nfrm)
{
    std::vector<std::string> child_nodes = node_info.GetChildNode();
    if (child_nodes[action_idx] != "r200")
    {
        return action_idx;
    }    
    
    int num_succ = int(reg_vals.size());

    unsigned int second_min_val = 2000000000U;
    unsigned int max_reg_val = 0;
    int second_min_val_idx = 0;
    // 往前找非allin动作的最小regret，判断是否符合frm条件，
    // 符合则选该动作，不符合，仍然选择allin
    for (int i = 0; i < num_succ; i++)
    {
        if (reg_vals[i] < second_min_val && i!=action_idx)
        {
            second_min_val = reg_vals[i];
            second_min_val_idx = i;
        }
        if (reg_vals[i] > max_reg_val)
        {
            max_reg_val = reg_vals[i];
        }        
    }

    if (second_min_val < max_reg_val * filter_th)
    {
        LOG(LT_INFO_TRANS, log_info.trans_id.c_str(), "RoleID=%d| handID=%d|"
                                                      "%s Filter Allin To %s(%u:%u)",
            log_info.role_id, log_info.hand_id,
            STREET_NAME[node_info.GetStreet()].c_str(),
            child_nodes[second_min_val_idx].c_str(),
            second_min_val, max_reg_val);

        return second_min_val_idx;
    }
    else
    {
        if (handle_nfrm)
        {
            LOG(LT_INFO_TRANS, log_info.trans_id.c_str(), "RoleID=%d| handID=%d|"
                                                          "%s Nfrm But Still Choose  %s(%u:%u)",
                log_info.role_id, log_info.hand_id,
                STREET_NAME[node_info.GetStreet()].c_str(),
                child_nodes[second_min_val_idx].c_str(),
                second_min_val, max_reg_val);

            return second_min_val_idx;
        }
        else
        {
            LOG(LT_INFO_TRANS, log_info.trans_id.c_str(), "RoleID=%d| handID=%d|"
                                                          "%s Can't Filter Allin To %s(%u:%u)",
                log_info.role_id, log_info.hand_id,
                STREET_NAME[node_info.GetStreet()].c_str(),
                child_nodes[second_min_val_idx].c_str(),
                second_min_val, max_reg_val);
            return action_idx;
        }
    }
}

int RuleFilter::FilterActionToSmallBetAndRandom(const ActionInfo &action_info,
                                                const NodeInfo &node_info,
                                                const std::vector<unsigned int> &reg_vals,
                                                const LogInfo &log_info,
                                                int action_idx, double pot_th, double filter_th)
{
    std::vector<std::string> child_nodes = node_info.GetChildNode();
    //int st = node_info.GetStreet();

    if (child_nodes[action_idx] == "f" || child_nodes[action_idx] == "c")
    {
        return action_idx;
    }

    int num_succ = int(reg_vals.size());
    unsigned int max_reg_val = 0;
    for (int i = 0; i < num_succ; i++)
    {
        if (reg_vals[i] > max_reg_val)
        {
            max_reg_val = reg_vals[i];
        }
    }

    std::string str_bet_to = child_nodes[action_idx].substr(1);
    int action_bet_to = 0;
    if (sscanf(str_bet_to.c_str(), "%i", &action_bet_to) != 1)
    {
        return action_idx;
    }
    int raise_amount = action_bet_to - action_info.bet_to;
    double pot_frac = double(raise_amount) / double(2 * action_info.bet_to);
    if (pot_frac > pot_th)
    {
        int min_val_idx = 0;
        unsigned int min_val = 2000000000U;
        int end_idx = action_idx;
        std::vector<std::pair<int, unsigned int>> candidate_actions;
        for (int i = 0; i < end_idx; i++)
        {
            double pot_frac = GetPotFrac(child_nodes, i, action_info.bet_to);
            if (pot_frac <= pot_th && reg_vals[i] < max_reg_val * filter_th)
            {
                candidate_actions.push_back(std::make_pair(i, reg_vals[i]));
            }
            if (min_val > reg_vals[i])
            {
                min_val = reg_vals[i];
                min_val_idx = i;
            }            
        }
        int candidate_cnt = int(candidate_actions.size());
        if (candidate_cnt == 0)
        {
            return min_val_idx;
        }
        return ActionIdxSelector::ChooseIdxByAllRandom(candidate_actions);

    }
    else
    {
        int end_idx = (node_info.GetStreet() == 0 ? action_idx + 1 : num_succ);

        std::vector<std::pair<int, unsigned int>> candidate_actions;
        for (int i = 0; i < end_idx; i++)
        {
            double pot_frac = GetPotFrac(child_nodes, i, action_info.bet_to);
            if (pot_frac <= pot_th && reg_vals[i] < max_reg_val * filter_th)
            {
                candidate_actions.push_back(std::make_pair(i, reg_vals[i]));
            }
        }
        int candidate_cnt = int(candidate_actions.size());
        if (candidate_cnt == 0)
        {
            return action_idx;
        }
        return ActionIdxSelector::ChooseIdxByAllRandom(candidate_actions);
    }
}

int RuleFilter::FilterLargeBetAction(const ActionInfo &action_info, const NodeInfo &node_info,
                                 const std::vector<unsigned int> &reg_vals,const LogInfo &log_info,
                                 int action_idx, double pot_th)
{  
    std::vector<std::string> child_nodes = node_info.GetChildNode();
    if (child_nodes[action_idx] == "c" || child_nodes[action_idx] == "f")
    {
        return action_idx;
    }

    std::string str_bet_to = child_nodes[action_idx].substr(1);
    int action_bet_to = 0;
    if (sscanf(str_bet_to.c_str(),"%i",&action_bet_to) != 1)
    {
        return action_idx;
    }
    int raise_amount = action_bet_to - action_info.bet_to;
    double pot_frac = double(raise_amount) / double(2 * action_info.bet_to);
    
    if (pot_frac < pot_th)
    {
        return action_idx;
    }    

    std::vector<std::pair<int,unsigned int>> candidate_actions;
    for (int i = 0; i < action_idx; i++)
    {
        double pot_frac = GetPotFrac(child_nodes, i, action_info.bet_to);
        if (pot_frac < pot_th)
        {
            candidate_actions.push_back(std::make_pair(i, reg_vals[i]));
        }
        candidate_actions.push_back(std::make_pair(i, reg_vals[i]));
    }

    int final_idx = ActionIdxSelector::ChooseIdxByMinReg(candidate_actions);


    LOG(LT_INFO_TRANS, log_info.trans_id.c_str(), "RoleID=%d| handID=%d|"
                                                  "Filter %s Action From (%s:%u) To (%s:%u)",
        log_info.role_id, log_info.hand_id, STREET_NAME[node_info.GetStreet()].c_str(),
        child_nodes[action_idx].c_str(),reg_vals[action_idx],
        child_nodes[final_idx].c_str(),reg_vals[final_idx]);

    return final_idx;
}

double RuleFilter::GetPotFrac(const std::vector<std::string> &child_nodes,int action_idx,int old_bet_to)
{
    if (child_nodes[action_idx] == "c")
    {
        return 0;
    }
    else if(child_nodes[action_idx] == "f")
    {
        return -1;
    }
    else
    {
        std::string str_bet_to = child_nodes[action_idx].substr(1);
        int action_bet_to = 0;
        if (sscanf(str_bet_to.c_str(), "%i", &action_bet_to) != 1)
        {
            return 999;
        }
        int raise_amount = action_bet_to - old_bet_to;
        double pot_frac = double(raise_amount) / double(2 * old_bet_to);
        return pot_frac;
    }
}