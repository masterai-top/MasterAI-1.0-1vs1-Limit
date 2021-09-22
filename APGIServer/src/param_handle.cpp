#include "param_handle.h"
#include "string_utils.h"

void ParamHandle::GetActionParamStr(const ActionParam &param, std::string &param_str)
{
    char rule_buf[512];
    snprintf(rule_buf, sizeof(rule_buf) - 1, "[(rule_type,filter_flag,pot_th,frm_th,handle_nfrm)"
             "(%d,%d,%0.3f,%0.3f,%d),(%d,%d,%0.3f,%0.3f,%d),(%d,%d,%0.3f,%0.3f,%d),(%d,%d,%0.3f,%0.3f,%d)]",
             param.rule_types[0], (int)(param.filter_flags[0]), param.pot_ths[0], param.frm_ths[0], (int)(param.handle_nfrom[0]),
             param.rule_types[1], (int)(param.filter_flags[1]), param.pot_ths[1], param.frm_ths[1], (int)(param.handle_nfrom[0]),
             param.rule_types[2], (int)(param.filter_flags[2]), param.pot_ths[2], param.frm_ths[2], (int)(param.handle_nfrom[0]),
             param.rule_types[3], (int)(param.filter_flags[3]), param.pot_ths[3], param.frm_ths[3], (int)(param.handle_nfrom[0]));

    param_str = std::string(rule_buf);
}

int ParamHandle::SetActionParam(const std::string &param_str, ActionParam &param)
{

    std::string str_frm_th = StringUtils::GetValueBykey(param_str.c_str(),
                             LABEL_FRM_TH.c_str());
    std::vector<double> tmp_frm_ths;
    int flag = ParseDoubles(str_frm_th, NUM_STREET, tmp_frm_ths);
    if (flag != 0)
    {
        return (-1000 + flag);
    }
    for(int i = 0; i < NUM_STREET; i++)
    {
        param.frm_ths[i] = tmp_frm_ths[i];
    }

    std::vector<bool> tmp_filter_flags;
    std::string str_filter_flag = StringUtils::GetValueBykey(param_str.c_str(),
                                  LABEL_FILTER_FLAG.c_str());
    flag = ParseBools(str_filter_flag, NUM_STREET, tmp_filter_flags);
    if (flag != 0)
    {
        return (-2000 + flag);
    }
    for(int i = 0; i < NUM_STREET; i++)
    {
        param.filter_flags[i] = tmp_filter_flags[i];
    }

    std::vector<double> tmp_pot_ths;
    std::string str_pot_th = StringUtils::GetValueBykey(param_str.c_str(),
                             LABEL_POT_TH.c_str());
    std::vector<double> tmp_pot_th_vals;
    flag = ParseDoubles(str_pot_th, NUM_STREET, tmp_pot_ths);
    if (flag != 0)
    {
        return (-3000 + flag);
    }
    for(int i = 0; i < NUM_STREET; i++)
    {
        param.pot_ths[i] = tmp_pot_ths[i];
    }

    std::vector<int> tmp_rule_types;
    std::string str_rule_type = StringUtils::GetValueBykey(param_str.c_str(),
                                LABEL_RULE_TYPE.c_str());
    flag = ParseInts(str_rule_type, NUM_STREET, tmp_rule_types);
    if (flag != 0)
    {
        return (-4000 + flag);
    }
    for(int i = 0; i < NUM_STREET; i++)
    {
        param.rule_types[i] = tmp_rule_types[i];
    }


    std::vector<bool> tmp_handle_nfrms;
    std::string str_handle_nfrm = StringUtils::GetValueBykey(param_str.c_str(),
                                  LABEL_HANDLE_NFRM.c_str());
    flag = ParseBools(str_handle_nfrm, NUM_STREET, tmp_handle_nfrms);
    if (flag != 0)
    {
        return (-5000 + flag);
    }
    for(int i = 0; i < NUM_STREET; i++)
    {
        param.handle_nfrom[i] = tmp_handle_nfrms[i];
    }

    return 0;
}





int ParamHandle::ParseInts(const std::string &int_str, int count, std::vector<int> &int_vals)
{
    std::vector<std::string> temp_split_str;
    StringUtils::SplitString(int_str, temp_split_str, LABEL_STREET_SEPATOR);
    if (int(temp_split_str.size()) != count)
    {
        return -2;//ERR_WRONG_PARAM_CNT;
    }

    int_vals.resize(count);
    for (int i = 0; i < count; i++)
    {
        int temp_val;
        if (sscanf(temp_split_str[i].c_str(), "%i", &temp_val) != 1)
        {
            return -1;//-1;
        }
        int_vals[i] = temp_val;
    }
    return 0;//0;
}

int ParamHandle::ParseBools(const std::string &bool_str, int count, std::vector<bool> &bool_vals)
{
    std::vector<std::string> temp_split_str;
    StringUtils::SplitString(bool_str, temp_split_str, LABEL_STREET_SEPATOR);
    if (int(temp_split_str.size()) != count)
    {
        return -2;//ERR_WRONG_PARAM_CNT;
    }

    bool_vals.resize(count);
    for (int i = 0; i < count; i++)
    {
        int temp_val;
        if (sscanf(temp_split_str[i].c_str(), "%i", &temp_val) != 1)
        {
            return -1;//-1;
        }
        if (temp_val == 1)
        {
            bool_vals[i] = true;
        }
        else if (temp_val == 0)
        {
            bool_vals[i] = false;
        }
        else
        {
            return -1;//-1;
        }
    }
    return 0;//0;
}

int ParamHandle::ParseDoubles(const std::string &double_str, int count, std::vector<double> &double_vals)
{
    std::vector<std::string> temp_split_str;
    StringUtils::SplitString(double_str, temp_split_str, LABEL_STREET_SEPATOR);
    if (int(temp_split_str.size()) != count)
    {
        return -2;//ERR_WRONG_PARAM_CNT;
    }

    double_vals.resize(count);
    for (int i = 0; i < count; i++)
    {
        double temp_val;
        if (sscanf(temp_split_str[i].c_str(), "%lf", &temp_val) != 1)
        {
            return -1;//-1;
        }
        double_vals[i] = temp_val;
    }
    return 0;//0;
}