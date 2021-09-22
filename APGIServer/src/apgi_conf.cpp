/**
* \file brain_conf.cpp
* \brief AI服务器配置类实现函数
*/

#include "pch.h"
#include "apgi_conf.h"

int SetTAction(int round, Action *pAction, string &sAction)
{
    if(round >= 4)
    {
        return -101;
    }

    char szAction[128] = {0};
    snprintf(szAction, sizeof(szAction) - 1, "%s", sAction.c_str());

    char *pStart = szAction;
    int index = 0;
    while(0 != *pStart && index < MAX_ROBOT_ACTION_CNT)
    {
        if('c' == *pStart)
        {
            pAction[index].type = a_call;
            index++;
        }
        else if('f' == *pStart)
        {
            pAction[index].type = a_fold;
            return 0;
        }
        else if('r' == *pStart)
        {
            pAction[index].type = a_raise;
            pAction[index].size = atoi(pStart + 1);
            index++;
        }

        pStart++;
    }

    if(index > 0)
    {
        return 0;
    }

    return -99;
}

/**
* \brief 构造函数
*/
CAPGIConf::CAPGIConf(void)
{
}

/**
* \brief 析构函数
*/
CAPGIConf::~CAPGIConf(void)
{
    for(map<string, CFRServer *>::iterator it = m_mapCfrServer.begin(); it != m_mapCfrServer.end(); it++)
    {
        delete it->second;
    }
    m_mapCfrServer.clear();
}

string CAPGIConf::GetCfrVersion(const string &cfrModel)
{
    map<string, CFRServer *>::iterator it = m_mapCfrServer.find(cfrModel);
    if(it != m_mapCfrServer.end())
    {
        return it->second->GetVersion();
    }

    return "";
}

CFRServer *CAPGIConf::GetCfrServer(const string &cfrModel)
{
    map<string, CFRServer *>::iterator it = m_mapCfrServer.find(cfrModel);
    if(it != m_mapCfrServer.end())
    {
        return it->second;
    }

    return NULL;
}


//nGameBeginTimes: 开局次数, 混合模型时使用
string  CAPGIConf::SelectCfrModel(int32 nAPGAiMode, uint32 nGameBeginTimes)
{
    if(0 == m_vCfrServer.size())
    {
        return "";
    }
    else if(1 == m_vCfrServer.size())
    {
        return m_vCfrServer[0].szCfrModel;
    }

    if(G_APG_AI_MODE_L1 == nAPGAiMode)
    {
        return m_vCfrServer[0].szCfrModel;
    }
    else if(G_APG_AI_MODE_L2 == nAPGAiMode)
    {
        return m_vCfrServer[1].szCfrModel;
    }
    else
    {
        uint32 nTempTimes = nGameBeginTimes % 10;
        if(nTempTimes < 3)
        {
            return m_vCfrServer[0].szCfrModel;
        }
        else
        {
            return m_vCfrServer[1].szCfrModel;
        }
    }
}

string  CAPGIConf::SelectValidModel(const string &strInvalidModel)
{
    for(uint32 i = 0; i < m_vCfrServer.size(); i++)
    {
        if(strInvalidModel != string(m_vCfrServer[i].szCfrModel))
        {
            return m_vCfrServer[i].szCfrModel;
        }
    }

    return "";
}


/**
* \brief 加载配置
* \param szConfigFile 配置文件
* \return 成功返回true，否则返回false
*/
bool CAPGIConf::LoadConfig(const char *szConfigFile)
{
    return ScriptEngine::LoadFile(szConfigFile, this);
}

/**
* \brief csv文件加载回调
* \param szFileName 文件名
* \param pFilePoint 文件数据指针
* \return 成功处理回调返回true，否则返回false
*/
bool CAPGIConf::OnFileLoad(const char *szFileName, dtscript::ICsvIterator *pFilePoint)
{
    return false;
}

/**
* \brief ini文件加载回调
* \param szFileName 文件名
* \param pFilePoint 文件数据指针
* \return 成功处理回调返回true，否则返回false
*/
bool CAPGIConf::OnFileLoad(const char *szFileName, dtscript::IIniIterator *pFilePoint)
{
    return LoadSvrConfig(szFileName, pFilePoint);
}

/**
* \brief xml文件加载回调
* \param szFileName 文件名
* \param pFilePoint 文件数据指针
* \return 成功处理回调返回true，否则返回false
*/
bool CAPGIConf::OnFileLoad(const char *szFileName, dtscript::IXmlIterator *pFilePoint)
{
    return false;
}

