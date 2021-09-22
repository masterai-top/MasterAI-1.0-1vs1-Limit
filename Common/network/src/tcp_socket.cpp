/**
* \file tcp_socket.cpp
* \brief tcp 套接字类函数的实现
*/

#include "pch.h"
#include "tcp_socket.h"
#include "socket_manager.h"
#include "event2/event.h"
#include "shstd.h"
#include "strutil.h"
#include "net_msg.h"
#include "system.pb.h"
#include "comm.pb.h"
#include "net_worker.h"
#include "google/protobuf/text_format.h"

using namespace dtutil;
using namespace shstd::fun;

uint64 htonl64(uint64 host64)
{
    uint64 ret=0;
    uint32 high = 0, low = 0;
    
    low  = host64 & 0xFFFFFFFF;
    high = (host64 >> 32) & 0xFFFFFFFF;
    low  = htonl(low);
    high = htonl(high);

    ret  =   low;
    ret  <<= 32;
    ret  |=  high;
    return ret;
}

uint64 ntohl64(uint64 net64)
{
    uint64 ret = 0;
    uint32 high = 0, low = 0;

    low  = net64 & 0xFFFFFFFF;
    high = (net64 >> 32) & 0xFFFFFFFF;
    low  = ntohl(low);
    high = ntohl(high);
    
    ret  =   low;
    ret  <<= 32;
    ret  |=  high;
    return ret;
}


//构造网络应答报文
int DoComNetPacket(LISTSMG *p, uint32 cmd, uint32 errcode, const char *szTransID, uint64 nCliConnID, const google::protobuf::Message *pbMsg, uint32 nRoleID)
{   
    strncpy(p->szTransID, szTransID, TID_LEN - 1);
    p->nCmd         = cmd;
    p->nErrCode     = errcode;
    p->nCliConnID   = nCliConnID;
    p->nRoleID      = nRoleID;

    if(NULL == pbMsg)
    {
        NetMsgHead head(PACKAGE_HEAD_LEN, cmd, errcode, nCliConnID, szTransID);
        head.head_hton();
        memcpy(p->cPacketBuffer, &head, PACKAGE_HEAD_LEN);
        p->len = PACKAGE_HEAD_LEN;
        return 0;
    }
    
    if(pbMsg->ByteSize() > (PACKAGE_LEN_MAX - 100)) {
        NetMsgHead head(PACKAGE_HEAD_LEN, cmd, Pb::ERR_COMM_LENGTH, nCliConnID, szTransID);
        head.head_hton();
        memcpy(p->cPacketBuffer, &head, PACKAGE_HEAD_LEN);
        p->len = PACKAGE_HEAD_LEN;

        return Pb::ERR_COMM_LENGTH;
    }
    else {
        NetMsgHead head(PACKAGE_HEAD_LEN + pbMsg->ByteSize(), cmd, errcode, nCliConnID, szTransID);
        head.head_hton();
        memcpy(p->cPacketBuffer, &head, PACKAGE_HEAD_LEN);        
        pbMsg->SerializeToArray(p->cPacketBuffer + PACKAGE_HEAD_LEN, pbMsg->ByteSize());
        p->len = PACKAGE_HEAD_LEN + pbMsg->ByteSize();
        return 0;
    }
}

//构造网络应答报文
//适用于内容是纯字符串的报文
int DoComNetPacket(LISTSMG *p, const char *szTransID, int fd, uint32 nUniqueID, uint64 nCliConnID, const char *buf, uint32 nRoleID)
{   
    //只发送buf的内容, head不发送,所有不需要进行网络字节序转换
    p->Reset();
    strncpy(p->szTransID, szTransID, TID_LEN - 1);
    p->nCmd         = Pb::CMD_BRAIN_TO_ROBOT_CLIENT;
    p->nErrCode     = 0;
    p->nCliConnID   = nCliConnID;
    p->nRoleID      = nRoleID;
    p->connfd       = fd;
    p->nUniqueID    = nUniqueID;
    p->len          = strlen(buf);
    
    if(p->len > (PACKAGE_LEN_MAX - 100)) {
        p->len = PACKAGE_LEN_MAX - 100;
    }

    memcpy(p->cPacketBuffer, buf, p->len);
    return 0;
}


//构造应答报文(包头只修改错误码+长度)
//参数   @p 为接收到客户端的报文指针
int DoResponsePacket(LISTSMG *p, uint32 errcode, const google::protobuf::Message *pbMsg, uint32 nRoleID)
{   
    //head的其他字段,如果没改变,则不需要进行网络字节序转换(原样返回,因为recv的时候,head的内容是没有变的)
    NetMsgHead* pHead   = (NetMsgHead*)p->cPacketBuffer;
    p->nRoleID = nRoleID;
    
    if(NULL == pbMsg)
    {     
        p->nErrCode = errcode;
        p->len      = PACKAGE_HEAD_LEN;

        //修改设置 m_nLen + m_nErrCode
        pHead->m_nErrCode = htonl(p->nErrCode);
        pHead->m_nLen     = htonl(p->len);
        return 0;         
    }

    //长度校验,理论当前不会出现这种错误
    if(pbMsg->ByteSize() > (PACKAGE_LEN_MAX - 100)) 
    {
        p->nErrCode = Pb::ERR_COMM_LENGTH;
        p->len      = PACKAGE_HEAD_LEN;

        //修改设置 m_nLen + m_nErrCode
        pHead->m_nErrCode = htonl(p->nErrCode);
        pHead->m_nLen     = htonl(p->len);
        return Pb::ERR_COMM_LENGTH;
    }
    else 
    {
        p->nErrCode = errcode;
        p->len      = PACKAGE_HEAD_LEN + pbMsg->ByteSize();

        //修改设置 m_nLen + m_nErrCode
        pHead->m_nErrCode = htonl(p->nErrCode);
        pHead->m_nLen     = htonl(p->len);

        //协议内存
        pbMsg->SerializeToArray(p->cPacketBuffer + PACKAGE_HEAD_LEN, pbMsg->ByteSize());
        p->len = PACKAGE_HEAD_LEN + pbMsg->ByteSize();
        return 0;
    }    
}


//构造应答报文(包头只修改错误码+长度)
//参数   @p 为接收到客户端的报文指针
int DoResponsePacket2(LISTSMG *p, uint32 errcode, const char *buf)
{   
    p->nErrCode     = errcode;
    p->len          = strlen(buf);
    
    if(p->len > (PACKAGE_LEN_MAX - 100)) {
        p->len = PACKAGE_LEN_MAX - 100;
    }

    memcpy(p->cPacketBuffer, buf, p->len);
    return 0; 
}

std::string ProtobufToString(const google::protobuf::Message &pbMsg)
{
    std::string str;
    google::protobuf::TextFormat::PrintToString(pbMsg, &str);
    replace_all(str, "  ", " ");
    replace_all(str, "{\n ", "{ ");
    replace_all(str, "}\n", "}");
    replace_all(str, "\n", "| ");
    replace_all(str, ": ", "=");
    
    return str;
}


