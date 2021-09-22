#ifndef _HAND_MAKER_H_
#define _HAND_MAKER_H_

#include <iostream>
#include <string>
#include <vector>
#include "cards.h"
class HandMaker
{
public:
    static int GetHandType(const Card *card,int cnt);

    static std::string GetHandTypeName(int idx);

    static void MakeTopPairHand(const Card *board_cards,int st,Card *top_pair_cards);

    static bool CheckIsAllDifferentRankFlopBoard(const Card *board_cards);

    static int GetFlopBoardType(const Card *board_cards);

    static int GetTurnBoardType(const std::vector<Card> &board_cards);

    static int GetTypeMinVal(int type);

private:
    static std::vector<std::string> _type_names;
    static std::vector<int> _hand_vals;
    HandMaker(/* args */) {}
    ~HandMaker() {}
};


#endif