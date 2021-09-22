/**
* \file client_manager.cpp
* \brief 
*/

#include "pch.h"
#include "register_server_manager.h"
#include "shstd.h"
#include "socket_manager.h"

using namespace network;

RegisterServer::RegisterServer():m_nSeqNo(0), m_nListenPort(0),      m_nResCnt(0) 
{
    memset(m_szRemoteAddr, 0, sizeof(m_szRemoteAddr));
    memset(m_szServerName, 0, sizeof(m_szServerName));

    //m_oConnInfo 有构造
}


StorageServer::StorageServer():m_nResCnt(0), m_nConnCnt(0) 
{
    memset(m_szRemoteAddr, 0, sizeof(m_szRemoteAddr));
    memset(m_szServerName, 0, sizeof(m_szServerName));

    //m_lstConnInfo 有构造
}

bool StorageServer::AddConn(const char *szTransID, const ConnInfo &oConn)
{
    for(uint32 i = 0; i < m_nConnCnt; i++)
    {   
        //sock已存在, 覆盖
        if(oConn.m_nSock == m_lstConnInfo[i].m_nSock) 
        {
            LOG(LT_WARN_TRANS, szTransID,  
                "Add conn cover| server_id=0x%x| addr=%s| old=(fd:%d, port:%d, id:0x%x)| new=(fd:%d, port:%d, id:0x%x)",
                m_nServerID,  m_szRemoteAddr, m_lstConnInfo[i].m_nSock, m_lstConnInfo[i].m_nPort, m_lstConnInfo[i].m_nUniqueID, oConn.m_nSock, oConn.m_nPort, oConn.m_nUniqueID
                );
            
            m_lstConnInfo[i] = oConn;
            return true;
        }
    }

    if(m_nConnCnt > (MAX_SERVER_REGISTER_CNT - 3)) //最多支持 [MAX_SERVER_REGISTER_CNT - 3]
    {
        LOG(LT_WARN_TRANS, szTransID, "Add conn conut limited| conn_cnt=%d", m_nConnCnt);
        return false;
    }

    m_lstConnInfo[m_nConnCnt] = oConn;
    ++m_nConnCnt;

    LOG(LT_INFO_TRANS, szTransID,  
        "Add conn last| server_id=0x%x| addr=%s| conn=(fd:%d, port:%d, id:0x%x)| conn_cnt=%d",
        m_nServerID,  m_szRemoteAddr, oConn.m_nSock, oConn.m_nPort, oConn.m_nUniqueID, m_nConnCnt
        );
                
    return true;
}

bool StorageServer::DelConn(const char *szTransID, int32 nSock, uint32 nUniqueID)
{
    bool bDelSucc = false;
    for(uint32 i = 0; i < m_nConnCnt; i++)
    {   
        if(nSock == m_lstConnInfo[i].m_nSock) 
        {
            if(nUniqueID != m_lstConnInfo[i].m_nUniqueID)
            {
                LOG(LT_WARN_TRANS, szTransID,  "Del conn invalid id| server_id=0x%x| addr=%s| fd=%d| id=0x%x(0x%x)",
                    m_nServerID,  m_szRemoteAddr, nSock, nUniqueID, m_lstConnInfo[i].m_nUniqueID
                    );
                break;
            }
            
            bDelSucc = true;
            m_lstConnInfo[i].Reset();
            --m_nConnCnt;

            //不是最后一个,用最后一个替换, 保证前m_nConnCnt为有效的链接(即0 - [m_nConnCnt - 1] 为有效链接)
            if(i != m_nConnCnt) 
            {   
                m_lstConnInfo[i] = m_lstConnInfo[m_nConnCnt];
                m_lstConnInfo[m_nConnCnt].Reset();

                LOG(LT_INFO_TRANS, szTransID,  "Del swap last| server_id=0x%x| addr=%s| last=%d| to=%d| conn_cnt=%d",
                    m_nServerID,  m_szRemoteAddr, m_nConnCnt, i, m_nConnCnt
                    );
            }
        }
    }

    return bDelSucc;
}

bool StorageServer::ToRegisterServer(const char *szTransID, RegisterServer &oServer)
{
    if(0 == m_nConnCnt || m_nConnCnt > (MAX_SERVER_REGISTER_CNT - 1)) 
    {
        //LOG(LT_ERROR_TRANS, szTransID,  
        //    "To register server| server_id=0x%x| addr=%s| name=%s| conn_cnt=%d",
        //    m_nServerID,  m_szRemoteAddr, m_szServerName, m_nConnCnt
        //    );
        return false;
    }

    uint32 index = (m_nTurnSeq++ % m_nConnCnt);
    
    //oServer.m_nSeqNo        = oIServer.;     
    oServer.m_nListenPort   = m_nListenPort;
    oServer.m_nResCnt       = m_nResCnt;   
    oServer.m_oConnInfo     = m_lstConnInfo[index];
    snprintf(oServer.m_szRemoteAddr, sizeof(oServer.m_szRemoteAddr), "%s", m_szRemoteAddr);
    snprintf(oServer.m_szServerName, sizeof(oServer.m_szServerName), "%s", m_szServerName);
            
    return true;
}


