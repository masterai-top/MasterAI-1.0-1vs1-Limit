/**
* \file thread.h
* \brief 线程封装类
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __THREAD_H__
#define __THREAD_H__

#include "framedef.h"
#include "thread/lock.h"
#include "thread/tevent.h"
#include "pthread.h"
#include <string>

namespace frame
{
    /**
    * \defgroup thread_group 线程
    * 提供了一组线程的相关函数
    */

    /**
    * \brief 线程封装类
    * \ingroup thread_group
    * 封装了线程的创建、停止通知、等待结束、运行判断等接口
    */
    class FRAME_EXPORT Thread
    {
    public:
        /**
        * \brief 构造函数
        */
        Thread(void);

        /**
        * \brief 析构函数
        */
        virtual ~Thread(void);

        /**
        * \brief 启动线程
        * \param name 线程名称
        *
        * 如果线程正在执行，则不会启动新线程
        */
        void Run(const char *name = "thread");

        /**
        * \brief 通知线程停止
        * \param wait 是否等待线程结束，默认为false
        *
        * 默认不等待线程结束，如果设置了wait为true，则函数一直阻塞到线程结束运行才返回
        */
        void Stop(bool wait = false);

        /**
        * \brief 强制杀死线程，等待线程结束
        *
        * 函数一直阻塞到线程结束运行才返回
        */
        void Kill();

        /**
        * \brief 判断线程是否正在运行
        * \return 是返回true，否则返回fasle
        */
        bool IsRunning();

    protected:
        /**
        * \brief 线程执行函数
        * \return 线程继续执行返回true，线程终止返回false
        * 
        * 此函数被私有的线程入口函数InternalThreadProc()调用，
        * 重载此虚函数以实现不同的线程执行处理
        */
        virtual bool OnThreadProc();

        /**
        * \brief 线程停止函数
        * 
        * 此函数被线程停止函数Stop()调用，以免重载的线程对象OnThreadProc()是阻塞等待的，
        * 重载此虚函数以实现重载对象通知退出阻塞等待
        */
        virtual void OnThreadStop();

    private:
        /**
        * \brief 构造函数
        * 防拷贝构造
        */
        Thread(Thread&);

        /**
        * \brief 检查停止通知事件是否被置位
        * \return true表示线程停止通知已被置位，否则返回fasle
        */
        bool CheckStop();

        /**
        * \brief 线程是否PTHREAD_CREATE_JOINABLE
        * \return 是返回true，否则返回false
        */
        bool Joinable();

        /**
        * \brief 线程入口函数
        * \param lpThread 线程对象
        *
        * 用于创建线程的入口函数，执行期间锁定m_Mutex表示线程正在运行
        */
        static void * InternalThreadProc(void *lpThread);

    private:

        pthread_t       m_Thread;           ///< 线程
        pthread_attr_t  m_Attr;             ///< 线程属性

        LockObject      m_Mutex;            ///< 线程锁
        TEvent          m_StopEvent;        ///< 停止信号

        volatile bool   m_bRunning;         ///< 是否运行
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif
        std::string     m_strName;          ///< 线程名称
#ifdef _MSC_VER
#pragma warning(pop)
#endif
    };
}

#endif // __THREAD_H__