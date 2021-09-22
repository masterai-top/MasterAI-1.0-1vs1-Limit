/**
* \file brain_svr.cpp
* \brief AI服务器类实现函数
*/

#include "pch.h"
#include "apgi_svr.h"
#include <signal.h>
#include "version.h"
#include "define.h"
#include "apgi_timer.h"
#include "run_game_hashmap.h"

#include "dt_robot.h"
using namespace network;

/**
* \brief 构造函数
*/
CAPGISvr::CAPGISvr(void)
{

}

/**
* \brief 析构函数
*/
CAPGISvr::~CAPGISvr(void)
{
}

/**
* \brief 服务初始化
* \return 初始化成功返回true，否则返回false
*/
bool CAPGISvr::OnInit()
{
    srand(time(NULL));

    //默认日志名文件的方式初始化
    if(0 != shstd::log::Init(CRunConfig::Instance()->GetDefaultLogPath().c_str(), CRunConfig::Instance()->GetDefaultLogFile().c_str(), "debug,info,warn,error", "default"))
    {
        printf("Log init failed| default_log_file=%s/%s.log\r\n", CRunConfig::Instance()->GetDefaultLogPath().c_str(), CRunConfig::Instance()->GetDefaultLogFile().c_str());
        return false;
    }

    // 加载启动配置文件
    // 加载启动配置文件
    std::string strCfg = CRunConfig::Instance()->GetConfPath() + CRunConfig::Instance()->GetLocalFileName();
    if(!CAPGIConf::Instance()->LoadConfig(strCfg.c_str()))
    {
        printf("Load config failed [%s]\r\n", strCfg.c_str());
        LOG(LT_ERROR_TRANS, "OnInit", "load config [ %s ] failed", strCfg.c_str());
        return false;
    }

    /*int nRet = apgi_load_conf(CRunConfig::Instance()->GetConfPath().c_str(), CRunConfig::Instance()->GetLocalFileName().c_str());
    if (0 != nRet)
    {
        printf("Load config failed [%s/%s]\r\n", CRunConfig::Instance()->GetConfPath().c_str(), CRunConfig::Instance()->GetLocalFileName().c_str());
        LOG(LT_ERROR_TRANS, "OnInit", "load config [ %s/%s ] failed| rc=%d", CRunConfig::Instance()->GetConfPath().c_str(), CRunConfig::Instance()->GetLocalFileName().c_str(), nRet);
        return false;
    }*/

    //配置日志名文件的方式初始化
    if(0 != shstd::log::Init(CAPGIConf::Instance()->m_oLogConf.m_szPath, CAPGIConf::Instance()->m_oLogConf.m_szFile, CAPGIConf::Instance()->m_oLogConf.m_szLevel, shstd::fun::GetRunUser()))
    {
        printf("Log init from config failed| default_log_file=%s/%s.log\r\n", CAPGIConf::Instance()->m_oLogConf.m_szPath, CAPGIConf::Instance()->m_oLogConf.m_szFile);
        return false;
    }


    //LOG(LT_INFO_TRANS, "APGI OnInit", "Server Start| ver=%s", GetSvrVersionInfo());

    //异步日志
    if(1 == CAPGIConf::Instance()->m_oLogConf.m_nAsynMode && shstd::log::SetAsynMode())
    {
        LOG(LT_INFO_TRANS, "APGI OnInit", "Log mode is asyn");;
    }

    //日志文件大小
    if(CAPGIConf::Instance()->m_oLogConf.m_nMaxSize > 5 * 1024 * 1024)
    {
        shstd::log::SetLogMaxSize(CAPGIConf::Instance()->m_oLogConf.m_nMaxSize);
    }

    //队列
    if(0 != CQUEUE_List::Instance()->CreateQueue(CAPGIConf::Instance()->m_nPacketNum))
    {
        printf("create net queue list failed\r\n");
        LOG(LT_ERROR_TRANS, "APGI_OnInit", "Create Queue failed!");
        return false;
    }

    //定时器管理器初始化[预分配定时器对象]
    if(!CTimerManager::Instance()->Init(CAPGIConf::Instance()->m_nTimerObjectNum))
    {
        LOG(LT_ERROR_TRANS, "APGI OnInit", "Timer manager init failed\r\n");
        printf("Timer manager init failed\r\n");
        return false;
    }

    //Game管理器初始化
    if(!CGameManager::Instance()->LoadConfig())
    {
        printf("Load game failed\r\n");
        LOG(LT_ERROR_TRANS, "ON_INIT", "Load GameConfig failed!");
        return false;
    }

    //cfr&data 服务的处理接口对象
    CCfrTask::Instance()->Init();
    CDataTask::Instance()->Init();

    //定时器回调对象
    CTimerRunGame::Instance()->Init();

    //Game对象的HashMap
    CRunGameHashMap::Instance()->Init(CAPGIConf::Instance()->m_nRunGameCnt, CAPGIConf::Instance()->m_nRunGameCnt);

    //全局ID锁
    if(!g_oRobotIDLocks.Init(10000) )
    {
        LOG(LT_ERROR_TRANS, "APGI OnInit", "RobotID locks init failed\r\n");
        printf("RobotID locks init failed\r\n");
        return false;
    }

    //BettingTree初始化. 将./data/NonTerminalBettingTree.csv文件初始化值内存
    if(!CBettingTree::Instance()->Init(CAPGIConf::Instance()->m_nBettingTreeRecords,
                                       CAPGIConf::Instance()->m_strBettingTreeRedis,
                                       CAPGIConf::Instance()->m_strBettingTreeRedisPass,
                                       CAPGIConf::Instance()->m_nTaskThreadCnt + 3,
                                       CAPGIConf::Instance()->m_strBettingTreeFile)
      )
    {
        printf("Betting tree init  failed\r\n");
        LOG(LT_ERROR_TRANS, "ON_INIT", "Betting tree Init failed");
        return false;
    }

    //算法相关库的初始化[源码由算法提供]
    int flag = DTRobot::Init(CAPGIConf::Instance()->m_oTModelParam);
    if (0 != flag)
    {
        printf("Init InterFace Failed err=%d\r\n", flag);
        LOG(LT_ERROR_TRANS, "ON_INIT", "Init InterFace Failed err=%d!", flag);
        return false;
    }

    // 创建网络环境
    if (!NetEngine::Create(CAPGIConf::Instance()->m_oServer.nServerID,
                           CONN_DEFAULT_TIMEOUT, CAPGIConf::Instance()->m_nMaxFd,
                           CAPGIConf::Instance()->m_nNetThreadCnt,
                           CAPGIConf::Instance()->m_nLimitedLogEnable,
                           CAPGIConf::Instance()->m_nTaskThreadCnt      //需要控制每个同一个机器人的消息,都在同一线程处理
                          )
       )
    {
        printf("Create socket manager failed\r\n");
        LOG(LT_ERROR_TRANS, "ON_INIT", "create socket manager failed!");
        return false;
    }

    //初始化Redis
    {
        try
        {
            if(!g_oHistoryRedis.Init(CAPGIConf::Instance()->m_strHistoryRedis, CAPGIConf::Instance()->m_nTaskThreadCnt))
            {
                LOG(LT_ERROR, "ON_INIT| History redis init failed! redis=%s", CAPGIConf::Instance()->m_strHistoryRedis.c_str());
                printf("ON_INIT| History redis init failed! redis=%s", CAPGIConf::Instance()->m_strHistoryRedis.c_str());
                return false;
            }
        }
        catch(Exception &e)
        {
            LOG(LT_ERROR, "ON_INIT| History  redis init exception| msg=%s\r\n", e.what());
            printf("ON_INIT| History  redis init exception| msg=%s\r\n", e.what());
            return false;
        }
        catch(...)
        {
            LOG(LT_ERROR, "ON_INIT| History redis init exception all\r\n");
            printf("ON_INIT| History redis init exception all\r\n");
            return false;
        }
    }

    pthread_t tid = 0;
    uint32 nThrSeq[500] = {0};
    for(uint32 i = 0; i < CAPGIConf::Instance()->m_nTaskThreadCnt; i++)
    {
        nThrSeq[i] = i;
        if(0 != pthread_create( &tid, NULL, &CAPGISvr::TaskIt, (void *)&nThrSeq[i]))
        {
            printf("create task thread failed\r\n");
            LOG(LT_ERROR_TRANS, "APGI_OnInit", "Task init failed| index=%u", i);
            return false;
        }
    }

    LOG(LT_DEBUG_TRANS, "APGI_OnInit", "Task init succ");

    //初始化datasvr连接器
    {
        if(!g_oDataConnector.Init(CAPGIConf::Instance()->m_nDataConnCnt, CAPGIConf::Instance()->m_vDataServer, CAPGIConf::Instance()->m_oServer,
                                  CTimerManager::Instance(),
                                  Pb::CMD_DATA_REGISTER,       //注册命令
                                  Pb::CMD_DATA_KEEP_ALIVE,     //心跳命令
                                  TIMER_ID_DATA_KEEPALIVE,            //心跳定时器ID
                                  CONN_KEEP_ALIVE_TIME,               //心跳间隔
                                  TIMER_ID_DATA_RECONNECT,            //重连定时器ID
                                  CONN_RECONNECT_TIME,                //重连间隔
                                  CONN_DEFAULT_TIMEOUT,               //重连超时
                                  "APGI_TO_DATA_CONNECTOR",           //日志标记
                                  TCP_HEAD_TYPE_DEFAULT
                                 ) )
        {
            printf("Init acpc connector failed\r\n");
            LOG(LT_ERROR, "Init acpc connector failed");
            return false;
        }
    }

    //初始化cfr连接器
    {
        if(!g_oCfrConnector.Init(CAPGIConf::Instance()->m_nCfrConnCnt, CAPGIConf::Instance()->m_vCfrServer, CAPGIConf::Instance()->m_oServer,
                                 CTimerManager::Instance(),
                                 0,                                  //注册命令
                                 0,                                  //心跳命令
                                 TIMER_ID_CFR_KEEPALIVE,             //心跳定时器ID
                                 CONN_KEEP_ALIVE_TIME,               //心跳间隔
                                 TIMER_ID_CFR_RECONNECT,             //重连定时器ID
                                 CONN_RECONNECT_TIME,                //重连间隔
                                 CONN_DEFAULT_TIMEOUT,               //重连超时
                                 "APGI_TO_CFR_CONNECTOR",          //日志标记
                                 TCP_HEAD_TYPE_STRING
                                ))
        {
            printf("Init cfr connector failed\r\n");
            LOG(LT_ERROR, "Init cfr connector failed");
            return false;
        }
    }

    //启动监听
    if (!CSocketManager::Instance()->Listen(CAPGIConf::Instance()->m_oServer.ip, CAPGIConf::Instance()->m_oServer.nPort, TCP_HEAD_TYPE_APGI))
    {
        printf("Listen  failed ip=%s:%d| msg=%s\r\n", CAPGIConf::Instance()->m_oServer.ip, CAPGIConf::Instance()->m_oServer.nPort, strerror(errno));
        LOG(LT_ERROR_TRANS, "ON_INIT", "socket listen failed! addr=%s:%d| msg=%s", CAPGIConf::Instance()->m_oServer.ip, CAPGIConf::Instance()->m_oServer.nPort, strerror(errno));
        return false;
    }


    LOG(LT_INFO_TRANS, "ON_INIT", "start server success!");
    printf("APGI server start success...\r\n");
    return true;
}

