#include "pch.h"
#include "server_connector.h"
#include "net_msg.h"
#include "comm_define.h"
using namespace frame;

CUnitConnector::CUnitConnector()
{   
    m_nTurnSeq   = 0;
    m_nConnCnt   = 0;
    m_pConnector = NULL;
}

CUnitConnector::~CUnitConnector()
{
    //Restore();
}

bool CUnitConnector::Init(uint32 nConnCnt,     //连接数
                IEventReactor *pEventReactor,  //事件Reactor
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

    m_nTurnSeq      = 0;
    m_nConnCnt      = nConnCnt;
    m_oRemoteServer = oRemoteServer;
    if(0 == m_nConnCnt || m_nConnCnt >= MAX_SERVER_REGISTER_CNT) 
    {
        LOG(LT_ERROR, "Unit connector init check connector count failed| cnt=%d", m_nConnCnt);
        return true;
    }
    
    m_pConnector    = new CConnector[m_nConnCnt];
    if(NULL == m_pConnector) 
    {
        LOG(LT_ERROR, "Unit connector init not memory");
        return false;
    }

    for(uint32 i = 0; i < m_nConnCnt; i++)
    {
        string strSymbol = strLogSymbol + "_" + to_string(i+1);
        if(!m_pConnector[i].Init(pEventReactor, oRemoteServer, oLocalServer, nRegisterCmd, nAliveCmd,
            nAliveTimerID, nAliveInterval, nReconnTimerID, nReconnInterval, nConnTimeout, strSymbol,
            bBinaryMode))
        {
            LOG(LT_ERROR, "Unit connector init connector failed| index=%d", i+1);
            return false;
        }
    }
    
    return true;
}

void CUnitConnector::Restore()
{
    if(NULL != m_pConnector)
    {
        for(uint32 i = 0; i < m_nConnCnt; i++) 
        {
            m_pConnector[i].Restore();
        }
        
        delete []m_pConnector;
        m_pConnector = NULL;
    }

    m_nConnCnt = 0;
}

bool CUnitConnector::SetAliveTime(uint32 nUniqueID)
{
    for(uint32 i = 0; i < m_nConnCnt; i++)
    {
        if(m_pConnector[i].GetUniqueID() == nUniqueID)
        {
            m_pConnector[i].SetAliveTime();
            return true;
        }
    }

    return false;
}

void CUnitConnector::DoRequest(LISTSMG * p, uint32 cmd, string &strTransID, uint64 nCliConnID, const google::protobuf::Message *pbMsg, uint32 nRoleID)
{
    uint32 nTempSeq = m_nTurnSeq++;
    for(uint32 i = 0; i < 3 && i < m_nConnCnt; i++)
    {
        uint32 index = (nTempSeq+i)%m_nConnCnt;
        if(m_pConnector[index].GetConnectID() > 0)
        {
            m_pConnector[index].DoRequest(p, cmd, strTransID, nCliConnID, pbMsg, nRoleID);
            return;
        }
    }

    LOG(LT_ERROR_TRANS, strTransID.c_str(), "Unit connector| do request failed| server_id=0x%x| cmd=%d", m_oRemoteServer.nServerID, cmd);
    CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
}

void CUnitConnector::DoRequest(LISTSMG * p, string &strTransID, uint64 nCliConnID, const char *szBuf, uint32 nRoleID)
{
    uint32 nTempSeq = m_nTurnSeq++;
    for(uint32 i = 0; i < 3 && i < m_nConnCnt; i++)
    {
        uint32 index = (nTempSeq+i)%m_nConnCnt;
        if(m_pConnector[index].GetConnectID() > 0)
        {
            m_pConnector[index].DoRequest(p, strTransID, nCliConnID, szBuf, nRoleID);
            return;
        }
    }

    LOG(LT_ERROR_TRANS, strTransID.c_str(), "Unit connector| do request failed| server_id=0x%x", m_oRemoteServer.nServerID);
    CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
}

