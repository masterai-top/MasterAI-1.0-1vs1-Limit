/**
* \file shhashmap.h
* \brief hashmap 模板
* 使用说明
     1. 需要实现(返回值uint32_t, 输入参数为_Tkey)的哈希函数
     2. _Tkey 支持自定义类型时,需要实现operator=.
     3. 非线程安全

     例子:
        struct TValue
        {   
            char    m_szBuf[10];

            static uint32_t MyHash(const int &key)    {
                return key;
            }
        };

        使用：
        SHHaspMap<int, TValue> oHm;
        err = hm.Init(50, TValue::MyHash, 100);
        if(0 != err) {
            printf("init failed| err=%d\r\n", err);
            exit(0);
        }

        TValue *v = NULL;
        oHm.Add(1, v); //--返回后对v进行修改
        
        v = oHm.Get(1);
        oHm.Del(1); 
*/

#ifndef __THASH_MAP_H_
#define __THASH_MAP_H_
#include <stdlib.h>
#include "shlog.h"
using namespace std;

namespace shstd
{
	namespace hashmap
	{
        //错误码定义
        const int SH_HM_OK                 = 0;
        const int SH_HM_NO_RECORD          = 1081;     //无记录
        const int SH_HM_NO_MEMORY          = 1082;     //存储节点不足 
        const int SH_HM_NOT_INIT           = 1083;     //未初始化
        const int SH_HM_HASH_MAX_CNT       = 1084;     //hash值太多主键了
        const int SH_HM_RECORD_EXIST       = 1085;     //记录已存在
        const int SH_HM_INTERNAL_ERR       = 1086;     //内部错误
    
        template <class _Tkey, class _Tval> 
        class SHHaspMap
        {
        public:
            typedef uint32_t (*FHASH)(const _Tkey &k);
            struct CHNode {
                _Tkey    key;
                _Tval    val;
                CHNode  *next;
                CHNode  *prev;
            };

            struct FreeList
            {
                CHNode *head;   //表头
                CHNode *tail;   //表位
            };

        public:
            SHHaspMap() 
            {
                m_pMyHash   = NULL;
                m_pBucket   = NULL;
                m_nBuckets  = 0;
                m_lstFree.head = NULL;
                m_lstFree.tail = NULL;
            }
            
            ~SHHaspMap()
            {
                CHNode *p;
                while(NULL != m_lstFree.head)
                {
                    p               = m_lstFree.head;
                    m_lstFree.head  = p->next;
                    delete p;
                }
                
                if(NULL != m_pBucket)
                {
                    for(uint32_t i = 0; i < m_nBuckets; i++)
                    {
                        while(NULL != m_pBucket[i])
                        {
                            p            = m_pBucket[i];
                            m_pBucket[i] = p->next;
                            delete p;
                        }
                    }
                    
                    delete []m_pBucket;
                    m_pBucket = NULL;
                }
            }
            
            /* 初始化
            *  @size     预分配节点数量(不大于5*buckets)
            *  @pHash    Hash函数的指针
            *  @buckets  桶的数量.  
            *  @Return   SH_HM_OK:成功    其他:失败
            */
            int Init(uint32_t size, FHASH pHash, uint32_t buckets)
            {
                if(size < 0) {
                    return -1;
                }
                
                if(size > 5*buckets)
                {
                    return -2;
                }

                if(NULL == pHash) {
                    return -3;
                }

                m_nTotalSize = size;
                m_nBuckets   = buckets;
                m_pMyHash    = pHash;

                CHNode *pTotal = new CHNode[m_nTotalSize];
                if(NULL != pTotal)
                {  
                    for(uint32_t i = 0; i < m_nTotalSize; i++)
                    {    
                        CHNode *p = pTotal + i;
                        //printf("mNode=%llu| mVal=%llu\r\n", (uint64_t)p, (uint64_t)&p->val);
                        PushNode(p);
                    }
                }
                else
                {
                    for(uint32_t i = 0; i < m_nTotalSize; i++)
                    {
                        CHNode *p = new CHNode;
                        if(NULL == p)
                        {
                            return -11;
                        }

                        //printf("mNode=%llu| mVal=%llu\r\n", (uint64_t)p, (uint64_t)&p->val);
                        PushNode(p);
                    }
                }

                m_nUseSize   = 0;
                m_pBucket = new CHNode*[m_nBuckets];
                if(NULL == m_pBucket) 
                {
                    return -4;
                }
                else 
                {
                    for(uint32_t i = 0; i < m_nBuckets; i++)
                    {
                        m_pBucket[i] = NULL;
                    }
                }

                return SH_HM_OK;
            }

