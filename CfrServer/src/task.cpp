/**
* \file bm_task.cpp
* \brief 网络任务处理类实现函数
*/

#include "pch.h"
#include "task.h"
#include "cfr_svr.h"
#include "net_msg.h"
#include "comm_define.h"
#include "system.pb.h"
#include <string.h>
#include "google/protobuf/text_format.h"

using namespace std;
using namespace network;

#include "string_utils.h"
#include "data_module.h"
/**
* \brief 构造函数
*/
CTask::CTask(void)
{
}

/**
* \brief 析构函数
*/
CTask::~CTask(void)
{
}

/**
* \brief 初始化
* \return 初始化成功返回true，失败返回false
*/
bool CTask::Init()
{
    return true;
}

/**
* \brief 释放
*/
void CTask::Restore()
{
}


void CTask::ErrorResponse(LISTSMG *const p, uint32 errCode, const std::string &errMsg)
{
    LOG(LT_INFO_TRANS, p->szTransID, "DO_ERR_RESPONSE| cmd=%d| time=%d| rc=%d| msg=%s", p->nCmd, p->recv_time.ToNow(), errCode, errMsg.c_str());

    DoResponsePacket2(p, errCode, "Failed: Unkown packet \r\n");
    CSocketManager::Instance()->Send(p);

}


void CTask::QueryAction(LISTSMG *const p)
{
    const std::string SEPARTOR = "#";
    const std::string KEY_STR = "KEY";

    int rc = Pb::ERR_COMM_INTERNAL;
    char buf[512] = {0};

    rc = Pb::OK;
    std::string request_str = StringUtils::GetValueBykey(p->cPacketBuffer,
                              KEY_STR.c_str(), SEPARTOR.c_str());
    std::string resp_str;
    DataModule::GetVals(request_str, resp_str);
    std::string previous_str(p->cPacketBuffer);
    previous_str = previous_str.substr(0, previous_str.find('\r') - 1);
    snprintf(buf, sizeof(buf) - 1, "%s#Resp=%s\r\n", previous_str.c_str(), resp_str.c_str());

    LOG(LT_INFO_TRANS, p->szTransID, "DO_QUERY_ACTION| time=%d| rc=%d| msg=%s", p->recv_time.ToNow(), rc, resp_str.c_str());

    DoResponsePacket2(p, rc, buf);
    CSocketManager::Instance()->Send(p);
}

void CTask::KeeyAlive(LISTSMG *const p)
{
    LOG(LT_INFO_TRANS, p->szTransID,
        "DO_KEEP_ALIVE| time=%d| buf=%s", p->recv_time.ToNow(), p->cPacketBuffer);

    CSocketManager::Instance()->Send(p);
}

void CTask::QueryVersion(LISTSMG *const p)
{
    LOG(LT_INFO_TRANS, p->szTransID,
        "DO_QUERY_VERSION| time=%d| buf=%s", p->recv_time.ToNow(), p->cPacketBuffer);

    int rc = Pb::ERR_COMM_INTERNAL;
    char buf[1024] = {0};

    rc = Pb::OK;
    snprintf(buf, sizeof(buf) - 1, "QUERY_VERSION#Version=%s,%s,%s#Random=%d#SumprobFlag=%d,%d,%d,%d#Iter=%d,%d,%d,%d\r\n",
             CConf::Instance()->m_strBettingVer.c_str(), CConf::Instance()->m_strBucketVer.c_str(),
             CConf::Instance()->m_strCfrVer.c_str(), CConf::Instance()->m_bRandomFlag,
             (int)CConf::Instance()->m_vecSumprobFlag[0], (int)CConf::Instance()->m_vecSumprobFlag[1],
             (int)CConf::Instance()->m_vecSumprobFlag[2], (int)CConf::Instance()->m_vecSumprobFlag[3],
             CConf::Instance()->m_vecIter[0], CConf::Instance()->m_vecIter[1],
             CConf::Instance()->m_vecIter[2], CConf::Instance()->m_vecIter[3]
            );

    LOG(LT_INFO_TRANS, p->szTransID, "DO_QUERY_VERSION| time=%d| rc=%d| msg=%s", p->recv_time.ToNow(), rc, buf);

    DoResponsePacket2(p, rc, buf);
    CSocketManager::Instance()->Send(p);
}


