#ifndef _NONTERMINAL_IDS_H_
#define _NONTERMINAL_IDS_H_

class ActionTree;
class Node;

void CountNumNonterminals(Node *root, int *num_nonterminals);
void CountNumNonterminals(ActionTree *betting_tree, int *num_nonterminals);

#endif
