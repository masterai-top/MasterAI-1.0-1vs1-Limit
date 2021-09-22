
#ifndef __APGI_CLIENT_HASH_MAP_H_
#define __APGI_CLIENT_HASH_MAP_H_

#include "typedef.h"
#include "thread/lock.h"
#include <string>
#include <cstring>
extern uint64 GToConnID(uint32 nUniqueID, int32 nSockFd);

class APGIClient
{
public:
    APGIClient(): m_nSockFd(-1), m_nUniqueID(0) {
        memset(m_szDesc, 0, sizeof(m_szDesc));
    }

    const APGIClient & operator=(const APGIClient &v) {
        memcpy(this, &v, sizeof(APGIClient));
        return *this;
    }

    uint64 GetConnID() const { return GToConnID(m_nUniqueID, m_nSockFd); }
    void   SetConnID(uint32 nUniqueID, int32 nSockFd) { SetUniqueID(nUniqueID); SetSockFd(nSockFd); }
    int32  GetSockFd() const { return m_nSockFd; }
    uint32 GetUniqueID() const { return m_nUniqueID; }
    

    uint32 GetLastTime() const { return m_nLastTime; }
    void   SetLastTime(uint32 v) { m_nLastTime = v; }
    
    
    std::string GetDesc() const { return std::string(m_szDesc); }
    void   SetDesc(const char *desc) {   snprintf(m_szDesc, sizeof(m_szDesc) - 1, "%s", desc); }

private:
    void   SetSockFd(int32 v) { m_nSockFd = v; }
    void   SetUniqueID(uint32 v) { m_nUniqueID = v; }
    
private:
    int         m_nSockFd;      // 连接fd
    uint32      m_nUniqueID;    // 连接的唯一ID
    char        m_szDesc[65];   // 连接描述
    uint32      m_nLastTime;    // 最新使用时间
};


struct HMClientNode {
    APGIClient      m_oClient;
    HMClientNode   *next;
};



//1.哈希表内部的锁,只保证hash的同步
//2.Get/Add/Del必须在全局锁的保护下进程操作,保证APGIClient数据的安全
class CAPGIClientHashMap
{
private:
    CAPGIClientHashMap();
    HMClientNode* PopNode();
    void   PushNode(HMClientNode *node);
    uint32 Hash(const uint64 &nConnID);
            
public:
    static CAPGIClientHashMap* Instance()
    {
        static CAPGIClientHashMap m_instance;
        return &m_instance;
    }
    ~CAPGIClientHashMap();
    
    bool        Init(uint32 size, uint32 buckets);
    APGIClient* Get(const char *szTransID, const uint64 &nConnID);
    int         Get(const char *szTransID, const uint64 &nConnID, APGIClient &oClient);
    int         Add(const char *szTransID, const APGIClient &oclient);
    int         Del(const char *szTransID, const uint64 &nConnID);
    bool        IsTimeOut(const char *szTransID, const uint64 &nConnID);
    
    uint32 Size();
    uint32 Free();
    void   Clear();
    void   testFuc();
    
private:
    HMClientNode          **m_pBucket;              //桶
    HMClientNode           *m_lstFree;              //空闲节点列表
    uint32                  m_nUseSize;             //已使用的节点数
    uint32                  m_nTotalSize;           //总节点数
    uint32                  m_nBuckets;             //桶的数量

    frame::LockObject       m_freeLock;              //空闲锁
    frame::LockObject      *m_hmLock;                //哈希锁
};



#endif 



