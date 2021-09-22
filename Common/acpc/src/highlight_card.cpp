//
// Created by Administrator on 2019/12/25 0025.
//

#include <random>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <set>
#include "highlight_card.h"

#define STD_HAND_NUM 5

const string CARD_SUITS = "shc";
const string CARD_RANKS = "23456789TJQKA";
const int ACE_RANK       = CARD_RANKS.find('A');
const int ACE_LOW_RANK  = -1;

string char2String(char c)
{
    ostringstream oStr;
    oStr << c;
    return oStr.str();
}

int card2Rank(const string &card)
{
    return CARD_RANKS.find(card[0]);
}

char card2Suit(const string &card)
{
    return card[1];
}

template <typename T>
map<T, int> countVecEle(const vector<T> &list){
    map<T, int> resMap;
    for (auto ele : list)
    {
        if (!resMap.count(ele)){
            resMap[ele] = 1;
        } else{
            resMap[ele] += 1;
        }
    }
    return resMap;
}


template <class K, class V>
struct CmpByValue {
    bool operator()(const pair<K, V> &p1, const pair<K, V> &p2) {
        return p1.second == p2.second ? p1.first > p2.first : p1.second > p2.second;
    }
};


template <typename K, typename V>
vector<pair<K, V>> sortMapByValue(const map<K, V> &m, bool reverse)
{
    vector<pair<K, V>> vec(m.begin(), m.end());
    if (reverse){
        sort(vec.begin(), vec.end(), CmpByValue<K, V>());
    } else{
        sort(vec.rbegin(), vec.rend(), CmpByValue<K, V>());
    }
    return vec;
}

template <typename T>
string vecToString(const vector<T>& vec, const string& separator)
{
    string res;
    ostringstream oss;
    if (!vec.empty())
    {
        for (auto &ele: vec)
        {
            oss.str("");
            oss << ele;
            res.append(oss.str()).append(separator);
        }
        res.erase(res.size() - 1);
    }
    return res;
}


int getCardIndex(const vector<string>& cards, const string &findCard, int i)
{
    int count = 0;
    int len = cards.size();
    for (int j = 0; j < len; j ++)
    {
        string card = cards.at(j);
        if (card.find(findCard) != string::npos)
        {
            if (count == i)
            {
                return j;
            }
            count ++;
        }
    }
    return -1;
}

int getCountFromPairs(const vector<pair<int, int>> &rankCounts, const int rank)
{
    for (auto &rc: rankCounts)
    {
        if (rc.first == rank)
        {
            return rc.second;
        }
    }
    return 0;
}

vector<int> getRanksOfSuit(const vector<string> &cards, char suit)
{
    vector<int> ranksOfSuit;
    for (auto &c: cards)
    {
        if (card2Suit(c) == suit)
        {
            ranksOfSuit.push_back(card2Rank(c));
        }
    }
    return ranksOfSuit;
}

void hotEncoding(const vector<int>& indexes, vector<int> &encodingIndexes)
{
    for (auto &i: indexes)
    {
        encodingIndexes[i] = 1;
    }
}

void pushCardIndex(const vector<string>& cards, const string &card, int i, vector<int> &winCardIndexes)
{
    int cardIndex = getCardIndex(cards, card, i);
    if (cardIndex != -1)
    {
        winCardIndexes.push_back(cardIndex);
    }
}

