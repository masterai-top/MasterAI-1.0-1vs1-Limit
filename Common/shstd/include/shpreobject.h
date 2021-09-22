/**
* \file sh_Tval.h
* \brief 预分配对象
*
* Copyright (c) 2019
* All rights reserved.
*/

#ifndef __SH_PRE_OBJECT_H__
#define __SH_PRE_OBJECT_H__
#include "shlock_in.h"
#include "shlog.h"

using namespace shstd::lock;

namespace shstd
{
    template <typename T>
    class PreObject
    {
    public:
        PreObject(void)
        {
            m_elems = NULL;
            m_size  = 0;
            m_init_size     = 0;
            m_dynamic_size  = 0;
            memset(m_szLogSymble, 0, sizeof(m_szLogSymble));
        }

        int Init(const char *log_symbol, size_t cnt)
        {
            if(cnt <= 1) {
                return -101;
            }
            
            m_size  = cnt;
            m_elems = (T **)malloc( sizeof(T *) * m_size );
            if(NULL == m_elems) {
                return -102;
            }
            
            for(size_t i=0; i<m_size; i++)
            {
                m_elems[i] = new T();
            }

            m_init_size = m_size;
            snprintf(m_szLogSymble, sizeof(m_szLogSymble) - 1, "%s", log_symbol);
            LOG(LT_WARN, "%s_INIT| dynamic new| dynamic_size=%u| init_size=%u", m_szLogSymble, m_dynamic_size, m_init_size);
            return 0;
        }

        /**
        * \brief 析构函数
        */
        virtual ~PreObject(void)
        {
            for(size_t i=0; i<m_size; i++)
            {
                delete m_elems[i];
            }

            free(m_elems);
            m_elems = NULL;
            m_size  = 0;
        }

        /**
        * \brief 弹出对象
        * \return 模板对象
        */
        T* Pop()
        {
            T* t = NULL;
            CSafeMutex lock(m_mutex);
            if (m_size <= 0)
            {
                //无可用节点,最多再申请1/3的节点
                if(m_dynamic_size > (m_init_size/3)) {
                    return NULL;
                }

                t = new T;
                if(NULL != t) {
                    ++m_dynamic_size;
                    LOG(LT_WARN, "%s| dynamic new| dynamic_size=%u| init_size=%u", m_szLogSymble, m_dynamic_size, m_init_size);
                }

                return t;
            }
            else 
            {            
                t = m_elems[--m_size];
                return t;
            }
        }

        /**
        * \brief 压入对象
        * \param t 模板对象
        */
        void Push(T *t)
        {
            if (NULL == t) {
                return;
            }
            
            CSafeMutex lock(m_mutex);            
            m_elems[m_size++] = t;
        }

        size_t size()
        {
            CSafeMutex lock(m_mutex);
            return m_size;
        }

        size_t init_size() {
            return m_init_size;
        }

        size_t dynamic_size() {
            CSafeMutex lock(m_mutex);
            return m_dynamic_size;
        }
        
    private:
        T               **m_elems;      //节点数据
        size_t          m_size;         //可用节点数量
        size_t          m_init_size;    //初始化的节点数量
        size_t          m_dynamic_size; //动态分配的节点数量
        CMutex          m_mutex;
        char            m_szLogSymble[32];  
    };
}

#endif // __SH_PRE_OBJECT_H__
