#include "GlobalConf.h"
#include "frame.h"
#include "system.pb.h"
#include "comm_define.h"
#include "server_define.h"

using namespace frame;


class CLocalConf2 : public dtscript::IScriptHandler
{
public:
    CLocalConf2(GlobalConf *pConf):m_pGlobalConf(pConf) { } 
    bool LoadConfig(const char *szConfigFile) 
    { 
        return ScriptEngine::LoadFile(szConfigFile, this);
    }

    virtual bool OnFileLoad(const char *szFileName, dtscript::IIniIterator *pFilePoint)
    {
        return LoadSvrConfig(szFileName, pFilePoint);
    }

private:
    bool LoadSvrConfig(const char *szFileName, dtscript::IIniIterator *pFilePoint)
    {  
        if (pFilePoint == NULL || NULL == m_pGlobalConf)
        {
            LOG(LT_ERROR, "load_global_conf %s ptr is null", m_pGlobalConf->strLocalFileName.c_str());
            return false;
        }
     
        //比赛规则
        IniVariant &varHost  = (*pFilePoint)["host"];
        if(varHost.Empty())
        {   
            LOG(LT_ERROR,  "load_global_conf %s no host segment failed", m_pGlobalConf->strLocalFileName.c_str());
            return false;
        }

        m_pGlobalConf->nLocalBalance    = uint16(varHost["host_balance"]);
        m_pGlobalConf->nLocalUm         = uint16(varHost["host_um"]);  
        m_pGlobalConf->nLocalAcpc       = uint16(varHost["host_acpc"]);    
        m_pGlobalConf->nLocalBrain      = uint16(varHost["host_brain"]);   
        m_pGlobalConf->nLocalTpa        = uint16(varHost["host_tpa"]);     
        m_pGlobalConf->nLocalCfr        = uint16(varHost["host_cfr"]);
        m_pGlobalConf->nLocalAlg        = uint16(varHost["host_alg"]);
        m_pGlobalConf->nMaxFd           = uint32(varHost["max_fd"]);

        //默认是"services.ini"
        //支持配置是测试环境一套算法匹配多套服务环境
        IniVariant &varServicesFile     = varHost["services_file"];
        if(!varServicesFile.Empty()) 
        {
            m_pGlobalConf->strServicesFileName = (const char *)(varServicesFile);
        }
        else 
        {
            m_pGlobalConf->strServicesFileName = "services.ini";
        }
        
        if(m_pGlobalConf->nMaxFd < 2048 || m_pGlobalConf->nMaxFd > 0x3FFFF)
        {
            LOG(LT_ERROR,  "load_global_conf %s max_fd failed| max_fd=%u", m_pGlobalConf->strLocalFileName.c_str(), m_pGlobalConf->nMaxFd);
            return false;
        }
        
        LOG(LT_INFO, 
            "load_global_conf %s| max_fd=%u| local_balance=%d| local_um=%d| local_acpc=%d| local_brain=%d| local_tpa=%d| local_cfr=%d| local_alg=%d| services_file=%s", 
            m_pGlobalConf->strLocalFileName.c_str(), m_pGlobalConf->nMaxFd, m_pGlobalConf->nLocalBalance, m_pGlobalConf->nLocalUm, m_pGlobalConf->nLocalAcpc, 
            m_pGlobalConf->nLocalBrain, m_pGlobalConf->nLocalTpa, m_pGlobalConf->nLocalCfr, m_pGlobalConf->nLocalAlg, 
            m_pGlobalConf->strServicesFileName.c_str()
            );

        
        //日志文件的个性化[所有模块都可个性化]
        string strLogFile    = m_pGlobalConf->strModuleName + "/Log/file";
        IniVariant &vLogFile = varHost[strLogFile.c_str()];
        if(!vLogFile.Empty()) {
            m_pGlobalConf->oModuleConf.strLogFile   = (const char *)(vLogFile);
            LOG(LT_INFO, "load_global_conf %s| load personalized succ| logfile=%s", m_pGlobalConf->strLocalFileName.c_str(), m_pGlobalConf->oModuleConf.strLogFile.c_str());
        }

        //acpc模块的个性化配置
        if("acpc" == m_pGlobalConf->strModuleName)
        {   
            ;
        }
                       
        return true;
    }

public:
    GlobalConf *m_pGlobalConf;
};

