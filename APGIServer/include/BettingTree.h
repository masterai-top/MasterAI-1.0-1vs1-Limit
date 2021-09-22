/**
* \file  BettingTree.h
* \brief Bettingtree 接口
*
*/

#ifndef __BETTING_TREE_H__
#define __BETTING_TREE_H__
#include <string>
#include <unordered_map>
#include "redis_base.h"
#include "frame.h"
#include "Def.h"


using namespace std;

class CBettingTree
{
public:
    ~CBettingTree();
    static CBettingTree* Instance()
    {
        static CBettingTree m_instance;
        return &m_instance;
    }

    bool  Init(uint32 nBTRecords, const string &strBTRedis,  const string &strBTRedisPass, uint32 nTaskRedisCnt, const string &strBTFile);
    int   GetNode(const char *szTransID, const string& strActionSeq, NodeInfo &node);

    
private:
    CBettingTree();
    int ParseStrToNode(const std::string& info_str,NodeInfo &node);
    static void *InitBTreeIt(void *);
    
private:
    CRedisBase                      m_oInitBTRedis;     //初始化betting tree数据至内存的redis的对象. [作用:从redis初始化m_BTHashTable]
    CRedisBase                      m_oTaskBTRedis;     //业务查询betting tree的redis的对象 [作用:当0=m_nInitState时,进行bettingTree的数据查询; 1=m_nInitState不再使用]
    uint32                          m_nBTRecords;       //betting tree记录数
    uint8                           m_nInitState;       //betting tree数据初始化状态 0:未初始化 1:已初始化(即m_BTHashTable数据初始化完成)
    string                          m_strBTFile;        //betting tree 文件名
    unordered_map<string, NodeInfo> m_BTHashTable;      //betting tree 哈希表(key=已执行的动作)
};

#endif // __BETTING_TREE_H__

