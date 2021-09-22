#include "hand_maker.h"

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include "cards.h"
#include "hand_evaluator.h"
#include "hand_value_tree.h"

const std::string NoDefinedTypeName = "NotDefined";   

std::vector<std::string> HandMaker::_type_names = {"highCard", "Pair",
                                                   "TwoPair", "ThreeOfAKind",
                                                   "Straight", "Flush", "FullHouse",
                                                   "Quads", "StraightFlush"};
std::vector<int> HandMaker::_hand_vals = {HoldemHandEvaluator::kNoPair, HoldemHandEvaluator::kPair,
                                          HoldemHandEvaluator::kTwoPair, HoldemHandEvaluator::kThreeOfAKind,
                                          HoldemHandEvaluator::kStraight, HoldemHandEvaluator::kFlush,
                                          HoldemHandEvaluator::kFullHouse, HoldemHandEvaluator::kQuads,
                                          HoldemHandEvaluator::kStraightFlush};

int HandMaker::GetHandType(const Card *card,int cnt)
{
    if (cnt < 5 || cnt > 7)
    {
        return -1;
    }
    int hand_val = HandValueTree::ValByCount(card,cnt);
    int idx = 0;
    for (idx = _hand_vals.size() - 1; idx >=0 ; idx--)
    {
        if (hand_val > _hand_vals[idx])
        {
            break;
        }        
    }
    return idx;
}

int HandMaker::GetTypeMinVal(int type)
{
    return _hand_vals[type];
}

std::string HandMaker::GetHandTypeName(int idx)
{
    if (idx >= int(_type_names.size()) || idx < 0)
    {
        return NoDefinedTypeName;
    }
    else
    {
        return _type_names[idx];
    }
}

bool HandMaker::CheckIsAllDifferentRankFlopBoard(const Card *board_cards)
{
    int rank0 = Rank(board_cards[0]);
    int rank1 = Rank(board_cards[1]);
    int rank2 = Rank(board_cards[2]);

    if ((rank0 != rank1) && (rank1 != rank2) && (rank0 != rank2))
    {
        return true;
    }
    else
    {
        return false;
    }
}

// 0表示互不相同，1表示一对加一张，2表示三条
int HandMaker::GetFlopBoardType(const Card *board_cards)
{
    int rank0 = Rank(board_cards[0]);
    int rank1 = Rank(board_cards[1]);
    int rank2 = Rank(board_cards[2]);

    if ((rank0 != rank1) && (rank1 != rank2) && (rank0 != rank2))
    {
        return 0;
    }
    else if ((rank0 == rank1) && (rank1 == rank2))
    {
        return 2;
    }
    else
    {
        return 1;
    }
}

void HandMaker::MakeTopPairHand(const Card *board_cards,int st,Card *top_pair_cards)
{
    if (st != 1)
    {
        return;
    }
    
    top_pair_cards[0] = board_cards[0];
    top_pair_cards[1] = board_cards[1];
    top_pair_cards[2] = board_cards[2];

    Card high_card;
    int rank0 = Rank(board_cards[0]);
    int rank1 = Rank(board_cards[1]);
    int rank2 = Rank(board_cards[2]);
    int suit0 = Suit(board_cards[0]);
    int num_suits = S_Game::NumSuits();
    int high_card_suit = 0;
    for (high_card_suit = 0; high_card_suit < num_suits; high_card_suit++)
    {
        if (high_card_suit != suit0)
        {
           break;
        }        
    }
    high_card = MakeCard(rank0,high_card_suit);
    top_pair_cards[3] = high_card;

    Card low_card;
    int num_ranks = S_Game::NumRanks();
    int low_card_rank = 0;
    for (low_card_rank = 0; low_card_rank < num_ranks; low_card_rank++)
    {
        if (low_card_rank != rank2 && low_card_rank != rank1 && low_card_rank != rank0)
        {
           break;
        }        
    }
    low_card = MakeCard(low_card_rank,0);
    top_pair_cards[4] = low_card;
}


int HandMaker::GetTurnBoardType(const std::vector<Card> &board_cards)
{
    // int card_cnt = int(board_cards.size());
    // std::vector<int> ranks;
    // ranks.resize(card_cnt);
    // for (int i = 0; i < card_cnt; i++)
    // {
    //     ranks[i] = Rank(board_cards[i]);
    // }
    // int is_same_rank_01 = (int)(ranks[0] == ranks[1]);
    // int is_same_rank_12 = (int)(ranks[1] == ranks[2]);
    // int is_same_rank_23 = (int)(ranks[2] == ranks[3]);
    
    // int type = 4 * (is_same_rank_01) + 2 * (is_same_rank_12) + 1 * (is_same_rank_23);
    
    // if (type == 0)
    // {
    //     return 0;//高牌
    // }
    // else if (type == 7)
    // {
    //     return 7;//四条
    // }
    // else if (type == 6 || type == 3)
    // {
    //     return 3;//三条
    // }
    // else if(type == 4|| type == 2 || type == 1)
    // {
    //     return 1;//一对
    // }
    // else if(type == 5)
    // {
    //     return 2;//两对
    // }

    return 9;
    
}