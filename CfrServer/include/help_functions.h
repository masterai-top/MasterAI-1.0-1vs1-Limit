#ifndef _HELP_FUNCTIONS_H_
#define _HELP_FUNCTIONS_H_

#include "cards.h"
#include <string>
#include <vector>

void Split(const char *line, char sep, bool allow_empty, std::vector<std::string> *comps);
void ParseInts(const std::string &s, std::vector<int> *values);

void SortCards(Card *cards, unsigned int n);




#endif