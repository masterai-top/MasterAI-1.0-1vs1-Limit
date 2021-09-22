#include <stdio.h>
#include <stdlib.h>

#include "action_tree.h"
#include "s_game.h"
#include "nonterminal_ids.h"

static void CountNumNonterminals2(Node *node, int *num_nonterminals)
{
    if (node->Terminal())
        return;
    int pa = node->PlayerActing();
    int st = node->Street();
    int nt_id = node->NonterminalID();
    int index = pa * (S_Game::MaxStreet() + 1) + st;
    if (nt_id >= num_nonterminals[index])
    {
        num_nonterminals[index] = nt_id + 1;
    }
    int num_succs = node->NumSuccs();
    for (int s = 0; s < num_succs; ++s)
    {
        CountNumNonterminals2(node->IthSucc(s), num_nonterminals);
    }
}

void CountNumNonterminals(Node *root, int *num_nonterminals)
{
    int max_street = S_Game::MaxStreet();
    int num_players = S_Game::NumPlayers();
    int num = num_players * (max_street + 1);
    for (int i = 0; i < num; ++i)
        num_nonterminals[i] = 0;
    CountNumNonterminals2(root, num_nonterminals);
}

void CountNumNonterminals(ActionTree *betting_tree, int *num_nonterminals)
{
    CountNumNonterminals(betting_tree->Root(), num_nonterminals);
}
