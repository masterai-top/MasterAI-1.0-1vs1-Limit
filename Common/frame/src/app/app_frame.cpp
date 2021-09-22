/**
* \file app_frame.cpp
* \brief 应用框架实现代码
*/

#include "pch.h"
#include "app/app_frame.h"
#include "app/app.h"
#include "app/appdef.h"
#include "strutil.h"
#include "system.h"
#include "path.h"
#include "file.h"
using namespace dtutil;

namespace frame
{
    /**
    * \brief 构造函数
    */
    AppFrame::AppFrame(void)
    {
        m_pApp = NULL;
    }

    /**
    * \brief 析构函数
    */
    AppFrame::~AppFrame(void)
    {
    }

    /**
    * \brief 初始化
    * \param pApp 应用指针
    * \param argc 参数数量
    * \param argv 参数列表
    * \return 初始化成功返回true，否则返回false
    */
    bool AppFrame::Init(App *pApp, int32 argc, char *argv[])
    {
        if (NULL == pApp)
        {
            return false;
        }

        std::string strAppName = "";
        uint32 nRunCmd = RUN_CMD_START;

        // 解析运行命令
        if (!ParseCmd(argc, argv, strAppName, nRunCmd))
        {
            return false;
        }

        m_pApp = pApp;
        bool bResult = false;

        switch(nRunCmd)
        {
        case RUN_CMD_START:
            bResult = OnCmdStart(strAppName.c_str());
            break;
        case RUN_CMD_RESTART:
            bResult = OnCmdRestart(strAppName.c_str());
            break;
        case RUN_CMD_STOP:      // 通知旧进程后自杀，不用返回true
            OnCmdStop(strAppName.c_str());
            break;
        case RUN_CMD_RELOAD:    // 通知旧进程后自杀，不用返回true
            OnCmdReload(strAppName.c_str());
            break;
        default:
            break;
        }

        if (bResult)
        {
            // 正式启动，初始化应用事件
            m_AppEvent.Init();

            // 回调应用自己的初始化逻辑
            bResult = m_pApp->OnInit();
        }

        return bResult;
    }

    /**
    * \brief 主循环
    */
    void AppFrame::Loop()
    {
        m_AppEvent.Dispatch();
    }

    /**
    * \brief 释放
    */
    void AppFrame::Restore()
    {
        // 应用自己的释放
        if (m_pApp != NULL)
        {
            m_pApp->OnRestore();
        }

        m_AppEvent.Restore();

        m_pApp = NULL;
    }

    /**
    * \brief 获得事件根基
    * \return 事件根基
    */
    event_base * AppFrame::GetEventBase()
    {
        return m_AppEvent.GetEventBase();
    }

    /**
    * \brief 获得事件响应器
    * \return 事件响应器
    */
    IEventReactor * AppFrame::GetEventReactor()
    {
        return m_AppEvent.GetEventReactor();
    }

    /**
    * \brief 获得事件调度器
    * \return 事件调度器
    */
    IEventDispatcher * AppFrame::GetEventDispatcher()
    {
        return m_AppEvent.GetEventDispatcher();
    }

    /**
    * \brief 获得应用对象
    */
    App * AppFrame::GetApp()
    {
        return m_pApp;
    }

    /**
    * \brief 获得应用事件
    * \return 应用事件
    */
    AppEvent * AppFrame::GetAppEvent()
    {
        return &m_AppEvent;
    }

    /**
    * \brief 启动命令处理函数
    * \param szAppName 应用名称
    * \return 成功返回true，否则返回false
    */
    bool AppFrame::OnCmdStart(const char *szAppName)
    {
        std::string strError = "";

        // 获得进程PID
        int32 nAppPID = GetAppPID(szAppName);

        // 已经启动过该应用，需用restart命令或者stop命令
        if (nAppPID != 0)
        {
            StrUtil::strformat(strError, "app already running![%d] self_pid[%d] please run [%s restart] or [%s stop] first!", nAppPID, System::GetPID(), szAppName, szAppName);
            throw AppException(strError);
        }

        // 写文件appname.pid;
        DumpAppPID(szAppName, System::GetPID());

        return true;
    }

    /**
    * \brief 重启命令处理函数
    * \param szAppName 应用名称
    * \return 成功返回true，否则返回false
    */
    bool AppFrame::OnCmdRestart(const char *szAppName)
    {
        // 获得进程PID
        int32 nAppPID = GetAppPID(szAppName);

        // 已经启动过该应用，杀掉旧进程
        if (nAppPID != 0)
        {
            System::KillPID(nAppPID);
        }

        // 写文件appname.pid;
        DumpAppPID(szAppName, System::GetPID());

        return true;
    }