class CServicesConf2 : public dtscript::IScriptHandler
{
public:
    CServicesConf2(GlobalConf *pConf):m_pGlobalConf(pConf) { } 
    bool LoadConfig(const char *szConfigFile) 
    { 
        return ScriptEngine::LoadFile(szConfigFile, this);
    }

    virtual bool OnFileLoad(const char *szFileName, dtscript::IIniIterator *pFilePoint)
    {
        return LoadSvrConfig(szFileName, pFilePoint);
    }

private:
    bool LoadSvrConfig(const char *szFileName, dtscript::IIniIterator *pFilePoint)
    {  
        if (pFilePoint == NULL || NULL == m_pGlobalConf)
        {
            LOG(LT_ERROR, "load_global_conf service.ini ptr is null");
            return false;
        }
     
        IniVariant &varServer  = (*pFilePoint)["server"];
        if(varServer.Empty())
        {   
            LOG(LT_ERROR,  "load_global_conf service.ini server segment failed");
            return false;
        }
        
        uint16 nBalanceCnt  = uint16(varServer["balance_server_cnt"]);
        uint16 nUmCnt       = uint16(varServer["um_server_cnt"]);
        uint16 nAcpcCnt     = uint16(varServer["acpc_server_cnt"]);
        uint16 nBrainCnt    = uint16(varServer["brain_server_cnt"]);
        uint16 nTpaCnt      = uint16(varServer["tpa_server_cnt"]);
        uint16 nDealerCnt   = uint16(varServer["dealer_server_cnt"]);
        uint16 nCfrCnt      = uint16(varServer["cfr_server_cnt"]);
        uint16 nAlgCnt      = uint16(varServer["alg_api_cnt"]);
        uint16 nBlockCnt    = uint16(varServer["blockchain_server_cnt"]);
        
        LOG(LT_INFO, "load_global_conf service.ini server cnt| balance_cnt=%d| um_cnt=%d| acpc_cnt=%d| brain_cnt=%d| tpa_cnt=%d| dealer_cnt=%d| cfr_cnt=%d| alg_cnt=%d| block_chain_cnt=%d", 
            nBalanceCnt, nUmCnt, nAcpcCnt, nBrainCnt, nTpaCnt, nDealerCnt, nCfrCnt, nAlgCnt, nBlockCnt);
        
        if(!LoadServerHost(pFilePoint, Pb::SERVER_TYPE_BALANCE, "balance_host_", nBalanceCnt, m_pGlobalConf->vBalanceServer))
        {
            LOG(LT_ERROR,  "load_global_conf service.ini balance host invalid");
            return false;
        }

        if(!LoadServerHost(pFilePoint, Pb::SERVER_TYPE_UM, "um_host_", nUmCnt, m_pGlobalConf->vUmServer))
        {
            LOG(LT_ERROR,  "load_global_conf service.ini um host invalid");
            return false;
        }

        if(!LoadServerHost(pFilePoint, Pb::SERVER_TYPE_ACPC, "acpc_host_", nAcpcCnt, m_pGlobalConf->vAcpcServer))
        {
            LOG(LT_ERROR,  "load_global_conf service.ini acpc host invalid");
            return false;
        }

        if(!LoadServerHost(pFilePoint, Pb::SERVER_TYPE_BRAIN, "brain_host_", nBrainCnt, m_pGlobalConf->vBrainServer))
        {
            LOG(LT_ERROR,  "load_global_conf service.ini brain host invalid");
            return false;
        }

        if(!LoadServerHost(pFilePoint, Pb::SERVER_TYPE_TPA, "tpa_host_", nTpaCnt, m_pGlobalConf->vTpaServer))
        {
            LOG(LT_ERROR,  "load_global_conf service.ini tpa host invalid");
            return false;
        }

        if(!LoadServerHost(pFilePoint, Pb::SERVER_TYPE_DEALER, "dealer_host_", nDealerCnt, m_pGlobalConf->vDealerServer))
        {
            LOG(LT_ERROR,  "load_global_conf service.ini dealer host invalid");
            return false;
        }

        if(!LoadServerHost(pFilePoint, Pb::SERVER_TYPE_CFR, "cfr_host_", nCfrCnt, m_pGlobalConf->vCfrServer))
        {
            LOG(LT_ERROR,  "load_global_conf service.ini cfr host invalid");
            return false;
        }

        if(!LoadServerHost(pFilePoint, Pb::SERVER_TYPE_BLOCKCHAIN, "blockchain_host_", nBlockCnt, m_pGlobalConf->vBlockChainServer))
        {
            LOG(LT_ERROR,  "load_global_conf service.ini blockchain host invalid");
            return false;
        }

        LOG(LT_INFO, "load_global_conf services.ini| balance_size=%d| um_size=%d| acpc_size=%d| brain_size=%d| tpa_size=%d| dealer_size=%d| cfr_size=%d| algapi_size=%d| blockchain_size=%d",
            m_pGlobalConf->vBalanceServer.size(), 
            m_pGlobalConf->vUmServer.size(), 
            m_pGlobalConf->vAcpcServer.size(), 
            m_pGlobalConf->vBrainServer.size(), 
            m_pGlobalConf->vTpaServer.size(), 
            m_pGlobalConf->vDealerServer.size(),
            m_pGlobalConf->vCfrServer.size(),
            m_pGlobalConf->vAlgApi.size(),
            m_pGlobalConf->vBlockChainServer.size()
            );
            

        if(m_pGlobalConf->strModuleName == "cfr")
        {
            if(0 == m_pGlobalConf->vCfrServer.size())
            {
                LOG(LT_ERROR, "load_global_conf services.ini invalid cfr server size");
                return false;
            }
        }
        else 
        {
            //redis
            IniVariant &varRedis  = (*pFilePoint)["redis"];
            if(varRedis.Empty())
            {   
                LOG(LT_ERROR,  "load_global_conf service.ini redis segment failed");
                return false;
            }
            
            m_pGlobalConf->strServerCenterRedis = (const char *)(varRedis["ServerCenter"]);
            m_pGlobalConf->strDataCenterRedis   = (const char *)(varRedis["DataCenter"]);
            m_pGlobalConf->strBettingTreeRedis  = (const char *)(varRedis["BettingTree"]);
            
            LOG(LT_INFO, 
                "load_global_conf service.ini| server_center_redis=%s| data_center_redis=%s| betting_tree_redis=%s", 
                m_pGlobalConf->strServerCenterRedis.c_str(), m_pGlobalConf->strDataCenterRedis.c_str(), m_pGlobalConf->strBettingTreeRedis.c_str()
                );
            
            if(m_pGlobalConf->strServerCenterRedis.empty() || m_pGlobalConf->strDataCenterRedis.empty() || m_pGlobalConf->strBettingTreeRedis.empty())
            {   
                LOG(LT_ERROR,  "load_global_conf service.ini redis invalid");
                return false;
            }
        
            if(0 == m_pGlobalConf->vBalanceServer.size() || 0 == m_pGlobalConf->vUmServer.size() || 0 == m_pGlobalConf->vAcpcServer.size() ||
               0 ==  m_pGlobalConf->vBrainServer.size() ) 
            {
                LOG(LT_ERROR, "load_global_conf services.ini invalid server size");
                return false;
            }
        }
        
        
        return true;
    }

