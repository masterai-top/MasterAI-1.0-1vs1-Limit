/**
* \file name_pipe.cpp
* \brief 有名管道类函数的实现
*/

#include "pch.h"
#include "pipe/name_pipe.h"


namespace frame
{
    /**
    * \brief 构造函数
    */
    NamePipe::NamePipe(void)
    {
        m_nPipeFd = -1;
    }

    /**
    * \brief 析构函数
    */
    NamePipe::~NamePipe(void)
    {
        m_nPipeFd = -1;
    }

    /**
    * \brief 打开管道
    * \param filename 管道名称
    * \return 打开成功返回true，否则返回false
    */
    bool NamePipe::Open(const char *filename)
    {
#ifndef WIN32
        if(access(filename, F_OK) == -1)
        {
            int32 nRes = mkfifo(filename, S_IFIFO|0666);
            if(nRes != 0)
            {
                return false;
            }
        }

        m_nPipeFd = open(filename, O_RDWR);

        return true;
#else
        return false;
#endif
    }

    /**
    * \brief 关闭管道
    */
    void NamePipe::Close()
    {
        close(m_nPipeFd);
    }

    /**
    * \brief 写入数据
    * \param pBuffer 消息数据
    * \param nLen 数据大小
    * \return 写入成功的数据大小，<0表示写入失败
    */
    int32 NamePipe::Write(LPCSTR pBuffer, size_t nLen)
    {
        // 获得锁
        m_Mutex.Lock();

        int32 nWriteLen = WriteBuffer(pBuffer, nLen);

        // 释放锁
        m_Mutex.Unlock();

        return nWriteLen;
    }

    /**
    * \brief 读管道数据
    * \param szBuffer 接收数据的缓冲区
    * \param nLen 接收数据的缓冲区大小
    * \return 接收数据大小，=0表示管道已经没数据了，<0表示接收失败
    */
    int32 NamePipe::Read(char *pBuffer, size_t nLen)
    {
        // 获得锁
        m_Mutex.Lock();

        int32 nReadLen = ReadBuffer(pBuffer, nLen);

        // 释放锁
        m_Mutex.Unlock();

        return nReadLen;
    }

    /**
    * \brief 阻塞等待信号
    * \return true表示有信号，false表示无信号
    */
    bool NamePipe::Wait()
    {
        return false;
    }

    /**
    * \brief 发送信号
    */
    void NamePipe::Post()
    {

    }

    /**
    * \brief 写入数据
    * \param pBuffer 消息数据
    * \param nLen 数据大小
    * \return 写入成功的数据大小，<0表示写入失败
    */
    int32 NamePipe::WriteBuffer(LPCSTR pBuffer, size_t nLen)
    {
        int32 nWriteLen = 0;
        uint16 nLength = (uint16)(nLen);
        if ( write(m_nPipeFd, (void *)(&nLength), sizeof(nLength)) < 0 )
        {
            return -1;
        }
        nWriteLen += sizeof(nLength);

        if ( write(m_nPipeFd, pBuffer, (uint32)(nLen)) < 0 )
        {
            return -1;
        }

        nWriteLen += (int32)(nLen);

        return nWriteLen;
    }

    /**
    * \brief 读数据
    * \param szBuffer 接收数据的缓冲区
    * \param nLen 接收数据的缓冲区大小
    * \return 接收数据大小，=0表示管道已经没数据了，<0表示接收失败
    */
    int32 NamePipe::ReadBuffer(char *pBuffer, size_t nLen)
    {
        int32 nReadLen = 0;

        uint16 nLength = 0;
        if ( read(m_nPipeFd, (void *)(&nLength), sizeof(nLength)) < 0 )
        {
            return -1;
        }

        nReadLen += sizeof(nLength);

        if ( read(m_nPipeFd, pBuffer, nLength) < 0 )
        {
            return -1;
        }

        nReadLen += nLength;

        return nReadLen;
    }
}