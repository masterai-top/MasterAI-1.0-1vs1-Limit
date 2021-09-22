

#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_


#include <string>
#include <iostream>
#include <vector>

class StringUtils
{
public:
    static std::string GetValueBykey(const char *szBuf, const char *key,const char *Seprator);

    static void SplitString(const std::string &s,
                            std::vector<std::string> &v, const std::string &c);
private:
    StringUtils(){}
    ~StringUtils(){}
};


#endif