/**
* \file shm_pipe.h
* \brief 共享内存管道头文件
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __SHM_PIPE_H__
#define __SHM_PIPE_H__

#include "framedef.h"
#include "memory/shm.h"
#include "pipe/shm_sem.h"

namespace frame
{
    /**
    * \defgroup pipe_group 网络
    * pipe 提供了一组管道的相关操作函数
    */	
	
    /**
    * \brief 共享内存管道
    * \ingroup pipe_group
    */
    class FRAME_EXPORT ShmPipe
    {
    public:
        /**
        * \brief 构造函数
        */
        ShmPipe(void);

        /**
        * \brief 析构函数
        */
        ~ShmPipe(void);

        /**
        * \brief 打开管道
        * \param nKey 共享内存key
        * \param nSize 共享内存大小
        * \param nSemKey 信号集key
        * \param bResume 是否内存恢复模式
        * \return 打开成功返回true，否则返回false
        */
        bool Open(int32 nKey, uint32 nSize, int32 nSemKey, bool bResume);

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
        * \brief 读数据
        * \param szBuffer 接收数据的缓冲区
        * \param nLen 接收数据的缓冲区大小
        * \return 接收数据大小=0表示管道已经没数据了<0表示接收失败
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
        * \return 接收数据大小=0表示管道已经没数据了<0表示接收失败
        */
        int32 ReadBuffer(char *pBuffer, size_t nLen);

        /**
        * \brief 获得剩余可写大小
        * \param nHead 当前读位置
        * \param nTail 当前写位置
        * \param nSize 管道大小
        * \return 剩余可写大小
        */
        uint32 GetLeftSize(uint32 nHead, uint32 nTail, uint32 nSize);

        /**
        * \brief 获得未读数据大小
        * \param nHead 当前读位置
        * \param nTail 当前写位置
        * \param nSize 管道大小
        * \return 未读数据大小
        */
        uint32 GetSize(uint32 nHead, uint32 nTail, uint32 nSize);

        /**
        * \brief 重置共享内存
        */
        void Reset();

    private:
        Shm             m_Shm;          ///< 共享内存对象
        ShmSem          m_Mutex;        ///< 信号互斥变量

        /// 管道数据头
        typedef struct _PIPE_HEAD
        {
            uint32  head;               ///< 当前读位置
            uint32  tail;               ///< 当前写位置
            uint32  size;               ///< 管道大小

            /**
            * \brief 构造函数
            */
            _PIPE_HEAD()
            {
                head = 0;
                tail = 0;
                size = 0;
            }
        } PIPE_HEAD;

        PIPE_HEAD       *m_pHead;       ///< 管道数据头
        char            *m_pData;       ///< 管道数据
    };
}

#endif // __SHM_PIPE_H__
