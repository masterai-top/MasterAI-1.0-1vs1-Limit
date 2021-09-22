/**
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
*/

#ifndef __NW_EVENT_THREAD_H__
#define __NW_EVENT_THREAD_H__
#include "shstd.h"
#include "typedef.h"
#include "tcp_socket.h"
#include "list"

#define NW_ACCEPT_FD        (0)
#define NW_CONNECT_FD       (1)


namespace network
{
    enum {
        NOTIFY_TYPE_LISTEN      = (0), 
        NOTIFY_TYPE_CONNECT     = (1), 
        NOTIFY_TYPE_ACCEPT      = (2), 
        NOTIFY_TYPE_CLOSE       = (3), 
    };
    
    struct CNotifyFd
    {
        int     m_nType;
        int     m_nFd;
        char    m_szLocalAddr[65];
        uint16  m_nLocalPort;
        char    m_szRemoteAddr[65];
        uint16  m_nRemotePort;
        bool    m_bBinaryMode;
        ISocketHandler *m_pHandler;

        CNotifyFd() { memset(this, 0, sizeof(CNotifyFd)); }
        CNotifyFd(int type, int fd, const char *local_addr, uint16 local_port, const char *remote_addr, uint16 remote_port, bool binary_mode, ISocketHandler *pHandler)
        {
            m_nType         = type;
            m_nFd           = fd;
            m_nLocalPort    = local_port;
            m_nRemotePort   = remote_port;
            m_bBinaryMode   = binary_mode;
            m_pHandler      = pHandler;
            snprintf(m_szLocalAddr, sizeof(m_szLocalAddr) - 1, "%s", local_addr);
            snprintf(m_szRemoteAddr, sizeof(m_szRemoteAddr) - 1, "%s", remote_addr);
        }
    };

    /* 网络工作线程(libevent支持多线程的模式)
    *   1.一个线程对象对应一个event_base
    *   2.管理一个通知event, 对应的是一个Pipe
    *   3.当有Socket需要添加到该event_base时, 加入队列后, 并写入Pipe
    *   4.检测到pipe有事件,从队列取出socket信息，再关联到event_base中
    */
    class CWorkerThread
    {
    public:
        CWorkerThread();
        ~CWorkerThread();
        
        bool Init();
        int  GetNotifySendFd() { return m_nNotifySendFd; } 
        bool PushNotifyFd(const CNotifyFd &oNotify);
        
        static void  NotifyCallback(evutil_socket_t fd, short events, void *arg);
        static void* Run(void *);
            
    private:
        void  DoListen(const CNotifyFd &oNotify);
        void  DoConnect(const CNotifyFd &oNotify);
        void  DoAccept(const CNotifyFd &oNotify);
        void  DoClose(const CNotifyFd &oNotify);
            
    private:
        pthread_t   m_nThreadID;                //线程ID
        int         m_nNotifyReadFd;            //Pipe读的fd
        int         m_nNotifySendFd;            //Pipe发送的fd[写]

        struct event_base *m_base;              //事件根基
        struct event m_oNotifyEvent;            //Listen通知Pipe的事件
        
        LockObject           m_oLock;           //锁
        std::list<CNotifyFd> m_lstNotifyFd;     //fd通知
    };
}

#endif // __NW_EVENT_THREAD_H__


