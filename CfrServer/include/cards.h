#ifndef _CARDS_H_
#define _CARDS_H_

#include <string>

typedef int Card;
Card MakeCard(int rank, int suit);

#include "s_game.h"

#define Rank(card)           (card / S_Game::NumSuits())
#define Suit(card)           (card % S_Game::NumSuits())

bool InCards(Card c, const Card *cards, int num_cards);

std::string getCardsString(const Card *cards, int n);

#endif
