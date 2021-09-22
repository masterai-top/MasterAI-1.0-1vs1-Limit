#ifndef _HAND_TREE_H_
#define _HAND_TREE_H_

#include "p_card_tree.h"
#include "cards.h"

int GetPrivateHandIdxWithTotalCards(int st, const Card *cards);
int GetPrivateHandIdxWithTotalCards(int st, const Card *board, const Card *hole_cards);

#endif
