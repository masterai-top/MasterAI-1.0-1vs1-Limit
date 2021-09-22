/**
* \file socket.cpp
* \brief 网络 socket 类的实现代码

* IOCP中在WSASend以及WSARecv的时候出现WSA_IO_PENDING情况的说明

* 应该是windows网络编程第二版里面提到过。现在整理一下。

* 1：在IOCP中投递WSASend返回WSA_IO_PENDING的时候，表示异步投递已经成功，但是稍后发送才会完成。这其中涉及到了三个缓冲区。
* 网卡缓冲区，TCP/IP层缓冲区，程序缓冲区。
* 情况一：调用WSASend发送正确的时候（即立即返回，且没有错误），TCP/IP将数据从程序缓冲区中拷贝到TCP/IP层缓冲区中，然后不锁定该程序缓冲区，由上层程序自己处理。TCP/IP层缓冲区在网络合适的时候，将其数据拷贝到网卡缓冲区，进行真正的发送。
* 情况二：调用WSASend发送错误，但是错误码是WSA_IO_PENDING的时候，表示此时TCP/IP层缓冲区已满，暂时没有剩余的空间将程序缓冲区的数据拷贝出来，这时系统将锁定用户的程序缓冲区，按照书上说的WSASend指定的缓冲区将会被锁定到系统的非分页内存中。直到TCP/IP层缓冲区有空余的地方来接受拷贝我们的程序缓冲区数据才拷贝走，并将给IOCP一个完成消息。
* 情况三：调用WSASend发送错误，但是错误码不是WSA_IO_PENDING，此时应该是发送错误，应该释放该SOCKET对应的所有资源。

* 2：在IOCP中投递WSARecv的时候，情况相似。
* 情况一：调用WSARecv正确，TCP/IP将数据从TCP/IP层缓冲区拷贝到缓冲区，然后由我们的程序自行处理了。清除TCP/IP层缓冲区数据。
* 情况二：调用WSARecv错误，但是返回值是WSA_IO_PENDING，此时是因为TCP/IP层缓冲区中没有数据可取，系统将会锁定我们投递的WSARecv的buffer，直到TCP/IP层缓冲区中有新的数据到来。
* 情况三：调用WSARecv错误，错误值不是WSA_IO_PENDING，此时是接收出错，应该释放该SOCKET对应的所有资源。

* 在以上情况中有几个非常要注意的事情：
* 系统锁定非分页内存的时候，最小的锁定大小是4K(当然，这个取决于您系统的设置，也可以设置小一些，在注册表里面可以改，当然我想这些数值微软应该比我们更知道什么合适了)，所以当我们投递了很多WSARecv或者WSASend的时候，不管我们投递的Buffer有多大（0除外），系统在出现IO_PENGDING的时候，都会锁定我们4K的内存。这也就是经常有开发者出现WSANOBUF的情况原因了。

* 我们在解决这个问题的时候，要针对WSASend和WSARecv做处理
* 1：投递WSARecv的时候，可以采用一个巧妙的设计，先投递0大小Buf的WSARecv，如果返回，表示有数据可以接收，我们开启真正的recv将数据从TCP/IP层缓冲区取出来，直到WSA_IO_PENGDING.
* 2：对投递的WSARecv以及WSASend进行计数统计，如果超过了我们预定义的值，就不进行WSASend或者WSARecv投递了。
* 3:现在我们应该就可以明白为什么WSASend会返回小于我们投递的buffer空间数据值了，是因为TCP/IP层缓冲区小于我们要发送的缓冲区，TCP/IP只会拷贝他剩余可被Copy的缓冲区大小的数据走，然后给我们的WSASend的已发送缓冲区设置为移走的大小，下一次投递的时候，如果TCP/IP层还未被发送，将返回WSA_IO_PENGDING。
* 4：在很多地方有提到，可以关闭TCP/IP层缓冲区，可以提高一些效率和性能，这个从上面的分析来看，有这个可能，要实际的网络情况去实际分析了。

*/

#include "pch.h"
#include "socket.h"


namespace network
{
    /**
    * \brief 构造函数
    */
    Socket::Socket(void)
    {
        m_nSockID = INVALID_SOCKET;
        m_nState = SOCK_STATE_INIT;
    }

