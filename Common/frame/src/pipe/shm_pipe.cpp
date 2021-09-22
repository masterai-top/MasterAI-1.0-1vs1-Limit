/**
* \file shm_pipe.cpp
* \brief 共享内存管道类函数的实现
*/

#include "pch.h"
#include "pipe/shm_pipe.h"
#include "timeutil.h"
using namespace dtutil;

namespace frame
{
    /**
    * \brief 构造函数
    */
    ShmPipe::ShmPipe(void)
    {
        m_pHead = NULL;
        m_pData = NULL;
    }

    /**
    * \brief 析构函数
    */
    ShmPipe::~ShmPipe(void)
    {
        m_pHead = NULL;
        m_pData = NULL;
    }

    /**
    * \brief 打开管道
    * \param nKey 共享内存key
    * \param nSize 共享内存大小
    * \param nSemKey 信号集key
    * \param bResume 是否内存恢复模式
    * \return 打开成功返回true，否则返回false
    */
    bool ShmPipe::Open(int32 nKey, uint32 nSize, int32 nSemKey, bool bResume)
    {
        // 管道内存需要至少一个管道头的大小
        if (nSize < sizeof(PIPE_HEAD))
        {
            return false;
        }

        // 使用信号集，当别的进程也在初始化信号集时，有可能导致阻塞
        if (nSemKey > 0)
        {
            if ( m_Mutex.Init(nSemKey) < 0 )
            {
                return false;
            }
        }

        // 有可能已经创建了共享内存，先附加上去
        if (!m_Shm.Attach(nKey, nSize))
        {
            // 附加失败，则创建
            if (!m_Shm.Create(nKey, nSize))
            {
                return false;
            }
        }

        char *pBuffer = (char *)(m_Shm.GetData());

        // 前面sizeof(PIPE_HEAD)数据为数据头
        m_pHead = (PIPE_HEAD *)(pBuffer);
        // 后面为管道存储数据
        m_pData = pBuffer + sizeof(PIPE_HEAD);

        // 非内存恢复模式，需初始化管道头
        if (!bResume)
        {
            Reset();
        }

        return true;
    }

    /**
    * \brief 释放内存数据
    */
    void ShmPipe::Close()
    {
        m_pHead = NULL;
        m_pData = NULL;

        m_Shm.Detach();
    }

    /**
    * \brief 写入数据
    * \param pBuffer 消息数据
    * \param nLen 数据大小
    * \return 写入成功的数据大小，<0表示写入失败
    */
    int32 ShmPipe::Write(LPCSTR pBuffer, size_t nLen)
    {
        // 获得写锁
        m_Mutex.WriteLock();

        int32 nWriteLen = WriteBuffer(pBuffer, nLen);

        // 释放写锁
        m_Mutex.WriteUnlock();

        // 发送通知可读
        m_Mutex.Post();

        return nWriteLen;
    }

    /**
    * \brief 读数据
    * \param szBuffer 接收数据的缓冲区
    * \param nLen 接收数据的缓冲区大小
    * \return 接收数据大小，=0表示管道已经没数据了，<0表示接收失败
    */
    int32 ShmPipe::Read(char *pBuffer, size_t nLen)
    {
        // 获得读锁
        m_Mutex.ReadLock();

        int32 nReadLen = ReadBuffer(pBuffer, nLen);

        // 释放读锁
        m_Mutex.ReadUnlock();

        return nReadLen;
    }

    /**
    * \brief 阻塞等待信号
    * \return true表示有信号，false表示无信号
    */
    bool ShmPipe::Wait()
    {
        m_Mutex.Wait();

        return true;
    }

    /**
    * \brief 发送信号
    */
    void ShmPipe::Post()
    {
        m_Mutex.Post();
    }

    /**
    * \brief 写入数据
    * \param pMsg 消息数据
    * \param nLen 数据大小
    * \return 写入成功的数据大小，<0表示写入失败
    */
    int32 ShmPipe::WriteBuffer(LPCSTR pBuffer, size_t nLen)
    {
        if (NULL == pBuffer || 0 == nLen)
        {
            return -1;
        }

        // 未初始化管道
        if (NULL == m_pHead)
        {
            return -2;
        }

        // 必须获取值出来，避免其它进程修改
        uint32 nHead = m_pHead->head;
        uint32 nTail = m_pHead->tail;
        uint32 nSize = m_pHead->size;

        // 计算剩余可写入大小
        uint32 nLeftSize = GetLeftSize(nHead, nTail, nSize);

        // 每一次写入数据都是先写入sizeof(uint16)的数据大小，再写入数据

        // 剩余可写入大小不足
        if (nLeftSize < (uint32)(nLen) + (uint32)(sizeof(uint16)))
        {
            return -3;
        }

        // 先写入数据大小
        char *pLen = (char *)(&nLen);
        for (uint32 i = 0; i < (uint32)(sizeof(uint16)); ++i)
        {
            m_pData[nTail] = pLen[i];
            nTail = (nTail + 1) % nSize;
        }

        // 到管道尾写不完所有数据，需从管道头写，环状
        if (nTail + nLen > nSize)
        {
            // 先写到管道尾的大小
            uint32 nWriteLen = nSize - nTail;
            memcpy(m_pData + nTail, pBuffer, nWriteLen);
            // 再在管道头写剩下的数据
            memcpy(m_pData, pBuffer + nWriteLen, nLen - nWriteLen);
        }
        // 到管道尾能写完所有数据，直接一次写入
        else
        {
            memcpy(m_pData + nTail, pBuffer, nLen);
        }

        // 重新定位当前写位置
        m_pHead->tail = (nTail + nLen) % nSize;

        return (uint32)(nLen);
    }

