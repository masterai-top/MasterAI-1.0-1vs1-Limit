#include "pch.h"
#include "data_task.h"
#include "comm.pb.h"
#include "Third.pb.h"
#include "net_msg.h"
#include <string.h>
#include "google/protobuf/text_format.h"
#include "shstd.h"
#include "define.h"

using namespace std;


CDataTask::CDataTask(void)
{

}

CDataTask::~CDataTask(void)
{

}


bool CDataTask::Init()
{
    return true;
}

void CDataTask::Restore()
{

}


void CDataTask::DataKeepAliveResp(LISTSMG *const p)
{
    //需要区分客户端&服务端进行处理
    int rc = Pb::ERR_COMM_OTHER;
    string szMsg;

    Pb::CommKeepAlive pbKeepAlive;
    if (!pbKeepAlive.ParseFromArray(p->cPacketBuffer + PACKAGE_HEAD_LEN, p->len - PACKAGE_HEAD_LEN))
    {
        szMsg = "parse failed";
        rc = Pb::ERR_COMM_PROTO_PARSE;
        network::CSocketManager::Instance()->CloseConnect(p->connfd);
    }
    else if(Pb::OK != p->nErrCode)
    {
        network::CSocketManager::Instance()->CloseConnect(p->connfd);
        szMsg = "response failed";
        rc = Pb::ERR_COMM_INTERNAL;
    }
    else if(!g_oDataConnector.SetAliveTime(p->szTransID, p->nRemoteServerID, p->nUniqueID))
    {
        szMsg = "set alive time failed";
        rc    = Pb::ERR_COMM_INTERNAL;
        network::CSocketManager::Instance()->CloseConnect(p->connfd);
    }
    else
    {
        szMsg = "Succ";
        rc    = Pb::OK;
    }

    LOG(LT_INFO_TRANS, p->szTransID, "DO_DATA_KEEP_ALIVE_RESP| remote_server_id=0x%x| response_msg=%s| rc=%d| msg=%s",
        p->nRemoteServerID, pbKeepAlive.msg().c_str(), rc, szMsg.c_str());

    CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
}

void CDataTask::DataRegisterResp(LISTSMG *const p)
{
    int rc = Pb::ERR_COMM_OTHER;
    string szMsg;

    if(Pb::OK != p->nErrCode)
    {
        network::CSocketManager::Instance()->CloseConnect(p->connfd);
        szMsg = "response failed";
        rc = Pb::ERR_COMM_INTERNAL;
    }
    else
    {
        szMsg = "Succ";
        rc    = Pb::OK;
    }

    LOG(LT_INFO_TRANS, p->szTransID, "DO_DATA_REGISTER_RESP| rc=%d| msg=%s", rc, szMsg.c_str());
    CQUEUE_List::Instance()->SetNode(p, QUEUE_FREE);
}


void CDataTask::DataQueryUserStatResp(LISTSMG *const p)
{
    string szMsg;
    int rc = Pb::ERR_COMM_OTHER;
    Pb::ThirdGameStatDataRsp pbRsp;

    if (!pbRsp.ParseFromArray(p->cPacketBuffer + PACKAGE_HEAD_LEN, p->len - PACKAGE_HEAD_LEN))
    {
        szMsg = "parse failed";
        rc    = Pb::ERR_COMM_PROTO_PARSE;
        pbRsp.set_resultcode(Pb::THIRD_FAIL);
        pbRsp.set_err_msg(szMsg);
    }
    else if(Pb::OK != p->nErrCode)
    {
        szMsg = "response failed";
        rc    = Pb::ERR_COMM_INTERNAL;
        pbRsp.set_resultcode(Pb::THIRD_FAIL);
        pbRsp.set_err_msg(szMsg);
    }
    else
    {
        szMsg = "Succ(to apg)";
        rc    = Pb::OK;
        pbRsp.set_resultcode(Pb::THIRD_SUCC);
    }

    p->nUniqueID = p->nCliConnID >> 32;
    p->connfd    = p->nCliConnID & 0xFFFFFFFF;


    LOG(LT_INFO_TRANS, p->szTransID, "DO_QUERY_USER_STATDATA_RESP| player_id=%llu| cli_conn_id=%llx(%x:%d)| time=%u| rc=%d| msg=%s| statdata={%s}",
        pbRsp.uid(), p->nCliConnID, p->nUniqueID, p->connfd, p->recv_time.ToNow(), rc, szMsg.c_str(), ProtobufToString(pbRsp).c_str()
       );

    std::string sTid(p->szTransID);
    if(Pb::OK == rc)
    {
        DoApgiComNetPacket(p, Pb::THIRD_CMD_GAME_STATDATA_RSP, pbRsp.uid(), sTid.c_str(), Pb::THIRD_SUCC, &pbRsp);
    }
    else
    {
        DoApgiComNetPacket(p, Pb::THIRD_CMD_GAME_STATDATA_RSP, pbRsp.uid(), sTid.c_str(), Pb::THIRD_FAIL, &pbRsp);
    }
    network::CSocketManager::Instance()->Send(p);
}