/**
* \brief 服务释放
*/
void CAPGISvr::OnRestore()
{
    // 释放socket管理器
    CSocketManager::Instance()->Release();
    sleep(1);

    g_oDataConnector.Restore();
    g_oCfrConnector.Restore();

    CCfrTask::Instance()->Restore();
}

/**
* \brief 信号事件
* \param nSignalID 信号ID
*/
void CAPGISvr::OnSignal(int32 nSignalID)
{
    switch (nSignalID)
    {
    case SIGUSR1:
        break;

    default:
        break;
    }
}

void *CAPGISvr::TaskIt( void *arg )
{
    LISTSMG *p;
    pthread_detach( pthread_self() );

    const uint32 qIndex = (*((uint32 *)arg));
    if(qIndex >= CAPGIConf::Instance()->m_nTaskThreadCnt)
    {
        LOG(LT_ERROR, "APGISvr TaskIt invalid q_index=%d| pid=%u", qIndex, pthread_self());
        _exit(0);
    }

    LOG(LT_INFO_TRANS, "Init", "APGISvr TaskIt thread Start| pid=%u| q_index=%d", pthread_self(), qIndex);
    int num = 0;
    for ( ;; )
    {
        p = CQUEUE_List::Instance()->GetBlockNode(QUEUE_TASK + qIndex, &num);
        if (NULL == p)
        {
            usleep(5);
            continue;
        }

        if(p->recv_time.ToNow() > (int)CAPGIConf::Instance()->m_nPacketTimeout)
        {
            LOG(LT_ERROR_TRANS, p->szTransID, "TaskIt| Timeout packet queue_num=%d| time=%d", num, p->recv_time.ToNow());
            CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
            continue;
        }


        /*延时处理的类型报文
        * 1. 无其他任务时, 休眠100微妙再处理, 避免CPU使用率过高
        * 2. 超过延时时间. 需要在处理函数返回
        * 3. 其他情况, 每n次才执行一次              */
        /*if(PKT_TYPE_DELAY_DONE == p->cPacketType)
        {
            if(0 == num)  //1. 无其他任务时, 休眠10微妙再处理, 避免CPU使用率过高
            {
                usleep(10);
            }
            else if(p->recv_time.ToNow() > (int)p->nDelayTime)   //2. 超过延时时间. 需要在处理函数返回
            {
                ;
            }
            else if(0 != p->nDelayTimes%10)                  //3. 其他情况, 每n次才执行一次
            {
                p->nDelayTimes++;
                CQUEUE_List::Instance()->SetNode(p, QUEUE_TASK + qIndex);
                continue;
            }
        }*/

        if(num > 10000)
        {
            LOG(LT_WARN_TRANS, p->szTransID, "TaskIt| Recv packet queue_num=%d", num);
        }

        switch(p->nCmd)
        {
        //[cfr]非二级制命令
        case Pb::CMD_COMM_NOT_BINARY:
            CCfrTask::Instance()->DoProc(p);
            break;

        //--------------------- 以下来自APG的消息 -------------------
        case Pb::THIRD_CMD_GAME_BEGIN_NOTIFY:
            CAPGTask::Instance()->GameBeginNotify(p);
            break;

        case Pb::THIRD_CMD_GAME_SENDCARD_NOTIFY:
            CAPGTask::Instance()->GameSendCardsNotify(p);
            break;

        //case Pb::THIRD_CMD_GAME_TOKENTO_NOTIFY:
        //    CAPGTask::Instance()->GameTokenNotify(p);
        //    break;

        case Pb::THIRD_CMD_GAME_ACTION_NOTIFY:
            CAPGTask::Instance()->GameActionNotify(p);
            break;

        case Pb::THIRD_CMD_GAME_RESULT_NOTIFY:
            CAPGTask::Instance()->GameResultNotify(p);
            break;

        case Pb::THIRD_CMD_ROBOT_ACTION_REQ:
            CAPGTask::Instance()->GameRobotActionReq(p, qIndex);
            break;

        case Pb::THIRD_CMD_GAME_STATDATA_REQ:
            CAPGTask::Instance()->UserStatDataReq(p);
            break;

        case Pb::THIRD_CMD_GAME_QUERY_STATE_REQ:
            CAPGTask::Instance()->GameQueryStateReq(p);
            break;

        case Pb::THIRD_CMD_KEEPALIVE_REQ:
            CAPGTask::Instance()->KeepAliveReq(p);
            break;

        case Pb::THIRD_CMD_LOGIN_AISVR_REQ:
            CAPGTask::Instance()->RegisterReq(p);
            break;

        case TIMER_CMD_CHECK_RUNGAME_TIMEOUT:
            CAPGTask::Instance()->CheckRunGameTimeout(p);
            break;

        case TIMER_CMD_CHECK_CLIENT_TIMEOUT:
            CAPGTask::Instance()->CheckClientTimeout(p);
            break;


        ////////////////// datasvr的命令  ///////////////////////////////////
        case Pb::CMD_DATA_KEEP_ALIVE:
            CDataTask::Instance()->DataKeepAliveResp(p);
            break;

        case Pb::CMD_DATA_REGISTER:
            CDataTask::Instance()->DataRegisterResp(p);
            break;

        case Pb::CMD_DATA_QUERY_USER_STATDATA:
            CDataTask::Instance()->DataQueryUserStatResp(p);
            break;

        default:
            LOG(LT_INFO_TRANS, p->szTransID,
                "TaskIt recv unkow packet| cmd=%d| time=%d| rc=%d",  p->nCmd, p->recv_time.ToNow(), p->nErrCode);
            CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
            break;
        }
    }
}