    /**
    * \brief 析构函数
    */
    Socket::~Socket(void)
    {
    }

    /**
    * \brief 创建 socket
    * \param type socket 类型 SOCK_STREAM or SOCK_DGRAM
    * \param proto 传输协议编号 IPPROTO_TCP or IPPROTO_UDP
    * \return 成功返回true，否则返回false
    */
    bool Socket::Create(int32 type, int32 proto)
    {
#ifdef WIN32
        sockid nSockID = WSASocket(AF_INET, type, proto, NULL, 0, WSA_FLAG_OVERLAPPED);
#else
        sockid nSockID = socket(AF_INET, type, proto);
#endif

        if (nSockID == INVALID_SOCKET)
        {
            return false;
        }

        m_nSockID = nSockID;
        m_nLocalPort  = 0;
        m_nRemotePort = 0;
        memset(m_szLocalAddr, 0, sizeof(m_szLocalAddr));
        memset(m_szRemoteAddr, 0, sizeof(m_szRemoteAddr));
        
        return true;
    }

    /**
    * \brief 创建 socket
    * \param fd 文件描述符
    * \return 成功返回true，否则返回false
    */
    bool Socket::Create(sockid fd)
    {
        if (m_nState != SOCK_STATE_INIT)
        {
            return false;
        }

        m_nSockID = fd;

        m_nLocalPort  = 0;
        m_nRemotePort = 0;
        memset(m_szLocalAddr, 0, sizeof(m_szLocalAddr));
        memset(m_szRemoteAddr, 0, sizeof(m_szRemoteAddr));
        
        return true;
    }

    /**
    * \brief 关闭 socket
    */
    void Socket::Close()
    {
        if (m_nSockID != INVALID_SOCKET)
        {
#ifdef WIN32
            closesocket(m_nSockID);
#else
            close(m_nSockID);
#endif
            m_nSockID = INVALID_SOCKET;
        }

        m_nState = SOCK_STATE_INIT;
        m_nLocalPort  = 0;
        m_nRemotePort = 0;
        memset(m_szLocalAddr, 0, sizeof(m_szLocalAddr));
        memset(m_szRemoteAddr, 0, sizeof(m_szRemoteAddr));
    }

    /**
    * \brief 绑定
    * \param addr 绑定IP地址
    * \param port 绑定端口
    * \return 绑定成功返回true，否则返回false
    */
    bool Socket::Bind(const char *addr, uint32 port)
    {
        unsigned long nAddr = htons(INADDR_ANY);
        if (NULL != addr && '\0' != addr[0])
        {
            nAddr = inet_addr(addr);
        }

        struct sockaddr_in sa;
        memset(&sa, 0, sizeof(struct sockaddr_in));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = nAddr;
        sa.sin_port = htons(port);

        if (0 != bind(m_nSockID, (struct sockaddr *)&sa, sizeof(sa)))
        {
            return false;
        }

        m_nState = SOCK_STATE_BIND;

        return true;
    }

    /**
    * \brief 监听
    * \return 成功返回true，否则返回false
    */
    bool Socket::Listen()
    {
        if (m_nState != SOCK_STATE_BIND)
        {
            return false;
        }

        if (0 != listen(m_nSockID, SOMAXCONN))
        {
            return false;
        }

        m_nState = SOCK_STATE_LISTEN;

        return true;
    }

    /**
    * \brief 连接
    * \param addr 连接IP地址
    * \param port 连接端口
    * \return 连接成功返回1，失败返回-1，阻塞返回0
    */
    int32 Socket::Connect(const char *addr, uint32 port)
    {
        struct sockaddr_in sa;
        memset(&sa, 0, sizeof(struct sockaddr_in));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = inet_addr(addr);
        sa.sin_port = htons(port);

        // 设置远程地址
        snprintf(m_szRemoteAddr, sizeof(m_szRemoteAddr) - 1, "%s", inet_ntoa(sa.sin_addr));
        m_nRemotePort = ntohs(sa.sin_port);
        
        
        int32 ret = connect(m_nSockID, (struct sockaddr *)&sa, sizeof(sa));

        if (-1 == ret)
        {
#ifdef WIN32
            int32 nError = WSAGetLastError();
            if (nError != WSAEWOULDBLOCK)
#else
            if (errno != EINPROGRESS)
#endif
            {
                return -1;
            }
            else
            {
                m_nState = SOCK_STATE_CONNECT;
                return 0;
            }
        }

        m_nState = SOCK_STATE_NORMAL;

        return 1;
    }

