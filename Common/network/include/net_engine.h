/**
* \file net_engine.h
* \brief 网络引擎
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __NET_ENGINE_H__
#define __NET_ENGINE_H__

#include "typedef.h"
#include <string>
#include "CQUEUE_List.h"
#include "server_define.h"

struct event_base;

namespace network
{
    /**
    * \defgroup net_group 网络引擎
    * 提供了一组网络操作的相关函数
    */

    /**
    * \brief 套接字关闭原因
    * \ingroup net_group
    */
    enum SOCK_CLOSE_REASON
    {
        CLOSE_REASON_UNKNOW         = 0,    ///< 未知原因
        CLOSE_REASON_REMOTE_CLOSE   = 1,    ///< 远程关闭
        CLOSE_REASON_USER_CLOSE     = 2,    ///< 用户关闭
        CLOSE_REASON_TIME_OUT       = 3,    ///< 空闲超时
        CLOSE_REASON_EXCEED_LEN     = 4,    // 长度超出错误
        CLOSE_REASON_RECV_ERROR     = 5,    // 接收错误
    };

    enum SOCK_ERROR
    {
        SOCK_ERROR_UNKNOW = 0,          ///< 未知错误
        SOCK_ERROR_STATE_WRONG = 1,     ///< 状态错误
        SOCK_ERROR_RECV_BUFFER = 2,     ///< 接收缓存区不足
    };

    //创建SocketManager的参数
    struct CreateSMParam
    {
        uint32 nServerID;           //服务ID
        uint32 nTimeOut;            //超时时间(单位:秒)
    };
    
    /**
    * \brief 网络回调处理接口
    * \ingroup net_group
    */
    class ISocketHandler
    {
    public:
        /**
        * \brief 触发新连接事件
        * \param nRemoteFd 新连接fd
        */
        virtual int OnAccept(int32 nRemoteFd, const std::string &sRemoteAddr, uint16 nRemotePort, uint32 nUniqueID) { return 0 ;}

        /**
        * \brief 触发连接事件
        * \param nRemoteFd 远程fd
        * \param bSuccess 连接是否成功
        * \param bSuccess 连接唯一ID 
        */
        virtual int OnConnect(int32 nRemoteFd, bool bSuccess, uint32 nUniqueID, bool bBinaryMode) { return 0; }

        /**
        * \brief 触发可读事件
        * \param nRemoteFd 远程fd
        * \param pBuffer 可读数据内容
        * \param nLen 可读数据大小
        */
        virtual void OnRead(int32 nRemoteFd, LPCSTR pBuffer, size_t nLen) { }

        /**
        * \brief 触发可写事件
        * \param nRemoteFd 远程fd
        */
        virtual void OnWrite(int32 nRemoteFd) {}

        /**
        * \brief 触发网络错误
        * \param nRemoteFd 远程fd
        * \param nErrno 错误码
        */
        virtual void OnError(int32 nRemoteFd, int32 nErrno) {}

        /**
        * \brief 触发连接断开事件
        * \param nRemoteFd 远程fd
        * \param nReason 连接断开原因
        */
        virtual void OnClose(int32 nRemoteFd, int32 nReason) {}

        virtual ~ISocketHandler() = 0;
    };

    

    /**
    * \brief 网络引擎
    * \ingroup net_group
    */
    class  NetEngine
    {
    public:
        /*brief 创建网络工作环境
        * param  @nLocalServerID    模块ID  
                 @nTcpTimeOut       Tcp连接超时时间
                 @nSocketCnt        预分配Socket对象
                 @nDataThreadCnt    网络数据通信线程数
        * \return 成功:true
        */
        static bool  Create(uint32 nLocalServerID, uint32 nTcpTimeOut, uint32 nSocketCnt, uint32 nDataThreadCnt, uint8 nLimitedLogEnable);

        /**
        * \brief 释放套接字管理器
        * \param pSocketManager 套接字管理器
        * \return 释放成功返回true，否则返回false
        */
        //static bool ReleaseSocketManager(ISocketManager *pSocketManager);
    };
}

#endif // __NET_ENGINE_H__
