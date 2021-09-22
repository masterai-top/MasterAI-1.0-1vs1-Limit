#ifndef _MATCHSTATE_CONVERTOR_H
#define _MATCHSTATE_CONVERTOR_H

#include "Def.h"
#include "game.h"

class MatchStateConverotr
{
public:
    static int GetActionInfo(const MatchState &matchstate,ActionInfo &info);

    static int ConvertStrToMatchState(const char *line,MatchState &matchstate,const Game *acpc_game);
private:
    static void GetActionSeq(const MatchState &matchstate,ActionInfo &info);
    static std::string GetSingleActionStr(Action action);
    static void GetCards(const MatchState &matchstate,ActionInfo &info);
    MatchStateConverotr() {}
    ~MatchStateConverotr() {}
};





#endif