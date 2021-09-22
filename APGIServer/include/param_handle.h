#ifndef _PARAM_HANDLE_H
#define _PARAM_HANDLE_H
#include "Def.h"

class ParamHandle
{
public:
    static int SetActionParam(const std::string &param_str,ActionParam &param);

    static void GetActionParamStr(const ActionParam &param,std::string &param_str);

private:
    static int ParseInts(const std::string &int_str,int count,std::vector<int> &int_vals);

    static int ParseBools(const std::string &bool_str,int count,std::vector<bool> &bool_vals);

    static int ParseDoubles(const std::string &double_str,int count,std::vector<double> &double_vals);

    ParamHandle(){}
    ~ParamHandle(){}
};


#endif