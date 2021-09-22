/**
* \file thread.cpp
* \brief 线程封装类实现代码
*/

#include "pch.h"
#include "thread/thread.h"
#include "system.h"
using namespace dtutil;


#ifdef WIN32
#   define CATCHALL catch (...)
#else
#   include <cxxabi.h>
#   define CATCHALL catch (abi::__forced_unwind&) { throw; } catch (...)
#endif

namespace frame
{
    /**
    * \brief 构造函数
    */
    Thread::Thread(void) : m_bRunning(false)
    {
#ifndef WIN32
        m_Thread = 0;
#endif
    }

    /**
    * \brief 析构函数
    */
    Thread::~Thread(void)
    {
        //Lock lock(&m_Mutex);

        Kill();

#ifndef WIN32
        m_Thread = 0;
#endif
    }

    /**
    * \brief 启动线程
    * \param name 线程名称
    *
    * 如果线程正在执行，则不会启动新线程
    */
    void Thread::Run(const char *name /*= "thread"*/)
    {
        //Lock lock(&m_Mutex);

        // 正在执行中
        if (IsRunning())
        {
            return;
        }

        m_StopEvent.Reset();

        // 非分离状态(PTHREAD_CREATE_DETACHED)线程，pthread_join()才能有效。
        // 无需初始化线程属性，线程属性默认即是非分离状态。
        pthread_attr_init(&m_Attr);
        pthread_attr_setdetachstate(&m_Attr, PTHREAD_CREATE_JOINABLE);

        // 创建线程
        pthread_create(&m_Thread, &m_Attr, InternalThreadProc, (void*)this);

        m_bRunning = true;
        m_strName = name;
    }

    /**
    * \brief 通知线程停止
    * \param wait 是否等待线程结束，默认为false
    *
    * 默认不等待线程结束，如果设置了wait为true，则函数一直阻塞到线程结束运行才返回
    */
    void Thread::Stop(bool wait /*= false*/)
    {
        // 线程已经停止了
        if (!IsRunning())
        {
            return;
        }

        // 通知线程结束
        m_StopEvent.Set();

        // 若是重载对象在OnThreadProc中阻塞了，m_StopEvent.Set()无法通知结束
        // 通过重载对象的OnThreadStop函数，让重载对象通知线程退出阻塞
        OnThreadStop();

        // 设置了等待，则阻塞等待
        if (wait)
        {
            pthread_join(m_Thread, NULL);
        }
    }

    /**
    * \brief 强制杀死线程，等待线程结束
    *
    * 函数一直阻塞到线程结束运行才返回
    */
    void Thread::Kill()
    {
        // 线程已经停止了
        if (!IsRunning())
        {
            return;
        }

        // 通知线程结束
        // 使用pthread_cancel让线程抛出一个异常，catch该异常做退出线程处理
        pthread_cancel(m_Thread);

        // 阻塞等待
        pthread_join(m_Thread, NULL);
    }

    /**
    * \brief 判断线程是否正在运行
    * \return 是返回true，否则返回fasle
    */
    bool Thread::IsRunning()
    {
#ifdef WIN32
        // win32判断线程是否正在运行，有没更好的方法？
        return m_bRunning;
#else
        if (m_Thread == 0)
        {
            return false;
        }

        int err = pthread_kill(m_Thread,0);

        // 线程不存在或者已经退出
        if(err == ESRCH)
        {
            return false;
        }
        // 发送信号非法
        else if(err == EINVAL)
        {
            return false;
        }
        // 线程目前仍然存活
        else
        {
            return true;
        }
#endif
    }

    /**
    * \brief 线程执行函数
    * \return 线程继续执行返回true，线程终止返回false
    * 
    * 此函数被私有的线程入口函数InternalThreadProc()调用，
    * 重载此虚函数以实现不同的线程执行处理
    */
    bool Thread::OnThreadProc()
    {
        return true;
    }

    /**
    * \brief 线程停止函数
    * 
    * 此函数被线程停止函数Stop()调用，以免重载的线程对象OnThreadProc()是阻塞等待的，
    * 重载此虚函数以实现重载对象通知退出阻塞等待
    */
    void Thread::OnThreadStop()
    {
    }

    /**
    * \brief 检查停止通知事件是否被置位
    * \return true表示线程停止通知已被置位，否则返回fasle
    */
    bool Thread::CheckStop()
    {
        return m_StopEvent.Wait(0);
    }

    /**
    * \brief 线程是否PTHREAD_CREATE_JOINABLE
    * \return 是返回true，否则返回false
    */
    bool Thread::Joinable()
    {
        int32 nDetach = 0;
        pthread_attr_getdetachstate(&m_Attr, &nDetach);
        if (nDetach == PTHREAD_CREATE_JOINABLE)
        {
            return true;
        }

        return false;
    }

    /**
    * \brief 线程入口函数
    * \param lpThread 线程对象
    *
    * 用于创建线程的入口函数，执行期间锁定m_Mutex表示线程正在运行
    */
    void * Thread::InternalThreadProc(void *lpThread)
    {
        Thread *pThread = (Thread *)lpThread;
        if (NULL == pThread)
        {
            return 0;
        }

        // 设置本线程对Cancel信号的反应，state有两种值：PTHREAD_CANCEL_ENABLE（缺省）和 PTHREAD_CANCEL_DISABLE，
        // 分别表示收到信号后设为CANCLED状态和忽略CANCEL信号继续运行；old_state如果不为NULL则存入原来的Cancel状态以便恢复。
        //pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);		// 允许退出线程 
        // 设置本线程取消动作的执行时机，type有两种取值：PTHREAD_CANCEL_DEFERRED 和 PTHREAD_CANCEL_ASYNCHRONOUS，
        // 仅当Cancel状态为Enable时有效，分别表示收到信号后继续运行至下一个取消点再退出和立即执行取消动作（退出）；
        // old_type如果不为NULL则存入运来的取消动作类型值。
        //pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);   // 设置立即取消 

        while(!pThread->CheckStop())
        {
            //pthread_testcancel();/*the thread can be killed only here*/

            bool bContinue = false;
            try
            {
                // 线程执行处理
                bContinue = pThread->OnThreadProc();
            }
            CATCHALL        // 一旦调用了pthread_cancel()，被谋杀者所在线程会抛出一个异常
                // 1. 其实，最好不要用exception
                // 2. 如果要用exception，那就最好不要用pthread_cancel
                // 3. 如果非要用，那就老老实实re-throw exception吧（在catch all这种情况）
            {
                // 线程被cancel掉
            }

            // 线程要求终止
            if (!bContinue)
            {
                break;
            }
        }

        pThread->m_bRunning = false;

        return 0;
    }
}
