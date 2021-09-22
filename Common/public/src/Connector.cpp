#include "pch.h"
#include "Connector.h"
#include "net_msg.h"
#include "comm_define.h"
#include "socket_manager.h"
using namespace frame;
using namespace network;

CConnector::CConnector(void)
{
    m_nConnectID   = -1;
    m_nUniqueID    = 0;
    m_nAliveTime   = time(NULL);
    memset(m_szLogSymbol, 0, sizeof(m_szLogSymbol)); 
        
    m_pEventReactor  = NULL;
}


CConnector::~CConnector(void)
{
}


bool CConnector::Init( IEventReactor *pEventReactor, 
                        const DefServer &oRemoteServer,//远程服务Server对象
                        const DefServer &oLocalServer, //本模块Server对象
                        const uint32 &nRegisterCmd,    //注册命令
                        const uint32 &nAliveCmd,       //心跳命令
                        const uint64 &nAliveTimerID,   //心跳定时器ID
                        const uint32 &nAliveInterval,  //心跳间隔
                        const uint64 &nReconnTimerID,  //重连定时器ID
                        const uint32 &nReconnInterval, //重连间隔
                        const uint32 &nConnTimeout,    //重连超时
                        const string &strLogSymbol,    //日志标记
                        bool bBinaryMode               //二级制模式
                        )
{
    assert(NULL != pEventReactor);

    if(0 == oRemoteServer.nServerID || 0 == oLocalServer.nServerID) {
        LOG(LT_ERROR, "Init client handler| invalid server_id");
        return false;
    }

    m_oRemoteServer  = oRemoteServer;
    m_oLocalServer   = oLocalServer;
    snprintf(m_szLogSymbol, sizeof(m_szLogSymbol) - 1, "%s", strLogSymbol.c_str());
        
    m_nRegisterCmd   = nRegisterCmd;  
    m_nAliveCmd      = nAliveCmd;     
    m_nAliveTimerID  = nAliveTimerID; 
    m_nAliveInterval = nAliveInterval;
    m_nReconnTimerID = nReconnTimerID; 
    m_nReconnInterval= nReconnInterval;
    m_nConnTimeout   = nConnTimeout;
    
    m_pEventReactor  = pEventReactor; 
    m_bBinaryMode    = bBinaryMode;
    
    LOG(LT_INFO, "%s Init| server=%s:%d(0x%x)| local_server_id=0x%x| register_cmd=%u| alive_cmd=%u| alive_timer_id=%u| reconn_timer_id=%u| alive_interval=%d| reconn_interval=%d| conn_timeout=%u| binary_mode=%d", 
        m_szLogSymbol, m_oRemoteServer.ip, m_oRemoteServer.nPort, m_oRemoteServer.nServerID, m_oLocalServer.nServerID, m_nRegisterCmd, m_nAliveCmd, m_nAliveTimerID, m_nReconnTimerID, m_nAliveInterval, m_nReconnInterval, m_nConnTimeout, m_bBinaryMode
        );

    // 连接服务器
    if (!CSocketManager::Instance()->Connect(m_oRemoteServer.ip, m_oRemoteServer.nPort, this, bBinaryMode))
    {
        LOG(LT_ERROR, "%s connect failed| server_addr=%s:%d(0x%x)", m_szLogSymbol,m_oRemoteServer.ip, m_oRemoteServer.nPort, m_oRemoteServer.nServerID);
        return false;
    }
    
    return true;
}

void CConnector::Restore()
{
    if(m_nConnectID > 0)
    {
        CSocketManager::Instance()->CloseConnect(m_nConnectID); 
    }
}


string CConnector::GetNextTransID()
{
    string sTid = CSocketManager::Instance()->GetNextTransID(GetConnectID());
    if(sTid.length() > 5) {
        m_strLastTransID = sTid;
    }
   
    return m_strLastTransID;
}


