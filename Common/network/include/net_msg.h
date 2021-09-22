/**
* \file net_msg.h
* \brief 网络消息定义
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __NET_MSG_H__
#define __NET_MSG_H__

#include <arpa/inet.h>

#include "typedef.h"
#include "google/protobuf/message.h"
#include "netdef.h"
#include "CQUEUE_List.h"
    
#pragma pack (push,1)

extern uint64 htonl64(uint64 host64);
extern uint64 ntohl64(uint64 net64);


//消息头类
struct NetMsgHead
{
    uint32  m_nLen;                     //长度
    uint32  m_nCmd;                     //命令
    uint32  m_nErrCode;                 //错误码
    char    m_szTransID[TID_LEN];       //任务ID = 模块ID[2位]主机ID[3位]唯一子串[10位]-序列子串[4位], 共20位,最后补0
                                        //模块ID：2位，固定分配
                                        //主机ID：3位，每个模块主机的ID
                                        //唯一串：10位，客户端的每个TCP分配的一个唯一串
                                        //子串序列：4位，0001 - n，唯一串相同的TCP自增长，可循环使用
    uint64  m_nCliConnID;               //客户端(玩家终端)连接唯一ID(api根据该值匹配指定的客户端)
    char    m_sReserved[18];	        //保留
    
    
    //构造函数
    NetMsgHead()
    {
        memset(this, 0, sizeof(NetMsgHead));
    }

    //构造函数
    NetMsgHead(uint32 len, uint32 cmd, uint32 errcode, uint64 nCliConnID, const char *szTransID)
    {
        m_nLen      = len;
        m_nCmd      = cmd;
        m_nErrCode  = errcode;
        m_nCliConnID= nCliConnID;
        snprintf(m_szTransID, sizeof(m_szTransID), "%s", szTransID);
        m_szTransID[TID_LEN - 1] = 0;
    }

    //解析头
    bool UnpackFromArray(const char * pData, size_t nLen)
    {
        if (pData == NULL || nLen < sizeof(NetMsgHead))
        {
            return false;
        }

        m_nLen      = ntohl(((NetMsgHead *)(pData))->m_nLen);
        m_nCmd      = ntohl(((NetMsgHead *)(pData))->m_nCmd);
        m_nErrCode  = ntohl(((NetMsgHead *)(pData))->m_nErrCode);
        m_nCliConnID= ntohl64(((NetMsgHead *)(pData))->m_nCliConnID);
        memcpy(m_szTransID, ((NetMsgHead *)(pData))->m_szTransID, sizeof(m_szTransID) - 1);
        m_szTransID[TID_LEN - 1] = 0;

        return true;
    }

    void head_hton() {
        m_nLen      = htonl(m_nLen);
        m_nCmd      = htonl(m_nCmd);
        m_nErrCode  = htonl(m_nErrCode);
        m_nCliConnID= htonl64(m_nCliConnID);
    }
};
#pragma pack (pop)

/**
* \brief 构造网络包
* \param cmd 命令
* \param errcode 错误码
* \param szTransID 任务ID
* \param nRoleID 接收消息的roleID
* \param pbMsg pb消息
*/
extern int DoComNetPacket(LISTSMG *p, uint32 cmd, uint32 errcode, const char *szTransID, uint64 nCliConnID, const google::protobuf::Message *pbMsg, uint32 nRoleID = 0);
extern int DoComNetPacket(LISTSMG *p, const char *szTransID, int fd, uint32 nUniqueID, uint64 nCliConnID, const char *buf, uint32 nRoleID = 0);
extern int DoResponsePacket(LISTSMG *p, uint32 errcode, const google::protobuf::Message *pbMsg, uint32 nRoleID = 0);
extern int DoResponsePacket2(LISTSMG *p, uint32 errcode, const char *buf);

extern std::string ProtobufToString(const google::protobuf::Message &pbMsg);

#endif // __NET_MSG_H__