bool StorageServer::GetOneConn(const char *szTransID, ConnInfo &oConn, uint32 idx)
{
    if(0 == m_nConnCnt || m_nConnCnt > (MAX_SERVER_REGISTER_CNT - 1)) 
    {
        //LOG(LT_ERROR_TRANS, szTransID,  
        //    "Get one conninfo| server_id=0x%x| addr=%s| name=%s| conn_cnt=%d",
        //    m_nServerID,  m_szRemoteAddr, m_szServerName, m_nConnCnt
        //    );
        return false;
    }

    uint32 index = 0;
    if(idx >= m_nConnCnt) 
    {
        index = (m_nTurnSeq++ % m_nConnCnt);
    }
    else 
    {
        index = (idx % m_nConnCnt);
    }
    
    oConn = m_lstConnInfo[index];
            
    return true;
}

//分配连接索引
uint32 StorageServer::AllocConnIdx(const char *szTransID, int32 nSock, uint32 nOldIdx, const string &sLogMsg)
{
    if(0 == m_nConnCnt) 
    {
        return (MAX_SERVER_REGISTER_CNT + 1);
    }
    
    //索引不变
    if(nOldIdx < m_nConnCnt && nSock == m_lstConnInfo[nOldIdx].m_nSock) 
    {   
        return nOldIdx;
    }

    //找指定socket的下标
    for(uint32 i = 0; i < m_nConnCnt; i++)
    {   
        if(nSock == m_lstConnInfo[i].m_nSock) 
        {
            LOG(LT_INFO_TRANS, szTransID, "Alloc connect idx| fd=%d| idx=%u| old_idx=%u| unique_id=0x%x| %s", 
                nSock , i, nOldIdx, m_lstConnInfo[i].m_nUniqueID, sLogMsg.c_str()
                );
            return i;
        }
    }

    LOG(LT_WARN_TRANS, szTransID, "Alloc connect idx failed| fd=%d| old_idx=%u| %s", nSock , nOldIdx, sLogMsg.c_str());

    //随机分配
    return m_nTurnSeq++ % m_nConnCnt;    
}


CRegisterServerManager::CRegisterServerManager(void)
{
}

CRegisterServerManager::~CRegisterServerManager(void)
{
    //关闭所有连接...
}

bool CRegisterServerManager::Init()
{
    return true;
}


