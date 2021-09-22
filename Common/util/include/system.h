/**
* \file system.h
* \brief 系统函数封装类
*
* 提供系统相关的函数。
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "utildef.h"
#include "typedef.h"
#include <string>

namespace dtutil
{
    /**
    * \defgroup system_group 系统函数
    * system 提供了系统相关的函数接口
    */	
	
    /**
    * \brief 系统封装类
    * \ingroup system_group
    */
    class UTIL_EXPORT System
    {
    public:
        /**
        * \brief 线程休眠
        * \param msecs 休眠时间(毫秒)
        */
        static void SleepMS(uint32 msecs);

        /**
        * \brief 获取进程ID字符串
        * \return 进程ID字符串
        */
        static std::string GetPIDStr();

        /**
        * \brief 获取进程ID
        * \return 进程ID
        */
        static uint32 GetPID();

        /**
        * \brief 获取进程名称
        * \return 进程名称
        */
        static std::string GetProcessName();

        /**
        * \brief 根据进程名称获得进程ID
        * \param pname 进程名称
        * \param fpid 返回的进程ID列表
        * \return 成功找到返回true，否则返回fasle
        */
        static bool GetPIDByName(const char *pname, int32 *fpid);

        /**
        * \brief 获得指定进程ID的进程名称
        * \param pid 进程ID
        * \return 进程名称
        */
        static std::string GetPIDName(int32 pid);

        /**
        * \brief 查找进程是否存在
        * \param pid 进程ID
        * \return 进程存在返回true，否则返回false
        */
        static bool FindPID(int32 pid);

        /**
        * \brief 杀掉进程
        * \param pid 进程ID
        * \return 成功返回true，否则返回false
        */
        static bool KillPID(int32 pid);

        /**
        * \brief 获取线程ID字符串
        * \return 线程ID字符串
        */
        static std::string GetTIDStr();

        /**
        * \brief 获取线程ID
        * \return 线程ID
        */
        static uint32 GetTID();

        /**
        * \brief 开启守护进程模式
        */
        static void OpenDaemon();

        /**
        * \brief 得到CPU核数
        * \return CPU核数
        */
        static uint32 GetCPUNum();

        /**
        * \brief 得到物理内存大小
        * \return 物理内存大小 Kb
        */
        static uint64 GetMemorySize();
    };
}

#endif // __SYSTEM_H__