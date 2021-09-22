/**
* \file shm_channel.cpp
* \brief 共享内存双工通道类函数的实现
*/

#include "pch.h"
#include "app/shm_channel.h"


namespace frame
{
    /**
    * \brief 构造函数
    */
    ShmChannel::ShmChannel(void)
    {
    }

    /**
    * \brief 析构函数
    */
    ShmChannel::~ShmChannel(void)
    {
    }

    /**
    * \brief 创建共享内存双工通道
    * \param stRecvInfo 接收共享内存数据
    * \param stSendInfo 发送共享内存数据
    * \return 创建成功返回true，否则返回false
    */
    bool ShmChannel::Create(const SHM_CHANNEL_INFO &stRecvInfo, const SHM_CHANNEL_INFO &stSendInfo)
    {
        // 初始化进程间信号量，有可能会阻塞
        if (!m_RecvPipe.Open(stRecvInfo.nShmKey, stRecvInfo.nShmSize, stRecvInfo.nSemKey, stRecvInfo.bResume))
        {
            return false;
        }

        // 初始化进程间信号量，有可能会阻塞
        if (!m_SendPipe.Open(stSendInfo.nShmKey, stSendInfo.nShmSize, stSendInfo.nSemKey, stSendInfo.bResume))
        {
            m_RecvPipe.Close();
            return false;
        }

        return true;
    }

    /**
    * \brief 释放
    */
    void ShmChannel::Release()
    {
        m_RecvPipe.Close();
        m_SendPipe.Close();

        delete this;
    }

    /**
    * \brief 发送数据
    * \param pMsg 消息数据
    * \param nLen 数据大小
    * \return 发送成功的数据大小，<0表示发送失败
    */
    int32 ShmChannel::Send(LPCSTR pMsg, size_t nLen)
    {
        return m_SendPipe.Write(pMsg, nLen);
    }

    /**
    * \brief 接收数据
    * \param szBuffer 接收数据的缓冲区
    * \param nLen 接收数据的缓冲区大小
    * \return 接收数据大小，=0表示管道已经没数据了，<0表示接收失败
    */
    int32 ShmChannel::Recv(char *szBuffer, size_t nLen)
    {
        return m_RecvPipe.Read(szBuffer, nLen);
    }

    /**
    * \brief 阻塞等待信号
    * \return true表示有信号，false表示无信号
    */
    bool ShmChannel::Wait()
    {
        return m_RecvPipe.Wait();
    }

    /**
    * \brief 停止等待信号
    */
    void ShmChannel::Stop()
    {
        m_RecvPipe.Post();
    }
}