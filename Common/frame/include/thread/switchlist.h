/**
* \file switchlist.h
* \brief 线程交换列表
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __SWITCH_LIST_H__
#define __SWITCH_LIST_H__

#include "framedef.h"
#include "thread/lock.h"
#include <list>

namespace frame
{
    /**
    * \brief 线程交换列表
    */
    template <class T>
    class FRAME_TEMPLATE_EXPORT SwitchList
    {
    public:
        typedef std::list<T> ITEM_LIST;         ///< 对象列表

    public:
        /**
        * \brief 构造函数
        */
        SwitchList() : m_pInList(&m_lstItems1), m_pOutList(&m_lstItems2) { }

        /**
        * \brief 析构函数
        */
        ~SwitchList() { }

        /**
        * \brief 放入列表前
        * \param item 对象
        */ 
        void PushFront(T *item) 
        {
            Lock lock(&m_Mutex);
            if (NULL != item)
            {
                m_pInList->push_front(*item);
            }
        }

        /**
        * \brief 放入列表后
        * \param item 对象
        */ 
        void Push(T *item)
        {
            Lock lock(&m_Mutex);
            if (NULL != item)
            {
                m_pInList->push_back(*item);
            }
        }

        /**
        * \brief 反转列表
        */
        void Switch()
        {
            Lock lock(&m_Mutex);

            // 若输出列表有item，不处理
            if (m_pOutList->empty())
            {
                ITEM_LIST *pList = m_pInList;
                m_pInList = m_pOutList;
                m_pOutList = pList;
            }
        }

        /**
        * \brief 获取输入列表
        * \return 输入列表
        */
        ITEM_LIST & InList()
        {
            return *m_pInList;
        }

        /**
        * \brief 获取输出列表
        * \return 输出列表
        */
        ITEM_LIST & OutList()
        {
            return *m_pOutList;
        }

    private:
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif
        ITEM_LIST           m_lstItems1;        ///< 列表1
        ITEM_LIST           m_lstItems2;        ///< 列表2

        ITEM_LIST           *m_pInList;         ///< 当前输入列表
        ITEM_LIST           *m_pOutList;        ///< 当前输出列表

        LockObject          m_Mutex;            ///< 互斥锁
#ifdef _MSC_VER
#pragma warning(pop)
#endif
    };
}

#endif // __SWITCH_LIST_H__