#include <stdio.h>
#include <stdlib.h>

#include <memory>
#include <string>
#include <thread>

#include "pch.h"

#include "action_tree.h"
#include "p_card_tree.h"
#include "cluster_hands.h"
#include "cfr_street_values.h"
#include "global_def.h"
#include "cfr_values.h"
#include "s_game.h"
#include "reader.h"
#include "nonterminal_ids.h"
#include "error_code.h"
#include "global_def.h"

using std::string;
using std::unique_ptr;

void CFRValues::Initialize(const bool *players, const bool *streets, int root_bd, int root_bd_st,
                           const ClusterHands &buckets)
{
    root_bd_ = root_bd;
    root_bd_st_ = root_bd_st;
    int num_players = S_Game::NumPlayers();
    players_.reset(new bool[num_players]);
    for (int p = 0; p < num_players; ++p)
    {
        players_[p] = players == nullptr || players[p];
    }
    int max_street = S_Game::MaxStreet();
    streets_.reset(new bool[max_street + 1]);
    for (int st = 0; st <= max_street; ++st)
    {
        streets_[st] = streets == nullptr || streets[st];
    }

    num_holdings_.reset(new int[max_street + 1]);
    for (int st = 0; st <= max_street; ++st)
    {
        if (streets && !streets[st])
        {
            num_holdings_[st] = 0;
            continue;
        }
        if (buckets.None(st))
        {
            int num_local_boards = PCardTree::NumLocalBoards(root_bd_st_, root_bd_, st);
            int num_hole_card_pairs = S_Game::NumHoleCardPairs(st);
            num_holdings_[st] = num_local_boards * num_hole_card_pairs;
        }
        else
        {
            num_holdings_[st] = buckets.NumClusters(st);
        }
    }

    street_values_.reset(new AbstractCFRStreetValues *[max_street + 1]);
    for (int st = 0; st <= max_street; ++st)
        street_values_[st] = nullptr;
}

// 初始化
CFRValues::CFRValues(const bool *players, const bool *streets, int root_bd, int root_bd_st,
                     const ClusterHands &buckets, const ActionTree *betting_tree)
{
    Initialize(players, streets, root_bd, root_bd_st, buckets);

    int num_players = S_Game::NumPlayers();
    int max_street = S_Game::MaxStreet();
    int num = num_players * (max_street + 1);
    num_nonterminals_.reset(new int[num]);
    for (int p = 0; p < num_players; ++p)
    {
        for (int st = 0; st <= max_street; ++st)
        {
            int index = p * (max_street + 1) + st;
            if (players_[p] && (streets == nullptr || streets[st]))
            {
                num_nonterminals_[index] = betting_tree->NumNonterminals(p, st);
            }
            else
            {
                num_nonterminals_[index] = 0;
            }
        }
    }
}

CFRValues::~CFRValues(void)
{
    LOG(LT_INFO_TRANS, DataModuleName.c_str(), "Delete CFRValues");
    int max_street = S_Game::MaxStreet();
    for (int st = 0; st <= max_street; ++st)
        delete street_values_[st];
}

