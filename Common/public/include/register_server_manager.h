/**
* \file client_manager.h
* \brief 注册服务客户端管理器
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
*/

#ifndef __REGISTER_CLIENT_MANAGER_H__
#define __REGISTER_CLIENT_MANAGER_H__
#include "typedef.h"
#include "map"
#include "string"
#include "thread/lock.h"
#include "comm_define.h"
using namespace std;

//连接信息
struct ConnInfo
{
    int32          m_nSock;         //Socket
    int32          m_nPort;         //端口
    uint32         m_nUniqueID;     //连接标识

    ConnInfo(): m_nSock(-1),   m_nPort(0), m_nUniqueID(0) {}
    void Reset() { 
        m_nSock     = -1;
        m_nPort     = 0;
        m_nUniqueID = 0;
    }
};

//注册的服务信息
struct RegisterServer
{
    uint32          m_nSeqNo;           //编号(客户端从0-1开始编号)
    uint16          m_nListenPort;      //注册服务监听的端口
    uint32          m_nResCnt;          //资源计数
    char            m_szRemoteAddr[64]; //连接地址      
    char            m_szServerName[64]; //服务名称
    ConnInfo        m_oConnInfo;

    RegisterServer();
};

//内部存储结构
struct StorageServer
{
    uint32          m_nServerID;        //serverid
    uint16          m_nListenPort;      //注册服务监听的端口
    uint32          m_nResCnt;          //资源计数
    uint32          m_nConnCnt;         //有效连接数
    char            m_szRemoteAddr[64]; //连接地址      
    char            m_szServerName[64]; //服务名称
    uint32          m_nTurnSeq;         //轮询序列
    ConnInfo        m_lstConnInfo[MAX_SERVER_REGISTER_CNT];    //连接信息[一个serverid支持多个连接]
    
    StorageServer();
    bool    AddConn(const char *szTransID, const ConnInfo &oConn);
    bool    DelConn(const char *szTransID, int32 nSock, uint32 nUniqueID);
    bool    ToRegisterServer(const char *szTransID, RegisterServer &oServer);
    bool    GetOneConn(const char *szTransID, ConnInfo &oConn, uint32 idx);
    uint32  AllocConnIdx(const char *szTransID, int32 nSock, uint32 nOldIdx, const string &sLogMsg);
};

//注册服务管理
class CRegisterServerManager
{
public:
    ~CRegisterServerManager(void);
    static CRegisterServerManager* Instance()
    {
        static CRegisterServerManager m_instance;
        return &m_instance;
    }
    
    bool   Init();
    bool   Register(const char *szTransID, uint32 nServerID, const RegisterServer &oServer);
    void   UnRegister(const char *szTransID, uint32 nServerID, int32 nSock, uint32 nUniqueID);
    bool   GetServerInfo(const char *szTransID, uint32 nServerID, RegisterServer &oServer);    
    bool   GetServerConn(const char *szTransID, uint32 nServerID, ConnInfo &oConn, uint32 nConnIdx);
    uint32 GetSize();

    int    Select(const char *szTransID, uint32 &nServerID, RegisterServer &oServer);
    int    ModifyResCnt(const char *szTransID, uint32 nServerID, uint32 nResCnt);
    uint32 GetConnIdx(const char *szTransID, uint32 nServerID, int32 nSock, uint32 nOldIdx, const string &sLogMsg);
        
private:
    CRegisterServerManager(void);
    void   DebugPrintf(const char *szTransID);

private:
    frame::LockObject  m_lock;
    std::map<uint32, StorageServer>   m_mapServer;    //key=serverid
};

#endif // __REGISTER_CLIENT_MANAGER_H__