bool CRegisterServerManager::Register(const char *szTransID,  uint32 nServerID, const RegisterServer &oServer)
{
    std::string sMsg;
    frame::Lock lock(&m_lock);
    if(!CSocketManager::Instance()->IsValidConnected(oServer.m_oConnInfo.m_nSock, oServer.m_oConnInfo.m_nUniqueID)) 
    {   
        LOG(LT_ERROR_TRANS, szTransID,  "Register invlaid connected| server_id=0x%x| addr=%s| listen_port=%d| conn=(fd:%d, port:%d, id:0x%x)",
            nServerID, oServer.m_szRemoteAddr, oServer.m_nListenPort, oServer.m_oConnInfo.m_nSock, oServer.m_oConnInfo.m_nPort, oServer.m_oConnInfo.m_nUniqueID
            );
        return false;
    }
    
    std::map<uint32, StorageServer>::iterator it = m_mapServer.find(nServerID);
    if (it != m_mapServer.end())
    {
        StorageServer &oIServer = it->second;
        if(0 == oIServer.m_nConnCnt) 
        {
            sMsg = "add first again";
            snprintf(oIServer.m_szRemoteAddr, sizeof(oIServer.m_szRemoteAddr), "%s", oServer.m_szRemoteAddr);
            snprintf(oIServer.m_szServerName, sizeof(oIServer.m_szServerName), "%s", oServer.m_szServerName);
            oIServer.m_nListenPort  = oServer.m_nListenPort;
            if(!oIServer.AddConn(szTransID, oServer.m_oConnInfo))
            {
                LOG(LT_ERROR_TRANS, szTransID,  "Register add failed 111| server_id=0x%x| addr=%s| listen_port=%d| conn=(fd:%d, port:%d, id:0x%x)",
                    nServerID, oServer.m_szRemoteAddr, oServer.m_nListenPort, oServer.m_oConnInfo.m_nSock, oServer.m_oConnInfo.m_nPort, oServer.m_oConnInfo.m_nUniqueID
                    );
                return false;
            }
        }
        else if(0 != strcmp(oServer.m_szRemoteAddr, oIServer.m_szRemoteAddr))
        {
            //同一ID,当ip不一致时,不可注册
            LOG(LT_ERROR_TRANS, szTransID,  
                "Register addr limited| server_id=0x%x| old_addr=%s| new_addr=%s| new_conn=(fd:%d, port:%d, id:0x%x)",
                nServerID,  oIServer.m_szRemoteAddr, oServer.m_szRemoteAddr, oServer.m_oConnInfo.m_nSock, oServer.m_oConnInfo.m_nPort, oServer.m_oConnInfo.m_nUniqueID
                );
            return false;
        }
        else 
        {
            if(!oIServer.AddConn(szTransID, oServer.m_oConnInfo))
            {
                LOG(LT_ERROR_TRANS, szTransID, 
                    "Register add failed 222| server_id=0x%x| addr=%s| listen_port=%d| conn=(fd:%d, port:%d, id:0x%x)",
                    nServerID, oServer.m_szRemoteAddr, oServer.m_nListenPort, oServer.m_oConnInfo.m_nSock, oServer.m_oConnInfo.m_nPort, oServer.m_oConnInfo.m_nUniqueID
                    );
                return false;
            }

            sMsg = "add another";
        }
    }
    else 
    {
        //第一插入
        StorageServer oIServer;
        oIServer.m_nResCnt  = 0;
        oIServer.m_nConnCnt = 1;    //第一次插入在此赋值,其他在AddConn/DelConn变更
        oIServer.m_nListenPort      = oServer.m_nListenPort;
        oIServer.m_lstConnInfo[0]   = oServer.m_oConnInfo;
        oIServer.m_nServerID        = nServerID;
        snprintf(oIServer.m_szRemoteAddr, sizeof(oIServer.m_szRemoteAddr), "%s", oServer.m_szRemoteAddr);
        snprintf(oIServer.m_szServerName, sizeof(oIServer.m_szServerName), "%s", oServer.m_szServerName);
        
        m_mapServer[nServerID] = oIServer;
        sMsg = "add first";
    }

    DebugPrintf(szTransID);
    LOG(LT_INFO_TRANS, szTransID,  "Register succ| server_id=0x%x| addr=%s| listen_port=%d| conn=(fd=%d, port:%d, id=0x%x)| msg=%s",
        nServerID, oServer.m_szRemoteAddr, oServer.m_nListenPort, oServer.m_oConnInfo.m_nSock, oServer.m_oConnInfo.m_nPort, oServer.m_oConnInfo.m_nUniqueID, sMsg.c_str()
        );
    
    return true;
}

void CRegisterServerManager::UnRegister(const char *szTransID, uint32 nServerID, int32 nSock, uint32 nUniqueID)
{
    frame::Lock lock(&m_lock);
    std::map<uint32, StorageServer>::iterator it = m_mapServer.find(nServerID);
    if (it != m_mapServer.end())
    {   
        StorageServer &oIServer = it->second;
        if(!oIServer.DelConn(szTransID, nSock, nUniqueID))
        {
            LOG(LT_ERROR_TRANS, szTransID, "UnRegister del failed| server_id=0x%x| addr=%s| fd=%d| unique_id=0x%x",
                nServerID, oIServer.m_szRemoteAddr, nSock, nUniqueID
                );
        }
        else 
        {
            LOG(LT_INFO_TRANS, szTransID, "UnRegister del succ| server_id=0x%x| addr=%s| fd=%d| unique_id=0x%x| conn_cnt=%d",
                nServerID, oIServer.m_szRemoteAddr, nSock, nUniqueID, oIServer.m_nConnCnt
                );
            DebugPrintf(szTransID);
        }
    }
    else 
    {
        //找不到时,需要遍历删除 -- 2020.9.2 bug修复
        bool bSucc = false;
        for(std::map<uint32, StorageServer>::iterator it = m_mapServer.begin(); it != m_mapServer.end(); it++) 
        {
            StorageServer &oIServer = it->second;
            if(oIServer.DelConn(szTransID, nSock, nUniqueID))
            {
                LOG(LT_INFO_TRANS, szTransID, "UnRegister del succ(no serverid)| server_id=0x%x(0x%x)| addr=%s| fd=%d| unique_id=0x%x| conn_cnt=%d",
                    oIServer.m_nServerID, nServerID, oIServer.m_szRemoteAddr, nSock, nUniqueID, oIServer.m_nConnCnt
                    );

                bSucc = true;
                DebugPrintf(szTransID);
                break;
            }
        }

        if(!bSucc) {
            LOG(LT_ERROR_TRANS, szTransID, "UnRegister del not record| server_id=0x%x| fd=%d| unique_id=0x%x",
                nServerID, nSock, nUniqueID
                );
        }
    }
}