// For all types except Straight and Straight-flush.
void setWinCardIndexes(const vector<string>& cards, const vector<int> &rSortedCardRanks,
                       const vector<pair<int, int>> &rSortedRankCounts, char flushSuit, vector<int> &winCardIndexes)
{
    int rankSize;
    string card;
    if (flushSuit)
    {
        rankSize = rSortedCardRanks.size();
        for (int i = 0; i < rankSize && winCardIndexes.size() < STD_HAND_NUM; i ++)
        {
            card = char2String(CARD_RANKS[rSortedCardRanks.at(i)]);
            card.append(char2String(flushSuit));
            pushCardIndex(cards, card, 0, winCardIndexes);
        }
    } else{
        rankSize = rSortedRankCounts.size();
        for (int i = 0; i < rankSize; i ++)
        {
            int r = rSortedRankCounts.at(i).first;
            card = char2String(CARD_RANKS[r]);
            int rCount = getCountFromPairs(rSortedRankCounts, r);
            for (int j = 0; j < rCount && winCardIndexes.size() < STD_HAND_NUM; j ++)
            {
                pushCardIndex(cards, card, j, winCardIndexes);
            }
            // Special case: Four of a kind
            if (i == 0 && winCardIndexes.size() == STD_HAND_NUM - 1)
            {
                for (int j = 0; j < rankSize; j ++)
                {
                    if (rSortedCardRanks.at(j) != r)
                    {
                        pushCardIndex(cards, char2String(CARD_RANKS[rSortedCardRanks.at(j)]), 0, winCardIndexes);
                        return;
                    }
                }
            }
            // Special case: Two pairs
            if (i == 1 && rSortedRankCounts.size() == 4 && rSortedRankCounts.at(0).second == 2
                && rSortedRankCounts.at(1).second == 2 && rSortedRankCounts.at(2).second == 2)
            {
                for (int j = 0; j < rankSize; j ++)
                {
                    if (rSortedCardRanks.at(j) != rSortedRankCounts.at(0).first && rSortedCardRanks.at(j) != rSortedRankCounts.at(1).first)
                    {
                        pushCardIndex(cards, char2String(CARD_RANKS[rSortedCardRanks.at(j)]), 0, winCardIndexes);
                        return;
                    }
                }
            }
        }
    }
}

// For Straight and Straight-flush
void analyzeStraight(const vector<string> &cards, const vector<int> &rSortedCardRanks, char flushSuit, bool &result, int &maxRank, vector<int> &winCardIndexes)
{
    int rankSize = rSortedCardRanks.size();
    if (rankSize >= STD_HAND_NUM)
    {
        vector<int> new_rSortedCardRanks;
        new_rSortedCardRanks.insert(new_rSortedCardRanks.end(), rSortedCardRanks.begin(), rSortedCardRanks.end());
        // Consider the Straight: A2345
        if (find(new_rSortedCardRanks.begin(), new_rSortedCardRanks.end(), ACE_RANK) != new_rSortedCardRanks.end())
        {
            new_rSortedCardRanks.push_back(ACE_LOW_RANK);
            rankSize ++;
        }
        for (unsigned int i = 0; i <= rankSize - STD_HAND_NUM; i ++)
        {
            unsigned int j = i;
            while(j < i + STD_HAND_NUM - 1)
            {
                if (new_rSortedCardRanks.at(j) - 1 != new_rSortedCardRanks.at(j + 1))
                {
                    break;
                }
                j ++;
            }
            if (j == i + STD_HAND_NUM - 1)
            {
                string card;
                int cardIndex;
                int rank;
                for (unsigned int k = i; k < i + STD_HAND_NUM; k ++)
                {
                    rank = new_rSortedCardRanks.at(k);
                    // Consider the Straight: A2345
                    if (rank == ACE_LOW_RANK)
                    {
                        rank = ACE_RANK;
                    }
                    card = char2String(CARD_RANKS[rank]);
                    if (flushSuit)
                    {
                        card.append(char2String(flushSuit));
                    }
					pushCardIndex(cards, card, 0, winCardIndexes);
                }
                result = true;
                maxRank = new_rSortedCardRanks.at(i);
                break;
            }
        }
    }
}