namespace network
{
    void out_put_buf(LISTSMG *p)
    {
        #define PRINT_LINE_BYTES (50)
        
        int sndBtyes = p->len;
        int index    = 0;
        char szBuf[512] = {0};
        for(int i = 0; i < sndBtyes; i++)
        {
            index++;
            snprintf(szBuf + strlen(szBuf), 5, " %.02x", (unsigned char)p->cPacketBuffer[i]);
            if(0 == index%PRINT_LINE_BYTES)
            {   
                LOG(LT_DEBUG_TRANS, p->szTransID, "TC_SEND_MSG| total=%d| %.04d - %.04d Bytes =%s\r\n", p->len, (i+1) - PRINT_LINE_BYTES, i + 1, szBuf);
                memset(szBuf, 0, sizeof(szBuf));
                index = 0;
            }

            if((i+1) == sndBtyes)
            {
                LOG(LT_DEBUG_TRANS, p->szTransID, "TC_SEND_MSG| total=%d| %.04d - %.04d Bytes =%s\r\n", p->len, sndBtyes - sndBtyes%PRINT_LINE_BYTES, sndBtyes, szBuf);
            }
        }
    }

    /**
    * \brief 构造函数
    */
    TcpSocket::TcpSocket(void):Socket()
    {
        //m_pListenEvent  = NULL;
        //m_pReadEvent    = NULL;
        //m_pWriteEvent   = NULL;
        //m_pCloseEvent   = NULL;
        m_pSocketManager = NULL;
        m_pSocketHandler = NULL;
        m_bBinaryMode = true;
        m_bIsAcceptFd     = false;
        m_nRemoteServerID = 0;
        m_pRecvMsg        = NULL;

        m_oCloseEvent.ev_base   = NULL;
        m_oListenEvent.ev_base  = NULL;
        m_oReadEvent.ev_base    = NULL;
        m_oWriteEvent.ev_base   = NULL;
    }

    /**
    * \brief 析构函数
    */
    TcpSocket::~TcpSocket(void)
    {
        m_pSocketManager = NULL;
        m_pSocketHandler = NULL;
        m_bBinaryMode = true;
        
        Release(true);
    }

    void TcpSocket::Release(bool bCloseFd)
    {
        LOG(LT_INFO_TRANS, NextTransID().c_str(), "TcpSocket release| fd=%d| unique_id=0x%x", GetSockID(), GetUniqueID());
        
        Lock lock(&m_oLock);    //Release必在同一个线程调用, 不需要加锁 ?
        if(NULL != m_oListenEvent.ev_base)
        {
            event_del(&m_oListenEvent);
            m_oListenEvent.ev_base = NULL;
        }

        if(NULL != m_oReadEvent.ev_base) 
        {
            event_del(&m_oReadEvent);
            m_oReadEvent.ev_base = NULL;
        }

        if(NULL != m_oWriteEvent.ev_base) 
        {
            event_del(&m_oWriteEvent);
            m_oWriteEvent.ev_base = NULL;
        }

        if(NULL != m_oCloseEvent.ev_base) {
            event_del(&m_oCloseEvent);
            m_oCloseEvent.ev_base = NULL;
        }

        if(bCloseFd) {
            Close();
        }
        
        /*if(NULL != m_pListenEvent) {
            event_del(m_pListenEvent);
            event_free(m_pListenEvent);
            m_pListenEvent  = NULL;
        }
        if(NULL != m_pReadEvent) {
            event_del(m_pReadEvent);
            event_free(m_pReadEvent);
            m_pReadEvent    = NULL;
        }
        if(NULL != m_pWriteEvent) {
            event_del(m_pWriteEvent);
            event_free(m_pWriteEvent);
            m_pWriteEvent   = NULL;
        }
        if(NULL != m_pCloseEvent) {
            event_del(m_pCloseEvent);
            event_free(m_pCloseEvent);
            m_pCloseEvent   = NULL;
        }*/

        if(NULL != m_pRecvMsg)
        {            
            CQUEUE_List::Instance()->SetNode(m_pRecvMsg, QUEUE_FREE);
            m_pRecvMsg = NULL;        
        }
        
        m_bIsAcceptFd       = false;
        m_nRemoteServerID   = 0;
        m_bBinaryMode           = true;
        m_nUniqueID         = 0;
        m_nTidIndex         = 0;
        m_pSocketHandler    = NULL;
        memset(m_szTransID, 0, sizeof(m_szTransID));

        while(!m_lstSndPacket.empty())
        {
            LISTSMG *p = m_lstSndPacket.front();
            if(NULL != p) {
                m_lstSndPacket.pop_front();
                CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
            }
        }
    }
    /**
    * \brief 设置套接字管理器
    * \param pSocketManager 套接字管理器
    */
    void TcpSocket::SetSocketManager(CSocketManager *pSocketManager)
    {
        m_pSocketManager = pSocketManager;
    }

    /**
    * \brief 设置套接字事件回调
    * \param pSocketHandler 套接字事件回调
    * \param bPkgLen 读写是否自动处理长度
    */
    void TcpSocket::SetSocketHandler(ISocketHandler *pSocketHandler, bool bPkgLen /*= true*/)
    {
        m_pSocketHandler = pSocketHandler;
        m_bBinaryMode = bPkgLen;
    }

    /**
    * \brief 发送消息
    * \param pBuffer 待发送数据
    * \param nLen 待发送数据大小
    * \return 发送成功返回true，否则返回false
    * 把待发送的数据组包放入待发送列表，再通过SendPackage发送数据包。
    */
    bool TcpSocket::SendMsg(LISTSMG *p)
    {
        if (p == NULL)
        {
            return false;
        }

        if(p->len > (PACKAGE_LEN_MAX - 10)) {
            LOG(LT_ERROR_TRANS, p->szTransID, "SEND_MSG| check len failed| len=%d", p->len);
            return false;
        }
        else if(m_bBinaryMode) 
        {
            NetMsgHead oNetHead;
            oNetHead.UnpackFromArray((LPCSTR)p->cPacketBuffer, p->len);
            if(p->len != oNetHead.m_nLen) 
            {
                LOG(LT_ERROR_TRANS, oNetHead.m_szTransID, "SEND_MSG| (do length) check len failed| len=%d| buf_len=%d", p->len, oNetHead.m_nLen);
                return false;
            }
        }

        int nLeftBytes = p->len;
        int nDoneTimes = 0;
        {
            //发送数据
            Lock lock(&m_oLock);
            if(p->nUniqueID != GetUniqueID())
            {
                LOG(LT_ERROR_TRANS, p->szTransID, "SEND_MSG| check unique id failed| fd=%d| unique_id=0x%x| sock_unique_id=%x", p->connfd, p->nUniqueID, GetUniqueID());
                return false;
            }
        
            while(nLeftBytes > 0) 
            {
                int nBtyes = Send(p->cPacketBuffer + (p->len - nLeftBytes), nLeftBytes);
                if (nBtyes < 0 || nBtyes > nLeftBytes)
                {   
                    //错误
                    LOG(LT_ERROR_TRANS, p->szTransID, "SEND_MSG| send failed close| len=%d| fd=%d| rc=%d| msg=%s", p->len, GetSockID(), nBtyes, strerror(errno));
                    CloseConnect();
                    return false;
                }
                else if(0 == nBtyes) 
                {   
                    //堵塞. 尝试3次
                    if(nDoneTimes > 3) 
                    {
                        LOG(LT_ERROR_TRANS, p->szTransID, "SEND_MSG| send failed close| len=%d| fd=%d| rc=%d| msg=%s", p->len, GetSockID(), nBtyes, strerror(errno));
                        CloseConnect();
                        return false;
                    }

                    usleep(1000);
                    ++nDoneTimes;
                }
                else 
                {
                    nDoneTimes = 0;
                    if(nBtyes != nLeftBytes) 
                    {
                        LOG(LT_WARN_TRANS, p->szTransID, "SEND_MSG| need try again| len=%d| left_bytes=%d| send_bytes=%d", p->len, nLeftBytes, nBtyes);
                    }
                    
                    nLeftBytes -= nBtyes;
                }
            }
        }

        //发送成功
        if(!m_pSocketManager->IsLimitedLog()) 
        {
            LOG(LT_INFO_TRANS, p->szTransID, "SEND_MSG| fd=%d| cmd=%d| errcode=%u| len=%u| time=%d| unique_id=0x%x| cli_conn_id=0x%llx| role_id=%u",
                GetSockID(), p->nCmd, p->nErrCode, p->len, p->recv_time.ToNow(), m_nUniqueID, p->nCliConnID, p->nRoleID);
        }
        return true;
        
        // 将新的空包添加到末尾
        /*int nQSize = 0;        
        if(1) {
            Lock lock(&m_oLock);
            if(SOCK_STATE_NORMAL != GetState())
            {
                LOG(LT_ERROR_TRANS, p->szTransID, "TO_SEND_QUEUE| state invalid| state=%d", GetState());
                return false;
            }
            m_lstSndPacket.push_back(p); 
            nQSize = m_lstSndPacket.size();
        }

        //1.push_back前为空,才add事件
        //2.监听可写事件<要再锁内处理,与event_del对应>
        if(1 == nQSize) 
        {
            struct timeval tv = {5, 0};
            Lock lock(&m_oLock);    
            event_add(m_pWriteEvent, &tv);
        }
        
        return true;*/
    }

