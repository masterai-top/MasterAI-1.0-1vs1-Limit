#include "help_functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

void Split(const char *line, char sep, bool allow_empty,
           vector<string> *comps)
{
  comps->clear();
  int len = strlen(line);
  int i = 0;
  // Used to have while (i < len), but this does the wrong thing when
  // allow_empty is true and the line passed in ends in a separator
  // character.
  while (true)
  {
    int j = i;
    while (i < len && line[i] != sep)
      ++i;
    string s(line, j, i - j);
    // Strip any trailing \n
    if (s.size() > 0)
    {
      if (s[s.size() - 1] == '\n')
      {
        s.resize(s.size() - 1);
      }
    }
    if (s != "" || allow_empty)
    {
      // s can be empty if there was only new line in given field
      comps->push_back(s);
    }
    if (i == len)
      break;
    ++i; // Skip past separating character
  }
}

void ParseInts(const string &s, vector<int> *values)
{
  vector<string> comps;
  Split(s.c_str(), ',', false, &comps);
  int num_comps = comps.size();
  values->resize(num_comps);
  int val;
  for (int i = 0; i < num_comps; ++i)
  {
    const string &cs = comps[i];
    if (sscanf(cs.c_str(), "%i", &val) != 1)
    {
      fprintf(stderr, "Couldn't parse int value: %s\n", s.c_str());
      exit(-1);
    }
    (*values)[i] = val;
  }
}

// Only handles n <= 3
// 将cards按照从大到小排列
void SortCards(Card *cards, unsigned int n)
{
  if (n == 1)
  {
    return;
  }
  else if (n == 2)
  {
    if (cards[0] < cards[1])
    {
      Card temp = cards[0];
      cards[0] = cards[1];
      cards[1] = temp;
    }
  }
  else if (n == 3)
  {
    Card c0 = cards[0];
    Card c1 = cards[1];
    Card c2 = cards[2];
    if (c0 > c1)
    {
      if (c1 > c2)
      {
        // c0, c1, c2
      }
      else if (c0 > c2)
      {
        // c0, c2, c1
        cards[1] = c2;
        cards[2] = c1;
      }
      else
      {
        // c2, c0, c1
        cards[0] = c2;
        cards[1] = c0;
        cards[2] = c1;
      }
    }
    else
    {
      if (c0 > c2)
      {
        // c1, c0, c2
        cards[0] = c1;
        cards[1] = c0;
      }
      else if (c1 > c2)
      {
        // c1, c2, c0
        cards[0] = c1;
        cards[1] = c2;
        cards[2] = c0;
      }
      else
      {
        // c2, c1, c0
        cards[0] = c2;
        cards[1] = c1;
        cards[2] = c0;
      }
    }
  }
  else
  {
    fprintf(stderr, "Don't know how to sort %u cards\n", n);
    exit(-1);
  }
}