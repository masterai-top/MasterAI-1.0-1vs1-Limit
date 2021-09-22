#include "action_idx_selector.h"
#include "error_code.h"
#include "rng_generator.h"

int ActionIdxSelector::GetPureIdx(const std::vector<unsigned int> &vals,bool sumprob_flag)
{
    if (sumprob_flag)
    {
        int pure_idx = 0;
        unsigned int pure_val = 0;
        int cnt = int(vals.size());
        for (int i = 0; i < cnt; i++)
        {
            if (vals[i] > pure_val)
            {
                pure_val = vals[i];
                pure_idx = i;
            }
        }
        return pure_idx;
    }
    else
    {
        int cnt = int(vals.size());
        int pure_idx = 0;
        for (int i = 0; i < cnt; i++)
        {
            if (vals[i] == 0)
            {
                pure_idx = i;
                break;
            }
        }
        return pure_idx;
    }
    
}

int ActionIdxSelector::FRMChooseCandidate(const std::vector<unsigned int> &vals, bool sumprob_flag, double filter_th,
                          std::vector<std::pair<int, unsigned int>> &candidate_action)
{
    int cnt = vals.size();
    unsigned int max_val = 0;
    for(int i = 0;i< cnt;i++)
    {
        if(vals[i] > max_val)
        {
            max_val = vals[i];
        }
    }
    
    unsigned int val_th = (unsigned int)(max_val * filter_th);
    candidate_action.clear();
    if (sumprob_flag)
    {        
        for (int i = 0; i < cnt; i++)
        {
            if (vals[i] >= val_th)
            {
                candidate_action.push_back(std::make_pair(i,vals[i]));
            }            
        }                
    }
    else
    {
        for (int i = 0; i < cnt; i++)
        {
            if (vals[i] <= val_th)
            {
                candidate_action.push_back(std::make_pair(i, vals[i]));
            }
        }
    }

    return ERR_OK;
}

int ActionIdxSelector::ChooseIdxByRm(const std::vector<std::pair<int,double>> &candidate_action)
{
    double r = RngGenerator::GetRng();
	double cum = 0;
	int s = 0;
    int candidate_cnt = int(candidate_action.size());
	for (s = 0; s < candidate_cnt - 1; s++)
	{
		cum += candidate_action[s].second;
		if (r < cum)
			break;
	}    
	return candidate_action[s].first;    
}

int ActionIdxSelector::ChooseIdxByAllRandom(const std::vector<std::pair<int, unsigned int>> &candidate_action)
{
    int cnt = int(candidate_action.size());
    double rnd = RngGenerator::GetRng();
    if (rnd == 1)
    {
        return cnt - 1;
    }
    int candidate_idx = (cnt * rnd);
    return candidate_action[candidate_idx].first;
}

int ActionIdxSelector::ChooseIdxByMinReg(const std::vector<std::pair<int, unsigned int>> &candidate_action)
{
    int min_reg_idx = 0;
    unsigned int min_reg_val = 2000000000U;
    for (size_t i = 0; i < candidate_action.size(); i++)
    {
        if (candidate_action[i].second < min_reg_val)
        {
            min_reg_val = candidate_action[i].second;
            min_reg_idx = candidate_action[i].first;
        }        
    }
    return min_reg_idx;
    
}