    bool LoadServerHost(dtscript::IIniIterator *pFilePoint, uint8 nSvrType, const string &strSegmentPrefix, uint16 nServerCnt, vector<DefServer> &vServer)
    {
        assert(NULL != pFilePoint);

        for(uint16 i = 1; i <= nServerCnt; i++)
        {
            string strSegment = strSegmentPrefix + to_string(i);
            IniVariant &varSingle  = (*pFilePoint)[strSegment.c_str()];
            if(varSingle.Empty()) 
            {
                LOG(LT_ERROR, "load_global_conf service.ini server host invalid segment %s", strSegment.c_str());
                return false;
            }
            
            string sAddr;
            SERVERKEY sk;
            DefServer oServer;
            oServer.nPort       = uint16(varSingle["port"]);
            sAddr               = (const char *)(varSingle["ip"]);
            sk.nRegID           = uint8(varSingle["RegID"]);
            sk.nInstID          = uint16(varSingle["InstID"]);
            sk.nType            = nSvrType;

            if(Pb::SERVER_TYPE_CFR == nSvrType) {
                snprintf(oServer.szFlagParams, sizeof(oServer.szFlagParams) - 1, "%s", (const char *)(varSingle["model"]));
            }
            
            oServer.nServerID = sk.ToUint32();
            snprintf(oServer.ip, sizeof(oServer.ip) - 1, "%s", sAddr.c_str());
            LOG(LT_INFO, 
                "load_global_conf service.ini server host| segment=%s| server_id=0x%x| ip=%s| port=%d| flag_param=%s",
                strSegment.c_str(), oServer.nServerID, oServer.ip, oServer.nPort, oServer.szFlagParams
                );
            
            if(0 == sk.nRegID || 0 == sk.nInstID || 0 == nSvrType || strlen(oServer.ip) < 5) 
            {
                LOG(LT_ERROR, "load_global_conf service.ini server host invalid some ..");
                return false;
            }

            if(Pb::SERVER_TYPE_BRAIN != nSvrType && 0 == oServer.nPort)
            {
                LOG(LT_ERROR, "load_global_conf service.ini server host invalid port");
                return false;
            }

            vServer.push_back(oServer);
        }

        return true;
    }

public:
    GlobalConf *m_pGlobalConf;
};

