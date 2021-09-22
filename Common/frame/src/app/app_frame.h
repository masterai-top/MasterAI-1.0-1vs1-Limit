/**
* \file app_frame.h
* \brief 应用框架
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __APP_FRAME_H__
#define __APP_FRAME_H__

#include "typedef.h"
#include "memory/singleton.h"
#include "app/app_event.h"

#include <string>

namespace frame
{
    class App;

    /**
    * \brief 应用框架
    * \ingroup app_group
    */
    class AppFrame : public Singleton<AppFrame>
    {
    public:
        /**
        * \brief 构造函数
        */
        AppFrame(void);
        
        /**
        * \brief 析构函数
        */
        virtual ~AppFrame(void);

        /**
        * \brief 初始化
        * \param pApp 应用指针
        * \param argc 参数数量
        * \param argv 参数列表
        * \return 初始化成功返回true，否则返回false
        */
        bool Init(App *pApp, int32 argc, char *argv[]);

        /**
        * \brief 主循环
        */
        void Loop();

        /**
        * \brief 释放
        */
        void Restore();

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

    public:
        /**
        * \brief 获得应用对象
        */
        App * GetApp();

        /**
        * \brief 获得应用事件
        * \return 应用事件
        */
        AppEvent * GetAppEvent();

    private:
        /**
        * \brief 启动命令处理函数
        * \param szAppName 应用名称
        * \return 成功返回true，否则返回false
        */
        bool OnCmdStart(const char *szAppName);

        /**
        * \brief 重启命令处理函数
        * \param szAppName 应用名称
        * \return 成功返回true，否则返回false
        */
        bool OnCmdRestart(const char *szAppName);

        /**
        * \brief 停止命令处理函数
        * \param szAppName 应用名称
        * \return 成功返回true，否则返回false
        */
        bool OnCmdStop(const char *szAppName);

        /**
        * \brief 重载命令处理函数
        * \param szAppName 应用名称
        * \return 成功返回true，否则返回false
        */
        bool OnCmdReload(const char *szAppName);

    private:
        /**
        * \brief 解析运行命令
        * \param argc 参数数量
        * \param argv 参数列表
        * \param strAppName 进程名称
        * \param nCmd 运行命令
        * \return 成功返回true，否则返回false
        */
        bool ParseCmd(int32 argc, char *argv[], std::string &strAppName, uint32 &nCmd);

        /**
        * \brief 获得正在运行的应用进程ID
        * \param szAppName 进程名称
        * \return 正在运行的进程ID，没有运行返回0
        */
        int32 GetAppPID(const char *szAppName);

        /**
        * \brief 将进程ID写入文件
        * \param szAppName 进程名称
        * \param nPID 进程ID
        * \return 成功返回true，否则返回false
        */
        bool DumpAppPID(const char *szAppName, int32 nPID);

    private:
        App                 *m_pApp;                ///< 应用对象

        AppEvent            m_AppEvent;             ///< 应用事件中心
    };
}

#endif // __APP_FRAME_H__