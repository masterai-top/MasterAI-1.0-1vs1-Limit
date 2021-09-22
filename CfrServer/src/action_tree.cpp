#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "abstraction_def.h"
#include "action_tree.h"
#include "global_def.h"
#include "files.h"
#include "s_game.h"
#include "reader.h"
#include "nonterminal_ids.h"
#include "pch.h"

using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::vector;

Node::Node(int id, int street, int player_acting, const shared_ptr<Node> &call_succ,
           const shared_ptr<Node> &fold_succ, vector<shared_ptr<Node>> *bet_succs,
           int num_remaining, int bet_to)
{
    int num_succs = 0;
    if (call_succ)
    {
        ++num_succs;
    }
    if (fold_succ)
    {
        ++num_succs;
    }
    int num_bet_succs = 0;
    if (bet_succs)
    {
        num_bet_succs = bet_succs->size();
        num_succs += num_bet_succs;
    }
    if (num_succs > 0)
    {
        succs_.reset(new shared_ptr<Node>[num_succs]);
        int i = 0;
        if (call_succ)
            succs_[i++] = call_succ;
        if (fold_succ)
            succs_[i++] = fold_succ;
        for (int j = 0; j < num_bet_succs; ++j)
        {
            succs_[i++] = (*bet_succs)[j];
        }
    }
    id_ = id;
    last_bet_to_ = bet_to;
    num_succs_ = num_succs;
    flags_ = 0;
    if (call_succ)
        flags_ |= kHasCallSuccFlag;
    if (fold_succ)
        flags_ |= kHasFoldSuccFlag;
    flags_ |= (((unsigned short)street) << kStreetShift);
    if (player_acting > 255)
    {
        LOG(LT_ERROR_TRANS, DataModuleName.c_str(), "player_acting OOB: %u", player_acting);
        exit(-1);
    }
    if (num_remaining > 255)
    {
        LOG(LT_ERROR_TRANS, DataModuleName.c_str(), "num_remaining OOB: %u", num_remaining);
        exit(-1);
    }
    player_acting_ = player_acting;
    num_remaining_ = num_remaining;
}

Node::Node(Node *src)
{
    int num_succs = src->NumSuccs();
    if (num_succs > 0)
    {
        succs_.reset(new shared_ptr<Node>[num_succs]);
    }
    for (int s = 0; s < num_succs; ++s)
        succs_[s] = NULL;
    id_ = src->id_;
    last_bet_to_ = src->last_bet_to_;
    num_succs_ = src->num_succs_;
    flags_ = src->flags_;
    player_acting_ = src->player_acting_;
    num_remaining_ = src->num_remaining_;
}

Node::Node(int id, int last_bet_to, int num_succs, unsigned short flags,
           unsigned char player_acting, unsigned char num_remaining)
{
    id_ = id;
    last_bet_to_ = last_bet_to;
    num_succs_ = num_succs;
    if (num_succs > 0)
    {
        succs_.reset(new shared_ptr<Node>[num_succs]);
    }
    for (int s = 0; s < num_succs; ++s)
        succs_[s] = nullptr;
    flags_ = flags;
    player_acting_ = player_acting;
    num_remaining_ = num_remaining;
}


int Node::CallSuccIndex(void) const
{
    if (HasCallSucc())
        return 0;
    else
        return -1;
}

int Node::FoldSuccIndex(void) const
{
    if (HasFoldSucc())
    {
        if (HasCallSucc())
            return 1;
        else
            return 0;
    }
    else
    {
        return -1;
    }
}


int Node::DefaultSuccIndex(void) const
{
    if (HasFoldSucc())
    {
        return FoldSuccIndex();
    }
    else
    {
        return 0;
    }
}

bool Node::StreetInitial(void) const
{
    if (Terminal())
        return false;
    int csi = CallSuccIndex();
    // Not sure this can happen
    if (csi == -1)
        return false;
    Node *c = IthSucc(csi);
    return (!c->Terminal() && c->Street() == Street());
}

int ActionTree::NumNonterminals(int p, int st) const
{
    return num_nonterminals_[p * (S_Game::MaxStreet() + 1) + st];
}