bool CUnitConnector::IsConnected()
{
    uint32 nTempSeq = m_nTurnSeq;
    for(uint32 i = 0; i < 2 && i < m_nConnCnt; i++)
    {
        uint32 index = (nTempSeq+i)%m_nConnCnt;
        if(m_pConnector[index].GetConnectID() > 0)
        {
            return true;
        }
    }

    return false;
}


CServerConnector::CServerConnector()
{
    m_nUnitSize      = 0;
    m_pUnitConnector = NULL;
}

CServerConnector::~CServerConnector()
{

}


void CServerConnector::Restore()
{
    if(NULL != m_pUnitConnector)
    {
        for(uint32 i = 0; i < m_nUnitSize; i++) 
        {
            m_pUnitConnector[i].Restore();
        }
        delete []m_pUnitConnector;
        m_pUnitConnector = NULL;
    }

    m_nUnitSize      = 0;
}


bool CServerConnector::Init(uint32 nConnCnt,  const vector<DefServer> &vRemoteServer, const DefServer &oLocalServer,
                IEventReactor *pEventReactor,  //事件Reactor
                const uint32 &nRegisterCmd,    //注册命令
                const uint32 &nAliveCmd,       //心跳命令
                const uint64 &nAliveTimerID,   //心跳定时器ID
                const uint32 &nAliveInterval,  //心跳间隔
                const uint64 &nReconnTimerID,  //重连定时器ID
                const uint32 &nReconnInterval, //重连间隔
                const uint32 &nConnTimeout,    //重连超时
                const string &strLogSymbol,    //日志标记
                bool bBinaryMode               //二进制模式
          )
{
    m_nUnitSize = vRemoteServer.size();
    if(0 == m_nUnitSize || m_nUnitSize >= MAX_SERVER_REGISTER_CNT) {
        LOG(LT_ERROR, "Server connector init check size failed| size=%d", m_nUnitSize);
        return false;
    }

    m_pUnitConnector = new CUnitConnector[m_nUnitSize];
    if(NULL == m_pUnitConnector)
    {
        LOG(LT_ERROR, "Server connector init not memory");
        return false;
    }
    
    for(uint32 i = 0; i < m_nUnitSize; i++) 
    {
        if(!m_pUnitConnector[i].Init(nConnCnt, pEventReactor, vRemoteServer[i], oLocalServer, nRegisterCmd, nAliveCmd, 
            nAliveTimerID, nAliveInterval, nReconnTimerID, nReconnInterval, nConnTimeout, strLogSymbol, bBinaryMode)
            )
        {
            LOG(LT_ERROR, "Server connector init UnitConnector failed| index=%d", i);
            return false;
        }
    }

    //校验ServerID是连续的
    for(uint32 i = 0; i < m_nUnitSize; i++) 
    {
        uint32 index = m_pUnitConnector[i].GetRemoteServerID();
        index = (index&0xFF) - 1; 
        if(i != index)
        {
            LOG(LT_ERROR, "connector set invalid server_id | i=%d| server_id=0x%x| index=%d", i, m_pUnitConnector[i].GetRemoteServerID(), index);
            return false;
        }
    } 
    
    return true;
}

//根据server_id查询连接器对象
CUnitConnector* CServerConnector::GetUnitConnectorByServerID(const uint32 &nRemoteServerID)
{   
    uint32 index = (nRemoteServerID&0xFF) - 1;    
    if(0 == nRemoteServerID || index >= m_nUnitSize) 
    {
        return NULL;
    }

    return &m_pUnitConnector[index];
}

CUnitConnector* CServerConnector::GetUnitConnectorByIndex(const uint32 &nIndex)
{
    if(nIndex >= m_nUnitSize) 
    {
        return NULL;
    }

    return &m_pUnitConnector[nIndex];
}

bool CServerConnector::SetAliveTime(const char *szTransID, const uint32 &nRemoteServerID, uint32 nUniqueID)
{
    CUnitConnector *pUnit = GetUnitConnectorByServerID(nRemoteServerID);
    if(NULL == pUnit) 
    {
        LOG(LT_ERROR_TRANS, szTransID, "Server connector Get unit failed");
        return false;
    }

    return pUnit->SetAliveTime(nUniqueID);
}