bool CRegisterServerManager::GetServerInfo(const char *szTransID, uint32 nServerID, RegisterServer &oServer)
{
    frame::Lock lock(&m_lock);
    std::map<uint32, StorageServer>::iterator it = m_mapServer.find(nServerID);
    if (it == m_mapServer.end())
    {
        return false;
    }

    StorageServer &oIServer = it->second;    
    return oIServer.ToRegisterServer(szTransID, oServer);
}

bool CRegisterServerManager::GetServerConn(const char *szTransID, uint32 nServerID, ConnInfo &oConn, uint32 nConnIdx)
{
    oConn.Reset();
    frame::Lock lock(&m_lock);
    std::map<uint32, StorageServer>::iterator it = m_mapServer.find(nServerID);
    if (it == m_mapServer.end())
    {
        return false;
    }

    StorageServer &oIServer = it->second;    
    return oIServer.GetOneConn(szTransID, oConn, nConnIdx);
}

void CRegisterServerManager::DebugPrintf(const char *szTransID)
{
    int index = 0;  
    for(std::map<uint32, StorageServer>::iterator it = m_mapServer.begin(); it != m_mapServer.end(); it++)
    {
        StorageServer &oIServer = it->second;
        string strConnInfo;
        char szTemp[64] = {0};
        for(uint32 i = 0;  i < oIServer.m_nConnCnt; i++) {
            memset(szTemp, 0, sizeof(szTemp));
            snprintf(szTemp, sizeof(szTemp) - 1, "[fd:%d, port:%d, id=0x%x] ", 
                oIServer.m_lstConnInfo[i].m_nSock, oIServer.m_lstConnInfo[i].m_nPort, oIServer.m_lstConnInfo[i].m_nUniqueID);
            strConnInfo.append(szTemp);
        }
        
        LOG(LT_INFO_TRANS, szTransID, 
            "Register server info| index=%d| server_id=0x%x| name=%s| addr=%s| listen_port=%d| conn_cnt=%d| conn_info=%s", 
            ++index, oIServer.m_nServerID, oIServer.m_szServerName, oIServer.m_szRemoteAddr, oIServer.m_nListenPort, oIServer.m_nConnCnt, strConnInfo.c_str()
            );
    }
}

uint32 CRegisterServerManager::GetSize()
{
    return (uint32)m_mapServer.size();
}

int CRegisterServerManager::Select(const char *szTransID, uint32 &nServerID, RegisterServer &oServer)
{
    int nRet        = -99;
    uint32 nTempCnt = 0xFFFFFF;
    memset(&oServer, 0, sizeof(oServer));
    
    frame::Lock l(&m_lock);
    for (std::map<uint32, StorageServer>::iterator it = m_mapServer.begin(); it != m_mapServer.end(); ++it)
    {
        //memset(&oServer, 0, sizeof(oServer));  -- 不能设置为0, 当ToRegisterServer失败时,会保持上一次成功的值
        StorageServer &oIServer = it->second;
        if(0 == oIServer.m_nConnCnt) 
        {
            continue;
        }
        
        //大于某一指定值,不再分配
        //....

        //小于某一指定值,直接选择
        if(oIServer.m_nResCnt < 5)
        {
            if(!oIServer.ToRegisterServer(szTransID, oServer))  {
                continue;
            }

            nServerID = oIServer.m_nServerID;
            nRet      = 0;
            break;
        }

        //选一个最小的
        if(nTempCnt > oIServer.m_nResCnt)
        {
            if(!oIServer.ToRegisterServer(szTransID, oServer)) {
                continue;
            }

            nServerID   = oIServer.m_nServerID;
            nTempCnt    = oIServer.m_nResCnt;
            nRet        = 0;
        }
    }
   
    return nRet;
}


int CRegisterServerManager::ModifyResCnt(const char *szTransID, uint32 nServerID, uint32 nResCnt)
{
    frame::Lock l(&m_lock);
    std::map<uint32, StorageServer>::iterator it = m_mapServer.find(nServerID);
    if (it != m_mapServer.end() && it->second.m_nConnCnt > 0)
    {
        it->second.m_nResCnt = nResCnt; 

        DebugPrintf(szTransID);
        return 0;
    }

    return -99;
}

uint32 CRegisterServerManager::GetConnIdx(const char *szTransID, uint32 nServerID, int32 nSock, uint32 nOldIdx, const string &sLogMsg)
{
    frame::Lock lock(&m_lock);
    std::map<uint32, StorageServer>::iterator it = m_mapServer.find(nServerID);
    if (it == m_mapServer.end())
    {
        return MAX_SERVER_REGISTER_CNT + 1;
    }

    return it->second.AllocConnIdx(szTransID, nSock, nOldIdx, sLogMsg);    
}