    /**
    * \brief 接受连接
    * \param nListenFd 监听的socket
    * \return 成功返回true，否则返回false
    */
    bool Socket::Accept(int32 nListenFd)
    {
        sockaddr_in addrRemote;
#ifdef WIN32
       int32 nRemoteLen = sizeof(addrRemote);
#else
        socklen_t nRemoteLen = sizeof(addrRemote);
#endif

        sockid nAcceptFD = accept(nListenFd, (struct sockaddr *)(&addrRemote), &nRemoteLen);
        if (nAcceptFD < 0)
        {
            return false;
        }

        // 以接受到的连接socket初始化新连接socket
        if (!Create(nAcceptFD))
        {
            return false;
        }

        snprintf(m_szRemoteAddr, sizeof(m_szRemoteAddr) - 1, "%s", inet_ntoa(addrRemote.sin_addr));
        m_nRemotePort = ntohs(addrRemote.sin_port);
        
        // 设置接受的连接地址
        //if (!SetRemoteAddr(&addrRemote))
        //{
        //    return false;
        //}

        // 保持连接
        if (!SetKeepAlive(true))
        {
            return false;
        }

        // 不粘包
        if (!SetNoDelay(true))
        {
            return false;
        }

        // 非阻塞模式
        if (!SetNonBlock())
        {
            return false;
        }

        // 延时关闭
        if (!SetLinger())
        {
            return false;
        }

        // 可重复使用
        if (!SetReuse(true))
        {
            return false;
        }

        // 将接受的连接设置为互通状态
        m_nState = SOCK_STATE_NORMAL;

        return true;
    }

    /**
    * \brief 发送数据
    * \param pMsg 消息数据
    * \param nLen 数据大小
    * \return 成功返回发送的数据大小，否则返回<0，IO阻塞返回=0
    */
    int32 Socket::Send(LPCSTR pMsg, size_t nLen)
    {
        if (m_nState != SOCK_STATE_NORMAL)
        {
            return -1;
        }

        if (NULL == pMsg || nLen == 0)
        {
            return -2;
        }

        int32 nSendLen = send(m_nSockID, pMsg, (int32)(nLen), 0);
        if (nSendLen < 0)
        {
            // 返回值<0时并且(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)的情况下认为连接是正常的，堵塞了，等待继续发送。
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
            {
                return 0;
            }
            else
            {
                // 发送数据失败
                return -3;
            }
        }
        // 远程断开连接了
        else if (nSendLen == 0)
        {
            return -4;
        }

        return nSendLen;
    }

    /**
    * \brief 读取数据
    * \param pMsg 缓存数据
    * \param nLen 缓存数据大小
    * \return 成功返回读取数据大小，失败返回<0，=0表示已经没有数据了
    */
    int32 Socket::Recv(char *pMsg, size_t nLen)
    {
        if (m_nState != SOCK_STATE_NORMAL)
        {
            return -1;
        }

        if (NULL == pMsg || nLen == 0)
        {
            return -2;
        }

        int32 nRecvLen = recv(m_nSockID, pMsg, (int32)(nLen), 0);
        if (nRecvLen < 0)
        {
            // 返回值<0时并且(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)的情况下认为连接是正常的，继续接收。
            // 因上一次接收完缓冲区导致接收返回阻塞
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
            {
                return 0;
            }
            else
            {
                // 接收数据失败
                return -3;
            }
        }
        // 远程断开连接了
        else if (nRecvLen == 0)
        {
            return -4;
        }

        return nRecvLen;
    }

    /**
    * \brief 获得状态
    * \return socket 状态
    */
    Socket::SOCK_STATE Socket::GetState()
    {
        return m_nState;
    }