    /**
    * \brief 读数据
    * \param pBuffer 接收数据的缓冲区
    * \param nLen 接收数据的缓冲区大小
    * \return 接收数据大小，=0表示管道已经没数据了，<0表示接收失败
    */
    int32 ShmPipe::ReadBuffer(char *pBuffer, size_t nLen)
    {
        if (NULL == pBuffer || 0 == nLen)
        {
            return -1;
        }

        // 未初始化管道
        if (NULL == m_pHead)
        {
            return -2;
        }

        // 必须获取值出来，避免其它进程修改
        uint32 nHead = m_pHead->head;
        uint32 nTail = m_pHead->tail;
        uint32 nSize = m_pHead->size;

        // 已经没数据了
        if (nHead == nTail)
        {
            return 0;
        }

        // 获得未读数据大小
        uint32 nNotReadSize = GetSize(nHead, nTail, nSize);

        // 未读数据不够一个长度大小
        if (nNotReadSize < (uint32)(sizeof(uint16)))
        {
            return -3;
        }

        // 先读数据大小
        uint16 nBufLen = 0;
        char *pReadBuf = (char *)(&nBufLen);
        for (uint32 i = 0; i < (uint32)(sizeof(uint16)); ++i)
        {
            pReadBuf[i] = m_pData[nHead];
            nHead = (nHead + 1) % nSize;    // 因为可能已经到尾了，回转到首位置去
        }

        // 剩余未读数据大小
        nNotReadSize -= sizeof(uint16);

        // 剩余未读数据不够一次读取的大小
        if (nBufLen > nNotReadSize)
        {
            return -4;
        }

        // 接收缓冲区大小不够一次读取的大小
        if ((size_t)(nBufLen) > nLen)
        {
            // 直接丢弃这段数据
            // 重新定位当前读位置
            m_pHead->head = (nHead + nBufLen) % nSize;
            return -5;
        }

        // 该段数据到管道尾循环到管道头
        if (nHead + nBufLen > nSize)
        {
            // 先读取当前位置到管道尾的数据
            uint32 nReadLen = nSize - nHead;
            memcpy(pBuffer, m_pData + nHead, nReadLen);
            // 再读取管道头开始到该段数据尾的数据
            memcpy(pBuffer + nReadLen, m_pData, nBufLen - nReadLen);
        }
        // 该段数据不到管道尾，一次读完
        else
        {
            memcpy(pBuffer, m_pData + nHead, nBufLen);
        }

        // 重新定位当前读位置
        m_pHead->head = (nHead + nBufLen) % nSize;

        return (int32)(nBufLen);
    }

    /**
    * \brief 获得剩余可写大小
    * \param nHead 当前读位置
    * \param nTail 当前写位置
    * \param nSize 管道大小
    * \return 剩余可写大小
    */
    uint32 ShmPipe::GetLeftSize(uint32 nHead, uint32 nTail, uint32 nSize)
    {
        if (nTail >= nHead)
        {
            return nSize - nTail + nHead;
        }

        return nHead - nTail;
    }

    /**
    * \brief 获得未读数据大小
    * \param nHead 当前读位置
    * \param nTail 当前写位置
    * \param nSize 管道大小
    * \return 未读数据大小
    */
    uint32 ShmPipe::GetSize(uint32 nHead, uint32 nTail, uint32 nSize)
    {
        if (nTail >= nHead)
        {
            return nTail - nHead;
        }

        return nSize - nHead + nTail;
    }

    /**
    * \brief 重置共享内存
    */
    void ShmPipe::Reset()
    {
        // 获得读写锁
        m_Mutex.Lock();

        m_pHead->head = 0;
        m_pHead->tail = 0;
        m_pHead->size = m_Shm.GetSize() - sizeof(PIPE_HEAD);

        // 释放读写锁
        m_Mutex.Unlock();
    }
}