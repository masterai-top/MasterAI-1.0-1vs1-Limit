/**
* \file redis_connector.h
* \brief redis连接器
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __REDIS_CONNECTOR_H__
#define __REDIS_CONNECTOR_H__

#include "typedef.h"
#include "redis_base.h"

/**
* \brief redis连接器
*/
class CRedisConnector : public CRedisBase
{
public:   
   CRedisConnector(void);
    ~CRedisConnector(void);

    
    bool   DelPlayer(uint32 nRoleID);
    
};

#endif // __REDIS_CONNECTOR_H__

