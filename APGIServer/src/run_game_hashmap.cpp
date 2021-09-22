#include "run_game_hashmap.h"
#include "shlog.h"
#include "define.h"
#include "apgi_timer.h"
#include "apgi_conf.h"
CRunGameHashMap::CRunGameHashMap()
{
}

CRunGameHashMap::~CRunGameHashMap()
{
}

bool CRunGameHashMap::Init(uint32 size, uint32 buckets)
{
    if(size > 50*10000) {
        return -1;
    }
    
    if(size > 5*buckets)
    {
        return -2;
    }

    m_nTotalSize = size;
    m_nBuckets   = buckets;
    m_lstFree    = NULL;

    m_hmLock = new frame::LockObject[m_nBuckets];
    if(NULL == m_hmLock) {
        return -15;
    }
    
    HMRunGameNode *pTotal = new HMRunGameNode[m_nTotalSize];
    if(NULL != pTotal)
    {  
        for(uint32 i = 0; i < m_nTotalSize; i++)
        {    
            HMRunGameNode *p = pTotal + i;
            //printf("mNode=%llu| mVal=%llu\r\n", (uint64_t)p, (uint64_t)&p->val);
            PushNode(p);
        }
    }
    else
    {
        for(uint32 i = 0; i < m_nTotalSize; i++)
        {
            HMRunGameNode *p = new HMRunGameNode;
            if(NULL == p)
            {
                return -11;
            }

            //printf("mNode=%llu| mVal=%llu\r\n", (uint64_t)p, (uint64_t)&p->val);
            PushNode(p);
        }
    }

    m_nUseSize   = 0;
    m_pBucket = new HMRunGameNode*[m_nBuckets];
    if(NULL == m_pBucket) 
    {
        return -4;
    }
    else 
    {
        for(uint32 i = 0; i < m_nBuckets; i++)
        {
            m_pBucket[i] = NULL;
        }
    }

    return 0;
}


/* 查询记录
*  @Return    记录的指针
*/
CRunGame* CRunGameHashMap::Get(const char *szTransID, const int64 &nRobotID, bool bUpdateTime)
{
    uint32 hash      = Hash(nRobotID);    
    
    if(1)
    {
        frame::Lock l(&m_hmLock[hash]);
        HMRunGameNode *pNode = m_pBucket[hash];
        while(NULL != pNode)
        {
            if(pNode->m_oGame.GetRobotID() == nRobotID) 
            {
                if(bUpdateTime) {
                    pNode->m_oGame.SetLastTime(time(NULL));
                }
                
                return &pNode->m_oGame;
            }

            pNode = pNode->next;
        }
    }

    return NULL;
}


/* 添加记录:添加key-value对. value的内容由输出参数的地址再进行设置[若设置失败需要调用者主动删除]
*  @pVal     输出参数
*  @Return   见“错误码定义”
*/
int CRunGameHashMap::Add(const char *szTransID, const CRunGame &oGame)
{
    if(0 == oGame.GetRobotID()) {
        return -9001;
    }
    
    uint32 hash     = Hash(oGame.GetRobotID());
    if(1)
    {
        int nHCnt = 0;
        frame::Lock l(&m_hmLock[hash]);
        HMRunGameNode *en = m_pBucket[hash];
        while(NULL != en)
        {
            if(en->m_oGame.GetRobotID() == oGame.GetRobotID()) 
            {   
                //记录已存在
                return -9004;
            }

            en = en->next;
            ++nHCnt;
        }

        if(nHCnt > 20) {
            LOG(LT_WARN_TRANS, szTransID, "RUN_GAME_HASH_MAP| %u hash have %d records", hash, nHCnt);
        }
        else if(nHCnt > 100) {
            LOG(LT_ERROR_TRANS, szTransID, "RUN_GAME_HASH_MAP| %u hash records limited(%u)", hash, nHCnt);
            return -9005;
        }

        //获取节点
        HMRunGameNode *pNode = PopNode();
        if(NULL == pNode) 
        {
            LOG(LT_ERROR_TRANS, szTransID, "RUN_GAME_HASH_MAP| out of memory");
            return -9006;
        }

        //printf("robot_id=%lld| addr=%llu\r\n", oRoom.GetRobotID(), (uint64)pNode);

        //赋值
        pNode->m_oGame = oGame;
        pNode->next      = NULL;
        pNode->m_oGame.SetLastTime(time(NULL));
        
        
        //加入hashmap
        if(NULL == m_pBucket[hash])
        {
            //桶的第一次插入
            m_pBucket[hash] = pNode;
            pNode->next     = NULL;
        }
        else
        {
            //桶已存在其他节点,插入头部
            pNode->next     = m_pBucket[hash];
            m_pBucket[hash] = pNode;
        }
    }

    network::CTimerManager::Instance()->Attach(CTimerRunGame::Instance(), oGame.GetRobotID(), CAPGIConf::Instance()->m_nRobotTimeout*1000, -1, "ADD_RUN_GAME", szTransID);
    
    return 0;
}


