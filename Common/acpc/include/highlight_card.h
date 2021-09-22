#ifndef _HIGH_LIGHT_CARD_H__
#define _HIGH_LIGHT_CARD_H__
#include <map>
#include <vector>
#include <string>
using namespace std;


#define ROYAL_FLUSH      (10)
#define HIGH_CARD        (9)
#define PAIR             (8)
#define TWO_PAIR         (7)
#define THREE_OF_A_KIND  (6)
#define STRAIGHT         (5)
#define FLUSH            (4)
#define FULL_HOUSE       (3)
#define FOUR_OF_A_KIND   (2)
#define STRAIGHT_FLUSH   (1)

const map<int, string> TYPE_MAP = {
        {ROYAL_FLUSH, "RoyalFlush"},
        {HIGH_CARD, "HighCard"},
        {PAIR, "Pair"},
        {TWO_PAIR, "TwoPair"},
        {THREE_OF_A_KIND, "ThreeOfAKind"},
        {STRAIGHT, "Straight"},
        {FLUSH, "Flush"},
        {FULL_HOUSE, "FullHouse"},
        {FOUR_OF_A_KIND, "FourOfAKind"},
        {STRAIGHT_FLUSH, "StraightFlush"},
};

template <typename T> string vecToString(const vector<T>& vec, const string& separator = "-");

void hotEncodingWinCardIndexes(const vector<string> &cards, int& type, vector<int> &winCardEncoding);

void test_highlight_card();



#endif

