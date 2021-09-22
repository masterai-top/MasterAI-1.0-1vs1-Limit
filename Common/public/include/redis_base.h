/**
* \file redis_base.h
* \brief redis连接器基类
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __REDIS_BASE_H__
#define __REDIS_BASE_H__
#include "typedef.h"
#include "r3c.h"
#include <list>
#include "frame.h"
#include "comm_define.h"
using namespace std;

class CRedisBase
{
public:
    CRedisBase(void);
    virtual ~CRedisBase(void);

    bool Init(const std::string &strAddr, int nCnt, const std::string &strPass = "");
    void Restore();

    /******* 待同步数据的RoleID集合的公共接口(支持定时线程根据RoleID将用户Redis缓存的数据同步至数据库功能)  ****************/
    int    RoleIDToWaitSaveSet(uint32 nRoleID);
    int    RoleIDToWaitSaveSet(const std::vector<std::string>& vRoleID);
    void   GetRoleIDFromWaitSaveSet(std::vector<std::string>& vRoleID);
    void   DelRoleIDFromWaitSaveSet(uint32 nRoleID);

    /************** 用户角色的公共接口 ************************/
    void RoleInrcFields(uint32 nRoleId, const std::vector<std::pair<std::string, int64_t> >& increments,  std::vector<int64_t> *values);
    void RoleSetFields(uint32 nRoleID, const FieldNodes &map);
    bool RoleExpire(uint32 nRoleId, uint32 seconds);
    
    /************************* 公共接口 ***************************/
    int  SetValue(const std::string &k, const std::string &v);
    bool GetByKey(const std::string &k, std::string &v);

    int64_t DBSize();
    int64_t SCan(int64_t cursor, const std::string& pattern, int count, std::vector<std::string>* values);
    
protected:
    r3c::CRedisClient* PopRedisClient();
    void PushRedisClient(r3c::CRedisClient* pClient);
    
private:
    frame::LockObject           m_lock;             //锁
    list<r3c::CRedisClient*>    m_lstRedisClient;   //redis客户端
};

#endif // __REDIS_BASE_H__
