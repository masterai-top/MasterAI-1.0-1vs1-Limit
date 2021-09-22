/**
* \file name_pipe.h
* \brief 有名管道类
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __NAME_PIPE_H__
#define __NAME_PIPE_H__

#include "framedef.h"
#include "memory/shm.h"
#include "thread/lock.h"

namespace frame
{
    /**
    * \brief 有名管道类
    * \ingroup pipe_group
    */
    class FRAME_EXPORT NamePipe
    {
    public:
        /**
        * \brief 构造函数
        */
        NamePipe(void);

        /**
        * \brief 析构函数
        */
        ~NamePipe(void);

        /**
        * \brief 打开管道
        * \param filename 管道名称
        * \return 打开成功返回true，否则返回false
        */
        bool Open(const char *filename);

        /**
        * \brief 关闭管道
        */
        void Close();

        /**
        * \brief 写入数据
        * \param pBuffer 消息数据
        * \param nLen 数据大小
        * \return 写入成功的数据大小，<0表示写入失败
        */
        int32 Write(LPCSTR pBuffer, size_t nLen);

        /**
        * \brief 读管道数据
        * \param szBuffer 接收数据的缓冲区
        * \param nLen 接收数据的缓冲区大小
        * \return 接收数据大小，=0表示管道已经没数据了，<0表示接收失败
        */
        int32 Read(char *pBuffer, size_t nLen);

        /**
        * \brief 阻塞等待信号
        * \return true表示有信号，false表示无信号
        */
        bool Wait();

        /**
        * \brief 发送信号
        */
        void Post();

    private:
        /**
        * \brief 写入数据
        * \param pBuffer 消息数据
        * \param nLen 数据大小
        * \return 写入成功的数据大小，<0表示写入失败
        */
        int32 WriteBuffer(LPCSTR pBuffer, size_t nLen);

        /**
        * \brief 读数据
        * \param szBuffer 接收数据的缓冲区
        * \param nLen 接收数据的缓冲区大小
        * \return 接收数据大小，=0表示管道已经没数据了，<0表示接收失败
        */
        int32 ReadBuffer(char *pBuffer, size_t nLen);

    private:
        int32           m_nPipeFd;      ///< 管道fd
        LockObject      m_Mutex;        ///< 管道锁
    };
}

#endif // __NAME_PIPE_H__