/* 删除记录
*  成功删除时会自动调用 PushNode回收
*  @Return   见“错误码定义”
*/
int CRunGameHashMap::Del(const char *szTransID, const int64 &nRobotID)
{
    uint32 hash = Hash(nRobotID);
    int32  nRet = -99;
        
    //处在第一个节点
    if(1)
    {
        frame::Lock l(&m_hmLock[hash]);
        HMRunGameNode *pre = NULL;
        HMRunGameNode *del = m_pBucket[hash];
        while(NULL != del) 
        {
            if(del->m_oGame.GetRobotID() == nRobotID) 
            {
                //是否是桶的第一个节点
                if(NULL == pre) 
                {
                    //printf("Is del first node| yes | hash=%d| key=%llu\r\n", hash, nRoomID);
                    m_pBucket[hash] = del->next;
                }
                else 
                {
                    //printf("Is del first node| no  | hash=%d| key=%llu\r\n", hash, nRoomID);
                    pre->next = del->next;
                }

                del->m_oGame.SetRobotID(0);
                del->next = NULL;
                PushNode(del);
                
                nRet = 0;
                break;
            }

            pre = del;
            del = del->next;
        }
    }

    //注销定时器    
    network::CTimerManager::Instance()->Detach(CTimerRunGame::Instance(), nRobotID, szTransID);
    
    return nRet;
}

bool CRunGameHashMap::IsTimeOut(const char *szTransID, const int64 &nRobotID)
{
    uint32 hash     = Hash(nRobotID);    
    
    
    frame::Lock l(&m_hmLock[hash]);
    HMRunGameNode *pNode = m_pBucket[hash];
    while(NULL != pNode)
    {
        if(pNode->m_oGame.GetRobotID() == nRobotID) 
        {
            if((time(NULL) - pNode->m_oGame.GetLastTime()) > CAPGIConf::Instance()->m_nDataTimeout)
            {
                return true;
            }
            else {
                return false;
            }
        }

        pNode = pNode->next;
    }

    return true;
}


void CRunGameHashMap::Clear()
{   
    HMRunGameNode *node = NULL;
    HMRunGameNode *del  = NULL;

    for(uint32 i = 0; i < m_nBuckets; i++) 
    {
        frame::Lock l(&m_hmLock[i]);
        node = m_pBucket[i];
        while(NULL != node)
        {
            del       = node;
            node      = del->next;
            del->next = NULL;         
            PushNode(del);
        }
        
        m_pBucket[i] = NULL;
    }
}

uint32 CRunGameHashMap::Size() 
{ 
    //避免死锁而不加锁. 数据存在不正确的可能
    //frame::Lock l(&m_freeLock);
    return m_nUseSize; 
}

