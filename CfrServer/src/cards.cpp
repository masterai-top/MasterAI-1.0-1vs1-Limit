#include <stdio.h>
#include <stdlib.h>

#include <string>

#include "cards.h"

using std::string;

std::string getCardsString(const Card *cards, int n)
{
    std::string cardString;
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            cardString += " ";
        }

        int rank = Rank(cards[i]);
        int suit = Suit(cards[i]);

        if (rank < 8)
        {
            cardString += std::to_string((int)rank + 2);
        }
        else if (rank == 8)
        {
            cardString += "T";
        }
        else if (rank == 9)
        {
            cardString += "J";
        }
        else if (rank == 10)
        {
            cardString += "Q";
        }
        else if (rank == 11)
        {
            cardString += "K";
        }
        else if (rank == 12)
        {
            cardString += "A";
        }
        else
        {
            fprintf(stderr, "Illegal rank %i\n", rank);
            exit(-1);
        }

        switch (suit)
        {
        case 0:
            cardString += "c";
            break;
        case 1:
            cardString += "d";
            break;
        case 2:
            cardString += "h";
            break;
        case 3:
            cardString += "s";
            break;
        default:
            fprintf(stderr, "Illegal suit\n");
            exit(-1);
        }
    }
    return cardString;
}

Card MakeCard(int rank, int suit)
{
    return rank * S_Game::NumSuits() + suit;
}

bool InCards(Card c, const Card *cards, int num_cards)
{
    for (int i = 0; i < num_cards; ++i)
    {
        if (c == cards[i])
            return true;
    }

    return false;
}
