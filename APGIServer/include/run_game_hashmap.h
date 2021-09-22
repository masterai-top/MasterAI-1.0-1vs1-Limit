
#ifndef __RUN_GAME_HASH_MAP_H_
#define __RUN_GAME_HASH_MAP_H_

#include "typedef.h"
#include "thread/lock.h"
#include "run_game.h"

struct HMRunGameNode {
    CRunGame        m_oGame;
    HMRunGameNode  *next;
};


//1.哈希表内部的锁,只保证hash的同步
//2.Get/Add/Del必须在全局锁的保护下进程操作,保证APGIClient数据的安全
class CRunGameHashMap
{
private:
    CRunGameHashMap();
    HMRunGameNode* PopNode();
    void   PushNode(HMRunGameNode *node);
    uint32 Hash(const int64 &nRobotID);
            
public:
    static CRunGameHashMap* Instance()
    {
        static CRunGameHashMap m_instance;
        return &m_instance;
    }
    ~CRunGameHashMap();
    
    bool        Init(uint32 size, uint32 buckets);
    CRunGame*   Get(const char *szTransID, const int64 &nRobotID, bool bUpdateTime = true);
    int         Add(const char *szTransID, const CRunGame &oGame);
    int         Del(const char *szTransID, const int64 &nRobotID);
    bool        IsTimeOut(const char *szTransID, const int64 &nRobotID);
    uint32      GetLocalID();       
    
    uint32 Size();
    uint32 Free();
    void   Clear();
    void   testFuc();
    
    
private:
    HMRunGameNode         **m_pBucket;              //桶
    HMRunGameNode          *m_lstFree;              //空闲节点列表
    uint32                  m_nUseSize;             //已使用的节点数
    uint32                  m_nTotalSize;           //总节点数
    uint32                  m_nBuckets;             //桶的数量

    frame::LockObject       m_freeLock;              //空闲锁
    frame::LockObject      *m_hmLock;                //哈希锁
};



#endif 



