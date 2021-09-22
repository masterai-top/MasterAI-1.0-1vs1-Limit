#include "pch.h"
#include "redis_base.h"
#include <assert.h>

/**
* \brief 构造函数
*/
CRedisBase::CRedisBase(void)
{
}

/**
* \brief 析构函数
*/
CRedisBase::~CRedisBase(void)
{
    Restore();
}

/**
* \brief 初始化
* \param strAddr redis地址
* \return 初始化成功返回true，失败返回false
*/
bool CRedisBase::Init(const std::string &strAddr, int nCnt, const std::string &strPass)
{
    frame::Lock lock(&m_lock);
    for(int i = 0; i < nCnt; i++)
    {
        r3c::CRedisClient *pClient = NULL;
        if(strPass.empty()) {
            pClient= new r3c::CRedisClient(strAddr);
        }
        else {
            pClient = new r3c::CRedisClient(strAddr, strPass);
        }
        
        if (NULL == pClient)
        {
            return false;
        }
        
        m_lstRedisClient.push_back(pClient);
        LOG(LT_INFO, "Redis client index=%d| addr=%s", nCnt, strAddr.c_str());
    }

    return true;
}

/**
* \brief 释放
*/
void CRedisBase::Restore()
{
    frame::Lock lock(&m_lock);
    if(!m_lstRedisClient.empty())
    {
        r3c::CRedisClient *pClient = m_lstRedisClient.front();
        m_lstRedisClient.pop_front();
        delete pClient;
    }
}

r3c::CRedisClient* CRedisBase::PopRedisClient()
{
    r3c::CRedisClient *pClient = NULL;
    int index = 0;
 
    //必须获取到才返回,所以要求 对象的数量要大于线程数
    if(1) 
    {
        TO_AGAIN:
        if(1) {
            frame::Lock lock(&m_lock);
            if(!m_lstRedisClient.empty())
            {
                pClient = m_lstRedisClient.front();
                m_lstRedisClient.pop_front();
                return pClient;
            }
        }

        LOG(LT_WARN_TRANS, "", "POP_REDIS_CLIENT| client not enough index=%d", index++);

        //if(index > 10)
        {
            //初始化时,申请足够的客户端, 若获取不到, 表示队列回收有问题
            sleep(1);
            _exit(0);
        }
        goto TO_AGAIN;
    }
}

void CRedisBase::PushRedisClient(r3c::CRedisClient* pClient)
{
    frame::Lock lock(&m_lock);
    m_lstRedisClient.push_back(pClient);
}


//将roleid添加到 '待更新DB数据的RoleID的集合'
int CRedisBase::RoleIDToWaitSaveSet(uint32 nRoleID)
{
    std::string strDBRoleIDKey = KEY_WAIT_SAVE_TO_DB(1, 2, 1);;
    
    r3c::CRedisClient *pRedisCli = PopRedisClient();
    assert(NULL != pRedisCli);
    
    int nRet = pRedisCli->sadd(strDBRoleIDKey, std::to_string(nRoleID));
    PushRedisClient(pRedisCli);

    return nRet;
}

//将roleid添加到 '待更新DB数据的RoleID的集合'
int CRedisBase::RoleIDToWaitSaveSet(const std::vector<std::string>& vRoleID)
{
    //umsvr只有一个
    std::string strKey = KEY_WAIT_SAVE_TO_DB(1, 2, 1);
    
    r3c::CRedisClient *pRedisCli = PopRedisClient();
    assert(NULL != pRedisCli);
    
    int nRet = pRedisCli->sadd(strKey, vRoleID);
    PushRedisClient(pRedisCli);

    return nRet;
}


//从 '待更新DB数据的RoleID的集合' 提取roleID
void CRedisBase::GetRoleIDFromWaitSaveSet(std::vector<std::string>& vRoleID)
{
    vRoleID.clear();
    std::string strDBRoleIDKey = KEY_WAIT_SAVE_TO_DB(1, 2, 1);;
    
    r3c::CRedisClient *pRedisCli = PopRedisClient();
    assert(NULL != pRedisCli);
    
    pRedisCli->smembers(strDBRoleIDKey, &vRoleID);
    PushRedisClient(pRedisCli);
}

