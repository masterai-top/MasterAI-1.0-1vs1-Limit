/**
* \file public_conf.h
* \brief 公共配置
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __PUBLIC_CONF__
#define __PUBLIC_CONF__
#include <string>
#include "pch.h"
#include <map>
#include <vector>
#include "frame.h"
using namespace std;

enum {
    DEALER_MODE_AUTO        = 0,    //自动发牌的荷官模式
    DEALER_MODE_PLATFORM    = 1,    //平台发牌的荷官模式
    DEALER_MODE_BLOCKCHAIN  = 2,    //区块链发牌的荷官模式
};


/* 从运行程序名称 获取配置的相关信息
*  例:
   输入:      strExecName = "./acpcsvr_21"    
            1. m_strRunName         = "acpcsvr_21"
            2. m_strConfPath        = "../conf/"
            3. m_strLocalFileName   = "local_21.ini"
            4. m_strDefaultLogPath  = "../logs/"
            5. m_strDefaultLogFile  = "acpc_21"
*/
class CRunConfig
{
private:
    CRunConfig(){}
    
public:
    static CRunConfig* Instance()
    {
        static CRunConfig m_instance;
        return &m_instance;
    }

    bool  Init(const std::string &strExecName);
    const string& GetConfPath();
    const string& GetDefaultLogFile();
    const string& GetDefaultLogPath();
    const string& GetRunName();
    const string& GetLocalFileName();
    const string& GetCfgFileName() { return m_strCfgFileName; }
    
private:
    string   m_strRunName;          // 程序运行名称
    string   m_strConfPath;         // 配置文件路径
    string   m_strLocalFileName;    // local.ini文件名
    string   m_strCfgFileName;      // 配置日志名称
    
    string   m_strDefaultLogPath;   // 默认日志路径
    string   m_strDefaultLogFile;   // 默认日志文件名
};

//荷官信息
/*struct CDealer
{
    uint8   nEnable;                //荷官发牌开关 0:关闭 1:打开
    uint16  nPort;                  //荷官平台服务端口
    char    szAddr[64];             //荷官平台服务地址
    
    CDealer() : nEnable(0), nPort(0) {
        memset(szAddr, 0, sizeof(szAddr));
    }
};

//服务配置
struct CServer
{
    string strAddr;                 //服务IP
    unsigned short nPort;           //服务端口
    unsigned short nBattlePort;     //对战端口(brainsvr专有)

    CServer() { Reset(); }
    void Reset() {
        strAddr.clear();
        nPort       = 0;
        nBattlePort = 0;
    }
};

struct CLocalCfg
{
    string strHostIndex;
};


struct SingleHost
{
    string strAddr;                 //主机IP
    string strPragram;              //该主机运行的服务列表
    unsigned char  nRegID;          //分区ID
    unsigned short nInstID;         //实例ID 
};

struct CHostsCfg
{
    map<string, SingleHost> mapHost;    //主机map

    bool GetHost(const string &key, SingleHost &v);
};*/

//ID区间定义
struct CIDInterval
{   
    uint32 nServerID;               //服务ID
    uint32 nMinID;                  //最小ID
    uint32 nMaxID;                  //最大ID

    CIDInterval():nServerID(0), nMinID(0), nMaxID(0) {}
};

//比赛规则(开关打开时,按配置进行控制. 关闭则配置无效)
struct CMatchRule
{
    uint8  nMatchEnable;            //比赛开关(0:关闭 1:打开)
    uint32 nMatchHands;             //比赛局数
    uint16 nDayResetTimes;          //每天重置次数

    CMatchRule() {
        memset(this, 0, sizeof(CMatchRule));
    }
};

//公共配置
/*struct CPublicConf
{
    vector<CIDInterval> vRobotIDInterval;   // 机器人ID区间
    CMatchRule oMathcRule;                  // 比赛规则

    CLocalCfg    oLocalCfg;
    CHostsCfg    oHostsCfg;
    CServicesCfg oServicesCfg;
};
*/
//日志配置
struct CLogConf
{   
    char           m_szPath[256];           //路径
    char           m_szFile[256];           //文件名
    char           m_szLevel[64];           //级别
    uint8          m_nAsynMode;             //异步标记 1:异步
    unsigned long  m_nMaxSize;              //文件大小 单位(M)

    CLogConf() {
        memset(this, 0, sizeof(CLogConf));
    }
};


/*****************************************************
* 功能: 加载公共配置
* 参数: path路径
* 返回: 0:成功 
*****************************************************/
extern int Load_log_conf(dtscript::IIniIterator *pFilePoint, CLogConf &oConf);

#endif // __PUBLIC_CONF__