    /**
    * \brief 设置状态
    * \param state socket 状态
    */
    void Socket::SetState(Socket::SOCK_STATE state)
    {
        m_nState = state;
    }

    /**
    * \brief 获得 socket id
    * \return socket id
    */
    sockid Socket::GetSockID()
    {
        return m_nSockID;
    }

    /**
    * \brief 是否有效
    * \return 有效返回true，否则返回false
    */
    bool Socket::IsValid()
    {
        return m_nSockID != INVALID_SOCKET;
    }

    /**
    * \brief 设置远程地址
    * \param addr 远程地址
    * \return 设置成功返回true，否则返回false
    */
    void Socket::SetLocalAddr(sockaddr_in *addr)
    {
        if (NULL == addr)
        {
            return;
        }

        snprintf(m_szLocalAddr, sizeof(m_szLocalAddr) - 1, "%s", inet_ntoa(addr->sin_addr));
        m_nLocalPort = ntohs(addr->sin_port);
    }

    void Socket::SetRemoteAddr(const char *addr, unsigned short port)
    {
        snprintf(m_szRemoteAddr, sizeof(m_szRemoteAddr) - 1, "%s", addr);
        m_nRemotePort = port;
        
    }
    
    void Socket::SetLocalAddr(const char *addr, unsigned short port)
    {
        snprintf(m_szLocalAddr, sizeof(m_szLocalAddr) - 1, "%s", addr);
        m_nLocalPort = port;
    }

    std::string Socket::GetAddrMsg()
    {
        char szTemp[256] = {0};
        snprintf(szTemp, sizeof(szTemp) - 1, "local(%s:%d), remote(%s:%d)", 
            m_szLocalAddr, m_nLocalPort, m_szRemoteAddr, m_nRemotePort);

        return std::string(szTemp);
    }
        
    /**
    * \brief 获得本地地址(客户端connect成功后,获取当前连接的fd信息)
    * \return 远程地址
    */
    /*bool Socket::GetLocalAddr(std::string &sAddr, unsigned int &nPort)
    {
        if(0 == strlen(m_szAddr))
        {
            struct sockaddr_in guest;
            socklen_t guest_len = sizeof(guest);
            getsockname(m_nSockID, (struct sockaddr *)&guest, &guest_len);
            inet_ntop(AF_INET, &guest.sin_addr, m_szAddr, sizeof(m_szAddr) - 1);
            m_nPort = ntohs(guest.sin_port);
        }

        sAddr = m_szAddr;
        nPort = m_nPort;
        return true;
    }*/

    /**
    * \brief 获得远程地址(服务端accept客户端连连接后,获取该fd对应的客户端连接的信息)
    * \return 远程地址
    */
    /*bool Socket::GetRemoteAddr(std::string &sAddr, unsigned int &nPort)
    {
        if(0 == strlen(m_szAddr))
        {
            snprintf(m_szAddr, sizeof(m_szAddr) - 1, "%s", inet_ntoa(m_addrRemote.sin_addr));
            m_nPort = ntohs(m_addrRemote.sin_port);
        }
        
        sAddr = m_szAddr;
        nPort = m_nPort;
        return true;
    }*/

    /**
    * \brief 检查是否连接着
    * \return 是返回true，否则返回fasle
    */
    bool Socket::IsConnected()
    {
        int32 nErrno;

#ifdef WIN32
        int32 nErrLen = sizeof(nErrno);
        getsockopt(m_nSockID, SOL_SOCKET, SO_ERROR, (char *)&nErrno, &nErrLen);
#else
        socklen_t nErrLen = sizeof(nErrno);
        getsockopt(m_nSockID, SOL_SOCKET, SO_ERROR, &nErrno, &nErrLen);
#endif

        return (0 == nErrno);
    }

    /**
    * \brief 设置保持连接
    * \param flag 是否保持连接
    * \return 设置成功返回true，否则返回false
    */
    bool Socket::SetKeepAlive(bool flag)
    {
        int32 nOptVal = flag ? 1 : 0;
        if (setsockopt(m_nSockID, SOL_SOCKET, SO_KEEPALIVE, (char *)&nOptVal, sizeof(nOptVal)) == -1)
        {
            return false;
        }
        return true;
    }

