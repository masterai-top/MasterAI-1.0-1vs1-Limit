#include "dt_robot.h"

#include <map>
#include <stdio.h>
#include "shstd.h"


#include "global_variant.h"
#include "string_utils.h"

// S_Bot Including begin
#include "files.h"
#include "s_game.h"
#include "card_abstraction.h"
#include "hand_value_tree.h"
#include "p_card_tree.h"
#include "private_cards.h"
#include "typical_cards.h"
#include "typical_cards.h"
#include "cluster_hands.h"
// S_Bot Including end

#include "action_idx_selector.h"
#include "rule_filter.h"
#include "rng_generator.h"
#include "Def.h"
#include "param_factory.h"
#include "help_functions.h"

std::shared_ptr<ClusterHands> gClusterHands;

using namespace std;

int DTRobot::DoAction(const ActionParam &action_param, const ModelParam &model_param,
                        const ActionInfo &action_info, const NodeInfo &node_info,
                        const std::vector<unsigned int> &vals, const LogInfo &log_info, 
                        std::string &action_str,std::string &rate_str)
{
    if (!CheckCards(action_info))
    {
        return ERR_INVALID_CARD;
    }

    int st = action_info.st;
    int final_idx;
    if (model_param.sumprob_flag[st])
    {
        if (!action_param.filter_flags[st] || action_param.frm_ths[st] >= 1.0)
        {
            final_idx = ActionIdxSelector::GetPureIdx(vals,true);
        }
        else
        {
            std::vector<std::pair<int, unsigned int>> candidate_action;
            ActionIdxSelector::FRMChooseCandidate(vals,true,action_param.frm_ths[st],candidate_action);
            double sum = 0;
            for (size_t i = 0; i < candidate_action.size(); i++)
            {
                sum += candidate_action[i].second;
            }
            std::vector<std::pair<int, double>> candidate_action_probs;
            if (sum == 0)
            {
                for (size_t i = 0; i < candidate_action.size(); i++)
                {
                    double prob = i == 0 ? 1.0 : 0;
                    candidate_action_probs.push_back(std::make_pair(candidate_action[i].first,prob));
                }
            }
            else
            {
                for (size_t i = 0; i < candidate_action.size(); i++)
                {
                    double prob = candidate_action[i].second / sum;
                    candidate_action_probs.push_back(std::make_pair(candidate_action[i].first, prob));
                }
            }
            final_idx = ActionIdxSelector::ChooseIdxByRm(candidate_action_probs);
        }        
    }
    else
    {
        int action_idx = ActionIdxSelector::GetPureIdx(vals, false);
        if (!action_param.filter_flags[st])
        {
            final_idx = action_idx;
        }
        else
        {
            if (action_param.rule_types[st] == 1)
            {
                LOG(LT_INFO_TRANS, log_info.trans_id.c_str(), "RoleID=%d| handID=%d|"
                "Filter %s Action:RuleType=%d pot_th=%0.3f,frm_th=%0.3f",
                    log_info.role_id, log_info.hand_id, STREET_NAME[st].c_str(), action_param.rule_types[st],
                    action_param.pot_ths[st], action_param.frm_ths[st]);

                if (st == 0)
                {
                    final_idx = action_idx;
                }
                else if(st == 1)
                {
                    int filter_idx = action_idx;
                    filter_idx = RuleFilter::FilterFlopLargeBetAction(action_info, node_info,
                                                                      vals, log_info,action_idx,
                                                                      action_param.pot_ths[st]);

                    final_idx = filter_idx;
                }
                else
                {
                    int filter_idx = action_idx;
                    filter_idx = RuleFilter::FilterAllinAction(action_info,node_info,
                                                               vals, log_info,action_idx, action_param.frm_ths[st],
                                                               action_param.handle_nfrom[st]);

                    final_idx = filter_idx;
                }
            }
            else if(action_param.rule_types[st] == 2)
            {
                LOG(LT_INFO_TRANS, log_info.trans_id.c_str(), "RoleID=%d| handID=%d|"
                "Filter %s Action:RuleType=%d pot_th=%0.3f,frm_th=%0.3f",
                    log_info.role_id, log_info.hand_id, STREET_NAME[st].c_str(), action_param.rule_types[st],
                    action_param.pot_ths[st], action_param.frm_ths[st]);

                int filter_idx = RuleFilter::FilterLargeBetAction(action_info, node_info,
                                                                         vals, log_info,action_idx,
                                                                         action_param.pot_ths[st]);

                final_idx = filter_idx;
            }
            else
            {
                final_idx = action_idx;
            }
            
        }

        if (final_idx != action_idx)
        {
            LOG(LT_INFO_TRANS, log_info.trans_id.c_str(), "RoleID=%d| handID=%d|"
                                                          "Filter %s Action From %d To %d RuleType=%d pot_th=%0.3f,frm_th=%0.3f",
                log_info.role_id, log_info.hand_id, STREET_NAME[st].c_str(), action_idx, final_idx,
                action_param.rule_types[st], action_param.pot_ths[st], action_param.frm_ths[st]);
        }
    }
    
    std::vector<std::string> child_nodes = node_info.GetChildNode();
    action_str = child_nodes[final_idx];
    FormatStr(child_nodes,vals,rate_str);
    rate_str += (model_param.sumprob_flag[st] ?"[Sum]":"[Reg]");
    int pure_idx = ActionIdxSelector::GetPureIdx(vals,model_param.sumprob_flag[st]);
    rate_str += (pure_idx==final_idx ?"[Default]":"[" + action_str + "]");
    
    return ERR_OK;
}

