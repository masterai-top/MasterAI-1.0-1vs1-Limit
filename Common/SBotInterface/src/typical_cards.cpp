
#include <algorithm>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <memory>

#include "typical_cards.h"
#include "cards.h"
#include "s_game.h"



#include "global_variant.h"
#include "help_functions.h"

using std::unique_ptr;
using std::vector;

TypicalCards::TypicalCards(int n, const Card *previous, int num_previous,
                               int previous_suit_groups, bool maintain_suit_groups)
{
  n_ = n;
  suit_groups_.reset(nullptr);
  hand_values_.reset(nullptr);
  int max_card = S_Game::MaxCard();
  int index = 0;
  int num_remaining = S_Game::NumCardsInDeck() - num_previous;
  num_canon_ = 0;
  if (n == 1)
  {
    cards_.reset(new Card[num_remaining]);
    num_variants_.reset(new unsigned char[num_remaining]);
    canon_.reset(new int[num_remaining]);
    if (maintain_suit_groups)
    {
      suit_groups_.reset(new int[num_remaining]);
    }
    Card canon_cards[1];
    for (int c = 0; c <= max_card; ++c)
    {
      if (InCards(c, previous, num_previous))
        continue;
      cards_[index] = c;
      int num_mappings = NumMappings(&cards_[index], n,
                                     previous_suit_groups);
      num_variants_[index] = num_mappings;
      short encoding;
      if (num_mappings == 0)
      {
        ToTypical(&cards_[index], n, previous_suit_groups, canon_cards);
        encoding = canon_cards[0];
      }
      else
      {
        encoding = c;
        ++num_canon_;
      }
      canon_[index] = encoding;
      if (maintain_suit_groups)
      {
        UpdateSuitGroups(&cards_[index], n_, previous_suit_groups,
                         &suit_groups_[index]);
      }
      ++index;
    }
    if (index != num_remaining)
    {
      fprintf(stderr, "Index too large: %u nr %u np %u mc %u "
                      "prev %i %i %i %i %i\n",
              index, num_remaining, num_previous, max_card,
              (int)previous[0], (int)previous[1], (int)previous[2],
              (int)previous[3], (int)previous[4]);
      exit(-1);
    }
  }
  else if (n == 2)
  {
    int num = num_remaining * (num_remaining - 1) / 2;
    cards_.reset(new Card[2 * num]);
    num_variants_.reset(new unsigned char[num]);
    canon_.reset(new int[num]);
    if (maintain_suit_groups)
    {
      suit_groups_.reset(new int[num]);
    }

    Card canon_cards[2];
    for (int hi = 1; hi <= max_card; ++hi)
    {
      if (InCards(hi, previous, num_previous))
        continue;
      for (int lo = 0; lo < hi; ++lo)
      {
        if (InCards(lo, previous, num_previous))
          continue;
        cards_[index * 2] = hi;
        cards_[index * 2 + 1] = lo;
        int num_mappings = NumMappings(&cards_[index * 2], n,
                                       previous_suit_groups);
        num_variants_[index] = num_mappings;
        short encoding;
        if (num_mappings == 0)
        {
          ToTypical(&cards_[index * 2], n, previous_suit_groups, canon_cards);
          encoding = canon_cards[0] * (max_card + 1) + canon_cards[1];
        }
        else
        {
          encoding = hi * (max_card + 1) + lo;
          ++num_canon_;
        }
        canon_[index] = encoding;
        if (maintain_suit_groups)
        {
          UpdateSuitGroups(&cards_[index * 2], n_, previous_suit_groups,
                           &suit_groups_[index]);
        }
        ++index;
      }
    }
    if (index != num)
    {
      fprintf(stderr, "Index too large n 2 np %u: %u num %u\n",
              num_previous, index, num);
      exit(-1);
    }
  }
  else if (n == 3)
  {
    int num =
        num_remaining * (num_remaining - 1) * (num_remaining - 2) / 6;
    cards_.reset(new Card[3 * num]);
    num_variants_.reset(new unsigned char[num]);
    canon_.reset(new int[num]);
    if (maintain_suit_groups)
    {
      suit_groups_.reset(new int[num]);
    }
    Card canon_cards[3];
    for (int hi = 2; hi <= max_card; ++hi)
    {
      if (InCards(hi, previous, num_previous))
        continue;
      for (int mid = 1; mid < hi; ++mid)
      {
        if (InCards(mid, previous, num_previous))
          continue;
        for (int lo = 0; lo < mid; ++lo)
        {
          if (InCards(lo, previous, num_previous))
            continue;
          cards_[index * 3] = hi;
          cards_[index * 3 + 1] = mid;
          cards_[index * 3 + 2] = lo;
          int num_mappings = NumMappings(&cards_[index * 3], n,
                                         previous_suit_groups);
          num_variants_[index] = num_mappings;
          int encoding;
          if (num_mappings == 0)
          {
            ToTypical(&cards_[index * 3], n, previous_suit_groups, canon_cards);
            encoding = canon_cards[0] * (max_card + 1) * (max_card + 1) +
                       canon_cards[1] * (max_card + 1) + canon_cards[2];
          }
          else
          {
            encoding = hi * (max_card + 1) * (max_card + 1) +
                       mid * (max_card + 1) + lo;
            ++num_canon_;
          }
          canon_[index] = encoding;
          if (maintain_suit_groups)
          {
            UpdateSuitGroups(&cards_[index * 3], n_, previous_suit_groups,
                             &suit_groups_[index]);
          }
          ++index;
        }
      }
    }
    if (index != num)
    {
      fprintf(stderr, "Index too large n 3: %u num %u\n", index, num);
      exit(-1);
    }
  }
  else
  {
    fprintf(stderr, "TypicalCards: n %u not supported\n", n);
    exit(-1);
  }
  num_raw_ = index;
}

