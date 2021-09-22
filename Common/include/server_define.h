/**
* \file server_define.h
* \brief 服务器枚举定义
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __SERVER_DEFINE_H__
#define __SERVER_DEFINE_H__

#include "typedef.h"
#include <string>


/**
* \brief 服务器唯一ID
*/
typedef struct _SERVER_KEY
{
    uint8       nRegID;     // 服务区ID
    uint8		nType;		// 服务器类型
	uint16		nInstID;    // 服务器实例ID

    /**
    * \brief 构造函数
    */
	_SERVER_KEY():nRegID(0), nType(0), nInstID(0)
	{
	}

    _SERVER_KEY(uint8 reg_id, uint8 type, uint16 inst_id):nRegID(reg_id), nType(type), nInstID(inst_id)
    {
    }

    void Reset(uint8 reg_id, uint8 type, uint16 inst_id)
    {
        nRegID  = reg_id;
		nType   = type;
		nInstID = inst_id;
    }

    void Reset(uint32 nSvrID) 
    {
        nRegID  = (nSvrID >> 24) & 0xFF;
		nType   = (nSvrID >> 16) & 0xFF;
		nInstID = (nSvrID) & 0xFFFF;
    }
    
    /**
    * \brief 构造函数
    */
	_SERVER_KEY(uint32 nSvrID)
	{
        nRegID  = (nSvrID >> 24) & 0xFF;
		nType   = (nSvrID >> 16) & 0xFF;
		nInstID = (nSvrID) & 0xFFFF;
	}

    /**
    * \brief 拷贝构造函数
    */
	_SERVER_KEY(const _SERVER_KEY &other)
	{
        nRegID  = other.nRegID;
		nType   = other.nType;
		nInstID = other.nInstID;
	}

    /**
    * \brief 强制类型转换
    * \return 整型int32
    */
    uint32 ToUint32 ()
    {
        return uint32( (nRegID << 24) + (nType << 16) + nInstID );
    }

    /**
    * \brief ==比较重载
    */
	bool operator == (const _SERVER_KEY &other) const
	{
		return (nRegID == other.nRegID && nType == other.nType && nInstID == other.nInstID);
	}

    /**
    * \brief <比较重载
    */
	bool operator < (const _SERVER_KEY &other) const
	{
        if ( nRegID < other.nRegID )
        {
            return true;
        }
        else if ( nRegID > other.nRegID )
        {
            return false;
        }
        
        if ( nType < other.nType )
		{
			return true;
		}
        else if ( nType > other.nType )
        {
            return false;
        }

        return nInstID < other.nInstID;	
	}

    /**
    * \brief =赋值重载
    */
    _SERVER_KEY & operator = (const _SERVER_KEY &other)
    {
        if (this != &other)
        {
            nRegID  = other.nRegID;
            nType   = other.nType;
            nInstID = other.nInstID;
        }

        return *this;
    }
} SERVERKEY;




#endif // __SERVER_DEFINE_H__