/*class CHostsConf2 : public dtscript::IScriptHandler
{
public:
    CHostsConf2(GlobalConf *pConf):m_pGlobalConf(pConf) { }
    bool LoadConfig(const char *szConfigFile) 
    { 
        return ScriptEngine::LoadFile(szConfigFile, this);
    }
    
    virtual bool OnFileLoad(const char *szFileName, dtscript::IIniIterator *pFilePoint)
    {
        return LoadSvrConfig(szFileName, pFilePoint);
    }


private:
    bool LoadSpecifiedServer(IniVariant &varSingle, const HostsSegment &oSegment, const string &strServerName)
    {
        string strInstID     = strServerName + "svr_InstID";
        string strPort       = "services/server/" + strServerName + "_port";
        string strBattlePort = "services/server/" + strServerName + "_battle_port";

        SERVERKEY sk;
        sk.nRegID   = oSegment.nRegID;
        sk.nInstID  = uint16(varSingle[strInstID.c_str()]);
        if(0 == sk.nInstID) {
            sk.nInstID = oSegment.nInstID;
        }
        
        DefServer oServer;
        snprintf(oServer.ip, sizeof(oServer.ip) - 1, "%s", oSegment.strAddr.c_str());
        oServer.nPort       = uint16(varSingle[strPort.c_str()]);

        if(0 == strcmp(strServerName.c_str(), "balance"))
        {
            sk.nType            = Pb::SERVER_TYPE_BALANCE;
            oServer.nServerID   = sk.ToUint32();
            if(0 == oServer.nPort) {
                oServer.nPort = m_pGlobalConf->oServicesCfg.nBalancePort;
            }
            m_pGlobalConf->vBalanceServer.push_back(oServer);
        }
        else if(0 == strcmp(strServerName.c_str(), "um"))
        {
            sk.nType            = Pb::SERVER_TYPE_UM;
            oServer.nServerID   = sk.ToUint32();
            if(0 == oServer.nPort) {
                oServer.nPort = m_pGlobalConf->oServicesCfg.nUmPort;
            }
            m_pGlobalConf->vUmServer.push_back(oServer);
        }
        else if(0 == strcmp(strServerName.c_str(), "acpc"))
        {
            sk.nType            = Pb::SERVER_TYPE_ACPC;
            oServer.nServerID   = sk.ToUint32();
            if(0 == oServer.nPort) {
                oServer.nPort = m_pGlobalConf->oServicesCfg.nAcpcPort;
            }
            m_pGlobalConf->vAcpcServer.push_back(oServer);
        }
        else if(0 == strcmp(strServerName.c_str(), "brain"))
        {
            sk.nType            = Pb::SERVER_TYPE_BRAIN;
            oServer.nServerID   = sk.ToUint32();
            if(0 == oServer.nPort) {
                oServer.nPort = m_pGlobalConf->oServicesCfg.nBrainPort;
            }
            oServer.nBattlePort = uint16(varSingle[strBattlePort.c_str()]);
            if(0 == oServer.nBattlePort) {
                oServer.nBattlePort = m_pGlobalConf->oServicesCfg.nBrainBattlePort;
            }
            
            m_pGlobalConf->vBrainServer.push_back(oServer);
        }
        else if(0 == strcmp(strServerName.c_str(), "dealer"))
        {
            sk.nType            = Pb::SERVER_TYPE_DEALER;
            oServer.nServerID   = sk.ToUint32();
            if(0 == oServer.nPort) {
                oServer.nPort = m_pGlobalConf->oServicesCfg.nDealerPort;
            }
            m_pGlobalConf->vDealerServer.push_back(oServer);
        }
        else if(0 == strcmp(strServerName.c_str(), "tpa"))
        {
            sk.nType            = Pb::SERVER_TYPE_TPA;
            oServer.nServerID   = sk.ToUint32();
            if(0 == oServer.nPort) {
                oServer.nPort = m_pGlobalConf->oServicesCfg.nTpaPort;
            }
            m_pGlobalConf->vTpaServer.push_back(oServer);
        }
        else
        {
            LOG(LT_INFO, "load_global_conf hosts.ini| laod %s server failed", strServerName.c_str());
            return false;
        }

        //启动模块的专有个性化配置
        if(oSegment.strSegment == m_pGlobalConf->strHostsSegment && strServerName == m_pGlobalConf->strModuleName)
        {
            //日志文件的个性化[所有模块都可个性化]
            m_pGlobalConf->oLocalServer = oServer;
            string strLogFile    = strServerName + "/Log/file";
            IniVariant &vLogFile= varSingle[strLogFile.c_str()];
            if(!vLogFile.Empty()) {
                strLogFile   = (const char *)(vLogFile);
                snprintf(m_pGlobalConf->oLogConf.m_szPath, sizeof(m_pGlobalConf->oLogConf.m_szPath) - 1, "%s", strLogFile.c_str());
                LOG(LT_INFO, "load_global_conf hosts.ini| load personalized succ| logfile=%s", m_pGlobalConf->oLogConf.m_szPath);
            }

            //acpc模块的个性化配置
            if(0 == strcmp(strServerName.c_str(), "acpc"))
            {   
                string strTree    = "acpc/Config/betting_tree_filepath";
                IniVariant &vTree = varSingle[strTree.c_str()];
                if(!vTree.Empty()) {
                    m_pGlobalConf->oAcpcConf.strBettingTreeFilePath  = (const char *)(vTree);
                    LOG(LT_INFO, "load_global_conf hosts.ini| load personalized succ| betting_tree_filepath=%s", m_pGlobalConf->oAcpcConf.strBettingTreeFilePath.c_str());
                }
            }
        }

        LOG(LT_INFO, "load_global_conf hosts.ini| load %s server succ| server_id=0x%x| ip=%s| port=%d| battle_port=%d", 
            strServerName.c_str(), oServer.nServerID, oServer.ip, oServer.nPort, oServer.nBattlePort
            );

        return true;
    }
    
    bool LoadSvrConfig(const char *szFileName, dtscript::IIniIterator *pFilePoint)
    {  
        if (pFilePoint == NULL || NULL == m_pGlobalConf)
        {
            LOG(LT_ERROR,  "load_global_conf hosts.ini ptr is null");
            return false;
        }
     
        IniVariant &varHosts  = (*pFilePoint)["hosts"];
        if(varHosts.Empty())
        {   
            LOG(LT_ERROR,  "load_global_conf hosts.ini no hosts segment");
            return false;
        }
        
        uint32 nMaxIndex = uint16(varHosts["max_host_index"]);       
        for(uint32 i = 0; i < nMaxIndex; i++)
        {
            string strHost = "host_" + to_string(i+1) + "_index";
            IniVariant &varSingle  = (*pFilePoint)[strHost.c_str()];
            if(varSingle.Empty()) 
            {
                LOG(LT_ERROR, "load_global_conf hosts.ini invalid segment %s", strHost.c_str());
                return false;
            }

            //基本参数
            HostsSegment oSegment; 
            oSegment.strSegment = strHost;
            oSegment.strAddr    = (const char *)(varSingle["IP"]);
            oSegment.strPragram = (const char *)(varSingle["program"]);
            oSegment.nRegID     = uint8(varSingle["RegID"]);
            oSegment.nInstID    = uint16(varSingle["InstID"]);
            
            if(NULL != strstr(oSegment.strPragram.c_str(), "balancesvr") )
            {
                if(!LoadSpecifiedServer(varSingle, oSegment, "balance")) {
                    LOG(LT_ERROR, "load_global_conf hosts.ini balance failed| host_segment=%s| program=%s", strHost.c_str(), oSegment.strPragram.c_str());
                    return false;
                }
            }

            if(NULL != strstr(oSegment.strPragram.c_str(), "umsvr"))
            {
                if(!LoadSpecifiedServer(varSingle, oSegment, "um")) {
                    LOG(LT_ERROR, "load_global_conf hosts.ini um failed| host_segment=%s| program=%s", strHost.c_str(), oSegment.strPragram.c_str());
                    return false;
                }
            }

            if(NULL != strstr(oSegment.strPragram.c_str(), "acpcsvr"))
            {
                if(!LoadSpecifiedServer(varSingle, oSegment, "acpc")) {
                    LOG(LT_ERROR, "load_global_conf hosts.ini acpc failed| host_segment=%s| program=%s", strHost.c_str(), oSegment.strPragram.c_str());
                    return false;
                }
            }

            if(NULL != strstr(oSegment.strPragram.c_str(), "brainsvr"))
            {
                if(!LoadSpecifiedServer(varSingle, oSegment, "brain")) {
                    LOG(LT_ERROR, "load_global_conf hosts.ini brain failed| host_segment=%s| program=%s", strHost.c_str(), oSegment.strPragram.c_str());
                    return false;
                }
            }

            if(NULL != strstr(oSegment.strPragram.c_str(), "tpasvr"))
            {
                if(!LoadSpecifiedServer(varSingle, oSegment, "tpa")) {
                    LOG(LT_ERROR, "load_global_conf hosts.ini tpa failed| host_segment=%s| program=%s", strHost.c_str(), oSegment.strPragram.c_str());
                    return false;
                }
            }

            if(NULL != strstr(oSegment.strPragram.c_str(), "dealersvr"))
            {
                if(!LoadSpecifiedServer(varSingle, oSegment, "dealer")) {
                    LOG(LT_ERROR, "load_global_conf hosts.ini dealer failed| host_segment=%s| program=%s", strHost.c_str(), oSegment.strPragram.c_str());
                    return false;
                }
            }
            
            LOG(LT_INFO, "load_global_conf hosts.ini| load single segment succ| index=%u| segment=%s| program=%s", 
                i, strHost.c_str(), oSegment.strPragram.c_str());
        }

        LOG(LT_INFO, "load_global_conf hosts.ini| balance_size=%d| um_size=%d| acpc_size=%d| brain_size=%d| tpa_size=%d| dealer_size=%d",
            m_pGlobalConf->vBalanceServer.size(), 
            m_pGlobalConf->vUmServer.size(), 
            m_pGlobalConf->vAcpcServer.size(), 
            m_pGlobalConf->vBrainServer.size(), 
            m_pGlobalConf->vTpaServer.size(), 
            m_pGlobalConf->vDealerServer.size()
            );

        if(0 == m_pGlobalConf->vBalanceServer.size() || 0 == m_pGlobalConf->vUmServer.size() || 0 == m_pGlobalConf->vAcpcServer.size() ||
           0 ==  m_pGlobalConf->vBrainServer.size() || 0 == m_pGlobalConf->vTpaServer.size() || 0 == m_pGlobalConf->vDealerServer.size() ) 
        {
            LOG(LT_ERROR, "load_global_conf hosts.ini invalid server size");
            return false;
        }

        LOG(LT_INFO, "load_global_conf hosts.ini| local_server| server_id=0x%x| ip=%s| port=%d| battle_port=%d| logfile=%s", 
            m_pGlobalConf->oLocalServer.nServerID, m_pGlobalConf->oLocalServer.ip, m_pGlobalConf->oLocalServer.nPort, m_pGlobalConf->oLocalServer.nBattlePort, m_pGlobalConf->oLogConf.m_szFile
            );
        
        return true;
    }

public:
    GlobalConf *m_pGlobalConf;
};
*/
class CResourcesConf2 : public dtscript::IScriptHandler
{
public:
    CResourcesConf2(GlobalConf *pConf):m_pGlobalConf(pConf) { }
    bool LoadConfig(const char *szConfigFile) 
    { 
        return ScriptEngine::LoadFile(szConfigFile, this);
    }
    