    /**
    * \brief 关闭连接
    * 在关闭之前，会强制再发送一次待发送的消息列表出去
    */
    void TcpSocket::CloseConnect()
    {
        // 已经关闭了
        int nSockID = GetSockID();
        if (!IsValid())
        {
            return;
        }
        
        CNotifyFd oNotify(NOTIFY_TYPE_CLOSE, nSockID,  GetLocalAddr().c_str(), GetLocalPort(), 
            GetRemoteAddr().c_str(), GetRemotePort(), GetBinaryMode(), NULL);

        if(SOCK_STATE_LISTEN == GetState())
        {
            if(!CNetWorker::Instance()->GetAcceptThread()->PushNotifyFd(oNotify))
            {
                //理论上不会出错
                LOG(LT_ERROR_TRANS, NextTransID().c_str(), "Tcp socket close| notify failed| fd=%d| remote_addr=%s:%d| binary_mode=%d", 
                    nSockID, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort, oNotify.m_bBinaryMode
                    );
                Close();
                return;
            }
        }
        else 
        {
            if(!CNetWorker::Instance()->GetDataThread(nSockID)->PushNotifyFd(oNotify))
            {
                //理论上不会出错
                LOG(LT_ERROR_TRANS, NextTransID().c_str(), "Tcp socket close| notify failed| fd=%d| remote_addr=%s:%d| binary_mode=%d", 
                    nSockID, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort, oNotify.m_bBinaryMode
                    );
                Close();
                return;
            }
        }
        
       LOG(LT_INFO_TRANS, NextTransID().c_str(), "Tcp socket close notify| fd=%d| local_addr=%s:%d| remote_addr=%s:%d| binary_mode=%d", 
            nSockID, oNotify.m_szLocalAddr, oNotify.m_nLocalPort, oNotify.m_szRemoteAddr, oNotify.m_nRemotePort, oNotify.m_bBinaryMode
            );
        
        // 已经关闭了
        //if (!IsValid())
        //{
        //    return;
        //}
        
        // 激活关闭事件
        //if(NULL != m_oCloseEvent.ev_base) {
        //    event_active(&m_oCloseEvent, -1, 1);
        //}
    }

    //只能内部线程调用
    void TcpSocket::ActiveClose()
    {
        // 已经关闭了
        Lock lock(&m_oLock);
        if (!IsValid())
        {
            return;
        }
        
        // 激活关闭事件
        if(NULL != m_oCloseEvent.ev_base) {
            event_active(&m_oCloseEvent, -1, 1);
        }
        else {
            Close();
        }
    }
    /**
    * \brief 注册监听事件(服务端启动监听调用)
    * \return 注册成功返回true，否则返回false
    */
    bool TcpSocket::RegisterListen(event_base *pEventBase)
    {
        assert(NULL != pEventBase);
        assert(NULL != m_pSocketManager);
        
        if (!IsValid())
        {
            return false;
        }

        if (GetState() != Socket::SOCK_STATE_LISTEN)
        {
            return false;
        }

        Lock lock(&m_oLock);
        event_set(&m_oListenEvent, GetSockID(), EV_READ|EV_PERSIST, TcpSocket::AcceptCallback, (void *)(this));
        event_base_set(pEventBase, &m_oListenEvent);
        if (-1 == event_add(&m_oListenEvent, 0)) 
        {
            return false;
        }

        event_set(&m_oCloseEvent, -1, EV_PERSIST, TcpSocket::CloseCallback, (void *)(this));
        event_base_set(pEventBase, &m_oCloseEvent);
        if (-1 == event_add(&m_oCloseEvent, 0)) 
        {
            return false;
        }

        return true;
        
        /*m_pListenEvent = event_new(pEventBase, GetSockID(), EV_READ|EV_PERSIST, TcpSocket::AcceptCallback, (void *)(this));
        if (NULL == m_pListenEvent)
        {
            return false;
        }

        m_pCloseEvent = event_new(pEventBase, -1, EV_PERSIST, TcpSocket::CloseCallback, (void *)(this));
        if (NULL == m_pCloseEvent)
        {
            return false;
        }

        // 注册监听事件
        event_add(m_pListenEvent, NULL);

        // 注册断开事件
        event_add(m_pCloseEvent, NULL);

        return true;*/
    }
    
