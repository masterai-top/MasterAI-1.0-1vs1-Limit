/**
* \file net_engine.cpp
* \brief 网络引擎的实现
*/

#include "pch.h"
#include "net_engine.h"
#include "socket_manager.h"


namespace network
{

    ISocketHandler::~ISocketHandler()
    {
    }
   
    bool NetEngine::Create(uint32 nLocalServerID, uint32 nTcpTimeOut, uint32 nSocketCnt, uint32 nDataThreadCnt, uint8 nLimitedLogEnable)
    {
        if(!CSocketManager::Instance()->Init(nLocalServerID, nTcpTimeOut, nSocketCnt, nLimitedLogEnable))
        {
            LOG(LT_ERROR, "Net engine create| socket manager init failed");
            return false;
        }

        if(!CNetWorker::Instance()->Init(nDataThreadCnt))
        {
            LOG(LT_ERROR, "Net engine create| net worker init failed");
            return false;
        }

        return true;
    }
}