int CConnector::OnConnect(int32 nRemoteFd, bool bSuccess, uint32 nUniqueID, bool bBinaryMode)
{   
    if (bSuccess)
    {
        if(m_bBinaryMode != bBinaryMode)
        {
            LOG(LT_ERROR, "%s| on connect| server_addr=%s:%d(0x%x)| fd=%d| binary_mode=%d", 
                m_szLogSymbol, m_oRemoteServer.ip, m_oRemoteServer.nPort, m_oRemoteServer.nServerID, nRemoteFd, m_bBinaryMode
                );
            sleep(5);
            _exit(0);
        }
        
        m_nAliveTime = time(NULL);
        m_nConnectID = nRemoteFd;
        m_nUniqueID  = nUniqueID;
        
        
        string strAddr;
        SERVERKEY sk(m_oLocalServer.nServerID);
        CSocketManager::Instance()->SetRemoteServerID(m_nConnectID, m_oRemoteServer.nServerID);
        CSocketManager::Instance()->GetConnectInfo(m_nConnectID, strAddr, m_nLocalPort);
        snprintf(m_szLocalAddr, sizeof(m_szLocalAddr) - 1, "%s", strAddr.c_str());
        string strTransID = GetNextTransID();
        
        //非二级制协议模式
        if(!m_bBinaryMode) 
        {
            m_pEventReactor->DetachTimer(this, m_nReconnTimerID);
            m_pEventReactor->AttachTimer(this, m_nAliveTimerID, m_nAliveInterval, -1, "Connector::KeepAlive");
            LOG(LT_INFO_TRANS, strTransID.c_str(), "%s| on connect succ| fd=%d| unique_id=0x%x| binary_mode=%d| server=%s:%d(0x%x)| client=%s:%d",
                m_szLogSymbol, m_nConnectID, m_nUniqueID, m_bBinaryMode, m_oRemoteServer.ip, m_oRemoteServer.nPort, m_oRemoteServer.nServerID, m_szLocalAddr, m_nLocalPort);
            
            return 0;
        }
        
        //取一空闲包
        int num = 0;
        LISTSMG *p = CQUEUE_List::Instance()->GetNoBlockNode(QUEUE_FREE, &num);
        if(NULL == p) {
            LOG(LT_INFO_TRANS, strTransID.c_str(), "%s| on connect Get node failed| fd=%d| unique_id=0x%x| server=%s:%d(0x%x)| client=%s:%d",
                m_szLogSymbol, m_nConnectID, m_nUniqueID, m_oRemoteServer.ip, m_oRemoteServer.nPort, m_oRemoteServer.nServerID, m_szLocalAddr, m_nLocalPort);
            
            CSocketManager::Instance()->CloseConnect(m_nConnectID);
            return 0;
        }
        
        p->Reset();
        p->connfd       = nRemoteFd;
		p->cPacketType  = 0;
        p->nUniqueID    = m_nUniqueID;

        //注册/注销 定时器
        m_pEventReactor->DetachTimer(this, m_nReconnTimerID);
        m_pEventReactor->AttachTimer(this, m_nAliveTimerID, m_nAliveInterval, -1, "Connector::KeepAlive");

        Pb::CommSvrRegister pbReq; 
        Pb::ServerID *pbSvrID = pbReq.mutable_svr_id();
        pbReq.set_s_server_id(m_oRemoteServer.nServerID);
        pbReq.set_c_server_id(m_oLocalServer.nServerID);
        pbReq.set_addr(m_szLocalAddr);
        pbReq.set_port(m_nLocalPort);
        pbReq.set_server_name(m_szLogSymbol);
        pbSvrID->set_type(sk.nType);
        pbSvrID->set_reg_id(sk.nRegID);
        pbSvrID->set_inst_id(sk.nInstID);
        pbReq.set_addr(m_oLocalServer.ip);
        pbReq.set_port(m_oLocalServer.nPort);
        
        DoComNetPacket(p, m_nRegisterCmd, 0, strTransID.c_str(), 0, &pbReq);
        CSocketManager::Instance()->Send(p);     

        LOG(LT_INFO_TRANS, strTransID.c_str(), "%s| on connect succ| fd=%d| unique_id=0x%x| binary_mode=%d| server=%s:%d(0x%x)| client=%s:%d",
            m_szLogSymbol, m_nConnectID, m_nUniqueID, m_bBinaryMode, m_oRemoteServer.ip, m_oRemoteServer.nPort, m_oRemoteServer.nServerID, m_szLocalAddr, m_nLocalPort);

        LOG(LT_INFO_TRANS, strTransID.c_str(), "%s| register request|s_server_id=0x%x| c_server_id=0x%x| client=%s:%d, name=%s",
            m_szLogSymbol, pbReq.s_server_id(), pbReq.c_server_id(), pbReq.addr().c_str(), pbReq.port(), pbReq.server_name().c_str()
            );
    }
    // 失败，做定时重连
    else
    {
        m_pEventReactor->AttachTimer(this, m_nReconnTimerID, m_nReconnInterval, -1, "Connector::OnConnect");
        LOG(LT_WARN, "%s| on connect failed| server_addr=%s:%d(0x%x)| fd=%d", 
            m_szLogSymbol, m_oRemoteServer.ip, m_oRemoteServer.nPort, m_oRemoteServer.nServerID, nRemoteFd);
    }
    
    return 0;
}