            /* 查询记录
            *  @Return    记录的指针
            */
            _Tval* Get(const _Tkey &k)
            {
                uint32_t hash = Hash(k);

                if(NULL == m_pBucket || NULL == m_pBucket[hash]) {
                    return NULL;
                }
                
                CHNode *pNode = m_pBucket[hash];
                while(NULL != pNode)
                {
                    if(pNode->key == k) 
                    {
                        return &pNode->val;
                    }

                    pNode = pNode->next;
                }
                    
                return NULL;
            }
            
           
            /* 添加记录:添加key-value对. value的内容由输出参数的地址再进行设置[若设置失败需要调用者主动删除]
            *  @pVal     输出参数
            *  @Return   见“错误码定义”
            */
            int Add(const _Tkey &k, _Tval* &pVal)
            {
                uint32_t hash = Hash(k);
         
                if(NULL == m_pBucket) 
                {
                    return SH_HM_NOT_INIT;
                }

                if(NULL != pVal) {
                    return SH_HM_INTERNAL_ERR;
                }

                int nKCnt = 0;
                CHNode *en = m_pBucket[hash];
                while(NULL != en)
                {
                    //存在是否替换??
                    if(en->key == k) 
                    {   
                        //直接返回错误
                        return SH_HM_RECORD_EXIST;
                    }

                    en = en->next;
                    ++nKCnt;
                }

                if(nKCnt > 100) {
                    LOG(LT_WARN, "SH_HASH_MAP| the hash have %d records", nKCnt);
                }
                else if(nKCnt > 1000) {
                    LOG(LT_ERROR, "SH_HASH_MAP| the hash records limited| hash=%d", nKCnt);
                    return SH_HM_HASH_MAX_CNT;
                }

                //赋值
                CHNode *pNode = PopNode();
                if(NULL == pNode) {
                    return SH_HM_NO_MEMORY;
                }

                pNode->key  = k;
                pVal        = &pNode->val;
                //pNode->next = NULL;  -- PopNode已赋NULL
                //pNode->prev = NULL;  -- PopNode已赋NULL

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
                
                return SH_HM_OK;
            }

            
            /* 删除记录
            *  成功删除时会自动调用 PushNode回收
            *  @Return   见“错误码定义”
            */
            int Del(const _Tkey &k)
            {
                uint32_t hash = Hash(k);
                if(NULL == m_pBucket[hash])
                {
                    return SH_HM_NO_RECORD;
                }
                
                //处在第一个节点
                if(m_pBucket[hash]->key == k)
                {
                    CHNode *del = m_pBucket[hash];
                    m_pBucket[hash] = del->next;
                    del->next       = NULL;
                    /*if(NULL == m_pBucket[hash]) {
                        printf("del first node| hash=%d| key=%d| last node\r\n", hash, del->key);
                    }
                    else {
                        printf("del first node| hash=%d|  key=%d| unlast node\r\n", hash, del->key);
                    }*/
                    
                    PushNode(del);
                    return SH_HM_OK;
                }

                //不是第一个节点
                CHNode *pre = m_pBucket[hash];
                CHNode *del = pre->next;
                while(NULL != del)
                {   
                    if(del->key == k) 
                    {
                        pre->next = del->next;
                        del->next = NULL;
                        
                        /*if(NULL == pre->next) {
                            printf("del not first node| hash=%d| key=%d| last node\r\n", hash, del->key);
                        }
                        else {
                            printf("del not first node| hash=%d| key=%d| unlast node\r\n", hash, del->key);
                        }*/

                        PushNode(del);
                        return SH_HM_OK;
                    }

                    pre = del;
                    del = del->next;
                }
                

                return SH_HM_NO_RECORD;
            }

