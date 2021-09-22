#include <stdio.h>
#include <stdlib.h>

#include "p_card_tree.h"
#include "cluster_hands.h"
#include "abstraction_def.h"
#include "files.h"
#include "s_game.h"
#include "reader.h"
#include "pch.h"

ClusterHands::ClusterHands(const CardAbstraction &ca, bool numb_only)
{
    PCardTree::Create();
    int max_street = S_Game::MaxStreet();
    none_.reset(new bool[max_street + 1]);
    short_flag_ = new unsigned short *[max_street + 1];
    int_cluster_ = new int *[max_street + 1];
    for (int st = 0; st <= max_street; ++st)
    {
        short_flag_[st] = nullptr;
        int_cluster_[st] = nullptr;
    }
    char buf[500];
    num_cluster_.reset(new int[max_street + 1]);
    for (int st = 0; st <= max_street; ++st)
    {
        if (ca.Bucketing(st) == "none")
        {
            none_[st] = true;
            num_cluster_[st] = 0;
            continue;
        }
        none_[st] = false;
        sprintf(buf, "%s/num_buckets.%s.%i.%i.%i.%s.%i", Files::StaticBase(), S_Game::GameName().c_str(),
                S_Game::NumRanks(), S_Game::NumSuits(), max_street, ca.Bucketing(st).c_str(), st);
        Reader reader(buf);
        num_cluster_[st] = reader.ReadIntOrDie();
    }

    if (!numb_only)
    {
        for (int st = 0; st <= max_street; ++st)
        {
            if (none_[st])
                continue;
            int num_boards = PCardTree::NumBoards(st);
            int num_hole_card_pairs = S_Game::NumHoleCardPairs(st);
            // Need to use an unsigned int for this
            unsigned int num_hands = (unsigned int)num_boards * (unsigned int)num_hole_card_pairs;
            long long int lli_num_hands = num_hands;

            sprintf(buf, "%s/buckets.%s.%i.%i.%i.%s.%i", Files::StaticBase(), S_Game::GameName().c_str(),
                    S_Game::NumRanks(), S_Game::NumSuits(), max_street, ca.Bucketing(st).c_str(), st);
            Reader reader(buf);
            long long int file_size = reader.FileSize();
            if (file_size == lli_num_hands * 2)
            {
                short_flag_[st] = new unsigned short[num_hands];
                for (unsigned int h = 0; h < num_hands; ++h)
                {
                    short_flag_[st][h] = reader.ReadUnsignedShortOrDie();
                }
            }
            else if (file_size == lli_num_hands * 4)
            {
                int_cluster_[st] = new int[num_hands];
                for (unsigned int h = 0; h < num_hands; ++h)
                {
                    int_cluster_[st][h] = reader.ReadIntOrDie();
                }
            }
            else
            {
                fprintf(stderr, "BucketsInstance::Initialize: Unexpected file size %lli\n", file_size);
                exit(-1);
            }
        }
    }
}

ClusterHands::ClusterHands(void)
{
    short_flag_ = nullptr;
    int_cluster_ = nullptr;
}

ClusterHands::~ClusterHands(void)
{
    LOG(LT_INFO_TRANS, "ClusterHands", "Delete ClusterHands");

    int max_street = S_Game::MaxStreet();
    if (short_flag_)
    {
        for (int st = 0; st <= max_street; ++st)
        {
            if (short_flag_[st])
                delete[] short_flag_[st];
        }
        delete[] short_flag_;
    }
    if (int_cluster_)
    {
        for (int st = 0; st <= max_street; ++st)
        {
            if (int_cluster_[st])
                delete[] int_cluster_[st];
        }
        delete[] int_cluster_;
    }
}