/**
* \brief 加载服务器配置
* \param szFileName 文件名
* \param pFilePoint 文件数据指针
* \return 成功返回true，否则返回false
*/
bool CAPGIConf::LoadSvrConfig(const char *szFileName, dtscript::IIniIterator *pFilePoint)
{
    if (pFilePoint == NULL)
    {
        LOG(LT_ERROR_TRANS, "LOAD_CONFIG", "Invalid file name");
        return false;
    }

    IniVariant &varConfig       = (*pFilePoint)["Config"];
    IniVariant &varInterface    = (*pFilePoint)["Interface"];
    IniVariant &varBettingTree  = (*pFilePoint)["betting_tree"];
    IniVariant &varServer       = (*pFilePoint)["Server"];

    if (varConfig.Empty() || varInterface.Empty() || varBettingTree.Empty() || varServer.Empty())
    {
        LOG(LT_ERROR_TRANS, "LOAD_CONFIG", "Invalid config| segment");
        return false;
    }

    //业务控制
    m_nTaskThreadCnt       = uint32(varConfig["TaskThreadCnt"]);
    m_nPacketNum           = int32(varConfig["PacketNum"]);
    m_nTimerObjectNum      = uint32(varConfig["timer_object_num"]);
    m_nRunGameCnt          = uint32(varConfig["run_game_cnt"]);

    //网络配置
    m_nNetThreadCnt        = uint32(varConfig["net_thread_cnt"]);
    m_nTCPTimeOut          = uint32(varConfig["TCPTimeout"]);
    m_nDataTimeout         = uint32(varConfig["DataTimeout"]);
    m_nMaxFd               = uint32(varConfig["max_fd"]);
    m_nConnObjectCnt       = uint32(varConfig["conn_object_cnt"]);
    m_nRobotTimeout        = uint32(varConfig["robot_timeout"]);
    m_nPacketTimeout       = uint32(varConfig["packet_timeout"]);

    m_nLimitedLogEnable    = uint8(varConfig["limited_log_enable"]);
    m_strHistoryRedis      = (const char *)(varConfig["history_redis"]);
    m_nUnsaveHistory       = uint8(varConfig["un_save_history"]);

    //datasvr服务配置
    m_nDataConnCnt         = uint32(varConfig["data_conn_cnt"]);
    uint8 nDataServerCnt   = uint8(varConfig["data_server_cnt"]);
    for(uint8 i = 0; i < nDataServerCnt; i++)
    {
        string sSegment = "DATA_SERVER_" + to_string(i + 1);

        IniVariant &varApg  = (*pFilePoint)[sSegment.c_str()];
        if (varApg.Empty())
        {
            LOG(LT_ERROR, "LOAD_CONFIG| Invalid config| [GPG]segment(%s)", sSegment.c_str());
            return false;
        }

        DefServer oDataServer;
        uint8   nInstID         = uint8(varApg["InstID"]);
        string  sip             = (const char *)(varApg["ip"]);
        SERVERKEY ask(1, Pb::SERVER_TYPE_DATA, nInstID);

        oDataServer.nPort        = uint16(varApg["port"]);
        oDataServer.nServerID    = ask.ToUint32();
        snprintf(oDataServer.ip, sizeof(oDataServer.ip) - 1, "%s", sip.c_str());
        m_vDataServer.push_back(oDataServer);


        LOG(LT_INFO, "LOAD_CONFIG| Data Server| segment=%s| server_id=0x%x| addr=%s:%d", sSegment.c_str(), oDataServer.nServerID, oDataServer.ip, oDataServer.nPort);
    }

    //本模块Server配置信息
    uint8   nLInstID        = uint8(varServer["InstID"]);
    string  sip             = (const char *)(varServer["ip"]);
    SERVERKEY lsk(1, Pb::SERVER_TYPE_APGI, nLInstID);
    m_oServer.nServerID     = lsk.ToUint32();
    m_oServer.nPort         = uint16(varServer["port"]);
    snprintf(m_oServer.ip, sizeof(m_oServer.ip) - 1, "%s", sip.c_str());

    //betting tree配置
    m_nBettingTreeRecords       = (uint32)(varBettingTree["records"]);
    m_strBettingTreeRedis       = (const char *)(varBettingTree["addr"]);
    m_strBettingTreeRedisPass   = (const char *)(varBettingTree["pass"]);
    m_strBettingTreeFile        = (const char *)(varBettingTree["data_file"]);

    //cfr配置相关
    uint8 nCfrServerCnt    = uint8(varConfig["cfr_server_cnt"]);
    m_nCfrConnCnt          = uint32(varConfig["cfr_conn_cnt"]);


    m_oTModelParam.game_param_path       = (const char *)(varInterface["GamePram"]);
    m_oTModelParam.card_param_path       = (const char *)(varInterface["CardParam"]);
    m_oTModelParam.static_dir            = (const char *)(varInterface["StaticDir"]);
    uint32 tmp_val                       = 0;
    m_oTModelParam.random_flag           = (tmp_val == 1);

    //CFR服务配置
    for(uint8 i = 0; i < nCfrServerCnt; i++)
    {
        string sSegment = "CFR_SERVER_" + to_string(i + 1);

        IniVariant &varCfr  = (*pFilePoint)[sSegment.c_str()];
        if (varCfr.Empty())
        {
            LOG(LT_ERROR, "LOAD_CONFIG| Invalid config| [cfr]segment(%s)", sSegment.c_str());
            return false;
        }

        CFRServer *pCfr = new CFRServer;
        if(NULL == pCfr)
        {
            LOG(LT_ERROR, "LOAD_CONFIG| new cfr server failed");
            return false;
        }

        pCfr->oModelParam = m_oTModelParam;


        string  strKeyVersion       = (const char *)(varCfr["version"]);
        string  strCfrParam         = (const char *)(varCfr["CfrParam"]);
        string strSumprobFlag       = (const char *)(varCfr["SumprobFlag"]);

        uint8   nInstID             = uint8(varCfr["InstID"]);
        SERVERKEY sk(1, Pb::SERVER_TYPE_CFR, nInstID);


        //subprob标签处理
        if('1' == strSumprobFlag.at(0))
        {
            pCfr->oModelParam.sumprob_flag[0] = true;
        }
        if('1' == strSumprobFlag.at(2))
        {
            pCfr->oModelParam.sumprob_flag[1] = true;
        }
        if('1' == strSumprobFlag.at(4))
        {
            pCfr->oModelParam.sumprob_flag[2] = true;
        }
        if('1' == strSumprobFlag.at(6))
        {
            pCfr->oModelParam.sumprob_flag[3] = true;
        }

        char szCfrVersion[128] = {0};
        snprintf(szCfrVersion, sizeof(szCfrVersion) - 1, "\"%s(%d,%d,%d,%d)\"",
                 strKeyVersion.c_str(), pCfr->oModelParam.sumprob_flag[0], pCfr->oModelParam.sumprob_flag[1], pCfr->oModelParam.sumprob_flag[2], pCfr->oModelParam.sumprob_flag[3]
                );
        pCfr->strVersion              = szCfrVersion;
        pCfr->nServerID               = sk.ToUint32();
        pCfr->oModelParam.cfr_param   = strCfrParam;
        m_mapCfrServer[strKeyVersion] = pCfr;

        DefServer oCfrServer;
        string  strIP           = (const char *)(varCfr["ip"]);
        oCfrServer.nPort        = uint16(varCfr["port"]);
        oCfrServer.nServerID    = sk.ToUint32();
        snprintf(oCfrServer.ip, sizeof(oCfrServer.ip) - 1, "%s", strIP.c_str());
        snprintf(oCfrServer.szCfrModel, sizeof(oCfrServer.szCfrModel) - 1, "%s", strKeyVersion.c_str());
        m_vCfrServer.push_back(oCfrServer);

        LOG(LT_INFO, "LOAD_CONFIG| CFR Server| segment=%s| server_id=0x%x| addr=%s:%d| version=%s| game=%s| card=%s| static=%s| random=%d| cfr_version=%s| cfr_param=%s",
            sSegment.c_str(), pCfr->nServerID, oCfrServer.ip, oCfrServer.nPort, strKeyVersion.c_str(), pCfr->oModelParam.game_param_path.c_str(), pCfr->oModelParam.card_param_path.c_str(),
            pCfr->oModelParam.static_dir.c_str(),  pCfr->oModelParam.random_flag,
            pCfr->GetVersion().c_str(), pCfr->oModelParam.cfr_param.c_str());
    }

    if(0 == m_nTimerObjectNum)
    {
        m_nTimerObjectNum = 50000;
    }

    if(m_nPacketTimeout < 2000)
    {
        m_nPacketTimeout = 2000;
    }

    if(0 == m_nRobotTimeout)
    {
        m_nRobotTimeout = 2;
    }


    int ret = LoadTRobotAction(pFilePoint);
    if(0 != ret)
    {
        LOG(LT_ERROR_TRANS, "LOAD_CONFIG", "Load TRobot action config| rc=%d", ret);
        return false;
    }

    LOG(LT_INFO_TRANS, "LOAD_CONFIG",
        "APGI| server_id=0x%x(%s:%d)| task_thread_cnt=%u| packet_num=%d| tcpTimeOut=%u| dataTimeout=%u| timer_object_num=%u| data_conn_cnt=%d| net_thread_cnt=%d| limited_log_enable=%d"
        "betting_tree_redis=[%s][%d][%s][%s]"
        "cfr_server_cnt=%d| data_server_cnt=%d| max_fd=%d| cfr_conn_cnt=%d| conn_object_cnt=%d| run_game_cnt=%d| TRobot_id=%llu| "
        "un_save_history=%d| robot_timeout=%u| packet_timeout=%d| history_redis=%s",
        m_oServer.nServerID, m_oServer.ip, m_oServer.nPort, m_nTaskThreadCnt, m_nPacketNum, m_nTCPTimeOut, m_nDataTimeout, m_nTimerObjectNum, m_nDataConnCnt, m_nNetThreadCnt, m_nLimitedLogEnable,
        m_strBettingTreeRedis.c_str(), m_nBettingTreeRecords, m_strBettingTreeRedisPass.c_str(), m_strBettingTreeFile.c_str(),
        m_vCfrServer.size(),  m_vDataServer.size(), m_nMaxFd, m_nCfrConnCnt, m_nConnObjectCnt, m_nRunGameCnt, m_oTRobotAction.nRobotID,
        m_nUnsaveHistory, m_nRobotTimeout, m_nPacketTimeout, m_strHistoryRedis.c_str()
       );

    //线程数控制.确保每个Uid的消息,都在同一个线程处理
    if(m_nPacketNum < 5000 || 0 == m_nTaskThreadCnt || 0 == m_nRunGameCnt || m_nTaskThreadCnt > (QUEUE_SEND - QUEUE_TASK - 1))
    {
        LOG(LT_ERROR_TRANS, "LOAD_CONFIG", "Invalid config| base");
        return false;
    }

    if(m_nTCPTimeOut < 30 || m_nDataTimeout < 30 || 0 == m_nDataConnCnt || 0 == m_nNetThreadCnt || 0 == m_nMaxFd || 0 == m_nCfrConnCnt || m_nConnObjectCnt < 1000)
    {
        LOG(LT_ERROR_TRANS, "LOAD_CONFIG", "Invalid config| Network config");
        return false;
    }

    if(m_strBettingTreeFile.empty() || m_strBettingTreeRedis.empty() || m_strBettingTreeRedisPass.empty()  || 0 == m_nBettingTreeRecords)
    {
        LOG(LT_ERROR_TRANS, "LOAD_CONFIG", "Invalid config| Redis config");
        return false;
    }

    if(0 == m_oServer.nServerID || 0 == m_oServer.nPort || 0 == strlen(m_oServer.ip)  || 0 == m_vCfrServer.size() || 0 == m_vDataServer.size())
    {
        LOG(LT_ERROR_TRANS, "LOAD_CONFIG", "Invalid config| Server config");
        return false;
    }

    if(0 != Load_log_conf(pFilePoint, m_oLogConf))
    {
        LOG(LT_ERROR_TRANS, "LOAD_CONFIG", "Invalid log config");
        return false;
    }



    return true;
}