//从 '待更新DB数据的RoleID的集合' 删除RoleID
void CRedisBase::DelRoleIDFromWaitSaveSet(uint32 nRoleID)
{ 
    std::string strDBRoleIDKey = KEY_WAIT_SAVE_TO_DB(1, 2, 1);;
    
    r3c::CRedisClient *pRedisCli = PopRedisClient();
    assert(NULL != pRedisCli);
    
    pRedisCli->srem(strDBRoleIDKey, std::to_string(nRoleID));
    PushRedisClient(pRedisCli);
}


//增加Role信息的计数
void CRedisBase::RoleInrcFields(uint32 nRoleId, const std::vector<std::pair<std::string, int64_t> >& increments,  std::vector<int64_t> *values)
{
    std::string strKey = "";
    if (nRoleId > MAX_ROBOT_ROLE_ID)
    {
        strKey = KEY_ROLE(nRoleId);
    }
    else
    {
        strKey = KEY_ROBOT(nRoleId);
    }
    
    r3c::CRedisClient *pRedisCli = PopRedisClient();
    assert(NULL != pRedisCli);
    
    if(pRedisCli->exists(strKey))
    {
        pRedisCli->hincrby(strKey, increments, values);
    }
    PushRedisClient(pRedisCli);
}

void CRedisBase::RoleSetFields(uint32 nRoleID, const FieldNodes &map)
{   
    std::string Key = "";
    if (nRoleID > MAX_ROBOT_ROLE_ID)
    {
        Key = KEY_ROLE(nRoleID);
    }
    else
    {
        Key = KEY_ROBOT(nRoleID);
    }

    r3c::CRedisClient *pRedisCli = PopRedisClient();
    assert(NULL != pRedisCli);   
    if(pRedisCli->exists(Key))
    {
        pRedisCli->hmset(Key, map);
    }
    PushRedisClient(pRedisCli);
}

bool CRedisBase::RoleExpire(uint32 nRoleId, uint32 seconds)
{
    std::string Key = "";
    if (nRoleId > MAX_ROBOT_ROLE_ID) {
        Key = KEY_ROLE(nRoleId);
    }
    else {
        Key = KEY_ROBOT(nRoleId);
    }
    
    r3c::CRedisClient *pRedisCli = PopRedisClient();
    assert(NULL != pRedisCli);
    
    bool ret = pRedisCli->expire(Key, seconds);
    PushRedisClient(pRedisCli);

    return ret;
}

bool CRedisBase::GetByKey(const std::string &k, std::string &v)
{   
    bool ret = false;
    v.clear();
    
    r3c::CRedisClient *pRedisCli = PopRedisClient();
    assert(NULL != pRedisCli);   
    ret = pRedisCli->get(k, &v);
    PushRedisClient(pRedisCli);

    return ret;
}

//成功返回0
int CRedisBase::SetValue(const std::string &k, const std::string &v)
{   
    r3c::CRedisClient *pRedisCli = PopRedisClient();
    assert(NULL != pRedisCli);
    int nRet = pRedisCli->set(k, v);
    PushRedisClient(pRedisCli);
    
    return nRet;
}

//返回值<0发送错误
int64_t CRedisBase::DBSize()
{   
    r3c::CRedisClient *pRedisCli = PopRedisClient();
    assert(NULL != pRedisCli);
    int64_t nRet = pRedisCli->dbsize();
    PushRedisClient(pRedisCli);

    return nRet;
}

//返回值<0, 发送错误
//返回值=0, 结束
//返回值>0, 下次的cursor
int64_t CRedisBase::SCan(int64_t cursor, const std::string& pattern, int count, std::vector<std::string>* values)
{   
    values->clear();
    r3c::CRedisClient *pRedisCli = PopRedisClient();
    assert(NULL != pRedisCli);
    int64_t nRet = pRedisCli->scan(cursor, pattern, count, values);
    PushRedisClient(pRedisCli);

    return nRet;
}


