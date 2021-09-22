/**
* \file socket_manager.cpp
* \brief 网络套接字管理类函数的实现
*/

#include "pch.h"
#include "socket_manager.h"
#include "shstd.h"
#include "system.pb.h"
using namespace shstd::hashmap;

/*#define SM_MAX_CONNECT_CNT    (65000)  //支持的最大连接数
static uint32 SMSocketHash(const int32 &nSock)    
{
    return (uint32)nSock;
}*/


namespace network
{
    /**
    * \brief 构造函数
    */
    CSocketManager::CSocketManager(void)
    {
        //m_pEventBase = NULL;
    }

    /**
    * \brief 析构函数
    */
    CSocketManager::~CSocketManager(void)
    {
        Release();
    }

    /**
    * \brief 创建
    * \param pEventBase 事件根基
    * \return 创建成功返回true，否则返回false
    */
    bool CSocketManager::Init(uint32 nLocalServerID, uint32 nTcpTimeOut, uint32 nSocketCnt, uint8 nLimitedLogEnable)
    {
        m_nLimitedLogEnable = nLimitedLogEnable;
        m_nSocketCnt        = nSocketCnt;
        if(m_nSocketCnt > 0x7FFFF) {
            LOG(LT_ERROR, "Socket manager init| check socket count| cnt=%u", m_nSocketCnt);
            return false;
        }

        m_pSocket = new TcpSocket[m_nSocketCnt];
        if(NULL == m_pSocket) {
            LOG(LT_ERROR, "Socket manager init| new tcp socket failed");
            return false;
        }
        
        for(uint32 i = 0; i < m_nSocketCnt; i++) {
            m_pSocket[i].SetSocketManager(this);
        }
        
        m_nLocalServerID    = nLocalServerID;
        m_nTcpTimeOut       = nTcpTimeOut;

        LOG(LT_INFO, "Socket manager init succ| socket_cnt=%u| limited_log_enable=%d", m_nSocketCnt, m_nLimitedLogEnable);
        
        return true;
    }

    /**
    * \brief 释放
    */
    void CSocketManager::Release()
    {   
        for(uint32 i = 0; i < m_nSocketCnt; i++) {
            m_pSocket[i].CloseConnect();
        }
    }

    /**
    * \brief 开启服务器监听服务
    * \param szAddr 监听IP地址
    * \param nPort 监听端口
    * \param pHandler 回调对象
    * \return 开启成功返回true，失败返回false
    */
    bool CSocketManager::Listen(const char *szAddr, uint16 nPort, bool bBinaryMode)
    {

        sockid nSockID = Socket::GlobalSocket(SOCK_STREAM, IPPROTO_TCP);
        if (nSockID <= 0)
        {
            LOG(LT_ERROR, "Socket manager listen failed| msg=%s", strerror(errno));
            return false;
        }
        else if((uint32)nSockID >= m_nSocketCnt) {
            LOG(LT_ERROR, "Socket manager listen| exceed socket| fd=%d", nSockID);
            close(nSockID);
            return false;
        }

        TcpSocket &oListener = m_pSocket[nSockID];
        bool bSuccess        = false; 
        do 
        {
            // 创建
            if (!oListener.Create(nSockID))
            {
                break;
            }
            // 保持连接
            if (!oListener.SetKeepAlive(true))
            {
                break;
            }
            // 不粘包
            if (!oListener.SetNoDelay(true))
            {
                break;
            }
            // 非阻塞模式
            if (!oListener.SetNonBlock())
            {
                break;
            }
            // 延时关闭
            if (!oListener.SetLinger())
            {
                break;
            }
            // 可重复使用
            if (!oListener.SetReuse(true))
            {
                break;
            }
            // 绑定
            if (!oListener.Bind(szAddr, nPort))
            {
                break;
            }
            // 开始监听
            if (!oListener.Listen())
            {
                break;
            }

            oListener.SetSocketHandler(NULL, bBinaryMode);
            CNotifyFd oNotify(NOTIFY_TYPE_LISTEN, oListener.GetSockID(), szAddr, nPort, "", 0, bBinaryMode, NULL);
            if(!CNetWorker::Instance()->GetAcceptThread()->PushNotifyFd(oNotify))
            {
                LOG(LT_ERROR, "Listen failed| errno=%d| errmsg=%s|", errno, strerror(errno));
                break;
            }
            
            bSuccess = true;
        } while (false);    // 利用循环来处理判断

        // 开启失败，关闭socket
        if (!bSuccess)
        {
            oListener.Close();  //不能调用Release, 导致多线程使用同一个event_base, 引起core
            return false;
        }

        LOG(LT_INFO, "Socket manager listen succ| fd=%d", oListener.GetSockID());
        
        return true;
    }

