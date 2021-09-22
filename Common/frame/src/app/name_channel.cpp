/**
* \file name_channel.cpp
* \brief 有名管道双工通道类函数的实现
*/

#include "pch.h"
#include "app/name_channel.h"


namespace frame
{
    /**
    * \brief 构造函数
    */
    NameChannel::NameChannel(void)
    {
    }

    /**
    * \brief 析构函数
    */
    NameChannel::~NameChannel(void)
    {
    }

    /**
    * \brief 创建有名双工通道
    * \param szRecvFileName 接收管道文件名
    * \param szSendFileName 发送管道文件名
    * \return 创建成功返回true，否则返回false
    */
    bool NameChannel::Create(const char *szRecvFileName, const char *szSendFileName)
    {
        if (!m_RecvPipe.Open(szRecvFileName))
        {
            return false;
        }

        if (!m_SendPipe.Open(szSendFileName))
        {
            m_RecvPipe.Close();
            return false;
        }

        return true;
    }

    /**
    * \brief 释放
    */
    void NameChannel::Release()
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
    int32 NameChannel::Send(LPCSTR pMsg, size_t nLen)
    {
        return m_SendPipe.Write(pMsg, nLen);
    }

    /**
    * \brief 接收数据
    * \param szBuffer 接收数据的缓冲区
    * \param nLen 接收数据的缓冲区大小
    * \return 接收数据大小，=0表示管道已经没数据了，<0表示接收失败
    */
    int32 NameChannel::Recv(char *szBuffer, size_t nLen)
    {
        return m_RecvPipe.Read(szBuffer, nLen);
    }

    /**
    * \brief 阻塞等待信号
    * \return true表示有信号，false表示无信号
    */
    bool NameChannel::Wait()
    {
        return m_RecvPipe.Wait();
}

    /**
    * \brief 停止等待信号
    */
    void NameChannel::Stop()
    {
        return m_RecvPipe.Post();
    }
}