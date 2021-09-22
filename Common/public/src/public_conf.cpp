#include "public_conf.h"
#include "comm_define.h"
#include "shstd.h"

using namespace shstd::fun;

/*  
*/
bool CRunConfig::Init(const std::string &strExecName)
{
    const char *pName = strExecName.c_str();
    const char *pLast = pName;
    const char *pPath = NULL;

    //仅支持**svr及brainbot名称
    //svr或bot后面有字段,则必须是"_"开始
    {
        const char *pTemp = NULL;
        if(NULL != (pTemp = strstr(pName, "svr")))
        {
            if(strlen(pTemp) > 3 && '_' != pTemp[3])
            {
                printf("Run name invalid| after svr not '_'\r\n");
                return false;
            }
        }
        else if(NULL != (pTemp = strstr(pName, "bot")))
        {
            if(strlen(pTemp) > 3 && '_' != pTemp[3])
            {
                printf("Run name invalid| after bot not '_'\r\n");
                return false;
            }
        }
    }
    

    //路径
    if(NULL != (pPath = strstr(pName, "/sbin/")))
    {
        m_strConfPath       = string(pName, pPath - pName) + "/conf/";
        m_strDefaultLogPath = string(pName, pPath - pName) + "/logs";        
    }
    else 
    {
        m_strConfPath       = "../conf/";
        m_strDefaultLogPath = "../logs";
    }
    
    //定为到程序名称
    if(NULL != (pLast = strrchr(pName, '/')))
    {
        m_strRunName        = pLast + 1;
        m_strDefaultLogFile = pLast + 1;
    }
    else 
    {
        return false;
    }

    //去掉svr字符
    replace_all(m_strDefaultLogFile, "svr", "");

    if(NULL != strstr(m_strRunName.c_str(), "cfrsvr") || NULL != strstr(m_strRunName.c_str(), "algdata"))
    {
        m_strLocalFileName = m_strRunName + ".ini";
        replace_all(m_strLocalFileName, "svr", "");
    }
    else
    {    
        //localfile="local_***.ini"
        const char *pRunName = m_strRunName.c_str();
        if(NULL != (pLast = strchr(pRunName, '_')))
        {
            m_strLocalFileName  = "local" + string(pLast) + ".ini";
        }
        else
        {
            m_strLocalFileName  = "local.ini";
        }
    }

    
    m_strCfgFileName = m_strRunName + ".ini";
    replace_all(m_strCfgFileName, "svr", "");
    
    printf("runName=%s| conf_path=%s| localFile=%s| default_log_file=%s/%s.log\r\n", 
        m_strRunName.c_str(),  m_strConfPath.c_str(), m_strLocalFileName.c_str(), m_strDefaultLogPath.c_str(), m_strDefaultLogFile.c_str()
        ); 

    return true;
}

const string& CRunConfig::GetConfPath()
{
    return m_strConfPath;
}

const string& CRunConfig::GetDefaultLogFile()
{
    return m_strDefaultLogFile;
}

const string& CRunConfig::GetDefaultLogPath()
{
    return m_strDefaultLogPath;
}

const string& CRunConfig::GetRunName()
{
    return m_strRunName;
}

const string& CRunConfig::GetLocalFileName()
{
    return m_strLocalFileName;
}