void CConnector::OnClose(int32 nRemoteFd, int32 nReason)
{
    string sTransID = GetNextTransID();

    if(m_nConnectID > 0 && nRemoteFd != m_nConnectID)
    {
        // 连接ID不匹配的情况
        // 暂时不处理
        LOG(LT_ERROR_TRANS, sTransID.c_str(), "%s| on close socket different| recv_fd=%d| fd=%d", m_szLogSymbol, nRemoteFd, m_nConnectID);
    }

    LOG(LT_INFO_TRANS, sTransID.c_str(), "%s| on close| fd=%d", m_szLogSymbol, m_nConnectID);

    m_nConnectID = -1;
    m_nUniqueID  = 0;
    m_nLocalPort      = 0;
    memset(m_szLocalAddr, 0, sizeof(m_szLocalAddr));
  
    
    m_pEventReactor->DetachTimer(this, m_nAliveTimerID);
    m_pEventReactor->AttachTimer(this, m_nReconnTimerID, m_nReconnInterval, -1, "Connector::OnClose");

}

void CConnector::OnTimer(uint64 nTimerID)
{
    if(nTimerID == m_nReconnTimerID) 
    {
        //重连
        DoReConnect();
    }
    else if(nTimerID == m_nAliveTimerID)
    {
        //心跳
        DoKeepAlive();
    }
    else 
    {
    }
}