    virtual bool OnFileLoad(const char *szFileName, dtscript::IIniIterator *pFilePoint)
    {
        return LoadSvrConfig(szFileName, pFilePoint);
    }

private:
    bool LoadSvrConfig(const char *szFileName, dtscript::IIniIterator *pFilePoint)
    {  
        if (pFilePoint == NULL)
        {
            return false;
        }
     
        if(NULL == m_pGlobalConf)
        {
            return false;
        }

        //比赛规则
        IniVariant &varRule  = (*pFilePoint)["match_rule"];
        if(!varRule.Empty())
        {   
            m_pGlobalConf->oMathcRule.nMatchEnable     = uint8(varRule["match_enable"]);
            m_pGlobalConf->oMathcRule.nMatchHands      = uint32(varRule["match_hands"]);
            m_pGlobalConf->oMathcRule.nDayResetTimes   = uint16(varRule["day_reset_times"]);
            if(1 == m_pGlobalConf->oMathcRule.nMatchEnable) {
                //if(0 == m_pGlobalConf->oMathcRule.nDayResetTimes || 0 == m_pGlobalConf->oMathcRule.nMatchHands)
                if(0 == m_pGlobalConf->oMathcRule.nMatchHands)
                {
                    LOG(LT_ERROR, "load_global_conf resources.ini invalid match param");
                    return false;
                }
            }
        }
        else {
            m_pGlobalConf->oMathcRule.nMatchEnable   = 0;
            m_pGlobalConf->oMathcRule.nMatchHands    = 0;
            m_pGlobalConf->oMathcRule.nDayResetTimes = 0;
        }

        LOG(LT_INFO, 
            "load_global_conf resources.ini match rule| match_enable=%d| match_hands=%u| day_reset_times=%d", 
            m_pGlobalConf->oMathcRule.nMatchEnable, m_pGlobalConf->oMathcRule.nMatchHands, m_pGlobalConf->oMathcRule.nDayResetTimes);
       
        //机器人ID区间
        IniVariant &varRobotID  = (*pFilePoint)["robot_id_interval"];
        if(varRobotID.Empty())
        {   
            LOG(LT_INFO, "load_global_conf resources.ini robot id interval is empty");
            return false;
        }

        m_pGlobalConf->vRobotIDInterval.clear();
        uint32 nCnt = uint16(varRobotID["cnt"]);
        for(uint32 i = 0; i < nCnt; i++)
        {
            string sServerID = "InstID" + to_string(i);
            string sMinID    = "min_id"   + to_string(i);
            string sMaxID    = "max_id"   + to_string(i);

            uint16 nInstID = uint16(varRobotID[sServerID.c_str()]);
            CIDInterval oID;
            SERVERKEY sk(1, Pb::SERVER_TYPE_BRAIN, nInstID);
            oID.nServerID   = sk.ToUint32();
            oID.nMinID      = uint32(varRobotID[sMinID.c_str()]);
            oID.nMaxID      = uint32(varRobotID[sMaxID.c_str()]);

            if(oID.nMaxID <= oID.nMinID || 0 == oID.nMinID)
            {
                LOG(LT_INFO, "load_global_conf resources.ini invalid min id| InstID=%d| min_id=%d| max_id=%d", 
                    nInstID, oID.nMinID, oID.nMaxID);
                return false;
            }
            else if(oID.nMaxID >= MAX_ROBOT_ROLE_ID)
            {
                LOG(LT_INFO, "load_global_conf resources.ini invalid max id| InstID=%d| max_id=%d", 
                    nInstID, oID.nMaxID);
                return false;
            }
            else
            {
                for(uint32 j = 0; j < m_pGlobalConf->vRobotIDInterval.size(); j++)
                {
                    if(oID.nMinID >= m_pGlobalConf->vRobotIDInterval[j].nMinID && oID.nMinID <= m_pGlobalConf->vRobotIDInterval[j].nMaxID)
                    {
                        LOG(LT_INFO, "load_global_conf resources.ini invalid min id| InstID=%d| min_id=%d| (server_id=0x%x, min_id=%d, max_id=%d)|", 
                            nInstID, oID.nMinID, m_pGlobalConf->vRobotIDInterval[j].nServerID, m_pGlobalConf->vRobotIDInterval[j].nMinID, m_pGlobalConf->vRobotIDInterval[j].nMaxID);
                        return false;
                    }
                    else if(oID.nMaxID >= m_pGlobalConf->vRobotIDInterval[j].nMinID && oID.nMaxID < m_pGlobalConf->vRobotIDInterval[j].nMaxID)
                    {
                        LOG(LT_INFO, "load_global_conf resources.ini invalid max id| InstID=%d| max_id=%d| (server_id=0x%x, min_id=%d, max_id=%d)|", 
                            nInstID, oID.nMaxID, m_pGlobalConf->vRobotIDInterval[j].nServerID, m_pGlobalConf->vRobotIDInterval[j].nMinID, m_pGlobalConf->vRobotIDInterval[j].nMaxID);
                        return false;
                    }
                }
            }
            
            m_pGlobalConf->vRobotIDInterval.push_back(oID);

            LOG(LT_INFO, 
                "load_global_conf resources.ini Robot id interval| index=%d| InstID=%d| server=0x%x| min_id=%u| max_id=%u", 
                i, nInstID, oID.nServerID, oID.nMinID, oID.nMaxID);
        }

        //公共配置 
        IniVariant &varPublic  = (*pFilePoint)["public"];           
        m_pGlobalConf->nDealerEnable            = uint8(varPublic["delaer_enable"]);
        m_pGlobalConf->nBlockChainEnable        = uint8(varPublic["blockchain_enable"]);
        m_pGlobalConf->nBettingTreeRecords      = uint32(varPublic["betting_tree_records"]);
        m_pGlobalConf->strBettingTreeRedisPass  = (const char*)(varPublic["bettring_tree_redis_pass"]);

        LOG(LT_INFO, "load_global_conf resources.ini public| dealer_enable=%d| block_chain_enbale=%d| betting_tree_records=%u| bettring_tree_redis_pass=%s", 
            m_pGlobalConf->nDealerEnable, m_pGlobalConf->nBlockChainEnable, m_pGlobalConf->nBettingTreeRecords, m_pGlobalConf->strBettingTreeRedisPass.c_str()
            );
        if(0 == m_pGlobalConf->nBettingTreeRecords)
        {
            LOG(LT_INFO, "load_global_conf resources.ini invalid betting tree records");
            return false;
        }        
        
        return true;
    }

public:
    GlobalConf *m_pGlobalConf;
};