void CFRValues::CreateStreetValues(int st, CFRValueType value_type, bool quantize)
{
    if (street_values_[st] == nullptr)
    {
        if (value_type == CFRValueType::CFR_CHAR)
        {
            street_values_[st] =
                new CFRStreetValues<unsigned char>(st, players_.get(), num_holdings_[st],
                                                   num_nonterminals_.get(), value_type);
        }
        else if (value_type == CFRValueType::CFR_SHORT)
        {
            if (quantize)
            {
                street_values_[st] =
                    new CFRStreetValues<unsigned char>(st, players_.get(), num_holdings_[st],
                                                       num_nonterminals_.get(), value_type);
            }
            else
            {
                street_values_[st] =
                    new CFRStreetValues<unsigned short>(st, players_.get(), num_holdings_[st],
                                                        num_nonterminals_.get(), value_type);
            }
        }
        else if (value_type == CFRValueType::CFR_INT)
        {
            if (quantize)
            {
                street_values_[st] =
                    new CFRStreetValues<unsigned char>(st, players_.get(), num_holdings_[st],
                                                       num_nonterminals_.get(), value_type);
            }
            else
            {
                street_values_[st] = new CFRStreetValues<int>(st, players_.get(), num_holdings_[st],
                        num_nonterminals_.get(), value_type);
            }
        }
        else if (value_type == CFRValueType::CFR_DOUBLE)
        {
            if (quantize)
            {
                street_values_[st] =
                    new CFRStreetValues<unsigned char>(st, players_.get(), num_holdings_[st],
                                                       num_nonterminals_.get(), value_type);
            }
            else
            {
                street_values_[st] =
                    new CFRStreetValues<double>(st, players_.get(), num_holdings_[st],
                                                num_nonterminals_.get(), value_type);
            }
        }
        else
        {
            fprintf(stderr, "Unknown value type\n");
            exit(-1);
        }
    }
}

void CFRValues::ReadSingleFile_Task(Node *node, Reader ***readers, void ***decompressors, int p, int st)
{
    LOG(LT_INFO_TRANS, DataModuleName.c_str(), "SingleFile Task: pa %i st %i Begin", p, st);
    ReadSingleFile(node, readers, decompressors, p, st);
    readFile_lock_.lock();
    file_count_++;
    readFile_lock_.unlock();
    LOG(LT_INFO_TRANS, DataModuleName.c_str(), "SingleFile Task: pa %i st %i End", p, st);
}

void CFRValues::ReadSingleFile(Node *node, Reader ***readers, void ***decompressors, int p, int st)
{
    if (node->Terminal())
        return;
    int curNode_St = node->Street();
    int num_succs = node->NumSuccs();
    int pa = node->PlayerActing();
    if (street_values_[st] && pa == p && curNode_St == st)
    {
        Reader *reader = readers[p][st];
        if (reader == nullptr)
        {
            LOG(LT_ERROR_TRANS, DataModuleName.c_str(), " pa %i st %i missing file", p, st);
            exit(-1);
        }
        street_values_[st]->ReadNode(node, reader, decompressors ? decompressors[pa][st] : nullptr);
    }
    for (int s = 0; s < num_succs; ++s)
    {
        ReadSingleFile(node->IthSucc(s), readers, decompressors, p, st);
    }
}

void CFRValues::Read(Node *node, Reader ***readers, void ***decompressors, int p)
{
    if (node->Terminal())
        return;
    int st = node->Street();
    int num_succs = node->NumSuccs();
    int pa = node->PlayerActing();
    if (street_values_[st] && pa == p)
    {
        Reader *reader = readers[p][st];
        if (reader == nullptr)
        {
            LOG(LT_ERROR_TRANS, DataModuleName.c_str(), "CFRValues::Read(): pa %i st %i missing file?", pa, st);
            exit(-1);
        }
        street_values_[st]->ReadNode(node, reader, decompressors ? decompressors[pa][st] : nullptr);
    }
    for (int s = 0; s < num_succs; ++s)
    {
        Read(node->IthSucc(s), readers, decompressors, p);
    }
}

Reader *CFRValues::InitializeReader(const char *dir, int p, int st, int it,
                                    const string &action_sequence, int root_bd_st, int root_bd,
                                    bool sumprobs, CFRValueType *value_type)
{
    char buf[500];

    int t;
    for (t = 0; t < 4; ++t)
    {
        unsigned char suffix;
        if (t == 0)
        {
            suffix = 'd';
            *value_type = CFRValueType::CFR_DOUBLE;
        }
        else if (t == 1)
        {
            suffix = 'i';
            *value_type = CFRValueType::CFR_INT;
        }
        else if (t == 2)
        {
            suffix = 'c';
            *value_type = CFRValueType::CFR_CHAR;
        }
        else if (t == 3)
        {
            suffix = 's';
            *value_type = CFRValueType::CFR_SHORT;
        }
        sprintf(buf, "%s/%s.%s.%u.%u.%u.%u.p%u.%c", dir, sumprobs ? "sumprobs" : "regrets",
                action_sequence.c_str(), root_bd_st, root_bd, st, it, p, suffix);

        if (FileExists(buf))
        {
            LOG(LT_INFO_TRANS, DataModuleName.c_str(), "Read cfr file %s", buf);
            break;
        }
    }
    if (t == 4)
    {
        LOG(LT_ERROR_TRANS, DataModuleName.c_str(), "Couldn't find file street=%d | player=%d", st, p);
        return nullptr;
    }
    Reader *reader = new Reader(buf);
    return reader;
}