int CAPGIConf::LoadTRobotAction(dtscript::IIniIterator *pFilePoint)
{
    if (pFilePoint == NULL)
    {
        LOG(LT_ERROR_TRANS, "LOAD_LOG_CONFIG", "Local iterator ptr is null");
        return -1;
    }

    //比赛规则
    IniVariant &varTRobot  = (*pFilePoint)["TestRobot"];
    if(varTRobot.Empty())
    {
        return 0;
    }

    memset(&m_oTRobotAction, 0, sizeof(m_oTRobotAction));
    m_oTRobotAction.nRobotID = uint64(varTRobot["robot_id"]);
    if(0 == m_oTRobotAction.nRobotID)
    {
        return 0;
    }


    int ret = 0;
    string sAtion  = (const char *)(varTRobot["0action"]);
    if(0 != (ret = SetTAction(0, m_oTRobotAction.oAction0, sAtion)))
    {
        return -1000 + ret;
    }

    sAtion  = (const char *)(varTRobot["1action"]);
    if(0 != (ret = SetTAction(1, m_oTRobotAction.oAction1, sAtion)))
    {
        return -2000 + ret;
    }

    sAtion  = (const char *)(varTRobot["2action"]);
    if(0 != (ret = SetTAction(2, m_oTRobotAction.oAction2, sAtion)))
    {
        return -3000 + ret;
    }

    sAtion  = (const char *)(varTRobot["3action"]);
    if(0 != (ret = SetTAction(3, m_oTRobotAction.oAction3, sAtion)))
    {
        return -4000 + ret;
    }


    char szAction[512] = {0};
    int len = 0;
    len += sprintf(szAction, "%s", "0action=");
    for(int j = 0; j < MAX_ROBOT_ACTION_CNT; j++)
    {
        len += sprintf(szAction + len, "%d:%d ",  m_oTRobotAction.oAction0[j].type, m_oTRobotAction.oAction0[j].size);
        if(a_fold == m_oTRobotAction.oAction0[j].type)
        {
            break;
        }
    }

    len += sprintf(szAction + len, "%s", "1action=");
    for(int j = 0; j < MAX_ROBOT_ACTION_CNT; j++)
    {
        len += sprintf(szAction + len, "%d:%d ",  m_oTRobotAction.oAction1[j].type, m_oTRobotAction.oAction1[j].size);
        if(a_fold == m_oTRobotAction.oAction1[j].type)
        {
            break;
        }
    }

    len += sprintf(szAction + len, "%s", "2action=");
    for(int j = 0; j < MAX_ROBOT_ACTION_CNT; j++)
    {
        len += sprintf(szAction + len, "%d:%d ",  m_oTRobotAction.oAction2[j].type, m_oTRobotAction.oAction2[j].size);
        if(a_fold == m_oTRobotAction.oAction2[j].type)
        {
            break;
        }
    }

    len += sprintf(szAction + len, "%s", "3action=");
    for(int j = 0; j < MAX_ROBOT_ACTION_CNT; j++)
    {
        len += sprintf(szAction + len, "%d:%d ",  m_oTRobotAction.oAction3[j].type, m_oTRobotAction.oAction3[j].size);
        if(a_fold == m_oTRobotAction.oAction3[j].type)
        {
            break;
        }
    }

    LOG(LT_INFO, "LOAD_CONFIG| TROBOT_ACTIONS| robot_id=%lld| actions=%s", m_oTRobotAction.nRobotID, szAction);

    return 0;
}



