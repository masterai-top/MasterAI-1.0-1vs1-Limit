/**
* \file array_pool.h
* \brief 数组池类
*
* 封装了一个线程安全的数组池类
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __ARRAY_POOL_H__
#define __ARRAY_POOL_H__

#include "framedef.h"
#include "thread/lock.h"

namespace frame
{
    /**
    * \brief 数组池类
    * \ingroup memory_group
    */
    template <typename T, size_t count>
    class FRAME_TEMPLATE_EXPORT ArrayPool
    {
    public:
        /**
        * \brief 构造函数
        */
        ArrayPool(void) : m_size(count)
        {
            m_elems = (T **)malloc( sizeof(T *) * m_size );
            for(size_t i=0; i<m_size; i++)
            {
                m_elems[i] = new T();
            }
        }

        /**
        * \brief 析构函数
        */
        virtual ~ArrayPool(void)
        {
            for(size_t i=0; i<m_size; i++)
            {
                delete m_elems[i];
            }

            free(m_elems);
            m_elems = NULL;
            m_size = 0;
        }

        /**
        * \brief 弹出对象
        * \return 模板对象
        * 若数组已空，new新对象
        */
        T* Pop()
        {
            Lock lock(&m_Lock);

            T* t = NULL;

            if (m_size <= 0)
            {
                t = new T();
            }
            else
            {
                t = m_elems[--m_size];
            }

            return t;
        }

        /**
        * \brief 压入对象
        * \param t 模板对象
        * 若数组已满，直接删除
        */
        void Push(T *t)
        {
            Lock lock(&m_Lock);

            if (NULL == t)
            {
                return;
            }

            if (m_size >= count)
            {
                delete t;
                return;
            }

            m_elems[m_size++] = t;
        }

        size_t size()
        {
            Lock lock(&m_Lock);
            return m_size;
        }
        
    private:
        T               **m_elems;      ///< 节点数据
        size_t          m_size;         ///< 节点数量
        LockObject      m_Lock;         ///< 锁
    };
}

#endif // __ARRAY_POOL_H__