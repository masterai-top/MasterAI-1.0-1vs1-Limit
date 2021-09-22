/**
* \file socket.h
* \brief 网络 socket 的头文件
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "typedef.h"
#include <string>

/// Socket ID
#ifdef WIN32
#   include <MSWSock.h>
#   include <WinSock2.h>
typedef SOCKET          sockid;
#else
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
typedef int             sockid;
#   define INVALID_SOCKET   (-1)
#endif

namespace network
{
    /**
    * \brief 网络 socket
    * \ingroup socket_group
    */
    class Socket
    {
    public:
        /**
        * \brief socket 状态
        * \ingroup socket_group
        */
        enum SOCK_STATE
        {
            SOCK_STATE_INIT     = 0,        ///< 初始状态
            SOCK_STATE_BIND     = 1,        ///< 绑定状态
            SOCK_STATE_LISTEN   = 2,        ///< 监听状态
            SOCK_STATE_CONNECT  = 3,        ///< 连接状态
            SOCK_STATE_NORMAL   = 4,        ///< 互通状态
            SOCK_STATE_CLOSE    = 5,        ///< 关闭状态
        };
    public:
        /**
        * \brief 构造函数
        */
        Socket(void);

        /**
        * \brief 析构函数
        */
        virtual ~Socket(void);

        /**
        * \brief 创建 socket
        * \param type socket 类型 SOCK_STREAM or SOCK_DGRAM
        * \param proto 传输协议编号 IPPROTO_TCP or IPPROTO_UDP
        * \return 成功返回true，否则返回false
        */
        bool Create(int32 type, int32 proto);

        /**
        * \brief 创建 socket
        * \param fd 文件描述符
        * \return 成功返回true，否则返回false
        */
        bool Create(sockid fd);

        /**
        * \brief 关闭 socket
        */
        void Close();

        /**
        * \brief 绑定
        * \param addr 绑定IP地址
        * \param port 绑定端口
        * \return 绑定成功返回true，否则返回false
        */
        bool Bind(const char *addr, uint32 port);

        /**
        * \brief 监听
        * \return 成功返回true，否则返回false
        */
        bool Listen();

        /**
        * \brief 连接
        * \param addr 连接IP地址
        * \param port 连接端口
        * \return 连接成功返回1，失败返回-1，阻塞返回0
        */
        int32 Connect(const char *addr, uint32 port);

        /**
        * \brief 接受连接
        * \param nListenFd 监听的socket
        * \return 成功返回true，否则返回false
        */
        bool Accept(int32 nListenFd);

        /**
        * \brief 发送数据
        * \param pMsg 消息数据
        * \param nLen 数据大小
        * \return 成功返回发送的数据大小，否则返回<0，IO阻塞返回=0
        */
        int32 Send(LPCSTR pMsg, size_t nLen);

        /**
        * \brief 读取数据
        * \param pMsg 缓存数据
        * \param nLen 缓存数据大小
        * \return 成功返回读取数据大小，失败返回<0，=0表示已经没有数据了
        */
        int32 Recv(char *pMsg, size_t nLen);

    public:
        /**
        * \brief 获得状态
        * \return socket 状态
        */
        Socket::SOCK_STATE GetState();

        /**
        * \brief 设置状态
        * \param state socket 状态
        */
        void SetState(Socket::SOCK_STATE state);

        /**
        * \brief 获得 socket id
        * \return socket id
        */
        sockid GetSockID();

        /**
        * \brief 获得端口
        * \return socket id
        */
        unsigned short GetLocalPort() { return    m_nLocalPort; }
        unsigned short GetRemotePort() { return     m_nRemotePort; }

        /**
        * \brief 获得IP地址
        * \return socket id
        */
        std::string GetLocalAddr() { return std::string(m_szLocalAddr); }
        std::string GetRemoteAddr() { return std::string(m_szRemoteAddr); }

        /**
        * \brief 设置远程地址
        * \param addr 远程地址
        * \return 设置成功返回true，否则返回false
        */
        void SetRemoteAddr(const char *addr, unsigned short port);
        void SetLocalAddr(const char *addr, unsigned short port);
        void SetLocalAddr(sockaddr_in *addr);
        
        std::string GetAddrMsg();
        
        /**
        * \brief 是否有效
        * \return 有效返回true，否则返回false
        */
        bool IsValid();

        

        /**
        * \brief 获得远程地址
        * \return 远程地址
        */
        //bool GetRemoteAddr(std::string &sAddr, unsigned int &nPort);
        

        //bool GetLocalAddr(std::string &sAddr, unsigned int &nPort);

        
        
        /**
        * \brief 检查是否连接着
        * \return 是返回true，否则返回fasle
        */
        bool IsConnected();

    public:
        /**
        * \brief 设置保持连接
        * \param flag 是否保持连接
        * \return 设置成功返回true，否则返回false
        */
        bool SetKeepAlive(bool flag);

        /**
        * \brief 设置立即发送
        * \param flag 是否立即发送
        * \return 设置成功返回true，否则返回false
        */
        bool SetNoDelay(bool flag);

        /**
        * \brief 设置非阻塞
        * \return 设置成功返回true，否则返回false
        */
        bool SetNonBlock();

        /**
        * \brief 设置延时关闭
        * \return 设置成功返回true，否则返回false
        */
        bool SetLinger();

        /**
        * \brief 设置重复使用
        * \param flag 是否重复使用
        * \return 设置成功返回true，否则返回false
        */
        bool SetReuse(bool flag);

        /**
        * \brief 设置发送缓冲区大小
        * \param size 要设置的缓冲区大小
        * \return 设置成功返回true，否则返回false
        */
        bool SetSndBufSize(uint32 size);

        /**
        * \brief 设置接收缓冲区大小
        * \param size 要设置的缓冲区大小
        * \return 设置成功返回true，否则返回false
        */
        bool SetRcvBufSize(uint32 size);

    public:
        /**
        * \brief 获得传输协议编号
        * \param proto 传输协议 "tcp" or "udp"
        * \return 传输协议编号 IPPROTO_TCP or IPPROTO_UDP
        */
        static int32 GetProtoByName(const char *proto);

        static sockid GlobalSocket(int32 type, int32 proto);
        static sockid GlobalAccept(int32 nListenFd, sockaddr_in &addrRemote);
        
    private:
        sockid          m_nSockID;              ///< ID
        SOCK_STATE      m_nState;               ///< 状态
        char            m_szLocalAddr[64];      //本地ip
        unsigned short  m_nLocalPort;           //本地端口
        char            m_szRemoteAddr[64];     //本地ip
        unsigned short  m_nRemotePort;          //本地端口
    };
}

#endif // __SOCKET_H__