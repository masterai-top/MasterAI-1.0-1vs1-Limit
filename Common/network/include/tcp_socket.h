/**
* \file tcp_socket.h
* \brief tcp 套接字类头文件
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __TCP_SOCKET_H__
#define __TCP_SOCKET_H__

#include "typedef.h"
#include "thread/lock.h"
#include "socket.h"
#include "net_engine.h"
#include "event.h"
#include <list>
#include <map>
using namespace frame;

namespace network
{
    class CSocketManager;

    /**
    * \brief tcp 套接字类
    * \ingroup net_group
    */
    class TcpSocket : public Socket
    {
    public:
        /**
        * \brief 构造函数
        */
        TcpSocket(void);

        /**
        * \brief 析构函数
        */
        virtual ~TcpSocket(void);

        /**
        * \brief 设置套接字管理器
        * \param pSocketManager 套接字管理器
        */
        void SetSocketManager(CSocketManager *pSocketManager);
        CSocketManager* GetSocketManager() { return m_pSocketManager; }
        /**
        * \brief 设置套接字事件回调
        * \param pSocketHandler 套接字事件回调
        * \param bPkgLen 读写是否自动处理长度
        */
        void SetSocketHandler(ISocketHandler *pSocketHandler, bool bPkgLen);

        /**
        * \brief 发送消息
        * \param pBuffer 待发送数据
        * \param nLen 待发送数据大小
        * \return 发送成功返回true，否则返回false
        * 把待发送的数据组包放入待发送列表，再通过SendPackage发送数据包。
        */
        bool SendMsg(LISTSMG *p);

        /**
        * \brief 关闭连接
        * 在关闭之前，会强制再发送一次待发送的消息列表出去
        */
        void CloseConnect();
        void ActiveClose();
        
        void Release(bool bCloseFd);
        
    public:
        /**
        * \brief 注册监听事件
        * \return 注册成功返回true，否则返回false
        */
        bool RegisterListen(event_base *pEventBase);

        /**
        * \brief 注册连接事件
        * \return 注册成功返回true，否则返回false
        */
        //bool RegisterConnect();

        /**
        * \brief 注册读写事件
        * \return 注册成功返回true，否则返回false
        */
        bool RegisterEvent(event_base *pEventBase);

        /**
        * \brief 注册断开事件
        * \return 注册成功返回true，否则返回false
        */
        //bool RegisterClose();

        /**
        * \brief 发送待发送包
        * \param bForced 是否强制发送
        * \return 发送成功返回true，否则返回false
        * win32平台发送数据，需锁定内存，发生阻塞若可写最终会发送出去，可直接删除
        * linux平台发送数据，不会锁定内存，发生阻塞不会发送出去，需要在可写时重发，发送成功可直接从待发送列表中删除
        */
        bool SendPackage();

        uint32  GetUniqueID() { return m_nUniqueID; }
        void    SetUniqueID(uint32 nID) { m_nUniqueID = nID; }
        bool    GetBinaryMode() { return m_bBinaryMode; }
        
        //设置主任务ID
        void    SetTransID(uint8 nSvrType, uint16 nInstID);
        
        bool    IsAcceptFd() { return m_bIsAcceptFd; }
        void    SetIsAcceptFd(bool v) { m_bIsAcceptFd = v; }
        
        void    SetRemoteServerID(uint32 nRemoteServerID) { m_nRemoteServerID = nRemoteServerID; }

        //返回当前连接的下一个任务ID[每个连接,主任务ID不变,子任务ID递增]
        std::string  NextTransID();
        
    private:       
        /**
        * \brief 新连接事件
        */
        void OnAccept();

        /**
        * \brief 连接事件
        */
        void OnConnect();

        /**
        * \brief 可读事件
        */
        void OnRead();

        /**
        * \brief 可写事件
        */
        void OnWrite();

        /**
        * \brief 错误事件
        * \param nErrno 错误码
        */
        void OnError(int32 nErrno);

        /**
        * \brief 关闭事件
        * \param nReason 关闭原因
        */
        void OnClose(int32 nReason);
        
        /**
        * \brief 监听回调处理
        * \param fd An fd or signal
        * \param events One or more EV_* flags
        * \param arg A user-supplied argument.
        */
        static void AcceptCallback(evutil_socket_t fd, short events, void *arg);

        /**
        * \brief 可读回调处理
        * \param bev the bufferevent that triggered the callback
        * \param ctx the user-specified context for this bufferevent
        */
        static void ReadCallback(evutil_socket_t fd, short events, void *arg);

        /**
        * \brief 可写回调处理
        * \param bev the bufferevent that triggered the callback
        * \param ctx the user-specified context for this bufferevent
        */
        static void WriteCallback(evutil_socket_t fd, short events, void *arg);

        /**
        * \brief 网络事件回调处理
        * \param bev the bufferevent for which the error condition was reached
        * \param what a conjunction of flags: BEV_EVENT_READING or BEV_EVENT_WRITING
                    to indicate if the error was encountered on the read or write path,
                    and one of the following flags: BEV_EVENT_EOF, BEV_EVENT_ERROR,
                    BEV_EVENT_TIMEOUT, BEV_EVENT_CONNECTED.
        * \param ctx the user-specified context for this bufferevent
        */
        //static void EventCallback(struct bufferevent *bev, short what, void *ctx);

        /**
        * \brief 断开连接回调处理
        * \param fd An fd or signal
        * \param events One or more EV_* flags
        * \param arg A user-supplied argument.
        */
        static void CloseCallback(evutil_socket_t fd, short events, void *arg);

    private:
        uint32              m_nUniqueID;                //SocketManager为每一个对象分配的唯一标记
        char                m_szTransID[TID_LEN - 5];   //主TransID
        uint16              m_nTidIndex;                //子TransID序列
        uint32              m_nRemoteServerID;          //连接对端的ServerID
        
        event               m_oListenEvent;            ///< 监听事件对象
        event               m_oReadEvent;              ///< 读事件对象
        event               m_oWriteEvent;             ///< 写事件对象
        event               m_oCloseEvent;             ///< 关闭事件对象

        CSocketManager      *m_pSocketManager;          ///< 套接字管理器
        ISocketHandler      *m_pSocketHandler;          ///< 事件回调
        bool                m_bBinaryMode;              /// 是否是二进制协议模式

        bool                m_bIsAcceptFd;              // 是服务端Accept得到的fd(该fd是服务端程序accept客户端连接的fd)
        LockObject          m_oLock;                    ///< 发送锁
        
        std::list<LISTSMG*> m_lstSndPacket;             //发送列表[暂时未使用]
        LISTSMG            *m_pRecvMsg;                 //接收数据的LISTSMG对象

        int                 m_nFreeQueueNum;            //空闲队列数量(仅日志使用)
    };
}

#endif // __TCP_SOCKET_H__