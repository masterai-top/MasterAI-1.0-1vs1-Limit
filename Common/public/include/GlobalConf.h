/**
* \file GlobalConf.h
* \brief 全量配置
*
* Copyright (c) 2019
* All rights reserved.
*
*/

#ifndef __GLOBAL_CONF__
#define __GLOBAL_CONF__
#include <string>
#include "pch.h"
#include "public_conf.h"
#include <vector>
using namespace std;

struct HostsSegment
{
    string strSegment;              //段名
    string strAddr;                 //主机IP
    string strPragram;              //该主机运行的服务列表
    unsigned char  nRegID;          //分区ID
    unsigned short nInstID;         //实例ID 
};


//服务定义
struct DefServer
{
    char ip[65];                    //服务IP
    unsigned short nPort;           //服务端口
    unsigned int   nServerID;       //服务ID
    char szName[64];                //服务名称
    char szFlagParams[128];         //标识参数
    
    DefServer() { memset(this, 0, sizeof(DefServer));  }
};

//定义模块个性化配置
struct DModuleConf
{   
    string              strLogFile;             // 日志文件
};

//全局的配置
struct GlobalConf
{
    string              strModuleName;          //模块名
    string              strLocalFileName;       //local_*.ini配置文件名
    string              strServicesFileName;    //services_*.ini配置文件名
    string              strServerCenterRedis;   //服务中心的redis服务
    string              strDataCenterRedis;     //数据中心的redis服务
    string              strBettingTreeRedis;    //betting tree 的redis地址
    uint32              nBettingTreeRecords;    //betting tree 的记录数

    //Redis密码
    string          strBettingTreeRedisPass;    //strBettingTreeRedis的密码
    
    uint32                  nMaxFd;             //最大支持的fd值
    unsigned char           nDealerEnable;      //荷官发牌开关        0:关闭 1:打开
    unsigned char           nBlockChainEnable;  //区块链[荷官]发牌开关 0:关闭 1:打开
    vector<CIDInterval>     vRobotIDInterval;   //机器人ID区间
    CMatchRule              oMathcRule;         //比赛规则
    DModuleConf             oModuleConf;        //模块的个性化配置

    
    
    vector<DefServer>       vBalanceServer;     //balance服务的监听服务
    vector<DefServer>       vUmServer;          //um服务的监听服务
    vector<DefServer>       vAcpcServer;        //acpc服务的监听服务
    vector<DefServer>       vBrainServer;       //brain服务的监听服务
    vector<DefServer>       vDealerServer;      //荷官服务的监听服务
    vector<DefServer>       vBlockChainServer;  //区块链[荷官]服务的监听服务
    vector<DefServer>       vTpaServer;         //第三方接入服务的监听服务
    vector<DefServer>       vCfrServer;         //cfrsvr
    vector<DefServer>       vAlgApi;            //algapi
    
    uint16                  nLocalBalance;      //本Balance对应的vBalanceServer下标
    uint16                  nLocalUm;           //本Um对应的vUmeServer下标
    uint16                  nLocalAcpc;         //本Acpc对应的vAcpcServer下标
    uint16                  nLocalBrain;        //本Brain对应的vBrainServer下标
    uint16                  nLocalTpa;          //本Tpa对应的vTpaServer下标
    uint16                  nLocalCfr;          //本cfr对应的vCfrServer下标
    uint16                  nLocalAlg;          //本algapi对应的vAlgApi下标
};


//加载全局的配置
extern int load_global_conf(const char *szPath, GlobalConf &oGlobalConf);

#endif // __GLOBAL_CONF__