    /**
    * \brief 连接服务器
    * \param szAddr 服务器IP地址
    * \param nPort 服务器端口
    * \param pHandler 回调对象
    * \return 连接成功返回true，失败返回false
    */
    bool CSocketManager::Connect(const char *szAddr, uint16 nPort, ISocketHandler *pHandler, bool bBinaryMode)
    {
        sockid nSockID = Socket::GlobalSocket(SOCK_STREAM, IPPROTO_TCP);
        if (nSockID <= 0)
        {
            LOG(LT_ERROR, "Socket manager connect failed| msg=%s", strerror(errno));
            return false;
        }
        else if((uint32)nSockID >= m_nSocketCnt) {
            LOG(LT_ERROR, "Socket manager connect| exceed socket| fd=%d", nSockID);
            close(nSockID);
            return false;
        }

        
        CNotifyFd oNotify(NOTIFY_TYPE_CONNECT, nSockID,  "", 0, szAddr, nPort, bBinaryMode, pHandler);
        if(!CNetWorker::Instance()->GetDataThread(nSockID)->PushNotifyFd(oNotify))
        {
            LOG(LT_ERROR, "Socket manager accept new| notify failed| fd=%d| remote_addr=%s:%d| binary_mode=%d", 
                nSockID, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort, oNotify.m_bBinaryMode
                );
        }
        else 
        {
           LOG(LT_INFO, "Socket manager new connect| fd=%d| remote_addr=%s:%d| binary_mode=%d", 
                nSockID, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort, oNotify.m_bBinaryMode
                );
        }
            
        return true;
    }

    /**
    * \brief 发送数据给单个连接对象
    * \param pMsg 消息数据
    * \param nLen 数据大小
    * \param nRemoteFd 指定发送连接对象ID
    * \return 发送成功返回true，否则返回false
    */
    void CSocketManager::Send(LISTSMG *p)
    {
        if(NULL == p) {
            return;
        }

        TcpSocket *pSocket = NULL;
        if(PKT_TYPE_DISCARD == p->cPacketType)
        {
            LOG(LT_INFO_TRANS, p->szTransID, "Socket manager send discard packet| fd=%d| unique_id=0x%x| pkt_type=%d", p->connfd, p->nUniqueID, p->cPacketType);
        }
        else if(NULL == (pSocket = GetTcpSocket(p->connfd)))
        {   
            LOG(LT_ERROR_TRANS, p->szTransID, "Socket manager send| find socket failed| fd=%d| unique_id=0x%x", p->connfd, p->nUniqueID);
        }
        else if(!pSocket->SendMsg(p)) 
        {
            LOG(LT_ERROR_TRANS, p->szTransID, "Socket manager send| do failed| fd=%d| unique_id=0x%x", p->connfd, p->nUniqueID);
        }

        CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
        return;
    }

    /**
    * \brief 获得连接信息
    * \param nFd 连接对象ID
    * \param strAddr 返回连接对象IP地址
    * \param nPort 返回连接对象端口
    * \param bLocal 是否获取本地连接信息 
    * \return 获取成功返回true，否则返回false
    */
    bool CSocketManager::GetConnectInfo(int32 nFd, std::string &strAddr, uint16 &nPort)
    {        
        TcpSocket *pSocket = GetTcpSocket(nFd);
        if (NULL == pSocket)
        {
            return false;
        }

        if(!pSocket->IsAcceptFd()) {
            strAddr = pSocket->GetLocalAddr();
            nPort   = pSocket->GetLocalPort();
        }
        else {
            strAddr = pSocket->GetRemoteAddr();
            nPort   = pSocket->GetRemotePort();
        }
        
        return true;
    }

    /**
    * \brief 设置连接对象消息回调
    * \param nFd 连接ID
    * \param pHandler 回调对象
    * \param bPkgLen 收发消息处理长度
    */
    void CSocketManager::SetSocketHandler(int32 nFd, ISocketHandler *pHandler, bool bPkgLen /*= true*/)
    {
        TcpSocket *pSocket = GetTcpSocket(nFd);
        if (NULL != pSocket)
        {
            pSocket->SetSocketHandler(pHandler, bPkgLen);
        }
    }

    /**
    * \brief 判断指定连接是否处于连接状态
    * \param nFd 连接ID
    * \return 连接中返回true，否则返回false
    */
    bool CSocketManager::IsValidConnected(int32 nFd, uint32 nUniqueID)
    {
        TcpSocket *pSocket = GetTcpSocket(nFd);
        if (NULL == pSocket)
        {
            return false;
        }

        if (!pSocket->IsValid())
        {
            return false;
        }

        if(pSocket->GetUniqueID() != nUniqueID) 
        {
            return false;
        }

        return true;
    }

