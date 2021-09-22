/**
* \file app_signal.cpp
* \brief 应用信号事件实现代码
*/

#include "pch.h"
#include "app/app_signal.h"
#include "app/app_frame.h"
#include "app/app.h"
#include "system.h"
using namespace dtutil;

namespace frame
{
    /**
    * \brief 构造函数
    */
    AppSignal::AppSignal(void)
    {
    }

    /**
    * \brief 析构函数
    */
    AppSignal::~AppSignal(void)
    {
    }

    /**
    * \brief 初始化
    * \return 初始化成功返回true，否则返回false
    */
    bool AppSignal::Init()
    {
        IEventReactor *pEventReactor = AppFrame::Instance().GetEventReactor();
        if (NULL == pEventReactor)
        {
            return false;
        }

#ifndef WIN32
        // 忽略信号
        signal(SIGHUP, SIG_IGN);
        signal(SIGPIPE, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGCHLD, SIG_IGN);

        // 安装信号
        pEventReactor->AttachSignal(this, SIGQUIT, "AppSignal::Init");
        pEventReactor->AttachSignal(this, SIGTERM, "AppSignal::Init");
        pEventReactor->AttachSignal(this, SIGKILL, "AppSignal::Init");
        pEventReactor->AttachSignal(this, SIGTSTP, "AppSignal::Init");
        pEventReactor->AttachSignal(this, SIGINT, "AppSignal::Init");
        pEventReactor->AttachSignal(this, SIGUSR1, "AppSignal::Init");
        pEventReactor->AttachSignal(this, SIGUSR2, "AppSignal::Init");
#endif

        return true;
    }

    /**
    * \brief 释放
    */
    void AppSignal::Restore()
    {
        IEventReactor *pEventReactor = AppFrame::Instance().GetEventReactor();
        if (NULL == pEventReactor)
        {
            return;
        }

#ifndef WIN32
        // 卸载信号
        pEventReactor->DetachSignal(this, SIGQUIT);
        pEventReactor->DetachSignal(this, SIGTERM);
        pEventReactor->DetachSignal(this, SIGKILL);
        pEventReactor->DetachSignal(this, SIGTSTP);
        pEventReactor->DetachSignal(this, SIGINT);
        pEventReactor->DetachSignal(this, SIGUSR1);
        pEventReactor->DetachSignal(this, SIGUSR2);
#endif
    }

    /**
    * \brief 发送信号事件
    * \param nSignalID 信号ID
    * \param nPID 进程ID，给本进程发送信号，nPID=0
    * \return 成功返回true，否则返回false
    */
    void AppSignal::SendSignal(int32 nSignalID, uint32 nPID)
    {
        if (nPID == 0)
        {
            nPID = System::GetPID();
        }

#ifndef WIN32
        // 向进程发送信号
        kill(nPID, nSignalID);
#endif
    }

    /**
    * \brief 信号事件处理
    * \param nSignalID 信号ID
    */
    void AppSignal::OnSignal(int32 nSignalID)
    {
        AppEvent *pAppEvent = AppFrame::Instance().GetAppEvent();
        if (NULL == pAppEvent)
        {
            return;
        }

        App *pApp = AppFrame::Instance().GetApp();
        if (NULL == pApp)
        {
            return;
        }

#ifndef WIN32
        switch (nSignalID)
        {
            /// 正常停服
        case SIGQUIT:   // ./proc_name stop
        case SIGTERM:   // kill proc_id
            pAppEvent->Exit();
            break;
            /// 立即终止
        case SIGINT:    // Ctrl + C
        case SIGTSTP:   // Ctrl + Z 
            pAppEvent->Break();
            break;
            /// 系统逻辑信号
        case SIGUSR1:
        case SIGUSR2:
        default:
            break;
        }
#endif

        pApp->OnSignal(nSignalID);
    }
}