            void Clear()
            {
                m_pNextNode  = NULL;
                
                CHNode *node = NULL;
                CHNode *del  = NULL;
                for(uint32_t i = 0; i < m_nBuckets; i++) 
                {
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
            
            int     Size() { return m_nUseSize; }
            int     Free() { return (m_nTotalSize - m_nUseSize); }

            /* 返回第一个节点指针, 用于遍历功能
            *  @Return  返回第一个节点的指针; 空则返回NULL
            */
            _Tval* Begin()
            {
                for(m_nCurBucket = 0; m_nCurBucket < m_nBuckets; m_nCurBucket++) 
                {
                    if(NULL != m_pBucket[m_nCurBucket])
                    {
                        m_pNextNode  = m_pBucket[m_nCurBucket]->next;
                        return &m_pBucket[m_nCurBucket]->val;
                    }
                }

                return NULL;
            }

            /* 返回下一个节点指针, 用于遍历功能
            *  @Return  返回下一个节点的指针; 已遍历完则返回NULL
            */
            _Tval* Next()
            {
                if(NULL != m_pNextNode) 
                {   
                    CHNode *node = m_pNextNode;
                    m_pNextNode  = m_pNextNode->next;
                    return &node->val;
                }

                m_nCurBucket++;
                for(; m_nCurBucket < m_nBuckets; m_nCurBucket++)
                {
                    if(NULL != m_pBucket[m_nCurBucket])
                    {
                        m_pNextNode  = m_pBucket[m_nCurBucket]->next;
                        return &m_pBucket[m_nCurBucket]->val;
                    }
                }                

                return NULL;
            }

        private:
            CHNode* PopNode()
            {
                if(NULL == m_lstFree.tail)
                {
                    return NULL;
                }

                //返回最后一个节点
                CHNode* p       = m_lstFree.tail;
                m_lstFree.tail  = p->prev;  //此时tail有可能是NULL
                p->next         = NULL;
                p->prev         = NULL;

                if(NULL == m_lstFree.tail) 
                {
                    m_lstFree.head = NULL;
                }

                ++m_nUseSize;
                
                return p;
            }
            
            void PushNode(CHNode *node)
            {
                if(NULL == node) 
                {
                    return;
                }

                //插入头
                if(NULL == m_lstFree.head)
                {   
                    //空表
                    m_lstFree.head = node;
                    m_lstFree.tail = node;
                    node->next = NULL;
                    node->prev = NULL;
                }
                else 
                {
                    node->next = m_lstFree.head;
                    node->prev = NULL;
                    m_lstFree.head->prev = node;
                    m_lstFree.head       = node;
                }
                    
                --m_nUseSize;
            }
            
            uint32_t Hash(const _Tkey &k)
            {
                 return ((*m_pMyHash)(k))%m_nBuckets;
            }
            
        private:
            CHNode    **m_pBucket;              //桶
            FreeList    m_lstFree;              //空闲节点列表
            uint32_t    m_nUseSize;             //已使用的节点数
            uint32_t    m_nTotalSize;           //总节点数
            uint32_t    m_nBuckets;             //桶的数量
            FHASH       m_pMyHash;              //hash函数指针

            //遍历的参数
            uint32_t    m_nCurBucket;           //当前bucket
            CHNode     *m_pNextNode;            //下一个节点
        };
    
    }
}


#endif //__THASH_MAP_H_


