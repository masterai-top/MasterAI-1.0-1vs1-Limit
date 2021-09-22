/**
* \file shhashmap.h
* \brief hashmap 模板
* 使用说明
     1. 需要实现(返回值uint32_t, 输入参数为_Tkey)的哈希函数
     2. _Tkey, _Tval 都支持自定义类型
     3. 非线程安全

     例子:
        struct TValue
        {   
            char    m_szBuf[10];

            const TValue & operator=(const TValue &v) {
                memcpy(m_szBuf, v.m_szBuf, sizeof(m_szBuf));
                return *this;
            }

            static uint32_t MyHash(const int &key)    {
                return key;
            }
        };

        使用：
        CTHaspMap<int, HValue> oHm;
        err = hm.Init(50, TValue::MyHash, 100);
        if(0 != err) {
            printf("init failed| err=%d\r\n", err);
            exit(0);
        }

        HValue v;
        oHm.Set(1, v);
        oHm.Get(1, v);
        oHm.Del(1); 
*/

#ifndef __THASH_MAP1_H_
#define __THASH_MAP1_H_
#include <stdlib.h>
#include "shlog.h"
using namespace std;

namespace shstd
{
	namespace hashmap
	{
	    //常亮定义
        const int DEFAULT_BUCKET_NUM   = 100000;

        //错误码定义
        const int HM_OK                 = 0;
        const int HM_NO_RECORD          = 1081;     //无记录
        const int HM_NO_MEMORY          = 1082;     //存储节点不足 
        const int HM_NOT_INIT           = 1083;     //未初始化
        const int HM_HASH_MAX_CNT       = 1084;     //hash值太多主键了
        const int HM_RECORD_EXIST       = 1085;     //记录已存在
        const int HM_INTERNAL_ERR       = 1086;     //内部错误
    
        template <class _Tkey, class _Tval> 
        class CTHaspMap
        {
        public:
            typedef uint32_t (*FHASH)(const _Tkey &k);
            struct CHNode {
                _Tkey    key;
                _Tval    val;
                CHNode  *next;
            };
            
        public:
            CTHaspMap() 
            {
                m_pMyHash  = NULL;
                m_pBucket  = NULL;
                m_nBuckets = 0;
                m_oFreeHead.next = NULL;
            }
            