int DTRobot::Init(const ModelParam &param)
{
    //
    LOG(LT_INFO_TRANS,gLoading.c_str(), "Init Rng Generator");
    RngGenerator::Init();
    
    //初始化文件
    LOG(LT_INFO_TRANS,gLoading.c_str(), "Setting Files");
    Files::Init("",param.static_dir);

    // 初始化game
    LOG(LT_INFO_TRANS,gLoading.c_str(), "Init Game");    
    std::unique_ptr<Params> game_params = ParamsFactory::CreateGameParams();
    game_params->ReadFromFile(param.game_param_path.c_str());
    int flag = S_Game::Initialize(*game_params);
    if(flag != ERR_OK)
    {
        LOG(LT_ERROR_TRANS,gLoading.c_str(), "Init Game err=%d!",flag);
        return flag;
    }    
    PrintGameInfo();
    if(!CheckIfGameValid())
    {
        LOG(LT_ERROR_TRANS,gLoading.c_str(), " Don't Support This Game Yet:");
        return ERR_CONFIG_INVALID_GAME;
    }

    if (!param.random_flag)
    {
        //创建手牌强度
        LOG(LT_INFO_TRANS, gLoading.c_str(), "Build HandVal Tree");
        HandValueTree::CreateAllTrees();

        // boardTree 建立
        LOG(LT_INFO_TRANS, gLoading.c_str(), "Building Board Tree");
        PCardTree::Create();
        PCardTree::CreateLookup();

        //bucket 建立
        LOG(LT_INFO_TRANS, gLoading.c_str(), "Building ClusterHands");
        std::unique_ptr<Params> card_params = ParamsFactory::CreateCardAbstractionParams();
        card_params->ReadFromFile(param.card_param_path.c_str());
        std::unique_ptr<CardAbstraction> card_abstraction;
        card_abstraction.reset(new CardAbstraction(*card_params));
        gClusterHands.reset(new ClusterHands(*card_abstraction, false));

    }

    
    return ERR_OK;

}

int DTRobot::GetKey(const ActionInfo &info, const ModelParam &model_param,
                    const NodeInfo &node_info, CFRKey &key)
{    
    if (!CheckCards(info))
    {
        return ERR_INVALID_CARD;
    }
    
    
    key.node_id = node_info.GetNodeId();
    key.pa = node_info.GetActingPlayer();
    key.num_succ = node_info.GetChildNodeCount();
    if(info.st != node_info.GetStreet())
    {
        return ERR_MISMATCH_ACTION_SEQ_AND_CARDS;
    }
    key.st = info.st;

    int buck_idx;
    if (model_param.random_flag)
    {
        buck_idx = int(RngGenerator::GetRng() * 100);
    }
    else
    {
        int flag = GetBucketIdx(info, buck_idx);
        if (flag != ERR_OK)
        {
            return flag;
        }
    }

    key.offset = buck_idx * key.num_succ;

    return ERR_OK;
}

