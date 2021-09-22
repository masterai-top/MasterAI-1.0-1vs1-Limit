#ifndef _ACTION_IDX_SELECTOR_H
#define _ACTION_IDX_SELECTOR_H

#include <vector>


class ActionIdxSelector
{
public:
    static int GetPureIdx(const std::vector<unsigned int> &vals,bool sumprob_flag);
    static int FRMChooseCandidate(const std::vector<unsigned int> &vals, bool sumprob_flag, double filter_th,
                     std::vector<std::pair<int, unsigned int>> &candidate_action);

    static int ChooseIdxByRm(const std::vector<std::pair<int,double>> &candidate_action);

    static int ChooseIdxByAllRandom(const std::vector<std::pair<int, unsigned int>> &candidate_action);

    static int ChooseIdxByMinReg(const std::vector<std::pair<int, unsigned int>> &candidate_action);
private:
    ActionIdxSelector(){}
    ~ActionIdxSelector(){}
};



#endif