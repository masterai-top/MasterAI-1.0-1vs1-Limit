
#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "cards.h"
#include "s_game.h"
#include "private_cards.h"

using std::vector;

int GetPrivateHandIdxWithTotalCards(int st, const Card *cards)
{
  int num_hole_cards = S_Game::NumCardsForStreet(0);
  const Card *board = cards + num_hole_cards;
  int num_board_cards = S_Game::NumBoardCards(st);
  if (num_hole_cards == 1)
  {
    int c = cards[0];
    int num_board_lower = 0;
    for (int i = 0; i < num_board_cards; ++i)
    {
      if ((int)board[i] < c)
        ++num_board_lower;
    }
    return c - num_board_lower;
  }
  else
  {
    Card hi = cards[0];
    Card lo = cards[1];
    int num_lower_lo = 0, num_lower_hi = 0;
    for (int i = 0; i < num_board_cards; ++i)
    {
      Card c = board[i];
      if (c < lo)
      {
        ++num_lower_lo;
        ++num_lower_hi;
      }
      else if (c < hi)
      {
        ++num_lower_hi;
      }
    }
    int hi_index = hi - num_lower_hi;
    int lo_index = lo - num_lower_lo;
    // The sum from 1... hi_index - 1 is the number of hole card pairs
    // containing a high card less than hi.
    return (hi_index - 1) * hi_index / 2 + lo_index;
  }
}

int GetPrivateHandIdxWithTotalCards(int st, const Card *board, const Card *hole_cards)
{
  int num_hole_cards = S_Game::NumCardsForStreet(0);
  int num_board_cards = S_Game::NumBoardCards(st);
  if (num_hole_cards == 1)
  {
    int c = hole_cards[0];
    int num_board_lower = 0;
    for (int i = 0; i < num_board_cards; ++i)
    {
      if ((int)board[i] < c)
        ++num_board_lower;
    }
    return c - num_board_lower;
  }
  else
  {
    Card hi = hole_cards[0];
    Card lo = hole_cards[1];
    int num_lower_lo = 0, num_lower_hi = 0;
    for (int i = 0; i < num_board_cards; ++i)
    {
      Card c = board[i];
      if (c < lo)
      {
        ++num_lower_lo;
        ++num_lower_hi;
      }
      else if (c < hi)
      {
        ++num_lower_hi;
      }
    }
    int hi_index = hi - num_lower_hi;
    int lo_index = lo - num_lower_lo;
    return (hi_index - 1) * hi_index / 2 + lo_index;
  }
}