TypicalCards::~TypicalCards(void)
{
}


bool TypicalCards::ToTypical2(const Card *cards, int num_cards, int suit_groups,
                              Card *canon_cards)
{
  int num_suits = S_Game::NumSuits();
  bool change_made = false;

  if (num_cards <= 0)
    return false;

  int *rank_used = new int[num_suits];

  int nsg = suit_groups;

  for (int i = 0; i != num_suits; ++i)
    rank_used[i] = 0;

  if (canon_cards != cards)
  {
    for (int i = 0; i < num_cards; ++i)
    {
      canon_cards[i] = cards[i];
    }
  }

  for (int i = 0; i < num_cards; ++i)
  {
    /* Get the current suit of the card */
    int old_suit = Suit(canon_cards[i]);
    /* Get the new suit */
    int new_suit = ((char *)&nsg)[old_suit];

    int rank = Rank(canon_cards[i]);
    rank_used[new_suit] |= 1 << rank;

    if (new_suit != old_suit)
    {
      canon_cards[i] = MakeCard(rank, new_suit);
      change_made = true;

      /* Change every other card of the same suit */
      for (int j = i + 1; j < num_cards; ++j)
      {
        if (Suit(canon_cards[j]) == old_suit)
        {
          rank = Rank(canon_cards[j]);
          canon_cards[j] = MakeCard(rank, new_suit);
        }
        else if (Suit(canon_cards[j]) == new_suit)
        {
          rank = Rank(canon_cards[j]);
          canon_cards[j] = MakeCard(rank, old_suit);
        }
      }
    }

    /* Update suit_groups */
    for (int m = 0; m < num_suits; ++m)
    {
      int n;
      for (n = 0; n < m; ++n)
      {
        if (((unsigned char *)&suit_groups)[n] ==
                ((unsigned char *)&suit_groups)[m] &&
            rank_used[n] == rank_used[m])
        {
          // Were the suits in the same group before, and they
          // now have the same cards again?  If so, recombine them.
          break;
        }
        else if (((unsigned char *)&nsg)[n] ==
                     ((unsigned char *)&nsg)[m] &&
                 rank_used[n] == rank_used[m])
        {
          // Have we found a new suit that is isomorphic, given the
          // cards we've seen so far?
          break;
        }
      }
      ((unsigned char *)&nsg)[m] = n;
    }
  }

  delete[] rank_used;

  return change_made;
}