int load_global_conf(const char *szPath, GlobalConf &oGlobalConf)
{
    //加载local.ini
    string sFileName = string(szPath) + oGlobalConf.strLocalFileName;
    CLocalConf2 oLocal(&oGlobalConf);
    if(!oLocal.LoadConfig(sFileName.c_str()))
    {
        LOG(LT_ERROR, "LOAD_GLOBAL_CONF| load %s failed", sFileName.c_str());  
        return -11;
    }
    LOG(LT_INFO, "LOAD_GLOBAL_CONF| load %s succ...", sFileName.c_str()); 
    
    //加载services.ini
    sFileName = string(szPath) + oGlobalConf.strServicesFileName;
    CServicesConf2 oServices(&oGlobalConf);
    if(!oServices.LoadConfig(sFileName.c_str()))
    {
        LOG(LT_ERROR,  "load services.ini failed");  
        return -13;
    }
    LOG(LT_INFO, "LOAD_GLOBAL_CONF| load %s succ...", sFileName.c_str());  

    //加载resources.ini
    string strResFile = string(szPath) + "resources.ini";
    CResourcesConf2 oRes(&oGlobalConf);
    if(!oRes.LoadConfig(strResFile.c_str()))
    {
        LOG(LT_ERROR,  "load resources failed| %s", strResFile.c_str());  
        return -2;
    }
    LOG(LT_INFO, "LOAD_GLOBAL_CONF| load %s succ...", sFileName.c_str()); 
    
    //加载hosts.ini
    /*sFileName = string(szPath) + "hosts.ini";
    CHostsConf2 oHosts(&oGlobalConf);
    if(!oHosts.LoadConfig(sFileName.c_str()))
    {
        LOG(LT_ERROR,  "load hosts.ini failed");  
        return -12;
    }
    LOG(LT_INFO, "LOAD_GLOBAL_CONF| load %s succ...", sFileName.c_str()); 
    */
    
    return 0;
}