    /**
    * \brief 注册事件
    * \return 注册成功返回true，否则返回false
    */
    bool TcpSocket::RegisterEvent(event_base *pEventBase)
    {   
        assert(NULL != pEventBase);
        assert(NULL != m_pSocketManager);
        
        if (GetState() != Socket::SOCK_STATE_NORMAL && GetState() != Socket::SOCK_STATE_CONNECT)
        {
            return false;
        }

        Lock lock(&m_oLock);
        event_set(&m_oCloseEvent, -1, EV_PERSIST, TcpSocket::CloseCallback, (void *)(this));
        event_base_set(pEventBase, &m_oCloseEvent);
        if (-1 == event_add(&m_oCloseEvent, 0)) 
        {
            return false;
        }
        
        event_set(&m_oReadEvent, GetSockID(), EV_READ|EV_TIMEOUT|EV_PERSIST, TcpSocket::ReadCallback, (void *)(this));
        event_base_set(pEventBase, &m_oReadEvent);

        event_set(&m_oWriteEvent, GetSockID(), EV_WRITE|EV_TIMEOUT|EV_PERSIST, TcpSocket::WriteCallback, (void *)(this));
        event_base_set(pEventBase, &m_oWriteEvent);

        //互通状态,添加读事件
        //连接状态,添加写事件
        if (GetState() == Socket::SOCK_STATE_NORMAL)
        {
            struct timeval tv = {m_pSocketManager->GetTcpTimeOut(), 0};
            if(-1 == event_add(&m_oReadEvent, &tv))
            {
                return false;
            }
        }
        else
        {   
            struct timeval tv1 = {5, 0};
            if(-1 == event_add(&m_oWriteEvent, &tv1))
            {
                return false;
            }
        }

        event_set(&m_oCloseEvent, -1, EV_PERSIST, TcpSocket::CloseCallback, (void *)(this));
        event_base_set(pEventBase, &m_oCloseEvent);
        if (-1 == event_add(&m_oCloseEvent, 0)) 
        {
            return false;
        }
        
        return true;
        
        /*m_pReadEvent = event_new(pEventBase, GetSockID(), EV_READ|EV_TIMEOUT|EV_PERSIST, TcpSocket::ReadCallback, (void *)(this));
        if (NULL == m_pReadEvent)
        {
            return false;
        }
        
        m_pWriteEvent = event_new(pEventBase, GetSockID(), EV_WRITE|EV_TIMEOUT|EV_PERSIST, TcpSocket::WriteCallback, (void *)(this));
        if (NULL == m_pWriteEvent)
        {
            return false;
        }

        m_pCloseEvent = event_new(pEventBase, -1, EV_PERSIST, TcpSocket::CloseCallback, (void *)(this));
        if (NULL == m_pCloseEvent)
        {
            return false;
        }

        if (GetState() == Socket::SOCK_STATE_NORMAL)
        {
            //互通状态,添加读事件
            struct timeval tv = {m_pSocketManager->GetTcpTimeOut(), 0};
            event_add(m_pReadEvent, &tv);
        }
        else
        {   
            //连接状态,添加写事件
            struct timeval tv1 = {5, 0};
            event_add(m_pWriteEvent, &tv1);
        }

        // 注册断开事件
        event_add(m_pCloseEvent, NULL);

        return true;*/
    }
    
    bool TcpSocket::SendPackage()
    {        
        LISTSMG *p = NULL;

    DO_AGAIN:
        p = NULL;
        if(1) {
            //取1个待发送Packet
            Lock lock(&m_oLock);
            if(m_lstSndPacket.empty())
            {
                event_del(&m_oWriteEvent);  //必须在lock状态下del, 会发生死锁吗?
                return true;
            }

            p = m_lstSndPacket.front();
            m_lstSndPacket.pop_front();
        }

        if (NULL == p) 
        {
            //理论不会出现
            LOG(LT_ERROR_TRANS, "", "SEND_MSG| packet is null");
            return false;
        }

        //发送数据
        int32 sndBtyes = Send(p->cPacketBuffer, p->len);
        if (sndBtyes < 0 || sndBtyes > p->len)
        {   
            //错误
            LOG(LT_ERROR_TRANS, p->szTransID, "SEND_MSG| send failed need close| len=%d| fd=%d| rc=%d", p->len, GetSockID(), sndBtyes);

            CloseConnect();
            CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
            return false;
        }
        else if (sndBtyes == 0 )
        {
            // 堵塞了
            Lock lock(&m_oLock);
            LOG(LT_WARN_TRANS, p->szTransID, "SEND_MSG| wait continue| fd=%d| len=%d| size=%lu", GetSockID(), p->len, m_lstSndPacket.size());
            
            if(m_lstSndPacket.size() > 100) 
            {
                CloseConnect();
                CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
                return false;
            }
            
            m_lstSndPacket.push_front(p);
            return true;
        }
        else if(sndBtyes < p->len)
        {
            // 发送了一部分,剩余待发送数据添加到队列头
            memcpy(p->cPacketBuffer, p->cPacketBuffer + sndBtyes, p->len - sndBtyes);
            p->len -= sndBtyes;

            LOG(LT_WARN_TRANS, p->szTransID, "SEND_MSG| wait continue| total=%d| send_len=%d| left_len=%d", p->len + sndBtyes, sndBtyes, p->len);
            Lock lock(&m_oLock);
            m_lstSndPacket.push_front(p);
            p = NULL;
            goto DO_AGAIN;
        }
        else 
        {
            //发送成功
            if(!m_pSocketManager->IsLimitedLog()) 
            {
                LOG(LT_INFO_TRANS, p->szTransID, "SEND_MSG| fd=%d| cmd=%d| errcode=%u| len=%u| unique_id=0x%x| cli_conn_id=0x%llx| role_id=%u",
                    GetSockID(), p->nCmd, p->nErrCode, p->len, m_nUniqueID, p->nCliConnID, p->nRoleID);
            }
            
            CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);    
            p = NULL;
            goto DO_AGAIN;
        }

        //理论不会出现
        LOG(LT_ERROR_TRANS, p->szTransID, "SEND_MSG| invalid done...");
        if(NULL !=  p) {
            CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
        }
        