bool CompareServer(const DefServer &s1, const DefServer &s2)
{
    return s1.nServerID < s2.nServerID; //由小到大排序
}


/*int apgi_load_conf(const char *szConfPath, const char *szFileName)
{
    //加载模块配置的配置
    string  strApgiCfg = string(szConfPath) + string(szFileName);
    if(!CAPGIConf::Instance()->LoadConfig(strApgiCfg.c_str()))
    {
        LOG(LT_ERROR_TRANS, "LOAD_CONFIG", "Load config failed| file=%s", strApgiCfg.c_str());
        return -22;
    }

    //排序
    //sort(CAPGIConf::Instance()->m_vAPGServer.begin(), CAPGIConf::Instance()->m_vAPGServer.end(), CompareServer);
    //for(uint32 i = 0; i < CAPGIConf::Instance()->m_vAPGServer.size(); i++) {
    //    LOG(LT_INFO, "Sort APG server| server_id=0x%x| ip=%s:%d", CAPGIConf::Instance()->m_vAPGServer[i].nServerID, CAPGIConf::Instance()->m_vAPGServer[i].ip, CAPGIConf::Instance()->m_vAPGServer[i].nPort);
    //}

    sort(CAPGIConf::Instance()->m_vCfrServer.begin(), CAPGIConf::Instance()->m_vCfrServer.end(), CompareServer);
    for(uint32 i = 0; i < CAPGIConf::Instance()->m_vCfrServer.size(); i++) {
        LOG(LT_INFO, "Sort APG server| server_id=0x%x| ip=%s:%d", CAPGIConf::Instance()->m_vCfrServer[i].nServerID, CAPGIConf::Instance()->m_vCfrServer[i].ip, CAPGIConf::Instance()->m_vCfrServer[i].nPort);
    }

    return 0;
}*/


