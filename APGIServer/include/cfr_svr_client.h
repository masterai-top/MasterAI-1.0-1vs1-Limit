#ifndef _CFR_SVR_CLIENT_H
#define _CFR_SVR_CLIENT_H
#include "Def.h"

class CfrSvrClient
{
public:
    
    static int ConvertKeyToRequestStr(const CFRKey &key, std::string &request_str, const std::string &cfrModel);
    static int ConvertResponseStrToVals(const std::string &response_str,int num_succ,
                                        std::vector<unsigned int> &vals);

private:
    

    CfrSvrClient(/* args */) {}
    ~CfrSvrClient() {}

};

#endif