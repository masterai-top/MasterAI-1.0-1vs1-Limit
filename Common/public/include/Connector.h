/**
* \brief 客户端连接器
*
*/

#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

#include "typedef.h"
#include "net_engine.h"
#include "GlobalConf.h"
#include "comm.pb.h"
using namespace std;
using namespace frame;

class CUnitConnector;

//连接器:每个对象是一个连接
class CConnector : public network::ISocketHandler, public frame::ITimerHandler
{
    friend class CUnitConnector;
    
public:
    CConnector(void);
    virtual ~CConnector(void);

    //初始化
    bool Init(IEventReactor *pEventReactor, 
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
    void Restore();
    
    //基类接口实现
    virtual int  OnConnect(int32 nRemoteFd, bool bSuccess, uint32 nUniqueID, bool bBinaryMode);
    virtual void OnClose(int32 nRemoteFd, int32 nReason);
    virtual void OnTimer(uint64 nTimerID);

               
    void   SetAliveTime()     { m_nAliveTime = time(NULL); }
    void   DoRequest(LISTSMG * p, uint32 cmd, string &strTransID, uint64 nCliConnID, const google::protobuf::Message *pbMsg, uint32 nRoleID);
    void   DoRequest(LISTSMG * p, string &strTransID, uint64 nCliConnID, const char *szBuf, uint32 nRoleID);

private:
    int32  GetConnectID()     { return m_nConnectID; }
    uint32 GetUniqueID()      { return m_nUniqueID; }
    uint32 GetRemoteServerID() { return m_oRemoteServer.nServerID; }
    string GetNextTransID();
    void   DoKeepAlive();
    void   DoReConnect();
    
private:
    int32               m_nConnectID;               // 连接ID
    uint32              m_nUniqueID;                // 连接的唯一ID
    char                m_szLocalAddr[64];          // 当前连接本地的IP地址
    uint16              m_nLocalPort;               // 当前连接本地的端口
    time_t              m_nAliveTime;               // 活跃时间
    string              m_strLastTransID;           // 最后一次TransID
    bool                m_bBinaryMode;              // 二进制协议

    
    DefServer           m_oRemoteServer;            // 远程服务Server对象
    DefServer           m_oLocalServer;             // 本模块Server对象
    char                m_szLogSymbol[64];          // 日志标记
    
    uint32              m_nRegisterCmd;             // 注册命令
    uint32              m_nAliveCmd;                // 心跳命令
    uint64              m_nAliveTimerID;            // 心跳TimerID
    uint32              m_nAliveInterval;           // 心跳间隔
    uint64              m_nReconnTimerID;           // 重连TimerID
    uint32              m_nReconnInterval;          // 重连时长
    uint32              m_nConnTimeout;             // 连接超时时长
    
    IEventReactor      *m_pEventReactor;            // Event框架
};


#endif // __CONNECTOR_H__