/*
class CLocalConf : public dtscript::IScriptHandler
{
public:
    CLocalConf(CPublicConf *pConf):m_pPubConf(pConf) { } 
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
        if (pFilePoint == NULL || NULL == m_pPubConf)
        {
            LOG(LT_ERROR_TRANS, "LOAD_PUBLIC_CONFIG", "Local config ptr is null");
            return false;
        }
     
        //比赛规则
        IniVariant &varHost  = (*pFilePoint)["host"];
        if(varHost.Empty())
        {   
            LOG(LT_ERROR_TRANS, "LOAD_PUBLIC_CONFIG", "Local config failed");
            return false;
        }

        m_pPubConf->oLocalCfg.strHostIndex = (const char *)(varHost["specified"]);;

        
        LOG(LT_INFO_TRANS,"LOAD_PUBLIC_CONFIG", "local config| host_index=%s", 
            m_pPubConf->oLocalCfg.strHostIndex.c_str());

        if(m_pPubConf->oLocalCfg.strHostIndex.empty())
        {   
            LOG(LT_ERROR_TRANS, "LOAD_PUBLIC_CONFIG", "Local config param invalid");
            return false;
        }
                               
        return true;
    }

public:
    CPublicConf *m_pPubConf;
};

class CServicesConf : public dtscript::IScriptHandler
{
public:
    CServicesConf(CPublicConf *pConf):m_pPubConf(pConf) { } 
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
        if (pFilePoint == NULL || NULL == m_pPubConf)
        {
            LOG(LT_ERROR_TRANS, "LOAD_PUBLIC_CONFIG", "Local Services ptr is null");
            return false;
        }
     
        //比赛规则
        IniVariant &varServer  = (*pFilePoint)["server"];
        if(varServer.Empty())
        {   
            LOG(LT_ERROR_TRANS, "LOAD_PUBLIC_CONFIG", "Services config failed");
            return false;
        }

        m_pPubConf->oServicesCfg.nBalancePort         = uint16(varServer["balance_port"]);;
        m_pPubConf->oServicesCfg.nUmPort              = uint16(varServer["um_port"]);
        m_pPubConf->oServicesCfg.nAcpcPort            = uint16(varServer["acpc_port"]);
        m_pPubConf->oServicesCfg.nBrainPort           = uint16(varServer["brain_port"]);;
        m_pPubConf->oServicesCfg.nBrainBattlePort     = uint16(varServer["brain_battle_port"]);
        m_pPubConf->oServicesCfg.nTpaPort             = uint16(varServer["tpa_port"]);;
        
        IniVariant &varRedis  = (*pFilePoint)["redis"];
        if(varRedis.Empty())
        {   
            LOG(LT_ERROR_TRANS, "LOAD_PUBLIC_CONFIG", "Services config failed");
            return false;
        }
        m_pPubConf->oServicesCfg.strServerCenterRedis = (const char *)(varRedis["ServerCenter"]);
        m_pPubConf->oServicesCfg.strDataCenterRedis   = (const char *)(varRedis["DataCenter"]);
        
        LOG(LT_INFO_TRANS,"LOAD_PUBLIC_CONFIG", 
            "services config| balance_port=%u| um_port=%u| acpc_port=%u| brain_port=%u| brain_battle_port=%u| tpa_port=%d| server_center_redis=%s| data_center_redis=%s", 
            m_pPubConf->oServicesCfg.nBalancePort, m_pPubConf->oServicesCfg.nUmPort, m_pPubConf->oServicesCfg.nAcpcPort, m_pPubConf->oServicesCfg.nBrainPort, 
            m_pPubConf->oServicesCfg.nBrainBattlePort, m_pPubConf->oServicesCfg.nTpaPort, m_pPubConf->oServicesCfg.strServerCenterRedis.c_str(), m_pPubConf->oServicesCfg.strDataCenterRedis.c_str()
            );

        if(m_pPubConf->oServicesCfg.strServerCenterRedis.empty() || m_pPubConf->oServicesCfg.strDataCenterRedis.empty())
        {   
            LOG(LT_ERROR_TRANS, "LOAD_PUBLIC_CONFIG", "services config redis invalid");
            return false;
        }

        if(0 == m_pPubConf->oServicesCfg.nBalancePort || 0 == m_pPubConf->oServicesCfg.nUmPort || 0 == m_pPubConf->oServicesCfg.nAcpcPort
            || 0 == m_pPubConf->oServicesCfg.nBrainPort || 0 == m_pPubConf->oServicesCfg.nBrainBattlePort || 0 == m_pPubConf->oServicesCfg.nTpaPort)
        {
            LOG(LT_ERROR_TRANS, "LOAD_PUBLIC_CONFIG", "services config port invalid");
            return false;
        }

        
        //荷官信息
        IniVariant &varDealer  = (*pFilePoint)["dealer"];
        if(!varDealer.Empty())
        {   
            m_pPubConf->oServicesCfg.oDealer.nEnable = uint8(varDealer["enable"]);
            snprintf(m_pPubConf->oServicesCfg.oDealer.szAddr, sizeof(m_pPubConf->oServicesCfg.oDealer.szAddr) - 1, "%s", (const char *)(varDealer["ip"]));
            m_pPubConf->oServicesCfg.oDealer.nPort   = uint16(varDealer["port"]);;

            LOG(LT_INFO_TRANS,"LOAD_PUBLIC_CONFIG", "services dealer config| enable=%d| ip=%s:%d", 
                m_pPubConf->oServicesCfg.oDealer.nEnable, m_pPubConf->oServicesCfg.oDealer.szAddr, m_pPubConf->oServicesCfg.oDealer.nPort);
            
            if(SWITCH_ENABLE == m_pPubConf->oServicesCfg.oDealer.nEnable) 
            {
                if(0 == strlen(m_pPubConf->oServicesCfg.oDealer.szAddr) || 0 == m_pPubConf->oServicesCfg.oDealer.nPort) 
                {
                    LOG(LT_ERROR_TRANS, "LOAD_PUBLIC_CONFIG", "Services config dealer invalid");
                    return false;
                }
            }
        }
                 
        return true;
    }

public:
    CPublicConf *m_pPubConf;
};

class CHostsConf : public dtscript::IScriptHandler
{
public:
    CHostsConf(CPublicConf *pConf):m_pPubConf(pConf) { }
    //virtual ~CServicesConf(void) {}    
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
        if (pFilePoint == NULL || NULL == m_pPubConf)
        {
            LOG(LT_ERROR_TRANS, "LOAD_PUBLIC_CONFIG", "Local Services ptr is null");
            return false;
        }
     
        //比赛规则
        IniVariant &varHosts  = (*pFilePoint)["hosts"];
        if(varHosts.Empty())
        {   
            LOG(LT_ERROR_TRANS, "LOAD_PUBLIC_CONFIG", "Hosts config [hosts] failed");
            return false;
        }
        
        uint32 nMaxIndex = uint16(varHosts["max_host_index"]);       
        for(uint32 i = 0; i < nMaxIndex; i++)
        {
            string strHost = "host_" + to_string(i) + "_index";
            IniVariant &varSingle  = (*pFilePoint)[strHost.c_str()];
            if(varSingle.Empty()) {
                continue;
            }

            SingleHost oSingle; 
            oSingle.strAddr    = (const char *)(varSingle["IP"]);
            oSingle.strPragram = (const char *)(varSingle["program"]);
            oSingle.nRegID     = uint8(varSingle["RegID"]);;
            oSingle.nInstID    = uint16(varSingle["InstID"]);;
            
            LOG(LT_INFO_TRANS,"LOAD_PUBLIC_CONFIG", "Hosts config| index=%u| segment=%s| reg_id=%u| inst_id=%u| addr=%s| program=%s", 
                m_pPubConf->oHostsCfg.mapHost.size(), strHost.c_str(), oSingle.nRegID, oSingle.nInstID, oSingle.strAddr.c_str(), oSingle.strPragram.c_str());

            if(oSingle.strAddr.empty() || oSingle.strPragram.empty() || 0 == oSingle.nRegID || 0 == oSingle.nInstID)
            {
                LOG(LT_ERROR_TRANS, "LOAD_PUBLIC_CONFIG", "Hosts config param empty");
                return false;
            }

            m_pPubConf->oHostsCfg.mapHost[strHost] = oSingle;            
        }

        if(0 == m_pPubConf->oHostsCfg.mapHost.size())
        {
            LOG(LT_ERROR_TRANS, "LOAD_PUBLIC_CONFIG", "Hosts config size if empty");
            return false;
        }
        
        return true;
    }

public:
    CPublicConf *m_pPubConf;
};

class CResourcesConf : public dtscript::IScriptHandler
{
public:
    CResourcesConf(CPublicConf *pConf):m_pPubConf(pConf) { }
    //virtual ~CResourcesConf(void) {}    
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
     
        if(NULL == m_pPubConf)
        {
            return false;
        }

        //比赛规则
        IniVariant &varRule  = (*pFilePoint)["match_rule"];
        if(!varRule.Empty())
        {   
            m_pPubConf->oMathcRule.nMatchEnable     = uint8(varRule["match_enable"]);
            m_pPubConf->oMathcRule.nMatchHands      = uint32(varRule["match_hands"]);
            m_pPubConf->oMathcRule.nDayResetTimes   = uint16(varRule["day_reset_times"]);
            if(1 == m_pPubConf->oMathcRule.nMatchEnable) {
                if(0 == m_pPubConf->oMathcRule.nDayResetTimes || 0 == m_pPubConf->oMathcRule.nMatchHands)
                {
                    LOG(LT_INFO_TRANS,"LOAD_PUBLIC_CONFIG", "check match rule failed");
                    return false;
                }
            }
        }
        else {
            m_pPubConf->oMathcRule.nMatchEnable   = 0;
            m_pPubConf->oMathcRule.nMatchHands    = 0;
            m_pPubConf->oMathcRule.nDayResetTimes = 0;
        }

        LOG(LT_INFO_TRANS,"LOAD_PUBLIC_CONFIG", 
            "resources config| match rule| match_enable=%d| match_hands=%u| day_reset_times=%d", 
            m_pPubConf->oMathcRule.nMatchEnable, m_pPubConf->oMathcRule.nMatchHands, m_pPubConf->oMathcRule.nDayResetTimes);
       
        //机器人ID区间
        IniVariant &varRobotID  = (*pFilePoint)["robot_id_interval"];
        if(varRobotID.Empty())
        {   
            LOG(LT_INFO_TRANS,"LOAD_PUBLIC_CONFIG", "robot id interval is empty");
            return false;
        }

        m_pPubConf->vRobotIDInterval.clear();
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
                LOG(LT_INFO_TRANS,"LOAD_PUBLIC_CONFIG", "resources config| invalid min id| InstID=%d| min_id=%d| max_id=%d", 
                    nInstID, oID.nMinID, oID.nMaxID);
                return false;
            }
            else if(oID.nMaxID >= MAX_ROBOT_ROLE_ID)
            {
                LOG(LT_INFO_TRANS,"LOAD_PUBLIC_CONFIG", "resources config| invalid max id| InstID=%d| max_id=%d", 
                    nInstID, oID.nMaxID);
                return false;
            }
            else
            {
                for(uint32 j = 0; j < m_pPubConf->vRobotIDInterval.size(); j++)
                {
                    if(oID.nMinID >= m_pPubConf->vRobotIDInterval[j].nMinID && oID.nMinID <= m_pPubConf->vRobotIDInterval[j].nMaxID)
                    {
                        LOG(LT_INFO_TRANS,"LOAD_PUBLIC_CONFIG", "resources config| invalid min id| InstID=%d| min_id=%d| (server_id=0x%x, min_id=%d, max_id=%d)|", 
                            nInstID, oID.nMinID, m_pPubConf->vRobotIDInterval[j].nServerID, m_pPubConf->vRobotIDInterval[j].nMinID, m_pPubConf->vRobotIDInterval[j].nMaxID);
                        return false;
                    }
                    else if(oID.nMaxID >= m_pPubConf->vRobotIDInterval[j].nMinID && oID.nMaxID < m_pPubConf->vRobotIDInterval[j].nMaxID)
                    {
                        LOG(LT_INFO_TRANS,"LOAD_PUBLIC_CONFIG", "resources config| invalid max id| InstID=%d| max_id=%d| (server_id=0x%x, min_id=%d, max_id=%d)|", 
                            nInstID, oID.nMaxID, m_pPubConf->vRobotIDInterval[j].nServerID, m_pPubConf->vRobotIDInterval[j].nMinID, m_pPubConf->vRobotIDInterval[j].nMaxID);
                        return false;
                    }
                }
            }
            
            m_pPubConf->vRobotIDInterval.push_back(oID);

            LOG(LT_INFO_TRANS,"LOAD_PUBLIC_CONFIG", 
                "resources config| Robot id interval| index=%d| InstID=%d| server=0x%x| min_id=%u| max_id=%u", 
                i, nInstID, oID.nServerID, oID.nMinID, oID.nMaxID);
        }
                       
        return true;
    }

public:
    CPublicConf *m_pPubConf;
};

*/