void ActionTree::FillTerminalArray(Node *node)
{
    if (node->Terminal())
    {
        int terminal_id = node->TerminalID();
        if (terminal_id >= num_terminals_)
        {
            LOG(LT_ERROR_TRANS, DataModuleName.c_str(), "Out of bounds terminal ID: %i (num terminals %i)",
                terminal_id, num_terminals_);
            exit(-1);
        }
        terminals_[terminal_id] = node;
        return;
    }
    for (int i = 0; i < node->NumSuccs(); ++i)
    {
        FillTerminalArray(node->IthSucc(i));
    }
}

void ActionTree::FillTerminalArray(void)
{
    terminals_.reset(new Node *[num_terminals_]);
    if (root_.get())
        FillTerminalArray(root_.get());
}

shared_ptr<Node> ActionTree::Read(Reader *reader, unordered_map<int, shared_ptr<Node>> *maps)
{
    int id = reader->ReadUnsignedIntOrDie();
    unsigned short last_bet_to = reader->ReadUnsignedShortOrDie();
    unsigned short num_succs = reader->ReadUnsignedShortOrDie();
    unsigned short flags = reader->ReadUnsignedShortOrDie();
    unsigned char pa = reader->ReadUnsignedCharOrDie();
    unsigned char num_remaining = reader->ReadUnsignedCharOrDie();
    int st = (int)((flags & Node::kStreetMask) >> Node::kStreetShift);
    int map_index = st * S_Game::NumPlayers() + pa;
    unordered_map<int, shared_ptr<Node>> *m = &maps[map_index];
    if (num_succs > 0)
    {

        unordered_map<int, shared_ptr<Node>>::iterator it;
        it = m->find(id);
        if (it != m->end())
            return it->second;
    }
    shared_ptr<Node> node(new Node(id, last_bet_to, num_succs, flags, pa, num_remaining));
    if (num_succs == 0)
    {
        ++num_terminals_;
        return node;
    }
    (*m)[id] = node;
    for (int s = 0; s < num_succs; ++s)
    {
        shared_ptr<Node> succ(Read(reader, maps));
        node->SetIthSucc(s, succ);
    }
    return node;
}

void ActionTree::Initialize(int target_player, const ActionAbstraction &ba)
{
    char buf[500];

    sprintf(buf, "%s/betting_tree.%s.%u.%s", Files::StaticBase(),
            S_Game::GameName().c_str(), S_Game::NumPlayers(),
            ba.ActionAbstractionName().c_str());
    Reader reader(buf);
    root_ = nullptr;
    num_terminals_ = 0;
    int max_street = S_Game::MaxStreet();
    int num_players = S_Game::NumPlayers();
    int num_maps = (max_street + 1) * num_players;
    unique_ptr<unordered_map<int, shared_ptr<Node>>[]>
            maps(new unordered_map<int, shared_ptr<Node>>[num_maps]);
    root_ = Read(&reader, maps.get());
    FillTerminalArray();
    num_nonterminals_.reset(new int[num_players * (max_street + 1)]);
    CountNumNonterminals(this, num_nonterminals_.get());
}

ActionTree::ActionTree(const ActionAbstraction &ba)
{
    Initialize(0, ba);
}

ActionTrees::ActionTrees(const ActionAbstraction &ba)
{
    int num_players = S_Game::NumPlayers();
    betting_trees_.reset(new shared_ptr<ActionTree>[num_players]);

    betting_trees_[0].reset(new ActionTree(ba));
    for (int p = 1; p < num_players; ++p)
    {
        betting_trees_[p] = betting_trees_[0];
    }
}

ActionTrees::~ActionTrees(void)
{
    LOG(LT_INFO_TRANS, "ActionTrees", "Delete ActionTrees");
}

const ActionTree *ActionTrees::GetBettingTree(void) const
{
    return betting_trees_[0].get();
}

Node *ActionTrees::Root(void) const
{
    return betting_trees_[0]->Root();
}

int ActionTrees::NumNonterminals(int p, int st) const
{
    return betting_trees_[0]->NumNonterminals(p, st);
}