int CFRValues::Read_MultiThread(const char *dir, const std::vector<int> &its, const ActionTree *betting_tree,
                                const std::string &action_sequence, int only_p, const std::vector<bool> &sumprobs, bool quantize)
{
    LOG(LT_INFO_TRANS,  DataModuleName.c_str(), "Start Reading cfr values multi thread");
    int num_players = S_Game::NumPlayers();
    Reader ***readers = new Reader **[num_players];
    void ***decompressors = nullptr;
    int max_street = S_Game::MaxStreet();

    if (only_p != -1)
    {
        LOG(LT_ERROR_TRANS,  DataModuleName.c_str(), "Don't Support Only One Player Yet");
        return ERR_LOADING_CFR_NOT_FULL_PLAYER;
    }


    for (int p = 0; p < num_players; ++p)
    {
        if (!players_[p])
        {
            LOG(LT_ERROR_TRANS,  DataModuleName.c_str(), "Don't Support Not Full Player Yet");
            return ERR_LOADING_CFR_NOT_FULL_PLAYER;
        }
        readers[p] = new Reader *[max_street + 1];
        for (int st = 0; st <= max_street; ++st)
        {
            if (!streets_[st])
            {
                LOG(LT_ERROR_TRANS,  DataModuleName.c_str(), "Don't Support Not Full Street Yet");
                return ERR_LOADING_CFR_NOT_FULL_STREET;
            }
            CFRValueType value_type;

            readers[p][st] = InitializeReader(dir, p, st, its[st], action_sequence, root_bd_st_, root_bd_,
                                              sumprobs[st], &value_type);
            if (readers[p][st] == nullptr)
            {
                return ERR_LOADING_CFR_NO_FILE;
            }

            if (street_values_[st] == nullptr)
            {
                CreateStreetValues(st, value_type, quantize);
            }
        }
    }
    file_count_ = 0;
    for (int p = 0; p < num_players; ++p)
    {
        int max_street = S_Game::MaxStreet();
        for (int st = 0; st < max_street + 1; st++)
        {
            std::thread t(&CFRValues::ReadSingleFile_Task, this, betting_tree->Root(), readers, decompressors, p, st);
            t.detach();
        }
    }
    int street_num = S_Game::MaxStreet() + 1;
    int player_num = S_Game::NumPlayers();

    while (true)
    {
        if (file_count_ == street_num * player_num)
        {
            for (int p = 0; p < num_players; ++p)
            {
                for (int st = 0; st <= max_street; ++st)
                {
                    if (!readers[p][st]->AtEnd())
                    {
                        LOG(LT_INFO_TRANS,  DataModuleName.c_str(), "Reader didn't get to end st=%d| p=%d|Pos= %lli| File size=%lli",
                            st, p, readers[p][st]->BytePos(), readers[p][st]->FileSize());
                        return ERR_LOADING_CFR_NOT_THE_END;
                    }
                    delete readers[p][st];
                }
                delete[] readers[p];
            }
            delete[] readers;
            delete[] decompressors;
            LOG(LT_INFO_TRANS,  DataModuleName.c_str(), "Read Total %d Files Done", file_count_);
            return ERR_OK;
        }
        else
        {
            sleep(5);
        }
    }
}