uint32 CRunGameHashMap::Free() 
{ 
    //避免死锁而不加锁. 数据存在不正确的可能
    //frame::Lock l(&m_freeLock);
    return (m_nTotalSize - m_nUseSize); 
}


HMRunGameNode* CRunGameHashMap::PopNode()
{
    frame::Lock l(&m_freeLock);
    if(NULL == m_lstFree)
    {
        return NULL;
    }
    
    HMRunGameNode* p = m_lstFree;
    m_lstFree     = p->next;
    p->next       = NULL;
    
    ++m_nUseSize;
    
    return p;
}

void CRunGameHashMap::PushNode(HMRunGameNode *node)
{
    if(NULL == node) 
    {
        return;
    }

    frame::Lock l(&m_freeLock);
    node->next = m_lstFree;
    m_lstFree  = node;
    
    --m_nUseSize;
}


uint32 CRunGameHashMap::Hash(const int64 &nRobotID)
{
     return nRobotID%m_nBuckets;
}


void CRunGameHashMap::testFuc()
{
    #define T_HMAP_SIZE (10)
    int err = Init(T_HMAP_SIZE, 3);
    if(0 != err) {
        printf("HMRunGame| init failed| rc=%d\r\n", err);
        return ;
    }
    
    Del("", 1);
    Del("", 2);
    Get("", 1);
    Get("", 2);

    int nTTimes = 0;
    
    DO_AGAIN:
    printf("\r\n\r\n********************** done *********************** \r\n");
    for(int i = 0; i < T_HMAP_SIZE + 2; i++)
    {   
        string s = "TData--" + std::to_string(i);
        CRunGame oClient;
        oClient.SetRobotID(i);
        
        err = Add("", oClient);
        if(0 != err) 
        {
            printf("HMRunGame| add failed| rc=%d| robot_id=%lld\r\n", err, oClient.GetRobotID());        
            continue;
        }
        else {
            //printf("HMRunGame| add succ..| rc=%d| robot_id=%lld| value=%s\r\n", err, oRoom.GetRobotID(), s.c_str());        
        }
    }
    //printf("\r\n");
    
    //随机删除2个
    for(int i = 0; i < 2; i++)
    {
        int k = rand()%T_HMAP_SIZE;
        err = Del("", k);
        printf("HMRunGame| del rand..| rc=%d| robot_id=%d\r\n", err, k);        
    }
    printf("\r\n");
    
    for(int i = T_HMAP_SIZE; i < (2 + T_HMAP_SIZE); i++)
    {
        string s = "TData[again] --" + std::to_string(i);
        CRunGame oClient;
        oClient.SetRobotID(i);
        
        err = Add("", oClient);
        if(0 != err) 
        {
            printf("HMRunGame| add failed| rc=%d| robot_id=%lld\r\n", err, oClient.GetRobotID());        
            continue;
        }
        else {
            printf("HMRunGame| add succ..| rc=%d| robot_id=%lld| value=%s\r\n", err, oClient.GetRobotID(), s.c_str());        
        }
    }
    
    printf("\r\n");    
    for(int i = 0; i < T_HMAP_SIZE+3; i++) 
    {
        CRunGame *pRoom = Get("", i);
        if(NULL != pRoom)
        {
            printf("Get succ| key=%.02d| robot_id=%lld| size=%d| free=%d\r\n", 
                i, pRoom->GetRobotID(), Size(), Free());
        }
        else
        {
            printf("Get succ| key=%.02d| no record\r\n", i);
        }
    }

    printf("\r\n");
    nTTimes++;
    if(0 == nTTimes%2) {
        Clear();
        printf("Clear succ| size=%u| free=%d\r\n", Size(), Free());
    }
    else {
        for(int i = 0; i < T_HMAP_SIZE+3; i++) {
            Del("", i);
        }
        
        printf("Del succ| size=%u| free=%d\r\n", Size(), Free());
    }
    
    sleep(5);
    goto DO_AGAIN;
}



