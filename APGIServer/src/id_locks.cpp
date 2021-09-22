#include "id_locks.h"
#include "define.h"


CIDLocks::CIDLocks()
{
    m_nLockSize = 0;
    m_pLock     = NULL;
}

CIDLocks::~CIDLocks()
{
    if(NULL != m_pLock)
    {
        delete []m_pLock;
    }
}

bool CIDLocks::Init(uint32 nLockSize)
{
    if(0 == nLockSize)
    {
        return false;
    }

    m_nLockSize = nLockSize;
    m_pLock     = new frame::LockObject[m_nLockSize];
    if(NULL == m_pLock)
    {
        return false;
    }

    return true;
}

frame::LockObject *CIDLocks::GetLock(const char *szTransID, const uint64 &id)
{
    uint32 hash = (id) % m_nLockSize;

    return (m_pLock + hash);
}


CRobotIDLock::CRobotIDLock(const char *szTransID, const uint64 &nRobotID, CIDLocks &oLocks) : m_nRobotID(nRobotID), m_szTransID(szTransID)
{
    m_lock = oLocks.GetLock(m_szTransID.c_str(), m_nRobotID);
    if(m_lock)
    {
        m_lock->Lock();
        //LOG(LT_INFO_TRANS, m_szTransID.c_str(), "ROBOT_ID_LOCK| succ| robot_id=%llu", m_nRobotID);
    }
    else
    {
        LOG(LT_ERROR_TRANS, m_szTransID.c_str(), "ROBOT_ID_LOCK| failed| robot_id=%llu", m_nRobotID);
    }
}

CRobotIDLock::~CRobotIDLock()
{
    if (m_lock)
    {
        m_lock->Unlock();
        //LOG(LT_INFO_TRANS, m_szTransID.c_str(), "ROBOT_ID_UNLOCK| succ| robot_id=%llu", m_nRobotID);
    }
    else
    {
        LOG(LT_ERROR_TRANS, m_szTransID.c_str(), "ROBOT_ID_UNLOCK| failed| robot_id=%llu| unlock", m_nRobotID);
    }
}




