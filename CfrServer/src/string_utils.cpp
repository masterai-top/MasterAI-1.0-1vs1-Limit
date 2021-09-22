#include "string_utils.h"

#include <stdio.h>
#include "string.h"

using namespace std;

//从字符串获取指定key值的内容
//key的内容以“空格”或“\r\n” 分隔
//格式：key1=value1 key2=value2 ...
std::string StringUtils::GetValueBykey(const char *szBuf, const char *key, const char *Seprator)
{
    if(NULL == szBuf || NULL == key)
    {
        return std::string("");
    }

    const char *start = NULL, *end = NULL;
    if(NULL == (start = strstr(szBuf, key)))
    {
        return std::string("");
    }

    start += (strlen(key) + 1);
    if(NULL == (end = strstr(start, Seprator)))
    {
        if(NULL == (end = strstr(start, "\r\n")))
        {
            return std::string(start);
        }
    }

    std::string s(start, end - start);
    return s;

}

void StringUtils::SplitString(const std::string &s,
                              std::vector<std::string> &v, const std::string &c)
{
    v.clear();
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2 - pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}