            ~CTHaspMap()
            {
                CHNode *p;
                while(NULL != m_oFreeHead.next)
                {
                    p                = m_oFreeHead.next;
                    m_oFreeHead.next = p->next;
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
            *  @Return   HM_OK:成功    其他:失败
            */
            int Init(uint32_t size, FHASH pHash, uint32_t buckets = DEFAULT_BUCKET_NUM)
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
                m_nUseSize   = 0;
                m_nBuckets   = buckets;
                m_pMyHash    = pHash;
                
                for(uint32_t i = 0; i < m_nTotalSize; i++)
                {
                    CHNode *p = new CHNode;
                    if(NULL == p)
                    {
                        return -11;
                    }

                    p->next          = m_oFreeHead.next;
                    m_oFreeHead.next = p;
                }

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
               
                return HM_OK;
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
                
                CHNode *p = m_pBucket[hash];
                while(NULL != p)
                {
                    if(p->key == k) 
                    {
                        return &p->val;
                    }

                    p = p->next;
                }
                    
                return NULL;
            }
            
            /* 查询记录
            *  @Return    见“错误码定义”
            */
            int Get(const _Tkey &k, _Tval &v)
            {
                uint32_t hash = Hash(k);

                if(NULL == m_pBucket) {
                    return HM_NOT_INIT;
                }
                
                if(NULL == m_pBucket[hash])
                {
                    return HM_NO_RECORD;
                }

                CHNode *p = m_pBucket[hash];
                while(NULL != p)
                {
                    if(p->key == k) 
                    {
                        v = p->val;
                        return HM_OK;
                    }

                    p = p->next;
                }
                    
                return HM_NO_RECORD;
            }

            /* 设置(存储)记录
            *  @cover    记录已存在时存储被覆盖的值的指针
            *  @Return   见“错误码定义”
            */
            int Set(const _Tkey &k, const _Tval &v, _Tval *cover = NULL)
            {
                uint32_t hash = Hash(k);
         
                if(NULL == m_pBucket) 
                {
                    return HM_NOT_INIT;
                }

                int nKCnt = 0;
                
                CHNode *p = m_pBucket[hash];
                while(NULL != p)
                {
                    //存在则直接替换
                    if(p->key == k) 
                    {   
                        if(NULL != cover) {
                            *cover = p->val;
                        }
                        
                        p->val    = v;                        
                        return HM_OK;
                    }

                    p = p->next;
                    ++nKCnt;
                }

                if(nKCnt > 100) {
                    LOG(LT_WARN, "HASHMAP| the hash have %d records", nKCnt);
                }
                else if(nKCnt > 1000) {
                    LOG(LT_ERROR, "HASHMAP| the hash records limited| hash=%d", nKCnt);
                    return HM_HASH_MAX_CNT;
                }
                
                p = PopNode();
                if(NULL == p) 
                {
                    return HM_NO_MEMORY;
                }

                //赋值
                p->val  = v;
                p->key  = k;
                p->next = NULL;

                if(NULL == m_pBucket[hash])
                {
                    //桶的第一次插入
                    m_pBucket[hash] = p;
                    p->next         = NULL;
                }
                else
                {
                    //桶已存在其他节点,插入头部
                    p->next         = m_pBucket[hash];
                    m_pBucket[hash] = p;
                }

                
                return HM_OK;
            }

            
            /* 删除记录
            *  @v        成功删除记录的指针
            *  @Return   见“错误码定义”
            */
            int Del(const _Tkey &k, _Tval *v = NULL)
            {
                uint32_t hash = Hash(k);

                if(NULL == m_pBucket) {
                    return HM_NOT_INIT;
                }
                
                if(NULL == m_pBucket[hash])
                {
                    return HM_NO_RECORD;
                }

                CHNode *del = m_pBucket[hash];

                //处在第一个节点
                if(m_pBucket[hash]->key == k)
                {
                    m_pBucket[hash] = del->next;
                    del->next       = NULL;
                    if(NULL != v) {
                        *v = del->val;
                    }
                    PushNode(del);
                    return HM_OK;
                }
                else
                {
                    CHNode *pre = del;
                    del         = del->next;
                    while(NULL != del)
                    {   
                        if(del->key == k) 
                        {
                            pre->next = del->next;
                            del->next = NULL;
                            if(NULL != v) {
                                *v = del->val;
                            }
                            PushNode(del);
                            return HM_OK;
                        }

                        pre = del;
                        del = del->next;
                    }
                }

                return HM_NO_RECORD;
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
            CHNode* Begin()
            {
                for(m_nCurBucket = 0; m_nCurBucket < m_nBuckets; m_nCurBucket++) 
                {
                    if(NULL != m_pBucket[m_nCurBucket])
                    {
                        m_pNextNode  = m_pBucket[m_nCurBucket]->next;
                        return m_pBucket[m_nCurBucket];
                    }
                }

                return NULL;
            }

            /* 返回下一个节点指针, 用于遍历功能
            *  @Return  返回下一个节点的指针; 已遍历完则返回NULL
            */
            CHNode* Next()
            {
                if(NULL != m_pNextNode) 
                {   
                    CHNode *node = m_pNextNode;
                    m_pNextNode  = m_pNextNode->next;
                    return node;
                }

                m_nCurBucket++;
                for(; m_nCurBucket < m_nBuckets; m_nCurBucket++)
                {
                    if(NULL != m_pBucket[m_nCurBucket])
                    {
                        m_pNextNode  = m_pBucket[m_nCurBucket]->next;
                        return m_pBucket[m_nCurBucket];
                    }
                }                

                return NULL;
            }
            
        private:
            uint32_t Hash(const _Tkey &k)
            {
                 return ((*m_pMyHash)(k))%m_nBuckets;
            }

            CHNode* PopNode()
            {
                if(NULL == m_oFreeHead.next)
                {
                    return NULL;
                }

                //返回第一个节点
                CHNode* p        = m_oFreeHead.next;
                m_oFreeHead.next = p->next;
                p->next          = NULL;

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
                node->next       = m_oFreeHead.next;
                m_oFreeHead.next = node;

                --m_nUseSize;
            }

         
        private:
            CHNode    **m_pBucket;              //桶
            CHNode      m_oFreeHead;            //空闲列表头指针 
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


#endif //__THASH_MAP1_H_