void CServerConnector::DoRequest(const uint32 &nRemoteServerID, LISTSMG *p, uint32 cmd, string &strTransID, uint64 nCliConnID, const google::protobuf::Message *pbMsg, uint32 nRoleID)
{

    CUnitConnector *pUnit = GetUnitConnectorByServerID(nRemoteServerID);
    if(NULL == pUnit) 
    {
        LOG(LT_ERROR_TRANS, strTransID.c_str(), "Server connector do request Get connector failed");
        CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
        return;
    }

    pUnit->DoRequest(p, cmd, strTransID, nCliConnID, pbMsg, nRoleID);
    
    return;
}

void CServerConnector::DoRequest(const uint32 &nRemoteServerID, LISTSMG * p, string &strTransID, uint64 nCliConnID, const char *szBuf, uint32 nRoleID)
{
    CUnitConnector *pUnit = GetUnitConnectorByServerID(nRemoteServerID);
    if(NULL == pUnit) 
    {
        LOG(LT_ERROR_TRANS, strTransID.c_str(), "Server connector do request Get connector failed");
        CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
        return;
    }

    pUnit->DoRequest(p, strTransID, nCliConnID, szBuf, nRoleID);
    return;
}


void CServerConnector::RandRequest(LISTSMG *p, uint32 cmd, string &strTransID, uint64 nCliConnID, const google::protobuf::Message *pbMsg, uint32 nRoleID)
{
    uint32 nRand = rand()%m_nUnitSize;
    CUnitConnector *pUnit = GetUnitConnectorByIndex(nRand);
    if(NULL == pUnit) 
    {
        LOG(LT_ERROR_TRANS, strTransID.c_str(), "Server connector do request Get connector failed| rand=%d", nRand);
        CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
        return;
    }

    pUnit->DoRequest(p, cmd, strTransID, nCliConnID, pbMsg, nRoleID);
    
    return;
}

void CServerConnector::RandRequest(LISTSMG * p, string &strTransID, uint64 nCliConnID, const char *szBuf, uint32 nRoleID)
{
    uint32 nRand = rand()%m_nUnitSize;
    CUnitConnector *pUnit = GetUnitConnectorByIndex(nRand);
    if(NULL == pUnit) 
    {
        LOG(LT_ERROR_TRANS, strTransID.c_str(), "Server connector do request Get connector failed| rand=%d", nRand);
        CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
        return;
    }

    pUnit->DoRequest(p, strTransID, nCliConnID, szBuf, nRoleID);
    
    return;
}


void CServerConnector::DoReport(const char *szLogSymbol, string &strTransID, uint32 cmd, const google::protobuf::Message *pbMsg)
{
    int num = 0;
    for(uint32 i = 0; i < m_nUnitSize; i++)
    {
        LISTSMG *p = CQUEUE_List::Instance()->GetNoBlockNode(QUEUE_FREE, &num);
        if(NULL == p) 
        {
            LOG(LT_ERROR_TRANS, strTransID.c_str(), "%s| Get node failed");
            return;
        }

        m_pUnitConnector[i].DoRequest(p, cmd, strTransID, 0, pbMsg, 0);
    }
}

bool CServerConnector::IsConnected(const uint32 &nRemoteServerID)
{
    CUnitConnector *pUnit = GetUnitConnectorByServerID(nRemoteServerID);
    if(NULL == pUnit) 
    {
       return false;
    }

    return pUnit->IsConnected();
}

bool CServerConnector::IsAllConnected()
{
    if(0 == m_nUnitSize) {
        return false;
    }
    
    for(uint32 i = 0; i < m_nUnitSize; i++)
    {
        CUnitConnector *pUnit = GetUnitConnectorByIndex(i);
        if(NULL == pUnit) {
            return false;
        }

        if(!pUnit->IsConnected()) {
            return false;
        }
    }

    return true;
}

