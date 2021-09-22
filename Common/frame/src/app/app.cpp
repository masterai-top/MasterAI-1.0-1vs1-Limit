#include "pch.h"
#include "app/app.h"
#include "app/app_frame.h"
#include "app/shm_channel.h"
#include "app/name_channel.h"
#include "app/thread_pool.h"


namespace frame
{
    /**
    * \brief 构造函数
    */
    App::App(void)
    {
    /**
    * WIN32 使用网络需要初始化
    * 只需在进程初始化时调用一次即可。
    */
#ifdef WIN32
        WSADATA  ws;
        WSAStartup(MAKEWORD(2,2), &ws);
#endif
    }

    /**
    * \brief 析构函数
    */
    App::~App(void)
    {
    /**
    * WIN32 使用网络需要清除
    * 只需在进程结束时调用一次即可。
    */
#ifdef WIN32
        WSACleanup();
#endif
    }

    /**
    * \brief 初始化
    * \param argc 参数数量
    * \param argv 参数列表
    * \return 初始化成功返回true，否则返回false
    */
    bool App::Init(int32 argc, char *argv[])
    {
        return AppFrame::Instance().Init(this, argc, argv);
    }

    /**
    * \brief 主循环
    */
    void App::Loop()
    {
        AppFrame::Instance().Loop();
    }

    /**
    * \brief 释放
    */
    void App::Restore()
    {
        AppFrame::Instance().Restore();
    }

    /**
    * \brief 获得事件根基
    * \return 事件根基
    */
    event_base * App::GetEventBase()
    {
        return AppFrame::Instance().GetEventBase();
    }

    /**
    * \brief 获得事件响应器
    * \return 事件响应器
    */
    IEventReactor * App::GetEventReactor()
    {
        return AppFrame::Instance().GetEventReactor();
    }

    /**
    * \brief 获得事件调度器
    * \return 事件调度器
    */
    IEventDispatcher * App::GetEventDispatcher()
    {
        return AppFrame::Instance().GetEventDispatcher();
    }

    /**
    * \brief 创建共享内存双工通道
    * \param stRecvInfo 接收共享内存数据
    * \param stSendInfo 发送共享内存数据
    * \return 创建的通道
    */
    IChannel * App::CreateShmChannel(const SHM_CHANNEL_INFO &stRecvInfo, const SHM_CHANNEL_INFO &stSendInfo)
    {
        ShmChannel *pChannel = new ShmChannel();
        if (NULL == pChannel)
        {
            return NULL;
        }

        if ( !pChannel->Create(stRecvInfo, stSendInfo) )
        {
            delete pChannel;
            return NULL;
        }

        return pChannel;
    }

    /**
    * \brief 创建有名双工通道
    * \param szRecvFileName 接收管道文件名
    * \param szSendFileName 发送管道文件名
    * \return 创建的通道
    */
    IChannel * App::CreateNameChannel(const char *szRecvFileName, const char *szSendFileName)
    {
        NameChannel *pChannel = new NameChannel();
        if (NULL == pChannel)
        {
            return NULL;
        }

        if ( !pChannel->Create(szRecvFileName, szSendFileName) )
        {
            delete pChannel;
            return NULL;
        }

        return pChannel;
    }

    /**
    * \brief 创建线程池
    * \param nType 线程池类型 THREAD_POOL_TYPE
    * \param nInitNum 初始线程数量
    * \param nMaxNum 最大线程数量
    * \param nIdleTime 线程等待时间(秒),-1=永远等待
    * \return 线程池对象接口
    */
    IThreadPool * App::CreateThreadPool(uint32 nType, uint32 nInitNum, uint32 nMaxNum, int32 nIdleTime /*= 60*/)
    {
        ThreadPool* pThreadPool = new ThreadPool();
        if (NULL == pThreadPool)
        {
            return NULL;
        }

        if ( !pThreadPool->Create(nType, nInitNum, nMaxNum, nIdleTime) )
        {
            delete pThreadPool;
            return NULL;
        }

        return pThreadPool;
    }

    /**
    * \brief 服务初始化
    * \return 初始化成功返回true，否则返回false
    */
    bool App::OnInit()
    {
        return true;
    }

    /**
    * \brief 服务释放
    */
    void App::OnRestore()
    {

    }

    /**
    * \brief 信号事件
    * \param nSignalID 信号ID
    */
    void App::OnSignal(int32 nSignalID)
    {

    }

    /**
    * \brief 设置事件日志函数
    * \param event_log_cb 日志函数
    */
    void App::SetEventCb(event_log_cb cb)
    {
        AppFrame::Instance().GetAppEvent()->SetLogCallBack(cb);
    }
}
