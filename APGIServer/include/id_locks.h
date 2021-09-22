#ifndef __ID_LOCKS_H__
#define __ID_LOCKS_H__
#include "typedef.h"
#include "frame.h"


class CIDLocks
{
public:
    CIDLocks(void);
    ~CIDLocks();
    
    bool Init(uint32 nLockSize);
    frame::LockObject* GetLock(const char *szTransID, const uint64 &id);


private:
    uint32              m_nLockSize;    //锁的数量
    frame::LockObject  *m_pLock;        //锁
};

//Room数据的锁
class CRobotIDLock
{   
public:
    CRobotIDLock(const char *szTransID, const uint64 &nRobotID, CIDLocks &oLocks);
    ~CRobotIDLock();

private:
    uint64              m_nRobotID;
    string              m_szTransID;
    frame::LockObject  *m_lock;
};



#endif // __TPA_ROOM_H__