void TypicalCards::ToTypical(const Card *cards, int num_cards, int suit_groups, Card *canon_cards)
{
  bool change_made = ToTypical2(cards, num_cards, suit_groups, canon_cards);
  if (change_made)
  {
    if (num_cards == 1)
    {
    }
    else if (num_cards == 2)
    {
      if (canon_cards[0] < canon_cards[1])
      {
        Card temp = canon_cards[0];
        canon_cards[0] = canon_cards[1];
        canon_cards[1] = temp;
      }
    }
    else if (num_cards == 3)
    {
      Card c0 = canon_cards[0];
      Card c1 = canon_cards[1];
      Card c2 = canon_cards[2];
      if (c0 >= c1 && c0 >= c2)
      {
        if (c1 >= c2)
        {
          // Don't need to do anything
        }
        else
        {
          canon_cards[1] = c2;
          canon_cards[2] = c1;
        }
      }
      else if (c1 >= c0 && c1 >= c2)
      {
        if (c0 >= c2)
        {
          canon_cards[0] = c1;
          canon_cards[1] = c0;
        }
        else
        {
          canon_cards[0] = c1;
          canon_cards[1] = c2;
          canon_cards[2] = c0;
        }
      }
      else
      {
        if (c0 >= c1)
        {
          canon_cards[0] = c2;
          canon_cards[1] = c0;
          canon_cards[2] = c1;
        }
        else
        {
          canon_cards[0] = c2;
          canon_cards[2] = c0;
        }
      }
    }
    else
    {
      fprintf(stderr, "ToTypical() only supports at most three cards currently: "
                      "num_cards %u\n",
              num_cards);
      exit(-1);
    }
    
  }
}

int NChooseK(int n, int k)
{
  int numer = 1, denom = 1;

  for (int i = 0; i < k; ++i)
  {
    numer *= n - i;
    denom *= k - i;
  }
  return numer / denom;
}

int TypicalCards::NumMappings(const Card *cards, int num_cards, int old_suit_groups)
{
  int group_used[4], suit_used[4], group_size[4];
  int num_suits = S_Game::NumSuits();
  // Start with all suits in group 0
  int new_suit_groups = old_suit_groups;
  for (int s = 0; s < num_suits; ++s)
    group_size[s] = 0;
  for (int s = 0; s < num_suits; ++s)
  {
    ++group_size[((unsigned char *)&new_suit_groups)[s]];
  }
  int start = 0;
  int num_mappings = 1;

  while (start < num_cards)
  {

    for (int s = 0; s < num_suits; ++s)
    {
      group_used[s] = 0;
      suit_used[s] = 0;
    }
    int start_rank = Rank(cards[start]);
    int end = start;
    do
    {
      Card c = cards[end];
      int s = Suit(c);
      ++suit_used[s];
      ++group_used[((unsigned char *)&new_suit_groups)[s]];
      ++end;
    } while (end < num_cards && Rank(cards[end]) == start_rank);

    for (int g = 0; g < num_suits; ++g)
    {
      int gu = group_used[g];
      if (gu == 0)
        continue;
      num_mappings *= NChooseK(group_size[g], gu);

      /* start updating group_size array */
      int t = group_size[g];
      group_size[g] = group_used[g];

      /* check that the smallest valued cards are chosen for each group */
      int s = g;
      while (1)
      {
        if (!suit_used[s])
        {
          // cards don't use the lowest valued suits - not canonical
          return 0;
        }
        if (!--group_used[g])
        {
          break;
        }
        while (((unsigned char *)&new_suit_groups)[++s] != g)
          ;
      }

      /* finish updating the grouping information */
      for (++s; s != num_suits; ++s)
      {
        if (((unsigned char *)&new_suit_groups)[s] == g)
        {
          ((unsigned char *)&new_suit_groups)[s] = s;
          group_size[s] = t - group_size[g];
          t = s;
          for (++s; s != num_suits; ++s)
          {
            if (((unsigned char *)&new_suit_groups)[s] == g)
            {
              ((unsigned char *)&new_suit_groups)[s] = t;
            }
          }
          break;
        }
      }
    }

    start = end;
  }
  return num_mappings;
}

void UpdateSuitGroups(const Card *cards, int num_cards, int old_suit_groups, int *new_suit_groups)
{
  int seen_ranks[4];
  int num_suits = S_Game::NumSuits();
  // Default of 0
  *new_suit_groups = 0;
  for (int s = 0; s < num_suits; ++s)
    seen_ranks[s] = 0;
  for (int i = 0; i < num_cards; ++i)
  {
    Card c = cards[i];
    char rank = Rank(c);
    int suit = Suit(c);
    seen_ranks[suit] |= (1 << rank);
  }

  // Suit group of the first suit (clubs) is always zero
  ((unsigned char *)new_suit_groups)[0] = 0;
  for (int s2 = 1; s2 < num_suits; ++s2)
  {
    int sr2 = seen_ranks[s2];
    int s1;
    for (s1 = 0; s1 < s2; ++s1)
    {
      if (((unsigned char *)&old_suit_groups)[s1] ==
              ((unsigned char *)&old_suit_groups)[s2] &&
          seen_ranks[s1] == sr2)
      {
        break;
      }
    }
    ((unsigned char *)new_suit_groups)[s2] = s1;
  }
}

