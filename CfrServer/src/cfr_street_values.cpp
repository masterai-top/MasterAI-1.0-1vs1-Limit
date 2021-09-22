#include <stdio.h>
#include <stdlib.h>

#include <memory>

#include "action_tree.h"
#include "p_card_tree.h"
#include "cluster_hands.h"
#include "cfr_street_values.h"
#include "global_def.h"
#include "s_game.h"
#include "reader.h"

#include "pch.h"
#include "error_code.h"

using std::shared_ptr;
using std::unique_ptr;

template <typename T>
CFRStreetValues<T>::CFRStreetValues(int st, const bool *players, int num_holdings,
                                    int *num_nonterminals, CFRValueType file_value_type)
{
    file_value_type_ = file_value_type;
    st_ = st;
    int num_players = S_Game::NumPlayers();
    players_.reset(new bool[num_players]);
    for (int p = 0; p < num_players; ++p)
    {
        players_[p] = players == nullptr || players[p];
    }
    num_holdings_ = num_holdings;
    num_nonterminals_.reset(new int[num_players]);
    for (int p = 0; p < num_players; ++p)
    {
        if (players_[p])
        {
            num_nonterminals_[p] = num_nonterminals[p * (S_Game::MaxStreet() + 1) + st_];
        }
        else
        {
            num_nonterminals_[p] = 0;
        }
    }
    data_ = nullptr;
}

template <typename T>
CFRStreetValues<T>::~CFRStreetValues(void)
{
    // This can happen for values for an all-in subtree.
    if (data_ == nullptr)
        return;
    int num_players = S_Game::NumPlayers();
    for (int p = 0; p < num_players; ++p)
    {
        if (data_[p] == nullptr)
            continue;
        int num_nt = num_nonterminals_[p];
        for (int i = 0; i < num_nt; ++i)
        {
            delete[] data_[p][i];
        }
        delete[] data_[p];
    }
    delete[] data_;
}

template <>
CFRValueType CFRStreetValues<unsigned char>::MyType(void) const
{
    return CFRValueType::CFR_CHAR;
}

template <>
CFRValueType CFRStreetValues<unsigned short>::MyType(void) const
{
    return CFRValueType::CFR_SHORT;
}

template <>
CFRValueType CFRStreetValues<int>::MyType(void) const
{
    return CFRValueType::CFR_INT;
}

template <>
CFRValueType CFRStreetValues<unsigned int>::MyType(void) const
{
    return CFRValueType::CFR_INT;
}

template <>
CFRValueType CFRStreetValues<double>::MyType(void) const
{
    return CFRValueType::CFR_DOUBLE;
}

// 指定p给data_分配内存
template <typename T>
void CFRStreetValues<T>::AllocateAndClear2(Node *node, int p)
{
    if (node->Terminal())
        return;
    int st = node->Street();
    if (st > st_)
        return;
    int num_succs = node->NumSuccs();
    if (st == st_)
    {
        int pa = node->PlayerActing();
        if (pa == p)
        {
            int nt = node->NonterminalID();
            // Check for reentrant nodes
            if (data_[p][nt] == nullptr)
            {
                int num_actions = num_holdings_ * num_succs;
                data_[p][nt] = new T[num_actions];
                for (int a = 0; a < num_actions; ++a)
                {
                    data_[p][nt][a] = 0;
                }
            }
        }
    }
    for (int s = 0; s < num_succs; ++s)
    {
        AllocateAndClear2(node->IthSucc(s), p);
    }
}

template <typename T>
void CFRStreetValues<T>::AllocateAndClear(Node *node, int p)
{
    if (!players_[p])
        return;
    if (data_ == nullptr)
    {
        int num_players = S_Game::NumPlayers();
        data_ = new T **[num_players];
        for (int p = 0; p < num_players; ++p)
            data_[p] = nullptr;
    }
    if (data_[p] == nullptr)
    {
        int num_nt = num_nonterminals_[p];
        data_[p] = new T *[num_nt];
        for (int i = 0; i < num_nt; ++i)
            data_[p][i] = nullptr;
    }
    AllocateAndClear2(node, p);
}

