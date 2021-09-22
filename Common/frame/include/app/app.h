/**
* \file app.h
* \brief 引用 app 库的头文件
*
* 应用只要继承该类，即可拥有一个基础应用的功能
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __APP_H__
#define __APP_H__

#include "framedef.h"
#include "typedef.h"
#include "exception.h"
#include "event2/event.h"
using namespace dtutil;

struct event_base;

namespace frame
{
    /**
    * \defgroup app_group 应用
    * 应用只要继承该类，即可拥有一个基础应用的功能
    */

    /**
    * \class AppException
    * \extends Exception
    * \brief 应用错误异常
    * \ingroup app_group
    *
    * 使用 EXCEPTION_DEF(AppException) 定义的异常
    * \sa Exception
    */
    EXCEPTION_DEF(AppException);        //!< 应用错误异常

    /**
    * \brief 共享内存管道创建信息
    * \ingroup app_group
    */
    typedef struct _SHM_CHANNEL_INFO
    {
        int32 nShmKey;      ///< 共享内存主键
        uint32 nShmSize;    ///< 共享内存大小
        int32 nSemKey;      ///< 信号集主键
        bool bResume;       ///< 是否内存恢复模式

        /**
        * \brief 构造函数
        */
        _SHM_CHANNEL_INFO()
        {
            nShmKey = -1;
            nShmSize = 0;
            nSemKey = -1;
            bResume = false;
        }
    } SHM_CHANNEL_INFO;

    /**
    * \brief 双工通道接口
    * \ingroup app_group
    */
    class IChannel
    {
    public:
        /**
        * \brief 释放
        */
        virtual void Release() = 0;

        /**
        * \brief 发送数据
        * \param pMsg 消息数据
        * \param nLen 数据大小
        * \return 发送成功的数据大小 <0表示发送失败
        */
        virtual int32 Send(LPCSTR pMsg, size_t nLen) = 0;

        /**
        * \brief 接收数据
        * \param szBuffer 接收数据的缓冲区
        * \param nLen 接收数据的缓冲区大小
        * \return 接收数据大小 =0表示管道已经没数据了<0表示接收失败
        */
        virtual int32 Recv(char *szBuffer, size_t nLen) = 0;

        /**
        * \brief 阻塞等待信号
        * \return true表示有信号，false表示无信号
        */
        virtual bool Wait() = 0;

        /**
        * \brief 停止等待信号
        */
        virtual void Stop() = 0;
    };

    /**
    * \brief 线程池类型
    * \ingroup app_group
    */
    enum THREAD_POOL_TYPE
    {
        THREAD_POOL_TYPE_SINGLE_QUEUE = 0,      ///< 单任务队列线程池
        THREAD_POOL_TYPE_MULTI_QUEUE = 1,       ///< 多任务队列线程池
    };

    /**
    * \brief 线程任务接口
    * \ingroup app_group
    */
    class IThreadTask
    {
    public:
        /**
        * \brief 获得执行线程
        * \return 执行线程索引
        */
        virtual uint32 GetThreadIndex() = 0;

        /**
        * \brief 线程池任务处理
        * \param nIndex 执行线程索引
        * 该函数由工作线程调用
        */
        virtual void OnRun(uint32 nIndex) = 0;
    };

    /**
    * \brief 线程池接口
    * \ingroup app_group
    */
    class IThreadPool
    {
    public:
        /**
        * \brief 释放
        */
        virtual void Release() = 0;

        /**
        * \brief 线程池执行任务
        * \param pTask 线程任务处理回调
        * \param nPriority 线程任务优先级，值越小优先级越高
        * \return 成功返回true，否则返回false
        */
        virtual bool Run(IThreadTask *pTask, uint32 nPriority = 0) = 0;
    };


    class IEventReactor;
    class IEventDispatcher;

    /**
    * \brief 应用框架类
    * \ingroup app_group
    */
    class FRAME_EXPORT App
    {
    public:
        /**
        * \brief 构造函数
        */
        App(void);

        /**
        * \brief 析构函数
        */
        virtual ~App(void);

        /**
        * \brief 初始化
        * \param argc 参数数量
        * \param argv 参数列表
        * \return 初始化成功返回true，否则返回false
        */
        bool Init(int32 argc, char *argv[]);

        /**
        * \brief 主循环
        */
        void Loop();

        /**
        * \brief 释放
        */
        void Restore();

    public:
        /**
        * \brief 获得事件根基
        * \return 事件根基
        */
        event_base * GetEventBase();

        /**
        * \brief 获得事件响应器
        * \return 事件响应器
        */
        IEventReactor * GetEventReactor();

        /**
        * \brief 获得事件调度器
        * \return 事件调度器
        */
        IEventDispatcher * GetEventDispatcher();

        /**
        * \brief 创建共享内存双工通道
        * \param stRecvInfo 接收共享内存数据
        * \param stSendInfo 发送共享内存数据
        * \return 创建的通道对象接口
        */
        IChannel * CreateShmChannel(const SHM_CHANNEL_INFO &stRecvInfo, const SHM_CHANNEL_INFO &stSendInfo);

        /**
        * \brief 创建有名双工通道
        * \param szRecvFileName 接收管道文件名
        * \param szSendFileName 发送管道文件名
        * \return 创建的通道对象接口
        */
        IChannel * CreateNameChannel(const char *szRecvFileName, const char *szSendFileName);

        /**
        * \brief 创建线程池
        * \param nType 线程池类型 THREAD_POOL_TYPE
        * \param nInitNum 初始线程数量
        * \param nMaxNum 最大线程数量
        * \param nIdleTime 线程等待时间(秒,-1=永远等待)
        * \return 线程池对象接口
        */
        IThreadPool * CreateThreadPool(uint32 nType, uint32 nInitNum, uint32 nMaxNum, int32 nIdleTime = 60);

    public:
        /**
        * \brief 服务初始化
        * \return 初始化成功返回true，否则返回false
        */
        virtual bool OnInit();

        /**
        * \brief 服务释放
        */
        virtual void OnRestore();

        /**
        * \brief 信号事件
        * \param nSignalID 信号ID
        */
        virtual void OnSignal(int32 nSignalID);

        /**
        * \brief 设置事件日志函数
        * \param event_log_cb 日志函数
        */
        virtual void SetEventCb(event_log_cb cb);
    };
}

#endif