    /**
    * \brief 设置立即发送
    * \param flag 是否立即发送
    * \return 设置成功返回true，否则返回false
    */
    bool Socket::SetNoDelay(bool flag)
    {
        int32 nOptVal = flag ? 1 : 0;
        if (setsockopt(m_nSockID, IPPROTO_TCP, TCP_NODELAY, (char *)&nOptVal, sizeof(nOptVal)) == -1)
        {
            return false;
        }
        return true;
    }

    /**
    * \brief 设置非阻塞
    * \return 设置成功返回true，否则返回false
    */
    bool Socket::SetNonBlock()
    {
#ifdef WIN32
        ULONG ulArgp = 1;	// FIONBIO：允许或禁止套接口s的非阻塞模式。argp指向一个无符号长整型，如允许非阻塞模式则非零，如禁止非阻塞模式则为零。
        if (ioctlsocket(m_nSockID, FIONBIO, &ulArgp) != 0)
        {
            return false;
        }
#else
        int32 nFlags = 1;
        if (ioctl(m_nSockID, FIONBIO, &nFlags))
        {
            return false;
        }

        nFlags = fcntl(m_nSockID, F_GETFL, 0);
        if (nFlags < 0)
        {
            return false;
        }

        nFlags = nFlags | O_NONBLOCK;
        if (fcntl (m_nSockID, F_SETFL, nFlags) < 0)
        {
            return false;
        }
#endif
        return true;
    }

    /**
    * \brief 设置延时关闭
    * \return 设置成功返回true，否则返回false
    */
    bool Socket::SetLinger()
    {
        linger ling = { 0, 0 };
        if (setsockopt(m_nSockID, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof(ling)) == -1)
        {
            return false;
        }
        return true;
    }

    /**
    * \brief 设置重复使用
    * \param flag 是否重复使用
    * \return 设置成功返回true，否则返回false
    */
    bool Socket::SetReuse(bool flag)
    {
        int32 nOptVal = flag ? 1 : 0;
        if (setsockopt(m_nSockID, SOL_SOCKET, SO_REUSEADDR, (char *)&nOptVal, sizeof(nOptVal)) == -1)
        {
            return false;
        }
        return true;
    }

    /**
    * \brief 设置发送缓冲区大小
    * \param size 要设置的缓冲区大小
    * \return 设置成功返回true，否则返回false
    */
    bool Socket::SetSndBufSize(uint32 size)
    {
        if (setsockopt(m_nSockID, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size)) == -1)
        {
            return false;
        }
        return true;
    }

    /**
    * \brief 设置接收缓冲区大小
    * \param size 要设置的缓冲区大小
    * \return 设置成功返回true，否则返回false
    */
    bool Socket::SetRcvBufSize(uint32 size)
    {
        if (setsockopt(m_nSockID, SOL_SOCKET, SO_RCVBUF, (char *)&size, sizeof(size)) == -1)
        {
            return false;
        }
        return true;
    }

    /**
    * \brief 获得传输协议编号
    * \param proto 传输协议 "tcp" or "udp"
    * \return 传输协议编号 IPPROTO_TCP or IPPROTO_UDP
    */
    int32 Socket::GetProtoByName(const char *proto)
    {
        struct protoent *p = NULL;

        if (NULL != proto)
        {
            p = getprotobyname(proto);
        }

        if (NULL == p)
        {
            return 0;
        }

        return p->p_proto;
    }

    sockid Socket::GlobalSocket(int32 type, int32 proto)
    {
       
#ifdef WIN32
        sockid nSockID = WSASocket(AF_INET, type, proto, NULL, 0, WSA_FLAG_OVERLAPPED);
#else
        sockid nSockID = socket(AF_INET, type, proto);
#endif

        return nSockID;
    }

    sockid Socket::GlobalAccept(int32 nListenFd, sockaddr_in &addrRemote)
    {
        //sockaddr_in addrRemote;
#ifdef WIN32
       int32 nRemoteLen = sizeof(addrRemote);
#else
        socklen_t nRemoteLen = sizeof(addrRemote);
#endif

        return  accept(nListenFd, (struct sockaddr *)(&addrRemote), &nRemoteLen);
    }
}

