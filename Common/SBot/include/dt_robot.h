#ifndef _DT_ROBOT_H
#define _DT_ROBOT_H
#include "Def.h"

class DTRobot
{
public:
    static int DoAction(const ActionParam &action_param, const ModelParam &model_param,
                        const ActionInfo &action_info, const NodeInfo &node_info,
                        const std::vector<unsigned int> &vals, const LogInfo &log_info, 
                        std::string &action_str,std::string &rate_str);

    static int Init(const ModelParam &param);

    static int GetKey(const ActionInfo &info,const ModelParam &model_param,const NodeInfo &node_info,CFRKey &key);

private:
    static void PrintGameInfo();
    static bool CheckIfGameValid();
    static int GetBucketIdx(const ActionInfo &info,int &buck_idx);
    static void FormatStr(const std::vector<std::string> &child_nodes,
                          const std::vector<unsigned int> &vals, std::string &format_str);
    
    static bool CheckCards(const ActionInfo &info);

    DTRobot(/* args */) {}
    ~DTRobot() {}
};

#endif