/**
* \file tevent.cpp
* \brief 信号事件类实现代码
*/

#include "pch.h"
#include "thread/tevent.h"
#include "timeutil.h"
using namespace dtutil;

namespace frame
{
    /**
    * \brief 构造函数
    * \param state 初始状态，默认false
    * \param reset 是否自动复位，默认false
    *
    * 非自动复位模式下，有信号状态一直持续到调用Reset()。
    * 自动复位模式，当有信号状态持续到单个等待线程被释放时，会自动复位为无信号状态
    */
    TEvent::TEvent(bool state /*= false*/, bool reset /*= false*/)
        : m_bState(state), m_bReset(reset)
    {
    }

    /**
    * \brief 析构函数
    */
    TEvent::~TEvent(void)
    {
        Lock lock(&m_Mutex);

        // 通知所有等待的线程
        m_Cond.Broadcast();

        // 释放mutex上的锁，使所有等待线程可以取得锁以从cond.wait()中返回
        // 设置超时为0，以便一旦所有线程退出wait()释放锁之后，
        // 可以立即取回mutex锁，以便继续执行析构
        m_Cond.TimeWait(lock, TimeUtil::Delay(0));
    }

    /**
    * \brief 设置为有信号状态
    */
    void TEvent::Set()
    {
        Lock lock(&m_Mutex);

        m_bState = true;

        // 通知等待线程
        if (m_bReset)
        {
            m_Cond.Signal();        // 自动复位时，只通知一个
        }
        else
        {
            m_Cond.Broadcast();
        }
    }

    /**
    * \brief 复位为无信号状态
    */
    void TEvent::Reset()
    {
        Lock lock(&m_Mutex);

        m_bState = false;
    }

    /**
    * \brief 等待有信号状态
    * \param secs 超时的秒，默认-1，表示不超时
    * \param msecs 超时的毫秒，默认0
    * \param nsecs 超时的纳秒，默认0
    * \return true表示有信号，false表示等待超时，或者Event被释放
    */
    bool TEvent::Wait(int32 secs /*= -1*/, int32 msecs /*= 0*/, int32 nsecs /*= 0*/)
    {
        Lock lock(&m_Mutex);

        if (!m_bState)
        {
            // 超时时间为0时，仅检查状态
            if ( (secs | msecs | nsecs) == 0 )
            {
                return false;
            }

            // secs < 0 时持续等待
            if (secs < 0)
            {
                m_Cond.Wait(lock);
            }
            // 超时时间大于0时，等待信号通知或超时
            else if ( !m_Cond.TimeWait( lock, TimeUtil::Delay(secs, msecs, nsecs) ) )
            {
                return false;   // 等待超时
            }
        }

        bool state = m_bState;

        // 如果自动复位，则复位
        if (m_bReset && m_bState)
        {
            m_bState = false;
        }

        return state;
    }
}