    /**
    * \brief 停止命令处理函数
    * \param szAppName 应用名称
    * \return 成功返回true，否则返回false
    */
    bool AppFrame::OnCmdStop(const char *szAppName)
    {
        // 获得进程PID
        int32 nAppPID = GetAppPID(szAppName);

        // 进程未启动，无法停止
        if (nAppPID == 0)
        {
            printf("app not running! please run [%s start] or [%s restart] first!\n", szAppName, szAppName);
            return false;
        }

#ifdef WIN32
        // 直接kill掉，旧进程未保存数据，不好
        // 可以考虑使用TCP连接发送消息处理
        System::KillPID(nAppPID);
        printf("force kill pid[%d]!\n", nAppPID);
#else
        // 向旧进程发送信号事件，通知处理
        m_AppEvent.GetSignalCenter()->SendSignal(SIGQUIT, nAppPID);
#endif

        return true;
    }

    /**
    * \brief 重载命令处理函数
    * \param szAppName 应用名称
    * \return 成功返回true，否则返回false
    */
    bool AppFrame::OnCmdReload(const char *szAppName)
    {
        // 获得进程PID
        int32 nAppPID = GetAppPID(szAppName);

        // 进程未启动，无法停止
        if (nAppPID == 0)
        {
            printf("app not running! please run [%s start] or [%s restart] first!\n", szAppName, szAppName);
            return false;
        }

#ifdef WIN32
        // 可以考虑使用TCP连接发送消息处理
        printf("not support notify other process reload!\n");
#else
        // 向旧进程发送信号事件，通知处理
         m_AppEvent.GetSignalCenter()->SendSignal(SIGUSR1, nAppPID);
#endif

        return true;
    }

    /**
    * \brief 解析运行命令
    * \param argc 参数数量
    * \param argv 参数列表
    * \param strAppName 进程名称
    * \param nCmd 运行命令
    * \return 成功返回true，否则返回false
    */
    bool AppFrame::ParseCmd(int32 argc, char *argv[], std::string &strAppName, uint32 &nCmd)
    {
        if (argc < 1)
        {
            return false;
        }

        // 修正'\'改为'/'
        for(size_t i=0; i<strlen(argv[0]); ++i)
        {
            if (argv[0][i] == '\\')
            {
                argv[0][i] = '/';
            }
        }

        char *s = strrchr(argv[0], '/');
        if (NULL == s)
        {
            strAppName = argv[0];
        }
        else
        {
            strAppName = ++s;
        }

        nCmd = RUN_CMD_START;

        for (int32 i=1; i<argc; ++i)
        {
            // 启动-若已经启动，无法启动
            if (0 == StrUtil::strnicmp(argv[i], "start", strlen("start")))
            {
                nCmd = RUN_CMD_START;
                break;
            }
            // 重新启动-若已经启动，杀掉旧进程
            else if (0 == StrUtil::strnicmp(argv[i], "restart", strlen("restart")))
            {
                nCmd = RUN_CMD_RESTART;
                break;
            }
            // 停止-杀掉旧进程，自杀
            else if (0 == StrUtil::strnicmp(argv[i], "stop", strlen("stop")))
            {
                nCmd = RUN_CMD_STOP;
                break;
            }
            // 重载-通知旧进程重载，自杀
            else if (0 == StrUtil::strnicmp(argv[i], "reload", strlen("reload")))
            {
                nCmd = RUN_CMD_RELOAD;
                break;
            }
        }

        return true;
    }

    /**
    * \brief 获得正在运行的应用进程ID
    * \param szAppName 进程名称
    * \return 正在运行的进程ID，没有运行返回0
    */
    int32 AppFrame::GetAppPID(const char *szAppName)
    {
        // 函数System::GetPIDByName()获取的进程ID，
        // linux下若是开启了守护进程，通过文件查找进程的方式，
        // 进程退出了，进程文件还没删除，这时候获取的进程其实已经退出了，
        // 对后面的启动进程判断有误，会导致错误！

        std::string strAppPIDFile;
        StrUtil::strformat(strAppPIDFile, APP_PID_FILE, szAppName);

        // 如果进程ID文件不存在，表示进程没在运行
        if (!Path::ExistFile(strAppPIDFile))
        {
            return 0;
        }

        File file;
        if (!file.Open(strAppPIDFile.c_str(), "r"))
        {
            return 0;
        }

        int32 nAppPID = StrUtil::strtoi32(file.GetLine());

        // 判断进程是否存在
        if (!System::FindPID(nAppPID))
        {
            return 0;
        }

        // 进程名是否相同
        if (0 != StrUtil::strnicmp(szAppName, System::GetPIDName(nAppPID).c_str(), strlen(szAppName)))
        {
            return 0;
        }

        return nAppPID;
    }

    /**
    * \brief 将进程ID写入文件
    * \param szAppName 进程名称
    * \param nPID 进程ID
    * \return 成功返回true，否则返回false
    */
    bool AppFrame::DumpAppPID(const char *szAppName, int32 nPID)
    {
        std::string strAppPIDFile;
        StrUtil::strformat(strAppPIDFile, APP_PID_FILE, szAppName);

        // 以覆写的方式打开进程文件
        File file;
        if (!file.Open(strAppPIDFile.c_str(), "wb"))
        {
            return false;
        }

        // 写入pid
        std::string strFileContext;
        StrUtil::strformat(strFileContext, "%d", nPID);

        file.Write(strFileContext.c_str(), strFileContext.length());

        file.Flush();

        file.Close();

        return true;
    }
}