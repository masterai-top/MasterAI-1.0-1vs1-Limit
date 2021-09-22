/**
* \file eventdef.h
* \brief 事件的基础定义
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __EVENT_DEFINE_H__
#define __EVENT_DEFINE_H__

#include "typedef.h"
#include <string>

namespace frame
{
    /**
    * \brief 事件引擎宏定义
    * \ingroup event_group
    */
    enum
    {
        HANDLE_COUNT_MAX = 20,                  ///< 事件处理最大层数
        REF_COUNT_MAX = 5,                      ///< 事件引用最大层数
        SUBSCRIBER_POOL_INIT_NUM = 50,          ///< 订阅者池初始数量
        REACTOR_EVENT_POOL_INIT_NUM = 50,       ///< 响应器事件池初始数量
    };

    /**
    * \brief 事件类型
    * \ingroup event_group
    */
    enum EVENT_TYPE
    {
        EVENT_TYPE_UNKNOW = 0,                  ///< 未知事件

        // 事件派发器
        EVENT_EXECUTE = 1,                      ///< 执行发布事件
        EVENT_VOTE = 2,                         ///< 票决发布事件

        // 事件响应器
        EVENT_SIGNAL = 3,                       ///< 信号响应事件
        EVENT_TIMER = 4,                        ///< 超时响应事件
        EVENT_SOCKET = 5,                       ///< 网络响应事件
        EVENT_NOTIFY = 6,                       ///< 通知响应事件
        EVENT_PIPE = 7,                         ///< 管道响应事件
    };

    /**
    * \brief 事件key
    * \ingroup event_group
    */
    typedef struct _EVENT_KEY
    {
        // 8 + 8 + 16 + 32 + 32 + 32 + 64 = 8 * 24 = 192 bit = 24 byte    
        uint64  m_nEventType : 8;   ///< 事件类型 EVENT_TYPE
        uint64  m_nReserve8 : 8;    ///< 预留8位
        uint64  m_nReserve16 : 16;  ///< 预留16位
        uint64  m_nReserve32 : 32;  ///< 预留32位
        uint32  m_nEventID;         ///< 事件ID
        uint32  m_nSrcType;         ///< 发送源类型
        uint64  m_nSrcID;           ///< 发送源标识

        /**
        * \brief 构造函数
        */
        _EVENT_KEY()
        {
            m_nEventType = 0;
            m_nReserve8 = 0;
            m_nReserve16 = 0;
            m_nReserve32 = 0;
            m_nSrcType = 0;
            m_nEventID = 0;
            m_nSrcID = 0;
        }

        /**
        * \brief 构造函数
        * \param nEventType 事件类型
        * \param nEventID 事件ID
        * \param nSrcType 发送源类型
        * \param nSrcID 发送源标识
        */
        _EVENT_KEY(uint8 nEventType, uint32 nEventID, uint32 nSrcType, uint64 nSrcID, uint8 nReserve8 = 0, uint16 nReserve16 = 0, uint32 nReserve32 = 0)
        {
            m_nEventType = nEventType;
            m_nEventID = nEventID;
            m_nSrcType = nSrcType;
            m_nSrcID = nSrcID;
            m_nReserve8 = nReserve8;
            m_nReserve16 = nReserve16;
            m_nReserve32 = nReserve32;
        }

        /**
        * \brief ==比较函数重载
        * \param key 比较对象
        * \return 相等返回true，不等返回false
        */
        bool operator == (const _EVENT_KEY &key)
        {
            return (m_nEventType == key.m_nEventType) && (m_nEventID == key.m_nEventID) &&
                (m_nSrcType == key.m_nSrcType) && (m_nSrcID == key.m_nSrcID) &&
                (m_nReserve8 == key.m_nReserve8) && (m_nReserve16 == key.m_nReserve16) &&
                (m_nReserve32 == key.m_nReserve32);
        }

        /**
        * \brief =赋值函数重载
        * \param key 赋值对象
        * \return 赋值后对象
        */
        _EVENT_KEY & operator = (const _EVENT_KEY &key)
        {
            m_nEventType = key.m_nEventType;
            m_nEventID = key.m_nEventID;
            m_nSrcType = key.m_nSrcType;
            m_nSrcID = key.m_nSrcID;
            m_nReserve8 = key.m_nReserve8;
            m_nReserve16 = key.m_nReserve16;
            m_nReserve32 = key.m_nReserve32;

            return *this;
        }

        /**
        * \brief <比较函数重载
        * \param key 比较对象
        * \return 小于比较对象返回true，否则返回false
        */
        bool operator < (const _EVENT_KEY &key) const
        {
            // 先比较事件类型
            if (m_nEventType < key.m_nEventType)
            {
                return true;
            }
            else if (m_nEventType > key.m_nEventType)
            {
                return false;
            }

            // 再比较事件ID
            if (m_nEventID < key.m_nEventID)
            {
                return true;
            }
            else if (m_nEventID > key.m_nEventID)
            {
                return false;
            }

            // 再比较发送源类型
            if (m_nSrcType < key.m_nSrcType)
            {
                return true;
            }
            else if (m_nSrcType > key.m_nSrcType)
            {
                return false;
            }

            // 再比较发送源ID
            if (m_nSrcID < key.m_nSrcID)
            {
                return true;
            }
            else if (m_nSrcID > key.m_nSrcID)
            {
                return false;
            }

            // 再比较预留1
            if (m_nReserve8 < key.m_nReserve8)
            {
                return true;
            }
            else if (m_nReserve8 > key.m_nReserve8)
            {
                return false;
            }

            // 再比较预留2
            if (m_nReserve16 < key.m_nReserve16)
            {
                return true;
            }
            else if (m_nReserve16 > key.m_nReserve16)
            {
                return false;
            }

            // 最后比较预留3
            if (m_nReserve32 < key.m_nReserve32)
            {
                return true;
            }
            else if (m_nReserve32 > key.m_nReserve32)
            {
                return false;
            }

            // 都相等
            return false;
        }
    } EVENT_KEY;
}

#endif // __EVENT_DEFINE_H__