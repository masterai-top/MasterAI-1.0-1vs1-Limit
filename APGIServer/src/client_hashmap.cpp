#include "client_hashmap.h"
#include "shlog.h"
#include "define.h"
#include "apgi_timer.h"
#include "apgi_conf.h"


uint64 GToConnID(uint32 nUniqueID, int32 nSockFd)
{
    uint64 v = nUniqueID;
    return ((v << 32) + nSockFd);
}


CAPGIClientHashMap::CAPGIClientHashMap()
{
}

CAPGIClientHashMap::~CAPGIClientHashMap()
{
}

bool CAPGIClientHashMap::Init(uint32 size, uint32 buckets)
{
    if(size > 50 * 10000)
    {
        return -1;
    }

    if(size > 5 * buckets)
    {
        return -2;
    }

    m_nTotalSize = size;
    m_nBuckets   = buckets;
    m_lstFree    = NULL;

    m_hmLock = new frame::LockObject[m_nBuckets];
    if(NULL == m_hmLock)
    {
        return -15;
    }

    HMClientNode *pTotal = new HMClientNode[m_nTotalSize];
    if(NULL != pTotal)
    {
        for(uint32 i = 0; i < m_nTotalSize; i++)
        {
            HMClientNode *p = pTotal + i;
            //printf("mNode=%llu| mVal=%llu\r\n", (uint64_t)p, (uint64_t)&p->val);
            PushNode(p);
        }
    }
    else
    {
        for(uint32 i = 0; i < m_nTotalSize; i++)
        {
            HMClientNode *p = new HMClientNode;
            if(NULL == p)
            {
                return -11;
            }

            //printf("mNode=%llu| mVal=%llu\r\n", (uint64_t)p, (uint64_t)&p->val);
            PushNode(p);
        }
    }

    m_nUseSize   = 0;
    m_pBucket = new HMClientNode*[m_nBuckets];
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
APGIClient *CAPGIClientHashMap::Get(const char *szTransID, const uint64 &nConnID)
{
    uint32 hash      = Hash(nConnID);

    if(1)
    {
        frame::Lock l(&m_hmLock[hash]);
        HMClientNode *pNode = m_pBucket[hash];
        while(NULL != pNode)
        {
            if(pNode->m_oClient.GetConnID() == nConnID)
            {
                pNode->m_oClient.SetLastTime(time(NULL));
                return &pNode->m_oClient;
            }

            pNode = pNode->next;
        }
    }

    return NULL;
}


int CAPGIClientHashMap::Get(const char *szTransID, const uint64 &nConnID, APGIClient &oClient)
{
    uint32 hash = Hash(nConnID);

    frame::Lock l(&m_hmLock[hash]);
    HMClientNode *pNode = m_pBucket[hash];
    while(NULL != pNode)
    {
        if(pNode->m_oClient.GetConnID() == nConnID)
        {
            pNode->m_oClient.SetLastTime(time(NULL));
            oClient = pNode->m_oClient;
            return 0;
        }

        pNode = pNode->next;
    }


    return -99;
}


/* 添加记录:添加key-value对. value的内容由输出参数的地址再进行设置[若设置失败需要调用者主动删除]
*  @pVal     输出参数
*  @Return   见“错误码定义”
*/
int CAPGIClientHashMap::Add(const char *szTransID, const APGIClient &oClient)
{
    if(0 == oClient.GetConnID())
    {
        return -9001;
    }

    if(oClient.GetConnID() != GToConnID(oClient.GetUniqueID(), oClient.GetSockFd()))
    {
        return -9002;
    }

    uint32 hash = Hash(oClient.GetConnID());


    if(1)
    {
        int nHCnt = 0;
        frame::Lock l(&m_hmLock[hash]);
        HMClientNode *en = m_pBucket[hash];
        while(NULL != en)
        {
            if(en->m_oClient.GetConnID() == oClient.GetConnID())
            {
                //记录已存在
                return -9004;
            }

            en = en->next;
            ++nHCnt;
        }

        if(nHCnt > 20)
        {
            LOG(LT_WARN_TRANS, szTransID, "APGI_CLIENT_HASH_MAP| %u hash have %d records", hash, nHCnt);
        }
        else if(nHCnt > 100)
        {
            LOG(LT_ERROR_TRANS, szTransID, "APGI_CLIENT_HASH_MAP| %u hash records limited(%u)", hash, nHCnt);
            return -9005;
        }

        //获取节点
        HMClientNode *pNode = PopNode();
        if(NULL == pNode)
        {
            LOG(LT_ERROR_TRANS, szTransID, "APGI_CLIENT_HASH_MAP| out of memory");
            return -9006;
        }

        //printf("conn_id=%llu| addr=%llu\r\n", oRoom.GetConnID(), (uint64)pNode);

        //赋值
        pNode->m_oClient = oClient;
        pNode->next      = NULL;
        pNode->m_oClient.SetLastTime(time(NULL));

        LOG(LT_INFO_TRANS, szTransID, "APGI_CLIENT_HASH_MAP| add succ| conn_id=%llu| unique_id=0x%x| fd=%d, desc=%s| hash=%d| cnt=%d",
            pNode->m_oClient.GetConnID(), pNode->m_oClient.GetUniqueID(), pNode->m_oClient.GetSockFd(), pNode->m_oClient.GetDesc().c_str(), hash, nHCnt
           );

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

    network::CTimerManager::Instance()->Attach(CAPGIClientTimer::Instance(), oClient.GetConnID(), CAPGIConf::Instance()->m_nTCPTimeOut * 1000, -1, "ADD_CLIENT", szTransID);

    return 0;
}


/* 删除记录
*  成功删除时会自动调用 PushNode回收
*  @Return   见“错误码定义”
*/
int CAPGIClientHashMap::Del(const char *szTransID, const uint64 &nConnID)
{
    uint32 hash = Hash(nConnID);
    int32  nRet = -99;

    //处在第一个节点
    if(1)
    {
        frame::Lock l(&m_hmLock[hash]);
        HMClientNode *pre = NULL;
        HMClientNode *del = m_pBucket[hash];
        while(NULL != del)
        {
            if(del->m_oClient.GetConnID() == nConnID)
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

                LOG(LT_INFO_TRANS, szTransID, "APGI_CLIENT_HASH_MAP| del succ| conn_id=%llu| unique_id=0x%x| fd=%d, desc=%s| hash=%d",
                    del->m_oClient.GetConnID(), del->m_oClient.GetUniqueID(), del->m_oClient.GetSockFd(), del->m_oClient.GetDesc().c_str(), hash
                   );

                del->m_oClient.SetConnID(0, 0);
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
    network::CTimerManager::Instance()->Detach(CAPGIClientTimer::Instance(), nConnID, szTransID);

    return nRet;
}

bool CAPGIClientHashMap::IsTimeOut(const char *szTransID, const uint64 &nConnID)
{
    uint32 hash     = Hash(nConnID);


    frame::Lock l(&m_hmLock[hash]);
    HMClientNode *pNode = m_pBucket[hash];
    while(NULL != pNode)
    {
        if(pNode->m_oClient.GetConnID() == nConnID)
        {
            if((time(NULL) - pNode->m_oClient.GetLastTime()) > CAPGIConf::Instance()->m_nTCPTimeOut)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        pNode = pNode->next;
    }

    //不存在返回true. --可能需要删除定时器
    return true;
}


void CAPGIClientHashMap::Clear()
{
    HMClientNode *node = NULL;
    HMClientNode *del  = NULL;

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

uint32 CAPGIClientHashMap::Size()
{
    //避免死锁而不加锁. 数据存在不正确的可能
    //frame::Lock l(&m_freeLock);
    return m_nUseSize;
}

uint32 CAPGIClientHashMap::Free()
{
    //避免死锁而不加锁. 数据存在不正确的可能
    //frame::Lock l(&m_freeLock);
    return (m_nTotalSize - m_nUseSize);
}


HMClientNode *CAPGIClientHashMap::PopNode()
{
    frame::Lock l(&m_freeLock);
    if(NULL == m_lstFree)
    {
        return NULL;
    }

    HMClientNode *p = m_lstFree;
    m_lstFree     = p->next;
    p->next       = NULL;

    ++m_nUseSize;

    return p;
}

void CAPGIClientHashMap::PushNode(HMClientNode *node)
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


uint32 CAPGIClientHashMap::Hash(const uint64 &nConnID)
{
    return nConnID % m_nBuckets;
}


void CAPGIClientHashMap::testFuc()
{
#define T_HMAP_SIZE (10)
#define T_HMAP_UNID (999)

    int err = Init(T_HMAP_SIZE, 3);
    if(0 != err)
    {
        printf("HMClient| init failed| rc=%d\r\n", err);
        return ;
    }

    Del("", GToConnID(T_HMAP_UNID, 1));
    Del("", GToConnID(T_HMAP_UNID, 2));
    Get("", GToConnID(T_HMAP_UNID, 1));
    Get("", GToConnID(T_HMAP_UNID, 2));

    int nTTimes = 0;

DO_AGAIN:
    printf("\r\n\r\n********************** done *********************** \r\n");
    for(int i = 0; i < T_HMAP_SIZE + 2; i++)
    {
        string s = "TData--" + std::to_string(i);
        APGIClient oClient;
        oClient.SetConnID(T_HMAP_UNID, i);

        err = Add("", oClient);
        if(0 != err)
        {
            printf("HMClient| add failed| rc=%d| conn_id=%llu\r\n", err, oClient.GetConnID());
            continue;
        }
        else
        {
            //printf("HMClient| add succ..| rc=%d| conn_id=%llu| value=%s\r\n", err, oRoom.GetConnID(), s.c_str());
        }
    }
    //printf("\r\n");

    //随机删除2个
    for(int i = 0; i < 2; i++)
    {
        int k = rand() % T_HMAP_SIZE;
        err = Del("", GToConnID(T_HMAP_UNID, k));
        printf("HMClient| del rand..| conn_id=%llu| rc=%d\r\n", GToConnID(T_HMAP_UNID, k), err);
    }
    printf("\r\n");

    for(int i = T_HMAP_SIZE; i < (2 + T_HMAP_SIZE); i++)
    {

        int nk = i;
        if(0 == nk % 2)
        {
            nk = rand() % (2 + T_HMAP_SIZE);
        }

        string s = "TData[again] --" + std::to_string(i);
        APGIClient oClient;
        oClient.SetDesc(s.c_str());
        oClient.SetConnID(T_HMAP_UNID, nk);

        err = Add("", oClient);
        if(0 != err)
        {
            printf("HMClient| add failed| rc=%d| conn_id=%llu\r\n", err, oClient.GetConnID());
            continue;
        }
        else
        {
            printf("HMClient| add succ..| rc=%d| conn_id=%llu| value=%s\r\n", err, oClient.GetConnID(), s.c_str());
        }
    }

    printf("\r\n");
    for(int i = 0; i < T_HMAP_SIZE + 3; i++)
    {
        APGIClient *pRoom = Get("", GToConnID(T_HMAP_UNID, i));
        if(NULL != pRoom)
        {
            printf("Get succ| key=%llu| conn_id=%llu| size=%d| free=%d| data=%s\r\n",
                   GToConnID(T_HMAP_UNID, i), pRoom->GetConnID(), Size(), Free(), pRoom->GetDesc().c_str());
        }
        else
        {
            printf("Get succ| key=%llu| no record\r\n", GToConnID(T_HMAP_UNID, i));
        }
    }

    printf("\r\n");
    nTTimes++;
    if(0 == nTTimes % 2)
    {
        Clear();
        printf("Clear succ| size=%u| free=%d\r\n", Size(), Free());
    }
    else
    {
        for(int i = 0; i < T_HMAP_SIZE + 3; i++)
        {
            Del("", GToConnID(T_HMAP_UNID, i));
        }

        printf("Del succ| size=%u| free=%d\r\n", Size(), Free());
    }

    sleep(5);
    goto DO_AGAIN;
}



