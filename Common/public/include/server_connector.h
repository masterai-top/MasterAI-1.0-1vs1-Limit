/**
* \file 服务连接器
* \brief 
*
* Copyright (c) 2020
* All rights reserved.
*
* \version 1.0
*/

#ifndef __SERVER_CONNECTOR_H__
#define __SERVER_CONNECTOR_H__

#include "typedef.h"
#include "net_engine.h"
#include "Connector.h"
#include <vector>
using namespace frame;
using namespace std;

//  服务连接器单元类
//  1.一个对象有多个"连接器"
//  2.每个连接器都是连接同一个(ip+listenPort)
class CUnitConnector
{    
public:
    CUnitConnector();
    ~CUnitConnector();
    //初始化
    bool Init( uint32 nConnCnt,                //连接数
                IEventReactor *pEventReactor,  //事件Reactor
                const DefServer &oRemoteServer,//远程服务Server对象
                const DefServer &oLocalServer, //本模块Server对象
                const uint32 &nRegisterCmd,    //注册命令
                const uint32 &nAliveCmd,       //心跳命令
                const uint64 &nAliveTimerID,   //心跳定时器ID
                const uint32 &nAliveInterval,  //心跳间隔
                const uint64 &nReconnTimerID,  //重连定时器ID
                const uint32 &nReconnInterval, //重连间隔
                const uint32 &nConnTimeout,    //重连超时
                const string &strLogSymbol,    //日志标记
                bool bBinaryMode               //二进制模式
                );
                
    void   Restore();
    uint32 GetRemoteServerID() { return m_oRemoteServer.nServerID; }
    
    bool   SetAliveTime(uint32 nUniqueID);
    void   DoRequest(LISTSMG * p, uint32 cmd, string &strTransID, uint64 nCliConnID, const google::protobuf::Message *pbMsg, uint32 nRoleID);
    void   DoRequest(LISTSMG * p, string &strTransID, uint64 nCliConnID, const char *szBuf, uint32 nRoleID);
    bool   IsConnected();
    
private:
    DefServer           m_oRemoteServer;            // 远程服务ID
    uint32              m_nTurnSeq;                 // 轮询序列
    uint32              m_nConnCnt;                 // 连接数
    CConnector         *m_pConnector;               // 连接器列表[共m_nConnCnt个对象]
};



//  服务连接器
//  1.一个对象有多个"服务连接器单元", 每个单元连接一个服务
//  2.一个对象对应的是一组服务.
class CServerConnector
{
public:
    CServerConnector(void);
    ~CServerConnector(void);

    bool  Init(uint32 nConnCnt,  const vector<DefServer> &vRemoteServer, const DefServer &oLocalServer,
                IEventReactor *pEventReactor,  //事件Reactor
                const uint32 &nRegisterCmd,    //注册命令
                const uint32 &nAliveCmd,       //心跳命令
                const uint64 &nAliveTimerID,   //心跳定时器ID
                const uint32 &nAliveInterval,  //心跳间隔
                const uint64 &nReconnTimerID,  //重连定时器ID
                const uint32 &nReconnInterval, //重连间隔
                const uint32 &nConnTimeout,    //重连超时
                const string &strLogSymbol,    //日志标记
                bool bBinaryMode               //二进制模式
          );

    void  Restore();
    
    bool  SetAliveTime(const char *szTransID, const uint32 &nRemoteServerID, uint32 nUniqueID);
    void  DoRequest(const uint32 &nRemoteServerID, LISTSMG * p, uint32 cmd, string &strTransID, uint64 nCliConnID, const google::protobuf::Message *pbMsg, uint32 nRoleID);
    void  DoRequest(const uint32 &nRemoteServerID, LISTSMG * p, string &strTransID, uint64 nCliConnID, const char *szBuf, uint32 nRoleID);
    void  RandRequest(LISTSMG * p, uint32 cmd, string &strTransID, uint64 nCliConnID, const google::protobuf::Message *pbMsg, uint32 nRoleID);
    void  RandRequest(LISTSMG * p, string &strTransID, uint64 nCliConnID, const char *szBuf, uint32 nRoleID);
    
    void  DoReport(const char *szLogSymbol, string &strTransID, uint32 cmd, const google::protobuf::Message *pbMsg);
    bool  IsConnected(const uint32 &nRemoteServerID);
    bool  IsAllConnected();
    
private:
    CUnitConnector* GetUnitConnectorByServerID(const uint32 &nRemoteServerID);
    CUnitConnector* GetUnitConnectorByIndex(const uint32 &nIndex);

private:
    uint32           m_nUnitSize;
    CUnitConnector  *m_pUnitConnector;
};

#endif // __SERVER_CONNECTOR_H__