//定时心跳
void CConnector::DoKeepAlive()
{
    string sTransID = GetNextTransID();
    
    //超时无心跳, 则关闭连接
    if((time(NULL) - m_nAliveTime) > m_nConnTimeout)
    {
        CSocketManager::Instance()->CloseConnect(m_nConnectID);
        LOG(LT_INFO_TRANS, sTransID.c_str(), "%s| keep alive timeout close| fd=%d| client=%s:%d", 
            m_szLogSymbol, m_nConnectID, m_szLocalAddr, m_nLocalPort);
    }
    else
    {
        //定时发起心跳包
        int num = 0;
        LISTSMG *p = CQUEUE_List::Instance()->GetNoBlockNode(QUEUE_FREE, &num);
        if(NULL == p) 
        {
            LOG(LT_ERROR_TRANS, sTransID.c_str(), "%s| keep alive Get node failed| fd=%d| client=%s:%d",
                m_szLogSymbol, m_nConnectID, m_szLocalAddr, m_nLocalPort);
            return;
        }

        p->connfd       = m_nConnectID;
		p->nUniqueID    = m_nUniqueID;
        if(m_bBinaryMode)
        {
            Pb::CommKeepAlive pbReq;   
            char szMsg[256] = {0};
            snprintf(szMsg, sizeof(szMsg) - 1, "%s_alive", m_szLogSymbol);
            pbReq.set_msg(szMsg);
            DoComNetPacket(p, m_nAliveCmd, 0, sTransID.c_str(), 0, &pbReq);
            CSocketManager::Instance()->Send(p);
            
            LOG(LT_INFO_TRANS, sTransID.c_str(), "%s| keep alive request1| fd=%d| client=%s:%d", 
                m_szLogSymbol, m_nConnectID, m_szLocalAddr, m_nLocalPort);
        }
        else 
        {
            char szBuf[256] = {0};
            snprintf(szBuf, sizeof(szBuf) - 1, "KEEP_ALIVE#%s#TransID=%s#\r\n", m_szLogSymbol, sTransID.c_str());
            DoComNetPacket(p, sTransID.c_str(), p->connfd, p->nUniqueID, 0, szBuf, 0);
            CSocketManager::Instance()->Send(p);
            LOG(LT_INFO_TRANS, sTransID.c_str(), "%s| keep alive request2| fd=%d| client=%s:%d", 
                m_szLogSymbol, m_nConnectID, m_szLocalAddr, m_nLocalPort
                );
        }
    }
}


//定时重连
void CConnector::DoReConnect()
{
    // 连接服务器
    if (!CSocketManager::Instance()->Connect(m_oRemoteServer.ip, m_oRemoteServer.nPort, this, m_bBinaryMode))
    {
        LOG(LT_ERROR, "%s| reconnect failed| server_addr=%s:%d(0x%x)", m_szLogSymbol, m_oRemoteServer.ip, m_oRemoteServer.nPort, m_oRemoteServer.nServerID);
        return;
    }

    m_strLastTransID = "";
    LOG(LT_INFO, "%s| reconnect again| server_addr=%s:%d(0x%x)", m_szLogSymbol, m_oRemoteServer.ip, m_oRemoteServer.nPort, m_oRemoteServer.nServerID);            
}

/*功能: 组包并发送至服务端
* @nCliConnID  : 客户端的连接ID[透传]
* @szTransID   : 任务ID(不传则取该对象的任务ID)
*/
void CConnector::DoRequest(LISTSMG * p, uint32 cmd, string &strTransID, uint64 nCliConnID, const google::protobuf::Message *pbMsg, uint32 nRoleID)
{
    if(!m_bBinaryMode) 
    {
        LOG(LT_ERROR_TRANS, strTransID.c_str(), "%s | Do_Request| protocal mode invalid");
        CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
        return;
    }
    
    if(0 == strTransID.length())  {
        strTransID = GetNextTransID();
    }
    
    p->Reset();
    p->connfd       = m_nConnectID;
	p->cPacketType  = 0;
    p->nUniqueID    = m_nUniqueID;
    p->recv_time.Restart();
    DoComNetPacket(p, cmd, 0, strTransID.c_str(), nCliConnID, pbMsg, nRoleID);
    CSocketManager::Instance()->Send(p);
}


void CConnector::DoRequest(LISTSMG * p, string &strTransID, uint64 nCliConnID, const char *szBuf, uint32 nRoleID)
{
    if(m_bBinaryMode) 
    {
        LOG(LT_ERROR_TRANS, strTransID.c_str(), "%s | Do_Request| protocal mode invalid");
        CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
        return;
    }
    
    p->Reset();
    p->cPacketType  = 0;
    p->recv_time.Restart();

    DoComNetPacket(p, strTransID.c_str(), m_nConnectID, m_nUniqueID, p->nCliConnID, szBuf, nRoleID);
    CSocketManager::Instance()->Send(p);
}