int Load_log_conf(dtscript::IIniIterator *pFilePoint, CLogConf &oConf)
{
    if (pFilePoint == NULL)
    {
        LOG(LT_ERROR_TRANS, "LOAD_LOG_CONFIG", "Local iterator ptr is null");
        return -1;
    }
 
    //比赛规则
    IniVariant &varLog  = (*pFilePoint)["Log"];
    if(varLog.Empty())
    {   
        LOG(LT_ERROR_TRANS, "LOAD_LOG_CONFIG", "Local variant config failed");
        return -2;
    }

    oConf.m_nAsynMode         = uint8(varLog["asynmode"]);
    oConf.m_nMaxSize          = uint32(varLog["maxsize"]);
    snprintf(oConf.m_szLevel, sizeof(oConf.m_szLevel) - 1, "%s", (const char *)(varLog["level"]));
    
    //从GlobalConf赋值来的配置参数
    if(strlen(oConf.m_szPath) < 5) {
        snprintf(oConf.m_szPath, sizeof(oConf.m_szPath) - 1, "%s", (const char *)(varLog["file"]));
    }
    
    
    char *start = oConf.m_szPath;
    char *end   = NULL;
    char *Last  = NULL;

    //去掉.log后缀
    if(NULL == (end = strstr(start, ".log")))
    {
        LOG(LT_ERROR_TRANS, "LOAD_LOG_CONFIG", "invalid log file %s", oConf.m_szPath);
        return -11;
    }
    *end = 0;

    //分别赋值path & file
    start = oConf.m_szPath;
    while(NULL != (end = strstr(start, "/")))
    {
        start = end + 1;
        Last  = end;
    }

    if(NULL == Last) {
        LOG(LT_ERROR_TRANS, "LOAD_LOG_CONFIG", "invalid log file 22");
        return -12;
    }
    snprintf(oConf.m_szFile, sizeof(oConf.m_szFile) - 1, "%s", start);
    *Last = 0;  //path结束位置置0
    
    LOG(LT_INFO_TRANS, "LOAD_LOG_CONFIG", 
        "Log config| file=%s/%s.log| level=%s| aysnmode=%d| maxsize=%luM", 
        oConf.m_szPath, oConf.m_szFile, oConf.m_szLevel, oConf.m_nAsynMode, oConf.m_nMaxSize
        );

    if(0 == strlen(oConf.m_szPath) || 0 == strlen(oConf.m_szFile) || 0 == strlen(oConf.m_szLevel))
    {
        LOG(LT_ERROR_TRANS, "LOAD_LOG_CONFIG", "invalid string");
        return -21;
    }

    //if(oConf.m_nMaxSize < 5 || oConf.m_nMaxSize > 2*1024) {
    //    LOG(LT_ERROR_TRANS, "LOAD_LOG_CONFIG", "invalid max size");
    //}
        
    oConf.m_nMaxSize = (oConf.m_nMaxSize*1024*1024);
                
    return 0;
}