int DTRobot::GetBucketIdx(const ActionInfo &info,int &buck_idx)
{   
    // 排序
    Card tmp_hole_cards[NUM_HCP];
    for (int i = 0; i < NUM_HCP; i++)
    {
        tmp_hole_cards[i] = info.hcp_cards[i];
    }
    SortCards(tmp_hole_cards, NUM_HCP);
    
    Card tmp_board_cards[MAX_BOADRD_NUM];
    for (int i = 0; i < MAX_BOADRD_NUM; i++)
    {
        tmp_board_cards[i] = info.board_cards[i];
    }

    int num = 0;
    for (int st = 1; st < NUM_STREET; ++st)
    {
        int num_street_cards = S_Game::NumCardsForStreet(st);
        SortCards(tmp_board_cards + num, num_street_cards);
        num += num_street_cards;
    }
    
    int st = info.st;
    // 获取bd和hcp
    int bd,hcp;
    if (st == 0)
    {
        hcp = GetPrivateHandIdxWithTotalCards(st, tmp_hole_cards);
        bd = 0;
    }
    else
    {
        Card canon_board[MAX_BOADRD_NUM];
        Card canon_hole_cards[NUM_HCP];
        CanonicalizeCards(tmp_board_cards, tmp_hole_cards, st, canon_board, canon_hole_cards);
        int flag = PCardTree::LookupBoard_SBot(canon_board,st,bd);
        if (flag != ERR_OK)
        {
            return flag;
        }
        
        hcp = GetPrivateHandIdxWithTotalCards(st, canon_board,canon_hole_cards);
    }

    // STEP3: 获取bucket_idx
    int num_hole_card_pairs = S_Game::NumHoleCardPairs(st);
    unsigned int h = ((unsigned int)bd) * ((unsigned int)num_hole_card_pairs) + hcp;
    buck_idx = gClusterHands->GetClusterIdx(st, h);

    return ERR_OK;
}

void DTRobot::PrintGameInfo()
{
    char gameInfoStr[4096];
    sprintf(gameInfoStr, "gameName=%s| maxStreet=%d| numPlayers=%d| "
    "numRanks=%d| numSuits=%d| smallBlind=%d| bigBlind=%d| firstPlayer=[%d",
            S_Game::GameName().c_str(), S_Game::MaxStreet(),
             S_Game::NumPlayers(), S_Game::NumRanks(), S_Game::NumSuits(),
            S_Game::SmallBlind(), S_Game::BigBlind(), S_Game::FirstToAct(0));
    int maxStreet = S_Game::MaxStreet();
    int len;
    for (int st = 1; st < maxStreet + 1; ++st)
    {
        len = strlen(gameInfoStr);
        sprintf(gameInfoStr + len, ",%d", S_Game::FirstToAct(st));
    }
    len = strlen(gameInfoStr);
    sprintf(gameInfoStr + len, "]| NumCardsForStreet=[%d", S_Game::NumCardsForStreet(0));
    for (int st = 1; st < maxStreet + 1; ++st)
    {
        len = strlen(gameInfoStr);
        sprintf(gameInfoStr + len, ",%d", S_Game::NumCardsForStreet(st));
    }
    len = strlen(gameInfoStr);
    sprintf(gameInfoStr + len, "]");
    LOG(LT_INFO_TRANS, gLoading.c_str(), "Loading Game Done :%s", gameInfoStr);
}

bool DTRobot::CheckIfGameValid()
{
    if (S_Game::MaxStreet() + 1 != NUM_STREET 
    || S_Game::NumBoardCards(NUM_STREET - 1) != MAX_BOADRD_NUM 
    || S_Game::NumCardsForStreet(0) != NUM_HCP 
    || S_Game::NumPlayers() != NUM_PLAYER
    || S_Game::MaxCard() != NUM_CARDS - 1)
    {
        return false;
    }
    return true;
}

void DTRobot::FormatStr(const std::vector<std::string> &child_nodes,
                        const std::vector<unsigned int> &vals, std::string &format_str)
{
    format_str = "[";
    for (size_t i = 0; i < child_nodes.size(); i++)
    {
        char buf[1024] = {0};
        sprintf(buf,"%s:%u",child_nodes[i].c_str(), vals[i]);
        format_str += buf;
        if (i < child_nodes.size() - 1)
        {
            format_str += ",    ";
        }
    }
    format_str += "]";
}

bool DTRobot::CheckCards(const ActionInfo &info)
{
    bool exist_flag[NUM_CARDS];
    memset(exist_flag,false,sizeof(exist_flag));
    for (int i = 0; i < NUM_HCP; i++)
    {
        int cur_card = info.hcp_cards[i];
        if (cur_card < 0 || cur_card >= NUM_CARDS)
        {
            return false;
        }
        if (exist_flag[cur_card])
        {
            return false;
        }
        exist_flag[cur_card] = true;        
    }
    int num_board_card = STREET_BOARD_CNT[info.st];
    for (int i = 0; i < num_board_card; i++)
    {
        int cur_card = info.board_cards[i];
        if (cur_card < 0 || cur_card >= NUM_CARDS)
        {
            return false;
        }
        if (exist_flag[cur_card])
        {
            return false;
        }
        exist_flag[cur_card] = true; 
    }
    return true;
}
