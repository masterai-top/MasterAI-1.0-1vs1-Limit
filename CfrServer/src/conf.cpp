/**
* \file conf.cpp
* \brief
*/
#include "pch.h"
#include "conf.h"
#include "system.pb.h"
#include "server_define.h"
/**
* \brief 构造函数
*/
CConf::CConf(void)
{
    m_nTaskThreadCnt = 1;
}

/**
* \brief 析构函数
*/
CConf::~CConf(void)
{

}

/**
* \brief 加载配置
* \param szConfigFile 配置文件
* \return 成功返回true，否则返回false
*/
bool CConf::LoadConfig(const char *szConfigFile)
{
    return ScriptEngine::LoadFile(szConfigFile, this);
}

/**
* \brief csv文件加载回调
* \param szFileName 文件名
* \param pFilePoint 文件数据指针
* \return 成功处理回调返回true，否则返回false
*/
bool CConf::OnFileLoad(const char *szFileName, dtscript::ICsvIterator *pFilePoint)
{
    return false;
}

/**
* \brief ini文件加载回调
* \param szFileName 文件名
* \param pFilePoint 文件数据指针
* \return 成功处理回调返回true，否则返回false
*/
bool CConf::OnFileLoad(const char *szFileName, dtscript::IIniIterator *pFilePoint)
{
    return LoadSvrConfig(szFileName, pFilePoint);
}

/**
* \brief xml文件加载回调
* \param szFileName 文件名
* \param pFilePoint 文件数据指针
* \return 成功处理回调返回true，否则返回false
*/
bool CConf::OnFileLoad(const char *szFileName, dtscript::IXmlIterator *pFilePoint)
{
    return false;
}



/**
* \brief 加载服务器配置
* \param szFileName 文件名
* \param pFilePoint 文件数据指针
* \return 成功返回true，否则返回false
*/
bool CConf::LoadSvrConfig(const char *szFileName, dtscript::IIniIterator *pFilePoint)
{
    if (pFilePoint == NULL)
    {
        return false;
    }

    dtscript::IniVariant &varConfig = (*pFilePoint)["Config"];
    dtscript::IniVariant &varFilePath = (*pFilePoint)["FilePath"];
    dtscript::IniVariant &varVersion = (*pFilePoint)["Version"];
    dtscript::IniVariant &varModel = (*pFilePoint)["Model"];
    dtscript::IniVariant &varUseSumFlag = (*pFilePoint)["SumFlag"];
    dtscript::IniVariant &varIterationList = (*pFilePoint)["Iteration"];

    if (varConfig.Empty() || varVersion.Empty() || varUseSumFlag.Empty()
            || varIterationList.Empty() || varModel.Empty())
    {
        LOG(LT_INFO_TRANS, "LOAD_CONFIG", "Invalid config empty");
        return false;
    }

    m_nTaskThreadCnt        = uint32(varConfig["TaskThreadCnt"]);
    m_nPacketNum            = int32(varConfig["PacketNum"]);
    m_nLimitedLogEnable     = uint8(varConfig["limited_log_enable"]);
    m_nMaxFd                = uint32(varConfig["MaxFd"]);;
    m_nNetThreadCnt         = uint32(varConfig["net_thread_cnt"]);;

    if(0 == m_nPacketNum)
    {
        m_nPacketNum = 3255;
    }

    LOG(LT_INFO_TRANS, "LOAD_CONFIG",
        "Cfr config| task_thread_cnt=%u| packet_num=%d| limited_log_enable=%d| max_fd=%d| net_thread_cnt=%d",
        m_nTaskThreadCnt, m_nPacketNum, m_nLimitedLogEnable, m_nMaxFd, m_nNetThreadCnt
       );


    if(0 == m_nTaskThreadCnt)
    {
        LOG(LT_INFO_TRANS, "LOAD_CONFIG", "Invalid config taskThread cnt");
        return false;
    }

    if(0 == m_nMaxFd)
    {
        LOG(LT_INFO_TRANS, "LOAD_CONFIG", "Invalid config maxfd");
        return false;
    }

    if(0 == m_nNetThreadCnt)
    {
        LOG(LT_INFO_TRANS, "LOAD_CONFIG", "Invalid config netThread cnt");
        return false;
    }

    dtscript::IniVariant &varServer = (*pFilePoint)["Server"];
    if (varServer.Empty())
    {
        LOG(LT_INFO_TRANS, "LOAD_CONFIG", "Invalid server empty");
        return false;
    }
    uint8   nRegID          = uint8(varServer["RegID"]);
    uint8   nInstID         = uint8(varServer["InstID"]);
    m_oServer.nPort         = uint32(varServer["port"]);
    snprintf(m_oServer.ip, sizeof(m_oServer.ip) - 1, "%s", (const char *)(varServer["ip"]));

    SERVERKEY sk(nRegID, Pb::SERVER_TYPE_CFR, nInstID);
    m_oServer.nServerID = sk.ToUint32();
    snprintf(m_oServer.szName, sizeof(m_oServer.szName) - 1, "cfrsvr_%d", nInstID);


    LOG(LT_INFO, "LOAD_CONFIG server=0x%x(%s:%d)",
        CConf::Instance()->m_oServer.nServerID, CConf::Instance()->m_oServer.ip, CConf::Instance()->m_oServer.nPort
       );

    if(0 == CConf::Instance()->m_oServer.nPort || 0 == strlen(CConf::Instance()->m_oServer.ip) || 0 == CConf::Instance()->m_oServer.nServerID)
    {
        LOG(LT_ERROR, "LOAD_CONFIG Invalid server param");
        return -20;
    }

    if(0 != Load_log_conf(pFilePoint, m_oLogConf))
    {
        LOG(LT_INFO_TRANS, "LOAD_CONFIG", "Invalid log config");
        return false;
    }

    m_strCfrDir = (const char *)(varFilePath["cfr_dir"]);
    m_strStaticDir = (const char *)(varFilePath["static_dir"]);
    m_strGameParamPath = (const char *)(varFilePath["game_param_path"]);
    m_strCardParamPath = (const char *)(varFilePath["card_param_path"]);
    m_strBettingParamPath = (const char *)(varFilePath["betting_param_path"]);
    m_strCfrParamPath = (const char *)(varFilePath["cfr_param_path"]);

    int nRandomFlagVal = int32(varModel["random"]);
    m_bRandomFlag = (nRandomFlagVal == 1);

    m_strBettingVer = (const char *)(varVersion["betting"]);
    m_strBucketVer = (const char *)(varVersion["bucket"]);
    m_strCfrVer = (const char *)(varVersion["cfr"]);


    const std::vector<std::string> vecKeys{"preflop", "flop", "turn", "river"};
    m_vecSumprobFlag.resize(vecKeys.size());
    m_vecIter.resize(vecKeys.size());
    for (size_t i = 0; i < vecKeys.size(); i++)
    {
        int nSumFlagVal = int32(varUseSumFlag[vecKeys[i].c_str()]);
        m_vecSumprobFlag[i] = (nSumFlagVal == 1);
        m_vecIter[i] = int32(varIterationList[vecKeys[i].c_str()]);
    }

    char our_version_buf[128] = {0};
    snprintf(our_version_buf, sizeof(our_version_buf) - 1, "%s(%d,%d,%d,%d)",
             CConf::Instance()->m_strCfrVer.c_str(),
             (int)CConf::Instance()->m_vecSumprobFlag[0], (int)CConf::Instance()->m_vecSumprobFlag[1],
             (int)CConf::Instance()->m_vecSumprobFlag[2], (int)CConf::Instance()->m_vecSumprobFlag[3]);

    m_strFinalVersion = std::string(our_version_buf);

    LOG(LT_INFO, "LOAD_CONFIG Model Pram Done!%s", GetConfigInfo().c_str());

    return true;
}

