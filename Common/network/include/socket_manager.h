/**
* \file socket_manager.h
* \brief 套接字管理器头文件
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __NW_SOCKET_MANAGER_H__
#define __NW_SOCKET_MANAGER_H__

#include "typedef.h"
#include "net_engine.h"
#include "tcp_socket.h"
#include "CQUEUE_List.h"
#include "comm_define.h"
#include <map>
#include "shhashmap.h"
#include "net_worker.h"

using namespace shstd::hashmap;

namespace network
{
    class CSocketManager 
    {
    private:
        CSocketManager(void);
    
    public:
        ~CSocketManager(void);
        static CSocketManager* Instance()
        {
            static CSocketManager m_instance;
            return &m_instance;
        }
        
        bool Init(uint32 nLocalServerID, uint32 nTcpTimeOut, uint32 nSocketCnt, uint8 nLimitedLogEnable);
        void Release();
        bool Listen(const char *szAddr, uint16 nPort, bool bBinaryMode);
        bool Connect(const char *szAddr, uint16 nPort, ISocketHandler *pHandler, bool bBinaryMode);
        void Send(LISTSMG *p);
        bool GetConnectInfo(int32 nFd, std::string &strAddr, uint16 &nPort);
        void SetSocketHandler(int32 nFd, ISocketHandler *pHandler, bool bPkgLen = true);
        bool IsValidConnected(int32 nFd, uint32 nUniqueID);
        bool CloseConnect(int32 nFd); //, uint32 nUniqueID);
        void CloseAllConnect();

        string  GetNextTransID(int32 nFd);
        bool    SetRemoteServerID(int32 nFd, uint32 nRemoteServerID);
        uint32  GetTcpTimeOut() { return m_nTcpTimeOut; }
        uint32  GetLocalServerID() { return m_nLocalServerID; }
        bool    IsLimitedLog() { return (SWITCH_ENABLE == m_nLimitedLogEnable); }
        
    public:
        TcpSocket * GetTcpSocket(int32 nFd);

        void    OnAccept(TcpSocket *pListener);
        bool    OnConnect(TcpSocket *pSocket);
        bool    OnClose(TcpSocket *pSocket);
        uint32  GetSequence();        
        
    private:
        LockObject          m_lock;                             // 锁
        uint16              m_nSequence;                        // 自增序列
        uint32              m_nTcpTimeOut;                      //连接超时时间(Socket Manager)
        uint32              m_nLocalServerID;                   // server_id
        uint32              m_nSocketCnt;                       // m_pSocket数量
        uint8               m_nLimitedLogEnable;                // 限量日志开关
        TcpSocket          *m_pSocket;                          // socket对象列表
    };
}

#endif // __SOCKET_MANAGER_H__