    /**
    * \brief 关闭指定连接
    * \param nFd 连接对象ID
    * \return 成功关闭返回true，否则返回false
    */
    bool CSocketManager::CloseConnect(int32 nFd)//, uint32 nUniqueID)
    {
        TcpSocket *pSocket = GetTcpSocket(nFd);
        if (NULL == pSocket)
        {
            return false;
        }
        /*else if(pSocket->GetUniqueID() != nUniqueID)
        {
            LOG(LT_ERROR, "Socket manager close check unique_id failed| fd=%d| close_unique_id=0x%x| unique_id=0x%x", nUniqueID, pSocket->GetUniqueID());    
            return false;
        }*/

        
        pSocket->CloseConnect();

        return true;
    }

    //获取连接的下一个TransID
    std::string  CSocketManager::GetNextTransID(int32 nFd)
    {
        TcpSocket *pSocket = GetTcpSocket(nFd);
        if (NULL == pSocket)
        {
            return std::string("");
        }
        
        return pSocket->NextTransID();
    }
    
    //设置注册ID
    bool CSocketManager::SetRemoteServerID(int32 nFd, uint32 nRemoteServerID)
    {
        TcpSocket *pSocket = GetTcpSocket(nFd);
        if (NULL == pSocket)
        {
            return false;
        }
        
        pSocket->SetRemoteServerID(nRemoteServerID);
        return true;
    }
    
    /**
    * \brief 关闭所有连接
    */
    void CSocketManager::CloseAllConnect()
    {
        Lock l(&m_lock);        
        for(uint32 i = 0; i < m_nSocketCnt; i++)
        {
            TcpSocket &oSocket = m_pSocket[i];
            if(Socket::SOCK_STATE_LISTEN != oSocket.GetState())
            {
                oSocket.CloseConnect();
            }
        }
    }
    
 

    /**
    * \brief 获得连接
    * \param nFd 连接ID
    * \return 连接对象
    */
    TcpSocket * CSocketManager::GetTcpSocket(int32 nFd)
    {
        if(nFd < 0 || (uint32)nFd >= m_nSocketCnt) {
            return NULL;
        }
        
        return &m_pSocket[nFd];
    }

    /**
    * \brief 接收到新连接
    * \param pListener 监听连接
    * \return 新连接对象
    */
    void CSocketManager::OnAccept(TcpSocket *pListener)
    {
        assert(NULL != pListener);
        
        // 使用新的连接对象准备连接
        sockaddr_in addrRemote;
        sockid nSockID = Socket::GlobalAccept(pListener->GetSockID(), addrRemote);
        if (nSockID <= 0)
        {
            LOG(LT_ERROR, "Socket manager accept failed| msg=%s", strerror(errno));
            return ;
        }
        else if((uint32)nSockID >= m_nSocketCnt) {
            LOG(LT_ERROR, "Socket manager accept| exceed socket| fd=%d", nSockID);
            close(nSockID);
            return;
        }

        CNotifyFd oNotify(NOTIFY_TYPE_ACCEPT, nSockID,  pListener->GetLocalAddr().c_str(), pListener->GetLocalPort(), 
            inet_ntoa(addrRemote.sin_addr), ntohs(addrRemote.sin_port), pListener->GetBinaryMode(), NULL);
        if(!CNetWorker::Instance()->GetDataThread(nSockID)->PushNotifyFd(oNotify))
        {
            LOG(LT_ERROR, "Socket manager accept new| notify failed| fd=%d| remote_addr=%s:%d| binary_mode=%d", 
                nSockID, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort, oNotify.m_bBinaryMode
                );
            close(nSockID);
            return;
        }
        
       LOG(LT_INFO, "Socket manager accept new socket| fd=%d| local_addr=%s:%d| remote_addr=%s:%d| binary_mode=%d", 
            nSockID, oNotify.m_szLocalAddr, oNotify.m_nLocalPort, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort, oNotify.m_bBinaryMode
            );        
        
        return;
    }

    /**
    * \brief 连接成功事件
    * \param pSocket 连接成功的对象
    * \return 成功返回true，否则返回false
    */
    bool CSocketManager::OnConnect(TcpSocket *pSocket)
    {
        /*if (NULL == pSocket)
        {
            return false;
        }
        
        SERVERKEY sk(m_nLocalServerID);
        pSocket->SetUniqueID(GetSequence());
        pSocket->SetTransID(sk.nType, sk.nInstID);
        */
        
        return true;
    }

    /**
    * \brief 连接断开事件
    * \param pSocket 断开连接的对象
    * \return 成功返回true，否则返回false
    */
    bool CSocketManager::OnClose(TcpSocket *pSocket)
    {
        if (NULL == pSocket)
        {
            return false;
        }

        LOG(LT_INFO, "Socket manager on close| fd=%d", pSocket->GetSockID());

        return true;
    }

    uint32  CSocketManager::GetSequence() 
    {
        uint32 nTime  = time(NULL)%0xFFFF;    
        Lock l(&m_lock);
        ++m_nSequence;
        return ((nTime << 16) + m_nSequence);
    }
}


