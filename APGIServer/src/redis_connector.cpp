/**
* \file redis_connector.cpp
* \brief redis连接器实现函数
*/

#include "pch.h"
#include "redis_connector.h"
#include "apgi_svr.h"
#include "comm_define.h"

/**
* \brief 构造函数
*/
CRedisConnector::CRedisConnector(void)
{

}

/**
* \brief 析构函数
*/
CRedisConnector::~CRedisConnector(void)
{

}


bool CRedisConnector::DelPlayer(uint32 nRoleID)
{
    std::string strBrainKey = KEY_ROBOT(nRoleID);

    r3c::CRedisClient *pRedisCli = PopRedisClient();
    assert(NULL != pRedisCli);
    pRedisCli->del(strBrainKey);
    PushRedisClient(pRedisCli);

    return true;
}

