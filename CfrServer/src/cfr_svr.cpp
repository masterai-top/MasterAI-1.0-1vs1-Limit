/**
* \file bm_svr.cpp
* \brief ACPC控制服务器类实现函数
*/

#include "pch.h"
#include "cfr_svr.h"
#include "CQUEUE_List.h"
#include "system.pb.h"
#include "comm_define.h"
#include "version.h"
#include "net_msg.h"

using namespace network;

#include "data_module.h"
#include "error_code.h"

CCfrSvr::CCfrSvr(void)
{

}

CCfrSvr::~CCfrSvr(void)
{
}

bool CCfrSvr::OnInit()
{
    //默认日志名文件的方式初始化
    if(0 != shstd::log::Init(CRunConfig::Instance()->GetDefaultLogPath().c_str(), CRunConfig::Instance()->GetDefaultLogFile().c_str(), "debug,info,warn,error", "default"))
    {
        fprintf(stderr, "Log init failed| default_log_file=%s/%s.log\r\n", CRunConfig::Instance()->GetDefaultLogPath().c_str(), CRunConfig::Instance()->GetDefaultLogFile().c_str());
        return false;
    }

    // 加载启动配置文件
    std::string strCfg = CRunConfig::Instance()->GetConfPath() + CRunConfig::Instance()->GetLocalFileName();
    if(!CConf::Instance()->LoadConfig(strCfg.c_str()))
    {
        printf("Load config failed\r\n");
        LOG(LT_ERROR_TRANS, "Cfr OnInit", "load config [ %s ] failed", strCfg.c_str());
        return false;
    }

    /*GlobalConf oGlobalConf;
    oGlobalConf.strLocalFileName = CRunConfig::Instance()->GetLocalFileName();
    oGlobalConf.strModuleName    = "cfr";
    int nRet = cfr_load_conf(CRunConfig::Instance()->GetConfPath().c_str(), CRunConfig.GetRunName().c_str());
    if (0 != nRet)
    {
        printf("Load config failed\r\n");
        LOG(LT_ERROR_TRANS, "Acpc OnInit", "load config [ %s ] failed| rc=%d", CRunConfig::Instance()->GetConfPath().c_str(), nRet);
        return false;
    }*/

    //配置日志名文件的方式初始化
    if(0 != shstd::log::Init(CConf::Instance()->m_oLogConf.m_szPath, CConf::Instance()->m_oLogConf.m_szFile, CConf::Instance()->m_oLogConf.m_szLevel, shstd::fun::GetRunUser()))
    {
        fprintf(stderr, "Log init from config failed| log_file=%s/%s.log\r\n",
                CConf::Instance()->m_oLogConf.m_szPath, CConf::Instance()->m_oLogConf.m_szFile);
        return false;
    }

    LOG(LT_INFO_TRANS, "Acpc OnInit", "Cfr server Start| ver=%s", GetSvrVersionInfo());
    if(1 == CConf::Instance()->m_oLogConf.m_nAsynMode)
    {
        shstd::log::SetAsynMode();
    }

    if(CConf::Instance()->m_oLogConf.m_nMaxSize > 5 * 1024 * 1024)
    {
        shstd::log::SetLogMaxSize(CConf::Instance()->m_oLogConf.m_nMaxSize);
    }

    if(0 != CQUEUE_List::Instance()->CreateQueue(CConf::Instance()->m_nPacketNum))
    {
        LOG(LT_ERROR_TRANS, "", "Create Queue failed!");
        return false;
    }


    // 创建网络环境
    if (!NetEngine::Create(CConf::Instance()->m_oServer.nServerID, CONN_DEFAULT_TIMEOUT, CConf::Instance()->m_nMaxFd, CConf::Instance()->m_nNetThreadCnt, CConf::Instance()->m_nLimitedLogEnable))
    {
        printf("Create socket manager failed\r\n");
        LOG(LT_ERROR_TRANS, "ON_INIT", "create socket manager failed!");
        return false;
    }

    CTask::Instance()->Init();


    pthread_t tid = 0;
    for(long i = 0; i < CConf::Instance()->m_nTaskThreadCnt; i++)
    {
        if(0 != pthread_create( &tid, NULL, &CCfrSvr::TaskIt, (void *)i))
        {
            fprintf(stderr, "create task thread failed\r\n");
            LOG(LT_ERROR_TRANS, "TASK_INIT", "Init task thread failed| index=%d", i);
            return false;
        }
    }

    if (!CSocketManager::Instance()->Listen(CConf::Instance()->m_oServer.ip, CConf::Instance()->m_oServer.nPort, false))
    {
        fprintf(stderr, "listen failed\r\n");
        LOG(LT_ERROR, "socket listen failed! addr[ %s ] port [ %d ]", CConf::Instance()->m_oServer.ip, CConf::Instance()->m_oServer.nPort);
        return false;
    }

    if (DataModule::Init() != ERR_OK)
    {
        LOG(LT_ERROR_TRANS, "", "Init Data Module Failed!");
    }

    LOG(LT_INFO_TRANS, "ON_INIT", "start server success!");
    fprintf(stderr, "Cfr Server start success...\r\n");

    sleep(1);

    return true;
}

/**
* \brief 服务释放
*/
void CCfrSvr::OnRestore()
{
    CSocketManager::Instance()->Release();
    sleep(1);
}

/**
* \brief 信号事件
* \param nSignalID 信号ID
*/
void CCfrSvr::OnSignal(int32 nSignalID)
{

}

void *CCfrSvr::TaskIt( void *arg )
{
    LISTSMG *p;
    pthread_detach( pthread_self() );
    long index = (long)arg;

    LOG(LT_INFO_TRANS, "Init", "TaskIt thread Start| pid=%u| index=%d", pthread_self(), index);

    int num = 0;
    for ( ;; )
    {
        p = CQUEUE_List::Instance()->GetBlockNode(QUEUE_CLIENT_READ, &num);
        if (NULL == p)
        {
            continue;
        }

        LOG(LT_DEBUG_TRANS, p->szTransID,
            "TaskIt| recv packet| fd=%d| cmd=%u| len=%d| time=%d| queue_num=%d",
            p->connfd, p->nCmd, p->len, p->recv_time.ToNow(), num
           );

        if(p->recv_time.ToNow() > 3000)
        {
            LOG(LT_ERROR_TRANS, p->szTransID, "TIME_OUT pakcet| queue_num=%d| time=%d", num, p->recv_time.ToNow());
            //CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
            //continue;
        }

        if(0 == strncmp(p->cPacketBuffer, "QUERY_ACTION", 12))
        {
            CTask::Instance()->QueryAction(p);
        }
        else if(0 == strncmp(p->cPacketBuffer, "KEEP_ALIVE", 10))
        {
            CTask::Instance()->KeeyAlive(p);
        }
        else if(0 == strncmp(p->cPacketBuffer, "QUERY_VERSION", 13))
        {
            CTask::Instance()->QueryVersion(p);
        }
        else
        {
            LOG(LT_WARN_TRANS, p->szTransID, "DO_UNKONW_PACKET| cmd=%d| time=%d| buf=%s", p->nCmd, p->recv_time.ToNow(), p->cPacketBuffer);
            DoResponsePacket2(p, Pb::ERR_COMM_INVALID_CMD, "Failed: Unkown packet \r\n");
            CSocketManager::Instance()->Send(p);
        }
    }
}