std::string CConf::GetConfigInfo() const
{
    char szFilePathInfoBuf[4096] = {0};

    snprintf(szFilePathInfoBuf, sizeof(szFilePathInfoBuf) - 1,
             "\n{CFRDir=%s\nStaticDir=%s\nGameFile=%s\nCardAbstractionFile=%s\nBettingAbstracionFile=%s\nCfrParamsFile=%s\n}\n",
             m_strCfrDir.c_str(), m_strStaticDir.c_str(), m_strGameParamPath.c_str(),
             m_strCardParamPath.c_str(), m_strBettingParamPath.c_str(),
             m_strCfrParamPath.c_str());

    char szModelBuf[1024] = {0};
    snprintf(szModelBuf, sizeof(szModelBuf) - 1,
             "{Model: RandomFlag=%d SumprobFlag=(%d,%d,%d,%d) Iter=(%d,%d,%d,%d)}\n",
             m_bRandomFlag, m_vecSumprobFlag[0], m_vecSumprobFlag[1], m_vecSumprobFlag[2], m_vecSumprobFlag[3],
             m_vecIter[0], m_vecIter[1], m_vecIter[2], m_vecIter[3]);

    char szVersionBuf[1024] = {0};
    snprintf(szVersionBuf, sizeof(szVersionBuf) - 1,
             "{Version: Betting=%s| Bucket=%s| Cfr=%s| FinalVer=%s}\n",
             m_strBettingVer.c_str(), m_strBucketVer.c_str(), m_strCfrVer.c_str(), m_strFinalVersion.c_str());

    std::string FinalString = std::string(szFilePathInfoBuf)
                              + std::string(szModelBuf) + std::string(szVersionBuf);
    return FinalString;
}