void analyzeCards(const vector<string>& cards, int &type, vector<int> &winCardIndexes)
{
    unsigned int cardSize = cards.size();
    vector<int> cardRanks(cardSize);
    transform(cards.begin(), cards.end(), cardRanks.begin(), card2Rank);

    map<int, int> rankCounts = countVecEle<int>(cardRanks);
    vector<pair<int, int>> rSortedRankCounts = sortMapByValue(rankCounts, true);

    // Unique
    set<int> cardRankSet(cardRanks.begin(), cardRanks.end());
    // Reverse the order
    vector<int> rSortedCardRanks(cardRankSet.rbegin(), cardRankSet.rend());

    // Four of a kind
    if (rSortedRankCounts.at(0).second == 4)
    {
        type = FOUR_OF_A_KIND;
        setWinCardIndexes(cards, rSortedCardRanks, rSortedRankCounts, false, winCardIndexes);
        return;
    }

    // Full house
    if (rSortedRankCounts.at(0).second == 3 && rSortedRankCounts.size() > 1 && rSortedRankCounts.at(1).second >= 2)
    {
        type = FULL_HOUSE;
        setWinCardIndexes(cards, rSortedCardRanks, rSortedRankCounts, false, winCardIndexes);
        return;
    }

    char flushSuit = 0;
    bool straight = false;

    // Flush
    vector<char> cardSuits(cardSize);
    transform(cards.begin(), cards.end(), cardSuits.begin(), card2Suit);
    map<char, int> suitCounts = countVecEle<char>(cardSuits);
    vector<pair<char, int>> sortedSuitCounts =  sortMapByValue(suitCounts, true);
    if (sortedSuitCounts.at(0).second >= STD_HAND_NUM)
    {
        flushSuit = sortedSuitCounts.at(0).first;
    }

    int maxStraightRank = -1;
    if (flushSuit)
    {
        // Straight flush
        vector<int> ranksOfSuit = getRanksOfSuit(cards, flushSuit);
        sort(ranksOfSuit.rbegin(), ranksOfSuit.rend());
        analyzeStraight(cards, ranksOfSuit, flushSuit, straight, maxStraightRank, winCardIndexes);
        if (straight)
        {
            type = STRAIGHT_FLUSH;
            if (maxStraightRank == ACE_RANK)
            {
                type = ROYAL_FLUSH;
            }
            return;
        }

        // Only flush
        type = FLUSH;
        setWinCardIndexes(cards, ranksOfSuit, rSortedRankCounts, flushSuit, winCardIndexes);
        return;
    }

    // Only straight
    analyzeStraight(cards, rSortedCardRanks, 0, straight, maxStraightRank, winCardIndexes);
    if (straight)
    {
        type = STRAIGHT;
        return;
    }

    // Other types
    setWinCardIndexes(cards, rSortedCardRanks, rSortedRankCounts, false, winCardIndexes);

    // Three of a kind
    if (rSortedRankCounts.at(0).second >= 3)
    {
        type = THREE_OF_A_KIND;
        return;
    }

    // Two Pair
    if (rSortedRankCounts.at(0).second == 2 && rSortedRankCounts.size() > 1 && rSortedRankCounts.at(1).second == 2)
    {
        type = TWO_PAIR;
        return;
    }

    // Pair
    if (rSortedRankCounts.at(0).second == 2)
    {
        type = PAIR;
        return;
    }

    // Single
    type = HIGH_CARD;
}

void hotEncodingWinCardIndexes(const vector<string> &cards, int& type, vector<int> &winCardEncoding)
{
    vector<int> winCardIndexes;
    analyzeCards(cards, type, winCardIndexes);
    hotEncoding(winCardIndexes, winCardEncoding);
}

int do_cards_highlight()
{    
    vector<string> cards{"7c", "Ts", "As", "6s", "8s", "Kh", "3d"};
    int type{};
    vector<int> winCardEncoding(cards.size());
    hotEncodingWinCardIndexes(cards, type, winCardEncoding);
    cout << "Cards: " << vecToString(cards) << endl;
    cout << TYPE_MAP.at(type) << " " << vecToString(winCardEncoding) << endl;
}

