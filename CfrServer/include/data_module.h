#ifndef _DATA_MODULE_H
#define _DATA_MODULE_H

#include <memory>
#include <vector>

class CFRValues;


class DataModule
{
public:
    static int Init();
    
    static void GetVals(const std::string &request_str, std::string &resp_str);

    static bool CheckVersion(const std::string &version_str);
private:
    
    static int CheckKeys(int nt,int st,int pa,int offset,int num_succ);

    static std::string ConvertValToStr(const std::vector<unsigned int> &vals);

    static std::string GenRandomValStr(int num_succ);

    static std::string JointStr(const std::string &res,const std::string &note,const std::string &val_str);

    static std::shared_ptr<CFRValues> cfr_values;
    static bool _init_flag;
    DataModule(/* args */){}
    ~DataModule(){}
};


#endif