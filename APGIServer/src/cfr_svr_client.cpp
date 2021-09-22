#include "cfr_svr_client.h"
#include "json/json.h"
#include "string_utils.h"
#include "apgi_conf.h"
#include <memory>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int CfrSvrClient::ConvertKeyToRequestStr(const CFRKey &key, std::string &request_str, const std::string &cfrModel)
{
    char buf[1024] = {0};
    snprintf(buf, sizeof(buf), "{\"mode\":%d, \"nt\":%d, \"st\":%d, \"pa\":%d, \"offset\":%d, \"num_succ\":%d,\"version\":%s}",
             1, key.node_id, key.st, key.pa, key.offset, key.num_succ, cfrModel.c_str());
    request_str = std::string(buf);
    return 0;//ERR_OK;
}

int CfrSvrClient::ConvertResponseStrToVals(const std::string &response_str, int num_succ,
        std::vector<unsigned int> &vals)
{
    Json::Reader reader;
    Json::Value value;
    if (reader.parse(response_str.c_str(), value))
    {
        std::string resp_stat = value["res"].asString();
        if (resp_stat != "OK")
        {
            return -1;//ERR_Failed_CFR_RESP;
        }
        std::string val_str = value["vals"].asString();
        std::vector<std::string> split_str;
        StringUtils::SplitString(val_str, split_str, ",");
        int cnt = int(split_str.size());
        if (cnt != num_succ)
        {
            return -2;//ERR_WRONG_VAL_CNT;
        }
        vals.resize(num_succ);
        for (int i = 0; i < cnt; i++)
        {
            unsigned int temp_val;
            if (sscanf(split_str[i].c_str(), "%u", &temp_val) != 1)
            {
                return -3;//ERR_INVALID_PARAM_VAL;
            }
            vals[i] = temp_val;
        }
    }
    else
    {
        return -4;//ERR_INVALID_CFR_RESP;
    }
    return 0;//ERR_OK;
}