template <typename T>
void CFRStreetValues<T>::GetOneStrategyVals(int p, int nt, int offset, int num_succs, unsigned int *vals) const
{
    T *my_vals = &data_[p][nt][offset];
    for (int i = 0; i < num_succs; i++)
    {
        vals[i] = my_vals[i];
    }
}

template <typename T>
void CFRStreetValues<T>::Set(int p, int nt, int h, int num_succs, T *vals)
{
    int offset = h * num_succs;
    for (int s = 0; s < num_succs; ++s)
    {
        data_[p][nt][offset + s] = vals[s];
    }
}

template <typename T>
void CFRStreetValues<T>::InitializeValuesForReading(int p, int nt, int num_succs)
{
    if (data_ == nullptr)
    {
        int num_players = S_Game::NumPlayers();
        data_ = new T **[num_players];
        for (int p = 0; p < num_players; ++p)
            data_[p] = nullptr;
    }
    if (data_[p] == nullptr)
    {
        int num_nt = num_nonterminals_[p];
        data_[p] = new T *[num_nt];
        for (int i = 0; i < num_nt; ++i)
            data_[p][i] = nullptr;
    }
    if (data_[p][nt] == nullptr)
    {
        int num_actions = num_holdings_ * num_succs;
        data_[p][nt] = new T[num_actions];
    }
    // Don't need to zero
}

template <>
unsigned char ***CFRStreetValues<unsigned char>::GetUnsignedCharData(void)
{
    return data_;
}

template <>
unsigned char ***CFRStreetValues<unsigned short>::GetUnsignedCharData(void)
{
    fprintf(stderr, "Cannot call GetUnsignedCharData()\n");
    exit(-1);
}

template <>
unsigned char ***CFRStreetValues<int>::GetUnsignedCharData(void)
{
    fprintf(stderr, "Cannot call GetUnsignedCharData()\n");
    exit(-1);
}

template <>
unsigned char ***CFRStreetValues<unsigned int>::GetUnsignedCharData(void)
{
    fprintf(stderr, "Cannot call GetUnsignedCharData()\n");
    exit(-1);
}

template <>
unsigned char ***CFRStreetValues<double>::GetUnsignedCharData(void)
{
    fprintf(stderr, "Cannot call GetUnsignedCharData()\n");
    exit(-1);
}

template <typename T>
void CFRStreetValues<T>::ReadNode(Node *node, Reader *reader, void *decompressor)
{
    int num_succs = node->NumSuccs();
    if (num_succs <= 1)
        return;
    int p = node->PlayerActing();
    int nt = node->NonterminalID();

    if (data_ && data_[p] && data_[p][nt])
    {
        return;
    }
    InitializeValuesForReading(p, nt, num_succs);
    if (decompressor)
    {
        fprintf(stderr, "Decompression not supported yet\n");
        exit(-1);
    }
    int num_actions = num_holdings_ * num_succs;
    if (file_value_type_ == CFRValueType::CFR_CHAR)
    {
        for (int a = 0; a < num_actions; ++a)
        {
            reader->ReadOrDie(&data_[p][nt][a]);
        }
    }
    else if (file_value_type_ == CFRValueType::CFR_SHORT)
    {
        for (int a = 0; a < num_actions; ++a)
        {
            reader->ReadOrDie(&data_[p][nt][a]);
        }
    }
    else if (file_value_type_ == CFRValueType::CFR_INT)
    {
        for (int a = 0; a < num_actions; ++a)
        {
            reader->ReadOrDie(&data_[p][nt][a]);
        }
    }
    else if (file_value_type_ == CFRValueType::CFR_DOUBLE)
    {
        for (int a = 0; a < num_actions; ++a)
        {
            reader->ReadOrDie(&data_[p][nt][a]);
        }
    }
}

template class CFRStreetValues<int>;
template class CFRStreetValues<double>;
template class CFRStreetValues<unsigned char>;
template class CFRStreetValues<unsigned short>;