using std::unique_ptr;

static int EncodeRanksOfRawSuitForStreet(int raw_suit, int street, const Card *raw_cards)
{
  int street_code = 0;
  int num_cards_for_street = S_Game::NumCardsForStreet(street);
  for (int i = 0; i < num_cards_for_street; ++i)
  {
    Card raw_card = raw_cards[i];
    int this_raw_suit = Suit(raw_card);
    if (this_raw_suit == raw_suit)
    {
      int rank = Rank(raw_card);
      street_code |= (1 << rank);
    }
  }
  return street_code;
}

static unsigned long long int EncodeRanksOfRawSuit(int raw_suit, const Card *raw_board,
                                                   const Card *raw_hole_cards,
                                                   int max_street)
{
  unsigned long long int code = 0;
  const Card *raw_board_ptr = raw_board;
  for (int st = 1; st <= max_street; ++st)
  {
    unsigned long long int street_code =
        EncodeRanksOfRawSuitForStreet(raw_suit, st, raw_board_ptr);
    code |= street_code << (16 * (max_street + 1 - st));
    int num_cards_for_street = S_Game::NumCardsForStreet(st);
    raw_board_ptr += num_cards_for_street;
  }
  if (raw_hole_cards)
  {
    unsigned long long int street_code =
        EncodeRanksOfRawSuitForStreet(raw_suit, 0, raw_hole_cards);
    code |= street_code;
  }
  return code;
}

void CanonicalizeCards(const Card *raw_board, const Card *raw_hole_cards, int max_street,
                       Card *canon_board, Card *canon_hole_cards, int *suit_mapping)
{
  unsigned long long int suit_codes[4];
  for (unsigned int s = 0; s < 4; ++s)
  {
    suit_codes[s] = EncodeRanksOfRawSuit(s, raw_board, raw_hole_cards, max_street);
  }
  int sorted_suits[4];
  bool used[4];
  for (int s = 0; s < 4; ++s)
    used[s] = false;
  for (int pos = 0; pos < 4; ++pos)
  {
    int best_s = -1;
    unsigned long long int best_rank_code = 0;
    for (int s = 0; s < 4; ++s)
    {
      if (used[s])
        continue;
      unsigned long long int rank_code = suit_codes[s];
      if (best_s == -1 || rank_code > best_rank_code)
      {
        best_rank_code = rank_code;
        best_s = s;
      }
    }
    sorted_suits[pos] = best_s;
    used[best_s] = true;
  }

  for (int i = 0; i < 4; ++i)
  {
    int raw_suit = sorted_suits[i];
    suit_mapping[raw_suit] = i;
  }

  
  int num_board_cards = S_Game::NumBoardCards(max_street);
  unique_ptr<Card[]> canon_street_cards(new Card[num_board_cards]);
  int index = 0;
  for (int st = 1; st <= max_street; ++st)
  {
    int num_cards_for_street = S_Game::NumCardsForStreet(st);
    for (int i = 0; i < num_cards_for_street; ++i)
    {
      Card raw_card = raw_board[index + i];
      canon_street_cards[i] = MakeCard(Rank(raw_card), suit_mapping[Suit(raw_card)]);
    }
    SortCards(canon_street_cards.get(), num_cards_for_street);
    for (int i = 0; i < num_cards_for_street; ++i)
    {
      canon_board[index + i] = canon_street_cards[i];
    }
    index += num_cards_for_street;
  }

  if (raw_hole_cards)
  {
    int num_hole_cards = S_Game::NumCardsForStreet(0);
    for (int i = 0; i < num_hole_cards; ++i)
    {
      Card raw_card = raw_hole_cards[i];
      canon_hole_cards[i] = MakeCard(Rank(raw_card), suit_mapping[Suit(raw_card)]);
    }
    SortCards(canon_hole_cards, num_hole_cards);
  }
}

void CanonicalizeCards(const Card *raw_board, const Card *raw_hole_cards, int max_street,
                       Card *canon_board, Card *canon_hole_cards)
{
  int suit_mapping[4];
  CanonicalizeCards(raw_board, raw_hole_cards, max_street, canon_board,
                    canon_hole_cards, suit_mapping);
}

