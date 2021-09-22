#ifndef _RULE_FILTER_H
#define _RULE_FILTER_H

#include "Def.h"

class RuleFilter
{
public:
    static int FilterFlopLargeBetAction(const ActionInfo &action_info, const NodeInfo &node_info,
                                 const std::vector<unsigned int> &reg_vals,const LogInfo &log_info,
                                 int action_idx, double pot_th);


    static int FilterTurnLargeBetAction(const ActionInfo &action_info, const NodeInfo &node_info,
                                 const std::vector<unsigned int> &reg_vals,const LogInfo &log_info,
                                 int action_idx, double pot_th);

    static int FilterAllinAction(const ActionInfo &action_info, const NodeInfo &node_info,
                                 const std::vector<unsigned int> &reg_vals,const LogInfo &log_info,
                                 int action_idx, double filter_th,bool handle_nfrm);

    static int FilterActionToSmallBetAndRandom(const ActionInfo &action_info, const NodeInfo &node_info,
                                               const std::vector<unsigned int> &reg_vals, 
                                               const LogInfo &log_info,
                                               int action_idx,double pot_th, double filter_th);

    static int FilterLargeBetAction(const ActionInfo &action_info, const NodeInfo &node_info,
                                 const std::vector<unsigned int> &reg_vals,const LogInfo &log_info,
                                 int action_idx, double pot_th);
    
    
private:

    static double GetPotFrac(const std::vector<std::string> &child_nodes,int action_idx,int old_bet_to);
    RuleFilter(/* args */){}
    ~RuleFilter(){}
};



#endif