        return false;
    }

    
    /**
    * \brief 新连接事件(listen的fd会调用这里, this指针指向listen的socket)
    */
    void TcpSocket::OnAccept()
    {
        assert(NULL != m_pSocketManager);
        
        m_pSocketManager->OnAccept(this);
        //if (NULL == pClient)
        //{    
        //    LOG(LT_ERROR, "LISTEN_ACCEPT_NEW_CONN| return null");
        //    return;
        //}
        
        //std::string strAddr;
        //uint32 nPort;
        //m_pSocketManager->GetConnectInfo(pClient->GetSockID(), strAddr, nPort);
        //pClient->m_bIsAcceptFd      = true;
        //pClient->m_nRemoteServerID  = 0;
        
        //LOG(LT_INFO, "LISTEN_ACCEPT_NEW_CONN| fd=%d| remote_addr=%s:%d| old_unique_id=0x%x| old_state=%d", 
        //    pClient->GetSockID(), strAddr.c_str(), nPort, pClient->m_nUniqueID, pClient.GetState()
        //    );
    }

    /**
    * \brief 连接事件
    */
    void TcpSocket::OnConnect()
    {
        assert(NULL != m_pSocketManager);

        m_nRemoteServerID = 0;
        m_bIsAcceptFd = false;
        
        if (IsConnected())
        {
            //获取本地IP,Port
            struct sockaddr_in guest;
            socklen_t guest_len = sizeof(guest);
            getsockname(GetSockID(), (struct sockaddr *)&guest, &guest_len);
            SetLocalAddr(&guest);
            
            // 设置套接字状态
            SetState(Socket::SOCK_STATE_NORMAL);

            //互通状态,添加读事件
            struct timeval tv = {60, 0};
            event_add(&m_oReadEvent, &tv);  //出错 ??


            LOG(LT_INFO_TRANS, NextTransID().c_str(), "Tcp socket connect succ| fd=%d| unique_id=0x%x| local_addr=%s:%d| remote_addr=%s:%d", 
                GetSockID(), m_nUniqueID, GetLocalAddr().c_str(), GetLocalPort(), GetRemoteAddr().c_str(), GetRemotePort()
                );
                
            if (NULL != m_pSocketHandler)
            {
                m_pSocketHandler->OnConnect((int32)(GetSockID()), true, m_nUniqueID, m_bBinaryMode);
            }
        }
        else 
        {   
            LOG(LT_INFO_TRANS, NextTransID().c_str(), "Tcp socket connect failed| fd=%d| unique_id=0x%x", GetSockID(), m_nUniqueID);
            if (NULL != m_pSocketHandler)
            {
                m_pSocketHandler->OnConnect((int32)(GetSockID()), false, m_nUniqueID, m_bBinaryMode);
            }
            
            Release(true);            
        }

        // 通知上层连接结果
        
    }

    void TcpSocket::SetTransID(uint8 nSvrType, uint16 nInstID)
    {
        m_nTidIndex = 1;
        memset(m_szTransID, 0, sizeof(m_szTransID));  
        snprintf(m_szTransID, sizeof(m_szTransID), 
            "%.02x%.03x%.08x%.02x-", nSvrType, nInstID, m_nUniqueID, rand()%0xFF);
        m_szTransID[sizeof(m_szTransID) - 1] = 0;
    }

    std::string TcpSocket::NextTransID()
    {   
        char szTransID[TID_LEN+1] = {0};

        //Lock lock(&m_oLock);
        snprintf(szTransID, TID_LEN, "%s%.04x", m_szTransID, m_nTidIndex++);
        return std::string(szTransID);
    }
    
    /**
    * \brief 可读事件
    */
    void TcpSocket::OnRead()
    {
        bool bAgain = false;
        //取一个节点
        if(NULL == m_pRecvMsg)
        {
            m_pRecvMsg = CQUEUE_List::Instance()->GetNoBlockNode(QUEUE_FREE, &m_nFreeQueueNum);
            if(NULL == m_pRecvMsg) {
                LOG(LT_ERROR_TRANS, m_szTransID, "READ_MSG| not node"); 
                _exit(0);
                return;
            }
            else if(m_nFreeQueueNum < 100) {
                LOG(LT_WARN_TRANS, m_szTransID, "READ_MSG| free queue about to run out| num=%d", m_nFreeQueueNum); 
            }
            
            
            m_pRecvMsg->recv_len = 0;
            if(m_bBinaryMode) 
            {
                m_pRecvMsg->len  = PACKAGE_HEAD_LEN;
            }
        }

        //处理长度
        if(m_bBinaryMode)
        {
            //长度校验
            if(m_pRecvMsg->len > (PACKAGE_LEN_MAX - 200) || m_pRecvMsg->recv_len >= m_pRecvMsg->len)
            {
                LOG(LT_ERROR_TRANS, m_pRecvMsg->szTransID, "READ_MSG| invalid length start| fd=%d| unique_id=0x%x| len=%u| read_len=%u| cmd=%u", 
                    GetSockID(), m_nUniqueID, m_pRecvMsg->len, m_pRecvMsg->recv_len, m_pRecvMsg->nCmd);
                ActiveClose();
                return;
            }

        RECV_AGAIN:
            //接收数据
            int nBytes = Recv(m_pRecvMsg->cPacketBuffer + m_pRecvMsg->recv_len, m_pRecvMsg->len - m_pRecvMsg->recv_len);
            if(nBytes < 0)
            {
                LOG(LT_INFO_TRANS, m_pRecvMsg->szTransID, "READ_MSG| recv failed| fd=%d| unique_id=0x%x| len=%u| read_len=%u| cmd=%u| rc=%d", 
                    GetSockID(), m_nUniqueID, m_pRecvMsg->len, m_pRecvMsg->recv_len, m_pRecvMsg->nCmd, nBytes);
                ActiveClose();
                return;
            }
            else if(0 == nBytes)
            {
                //发送堵塞
                return;
            }

            //重置超时时间
            //struct timeval tv = {60, 0};
            //event_add(m_pReadEvent, &tv);
            
            m_pRecvMsg->recv_len += (unsigned short)nBytes;
            if(m_pRecvMsg->recv_len == m_pRecvMsg->len)
            {
                //该分支, 理论上只会出现以下两种情形 
                //1. m_pRecvMsg->len==oNetHead.m_nLen    表示数据已接收完
                //2. m_pRecvMsg->len==PACKAGE_HEAD_LEN   表示只接收了包括(如果只有包头,那么也是1的情况)
                NetMsgHead oNetHead;
                if(!oNetHead.UnpackFromArray((LPCSTR)m_pRecvMsg->cPacketBuffer, PACKAGE_HEAD_LEN) || oNetHead.m_nLen < PACKAGE_HEAD_LEN || oNetHead.m_nLen > (PACKAGE_LEN_MAX - 200))
                {
                    char szTemp[PACKAGE_HEAD_LEN*4 + 10] = {0};
                    for(int i = 0; i < PACKAGE_HEAD_LEN; i++)
                    {
                        snprintf(szTemp + strlen(szTemp), sizeof(szTemp) - 1 - strlen(szTemp), "%.02x ", (uint8)m_pRecvMsg->cPacketBuffer[i]);
                    }

                    LOG(LT_ERROR_TRANS, oNetHead.m_szTransID, "READ_MSG| invalid message| fd=%d| unique_id=0x%x| len=%u| read_len=%u| cmd=%u| buf=%s", 
                        GetSockID(), m_nUniqueID, m_pRecvMsg->len, m_pRecvMsg->recv_len, oNetHead.m_nCmd, szTemp);
                    ActiveClose();
                    return ;
                }
                
                if(m_pRecvMsg->len == oNetHead.m_nLen)  //对应情形1
                {
                    //已接收了全部数据
                    m_pRecvMsg->len          = oNetHead.m_nLen;
            		m_pRecvMsg->connfd       = GetSockID();
            		m_pRecvMsg->cPacketType  = 0;
            		m_pRecvMsg->nPort        = GetRemotePort();
            		m_pRecvMsg->nCmd         = oNetHead.m_nCmd;
                    m_pRecvMsg->nErrCode     = oNetHead.m_nErrCode;
                    m_pRecvMsg->nUniqueID    = m_nUniqueID;
                    m_pRecvMsg->nCliConnID   = oNetHead.m_nCliConnID;   //服务端间的CliConnID由客户端处传过来,赋值即可
                    m_pRecvMsg->nRemoteServerID = m_nRemoteServerID;
                    m_pRecvMsg->recv_time.Restart();
                    memcpy(m_pRecvMsg->szTransID, oNetHead.m_szTransID, TID_LEN);
            		//memcpy(m_pRecvMsg->cPacketBuffer, (LPCSTR)pContext, m_pRecvMsg->len);
                    snprintf(m_pRecvMsg->sAddr, sizeof(m_pRecvMsg->sAddr) - 1, "%s", GetRemoteAddr().c_str());

            		if(!m_pSocketManager->IsLimitedLog()) 
            		{
                        LOG(LT_INFO_TRANS, m_pRecvMsg->szTransID, "READ_MSG| fd=%d| cmd=%d| errcode=%u| len=%u| pkt_len=%d| num=%d| unique_id=0x%x| cli_conn_id=0x%llx",
                            GetSockID(), oNetHead.m_nCmd, oNetHead.m_nErrCode, oNetHead.m_nLen, m_pRecvMsg->len, m_nFreeQueueNum, m_nUniqueID, m_pRecvMsg->nCliConnID);
                    }

                    //压力测试
                    #if 0
                    if(0x1030001 == m_pSocketManager->GetLocalServerID() && Pb::CMD_UM_LOGIN == m_pRecvMsg->nCmd) 
                    {
                        for(int i = 0; i < 3; i++) 
                        {
                            int32 sndBtyes = Send(m_pRecvMsg->cPacketBuffer, m_pRecvMsg->len);
                            if(sndBtyes != m_pRecvMsg->len) 
                            {
                                LOG(LT_ERROR_TRANS, m_pRecvMsg->szTransID, "DO_MY_TEST| Send fialed| fd=%d| uniqueid=0x%x| send_len=%d| pkt_len=%d", 
                                    m_pRecvMsg->connfd, m_pRecvMsg->nUniqueID,  sndBtyes, m_pRecvMsg->len
                                    );
                                CloseConnect();
                            }

                            LOG(LT_INFO_TRANS, m_pRecvMsg->szTransID, "DO_PRESS_TEST| SEND_MSG| index=%d| fd=%d| cmd=%d| errcode=%u| len=%u| pkt_len=%d| num=%d| unique_id=0x%x| cli_conn_id=0x%llx",
                                i, GetSockID(), oNetHead.m_nCmd, oNetHead.m_nErrCode, oNetHead.m_nLen, m_pRecvMsg->len, m_nFreeQueueNum, m_nUniqueID, m_pRecvMsg->nCliConnID);
                        }


                        LOG(LT_WARN_TRANS, m_pRecvMsg->szTransID, "DO_PRESS_TEST| READ_MSG| fd=%d| cmd=%d| errcode=%u| len=%u| pkt_len=%d| num=%d| unique_id=0x%x| cli_conn_id=0x%llx",
                            GetSockID(), oNetHead.m_nCmd, oNetHead.m_nErrCode, oNetHead.m_nLen, m_pRecvMsg->len, m_nFreeQueueNum, m_nUniqueID, m_pRecvMsg->nCliConnID);
                        
                        //CQUEUE_List::Instance()->SetNode(m_pRecvMsg, QUEUE_FREE);
                        //m_pRecvMsg = NULL;
                        m_pRecvMsg->Reset();
                        m_pRecvMsg->recv_len = 0;
                        if(m_bBinaryMode) 
                        {
                            m_pRecvMsg->len  = PACKAGE_HEAD_LEN;
                        }
                        return;
                    }
                    #endif
                    
                    CQUEUE_List::Instance()->SetNode(m_pRecvMsg, QUEUE_CLIENT_READ);
                    m_pRecvMsg = NULL;
                    
                    return;
                }
                else if(PACKAGE_HEAD_LEN == m_pRecvMsg->len)  //对应情形2
                {
                    //长度赋值
                    m_pRecvMsg->len   = oNetHead.m_nLen;
                    m_pRecvMsg->nCmd = oNetHead.m_nCmd;
                    memcpy(m_pRecvMsg->szTransID, oNetHead.m_szTransID, TID_LEN);

                    //长度校验
                    if(m_pRecvMsg->len > (PACKAGE_LEN_MAX - 100))
                    {
                        LOG(LT_ERROR_TRANS, oNetHead.m_szTransID, "READ_MSG| max length| fd=%d| unique_id=0x%x| len=%u| read_len=%u| cmd=%u", 
                            GetSockID(), m_nUniqueID, m_pRecvMsg->len, m_pRecvMsg->recv_len, oNetHead.m_nCmd);
                        ActiveClose();
                    }

                    //goto继续接收包体
                    if(!bAgain) {
                        bAgain = true;
                        goto RECV_AGAIN;
                    }

                    //只收到包头的日志
                    LOG(LT_INFO_TRANS, oNetHead.m_szTransID, "READ_MSG| recv head| fd=%d| unique_id=0x%x| len=%u| read_len=%u| cmd=%u| num=%d", 
                        GetSockID(), m_nUniqueID, oNetHead.m_nLen, m_pRecvMsg->len, oNetHead.m_nCmd, m_nFreeQueueNum);
                    return;
                }
                else
                {
                    //未完全接收数据(不会出现这种情况)
                    LOG(LT_ERROR_TRANS, oNetHead.m_szTransID, "READ_MSG| invalid length| fd=%d| unique_id=0x%x| len=%u| read_len=%u| net_len=%u| cmd=%u", 
                        GetSockID(), m_nUniqueID, m_pRecvMsg->len, m_pRecvMsg->recv_len, oNetHead.m_nLen, oNetHead.m_nCmd);
                    ActiveClose();
                    return;
                }
            }
            else if(m_pRecvMsg->recv_len < m_pRecvMsg->len)
            {
                //继续接收
                return;
            }
            else
            {   
                LOG(LT_ERROR_TRANS, m_pRecvMsg->szTransID, "READ_MSG| invalid length| fd=%d| unique_id=0x%x| len=%u| read_len=%u| cmd=%u", 
                    GetSockID(), m_nUniqueID, m_pRecvMsg->len, m_pRecvMsg->recv_len, m_pRecvMsg->nCmd);
                ActiveClose();
                return;
            }
        }
        else
        {
            //不处理长度的情形
            //长度校验
            if(m_pRecvMsg->len > (PACKAGE_LEN_MAX - 200))
            {
                LOG(LT_ERROR_TRANS, NextTransID().c_str(), "READ_MSG| disable binary| invalid length start| fd=%d| unique_id=0x%x| read_len=%u", GetSockID(), m_nUniqueID, m_pRecvMsg->recv_len);
                ActiveClose();
                return;
            }
            
            //接收数据
            int nBytes = Recv(m_pRecvMsg->cPacketBuffer + m_pRecvMsg->recv_len, PACKAGE_LEN_MAX - m_pRecvMsg->len - 100);
            if(nBytes < 0)
            {
                LOG(LT_ERROR_TRANS, NextTransID().c_str(), "READ_MSG| fd=%d| unique_id=0x%x| disable binary| recv failed| rc=%d", GetSockID(), m_nUniqueID, nBytes);
                ActiveClose();
                return;
            }
            else if(0 == nBytes)
            {
                //发送堵塞
                return;
            }

            m_pRecvMsg->recv_len += (unsigned short)nBytes;
            m_pRecvMsg->len       =  m_pRecvMsg->recv_len;
            m_pRecvMsg->cPacketBuffer[m_pRecvMsg->len] = 0;

            //找行标记
            FIND_LINE:
            const char *pLineEnd = strstr(m_pRecvMsg->cPacketBuffer, "\n");
            if(NULL == pLineEnd) {
                //没有行结束符, 则继续接收
                LOG(LT_DEBUG_TRANS, NextTransID().c_str(), "READ_MSG| disable binary| not end| fd=%d| unique_id=0x%x| len=%u", GetSockID(), m_nUniqueID, m_pRecvMsg->len);
                return;
            }

            uint16 nLineLen   = pLineEnd - m_pRecvMsg->cPacketBuffer + 1;          
            LISTSMG *pNextMsg = NULL;   

            //不只一行时,缓存剩余的内容
            if(nLineLen < m_pRecvMsg->len)
            {
                pNextMsg = CQUEUE_List::Instance()->GetNoBlockNode(QUEUE_FREE, &m_nFreeQueueNum);
                if(NULL == pNextMsg) {
                    LOG(LT_ERROR_TRANS, m_szTransID, "READ_MSG| disable binary| not node"); 
                    sleep(1);
                    _exit(0);
                    return;
                }
                else if(m_nFreeQueueNum < 100) {
                    LOG(LT_WARN_TRANS, m_szTransID, "READ_MSG| disable binary| free queue about to run out| num=%d", m_nFreeQueueNum); 
                }
                

                //缓存的内容&长度
                pNextMsg->recv_len = m_pRecvMsg->len - nLineLen;
                pNextMsg->len      = pNextMsg->recv_len;
                memcpy(pNextMsg->cPacketBuffer, m_pRecvMsg->cPacketBuffer + nLineLen, pNextMsg->recv_len);
                pNextMsg->cPacketBuffer[pNextMsg->len] = 0;
                LOG(LT_DEBUG_TRANS, m_szTransID, "READ_MSG| disable binary| cache len=%d| buf=%s", pNextMsg->len, pNextMsg->cPacketBuffer); 
                
                //决断当前数据
                m_pRecvMsg->recv_len = nLineLen;
                m_pRecvMsg->len      =  m_pRecvMsg->recv_len;
            }

            m_pRecvMsg->connfd      = GetSockID();
    		m_pRecvMsg->nPort       = GetRemotePort();
    		m_pRecvMsg->nUniqueID   = m_nUniqueID;
    		m_pRecvMsg->nCliConnID  = GetSockID();
    		m_pRecvMsg->nCliConnID  <<= 32; 
    		m_pRecvMsg->nCliConnID  +=  m_nUniqueID;      //算法客户端的CliConnID=(fd<<32 + 唯一ID)
    		m_pRecvMsg->nRemoteServerID = m_nRemoteServerID;
            m_pRecvMsg->recv_time.Restart();
            m_pRecvMsg->cPacketBuffer[m_pRecvMsg->len] = 0;
            
            snprintf(m_pRecvMsg->sAddr, sizeof(m_pRecvMsg->sAddr) - 1, "%s", GetRemoteAddr().c_str());
            
            if(0 == StrUtil::strncmp(m_pRecvMsg->cPacketBuffer, "MATCHSTATE", 10 )){
                snprintf(m_pRecvMsg->szTransID, TID_LEN, "%s", NextTransID().c_str());
                m_pRecvMsg->nCmd = Pb::CMD_BRAIN_ROBOT_ACTION;
            }
            else if(0 == StrUtil::strncmp(m_pRecvMsg->cPacketBuffer, "KeepAlive", 9)) {
                snprintf(m_pRecvMsg->szTransID, TID_LEN, "%s", NextTransID().c_str());
                m_pRecvMsg->nCmd = Pb::CMD_BRAIN_ROBOT_ALIVE;
            }
            else if(0 == StrUtil::strncmp(m_pRecvMsg->cPacketBuffer, "VERSION: ", 9)) {
                snprintf(m_pRecvMsg->szTransID, TID_LEN, "%s", NextTransID().c_str());
                m_pRecvMsg->nCmd = Pb::CMD_BRAIN_ROBOT_LOGIN;
            }
            else {
                string sTransID  = GetValueBykey(m_pRecvMsg->cPacketBuffer, "TransID", "#");
                snprintf(m_pRecvMsg->szTransID, TID_LEN, "%s", sTransID.c_str());
                m_pRecvMsg->nCmd = Pb::CMD_COMM_NOT_BINARY;
            }
            
            
            if(!m_pSocketManager->IsLimitedLog()) 
            {
                LOG(LT_INFO_TRANS, m_pRecvMsg->szTransID, "READ_MSG| disable binary| fd=%d| cmd=%d| len=%u| num=%d| unique_id=0x%x| cli_conn_id=0x%llx| buf=%s",
                    m_pRecvMsg->connfd, m_pRecvMsg->nCmd, m_pRecvMsg->len, m_nFreeQueueNum, m_nUniqueID, m_pRecvMsg->nCliConnID, m_pRecvMsg->cPacketBuffer);
            }
            
            //数据包添加到业务队列                
            CQUEUE_List::Instance()->SetNode(m_pRecvMsg, QUEUE_CLIENT_READ);

            m_pRecvMsg = NULL;
            if(NULL != pNextMsg) {
                m_pRecvMsg = pNextMsg;
                goto FIND_LINE;
            }
        }
    }

    /**
    * \brief 可写事件
    */
    void TcpSocket::OnWrite()
    {
        // win32平台：pPackage被锁定内存，最终会被发送出去，所以这里要从待发送队列中删除掉
        // linux平台：pPackage不会被锁定内存，最终也不会发送出去，所以这里不需要删除
#ifdef WIN32    // win32阻塞的包被锁定内存,已经从列表中删除
        SendPackage();
#else           // linux阻塞的包还在列表中，这里需要强制再发送一次
        SendPackage();
#endif

        // 回调上层处理
        if (NULL != m_pSocketHandler)
        {
            m_pSocketHandler->OnWrite((int32)(GetSockID()));
        }
    }

    /**
    * \brief 错误事件
    * \param nErrno 错误码
    */
    void TcpSocket::OnError(int32 nErrno)
    {
        if (NULL != m_pSocketHandler)
        {
            m_pSocketHandler->OnError((int32)(GetSockID()), nErrno);
        }
    }

    /**
    * \brief 关闭事件
    * \param nReason 关闭原因
    */
    void TcpSocket::OnClose(int32 nReason)
    {
        assert(NULL != m_pSocketManager);
        
        // 正在关闭中
        SOCK_STATE nState = GetState();
        if (nState == SOCK_STATE_CLOSE)
        {
            return;
        }

        // 设置socket正在关闭
        SetState(SOCK_STATE_CLOSE);

        //通知业务线程
        if(!m_bBinaryMode || IsAcceptFd()) 
        {
            int num;
            LISTSMG *p = CQUEUE_List::Instance()->GetNoBlockNode(QUEUE_FREE, &num);
            if(NULL == p) {
                //注: 会导致内存数据服务是否
                LOG(LT_ERROR_TRANS, m_szTransID, "READ_MSG| on-read| not node"); 
            }
            else
            {
                p->connfd       = GetSockID();
        		p->nPort        = GetRemotePort();
        		p->nUniqueID    = m_nUniqueID;
        		p->cPacketType  = PKT_TYPE_DISCARD;
        		p->nRemoteServerID = m_nRemoteServerID;
        		p->recv_time.Restart();
                snprintf(p->sAddr, sizeof(p->sAddr) - 1, "%s", GetRemoteAddr().c_str());                
                if(!m_bBinaryMode)
                {                    
                    //注: 算法客户端的CliConnID=(fd<<32 + 唯一ID)
                    uint64 nConnID = GetSockID();
    		        nConnID  <<= 32; 
    		        nConnID  +=  m_nUniqueID;     

                    //此处m_nRemoteServerID为RoleID
                    p->len = snprintf(p->cPacketBuffer, 64, "Leave RoleID=%u", m_nRemoteServerID);
                    p->cPacketBuffer[p->len] = 0;

                    p->nCliConnID = nConnID;
                    p->nCmd       = Pb::CMD_BRAIN_ROBOT_EXIT;
                    snprintf(p->szTransID, TID_LEN - 1, "%s", NextTransID().c_str());
                    
                    LOG(LT_INFO_TRANS, p->szTransID, "READ_MSG| disable binary| Close| fd=%d| cmd=%d| len=%u| num=%d| unique_id=0x%x| cli_conn_id=0x%llx",
                        p->connfd, p->nCmd, p->len, num, m_nUniqueID, p->nCliConnID);
                }
                else
                {
                    Pb::CommSvrUnregister pbReq;
                    SERVERKEY oSvrKey(m_nRemoteServerID);
                    Pb::ServerID *pbSvrID = pbReq.mutable_svr_id(); 
                    pbSvrID->set_reg_id(oSvrKey.nRegID);
                    pbSvrID->set_type(oSvrKey.nType);
                    pbSvrID->set_inst_id(oSvrKey.nInstID);
                    
                    DoComNetPacket(p, Pb::CMD_COMM_SVR_UNREGISTER, 0, NextTransID().c_str(), 0, &pbReq, 0);
                    LOG(LT_INFO_TRANS, p->szTransID, "READ_MSG| Close| fd=%d| cmd=%d| len=%u| num=%d| unique_id=0x%x| cli_conn_id=0x%llx",
                        p->connfd, p->nCmd, p->len, num, m_nUniqueID, p->nCliConnID);
                }
                
                //数据包添加到业务队列                
                CQUEUE_List::Instance()->SetNode(p, QUEUE_CLIENT_READ);                
            }
        }
        
        if (NULL != m_pSocketHandler)
        {
            m_pSocketHandler->OnClose((int32)(GetSockID()), nReason);
        }

        m_pSocketManager->OnClose(this);
        m_nRemoteServerID = 0;
        
        Release(true);
    }

    /**
    * \brief 监听回调处理
    * \param fd An fd or signal
    * \param events One or more EV_* flags
    * \param arg A user-supplied argument.
    */
    void TcpSocket::AcceptCallback(evutil_socket_t fd, short events, void *arg)
    {
        TcpSocket *pTcpSocket = (TcpSocket *)(arg);
        if (NULL == pTcpSocket)
        {
            return;
        }        

        if ( pTcpSocket->GetSockID() != fd )
        {
            return;
        }

        if (pTcpSocket->GetState() == Socket::SOCK_STATE_LISTEN)
        {
            pTcpSocket->OnAccept();
        }
        else
        {
            std::string strAddr;
            uint16 nPort;
            if(NULL != pTcpSocket->GetSocketManager()) {
                pTcpSocket->GetSocketManager()->GetConnectInfo(pTcpSocket->GetSockID(), strAddr, nPort);
            }
            LOG(LT_ERROR_TRANS, pTcpSocket->m_szTransID, "Accept event call back| state wrong| ip=%s%d| fd=%d", strAddr.c_str(), nPort, pTcpSocket->GetSockID());
            pTcpSocket->OnError(SOCK_ERROR_STATE_WRONG);    // 套接字状态不对
        }
    }

    /**
    * \brief 可读回调处理
    * \param bev the bufferevent that triggered the callback
    * \param ctx the user-specified context for this bufferevent
    */
    void TcpSocket::ReadCallback(evutil_socket_t fd, short events, void *arg)
    {
        TcpSocket *pTcpSocket = (TcpSocket *)(arg);
        if (NULL == pTcpSocket)
        {
            LOG(LT_ERROR_TRANS, pTcpSocket->m_szTransID, "Read event callback| find socket failed| fd=%d", fd);
            return;
        }        

        if ( pTcpSocket->GetSockID() != fd )
        {
            LOG(LT_ERROR_TRANS, pTcpSocket->m_szTransID, "Read event callback| find socket failed| fd=%d| sockid=%d", fd, pTcpSocket->GetSockID());
            pTcpSocket->Close();
            return;
        }


        if (events&EV_TIMEOUT) 
        {
            //可判断连接是否超时进行关闭
            /*if(pTcpSocket->m_bBinaryMode)
            {
                LOG(LT_DEBUG_TRANS, pTcpSocket->m_szTransID, "Read event callback| timeout| fd=%d", pTcpSocket->GetSockID());
            }
            else
            {
                //当前值关闭不处理头长度的情况(即机器人的连接)
                pTcpSocket->CloseConnect();
                LOG(LT_WARN_TRANS, pTcpSocket->NextTransID().c_str(), "Read event callback| timeout close| fd=%d", pTcpSocket->GetSockID());
            }*/

            //超时无数据则断开连接
            pTcpSocket->ActiveClose();
            LOG(LT_WARN_TRANS, pTcpSocket->NextTransID().c_str(), "Read event callback| timeout close| fd=%d", pTcpSocket->GetSockID());
            return ;
        }
        
        if (pTcpSocket->GetState() == Socket::SOCK_STATE_NORMAL)
        {
            pTcpSocket->OnRead();
        }
        else
        {
            std::string strAddr;
            uint16 nPort;
            if(NULL != pTcpSocket->GetSocketManager()) {
                pTcpSocket->GetSocketManager()->GetConnectInfo(pTcpSocket->GetSockID(), strAddr, nPort);
            }
            LOG(LT_ERROR_TRANS, pTcpSocket->m_szTransID, "Read call back| state wrong| ip=%s%d| fd=%d", strAddr.c_str(), nPort, pTcpSocket->GetSockID());
            pTcpSocket->OnError(SOCK_ERROR_STATE_WRONG);    // 套接字状态不对
        }
    }

    /**
    * \brief 可写回调处理
    * \param bev the bufferevent that triggered the callback
    * \param ctx the user-specified context for this bufferevent
    */
    void TcpSocket::WriteCallback(evutil_socket_t fd, short events, void *arg)
    {
        TcpSocket *pTcpSocket = (TcpSocket *)(arg);
        if (NULL == pTcpSocket)
        {
            return;
        }        

        if ( pTcpSocket->GetSockID() != fd )
        {
            return;
        }

        if (events&EV_TIMEOUT) 
        {        
            pTcpSocket->ActiveClose();
            LOG(LT_WARN_TRANS, pTcpSocket->NextTransID().c_str(), "Write event callback| timeout close| fd=%d", pTcpSocket->GetSockID());        
            return ;
        }

        if (pTcpSocket->GetState() == Socket::SOCK_STATE_CONNECT)
        {
            pTcpSocket->OnConnect();
        }
        else if (pTcpSocket->GetState() == Socket::SOCK_STATE_NORMAL)
        {   
            pTcpSocket->OnWrite();
        }
        else
        {
            std::string strAddr;
            uint16 nPort;
            if(NULL != pTcpSocket->GetSocketManager()) {
                pTcpSocket->GetSocketManager()->GetConnectInfo(pTcpSocket->GetSockID(), strAddr, nPort);
            }
            LOG(LT_ERROR_TRANS, pTcpSocket->m_szTransID, "Write call back| state wrong| ip=%s:%d| fd=%d", strAddr.c_str(), nPort, pTcpSocket->GetSockID());
            pTcpSocket->OnError(SOCK_ERROR_STATE_WRONG);    // 套接字状态不对
        }
    }

    /**
    * \brief 断开连接回调处理
    * \param fd An fd or signal
    * \param events One or more EV_* flags
    * \param arg A user-supplied argument.
    */
    void TcpSocket::CloseCallback(evutil_socket_t fd, short events, void *arg)
    {
        TcpSocket *pTcpSocket = (TcpSocket *)(arg);
        if (NULL == pTcpSocket)
        {
            return;
        }        

        LOG(LT_WARN_TRANS, pTcpSocket->m_szTransID, "Close event callback| fd=%d", pTcpSocket->GetSockID());  
        pTcpSocket->OnClose(CLOSE_REASON_USER_CLOSE);
    }
}
