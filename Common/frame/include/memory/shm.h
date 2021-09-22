/**
* \file shm.h
* \brief 共享内存类
*
* 封装了共享内存的接口。
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __SHM_H__
#define __SHM_H__

#include "framedef.h"
#include "typedef.h"

/// 共享内存ID
#ifdef WIN32
typedef void *          shmid;
#else
typedef int             shmid;
#endif

namespace frame
{
    /**
    * \brief 共享内存类
    * \ingroup memory_group
    */
    class FRAME_EXPORT Shm
    {
    public:
        /**
        * \brief 构造函数
        */
        Shm(void);

        /**
        * \brief 析构函数
        */
        ~Shm(void);

        /**
        * \brief 创建共享内存
        * \param nKey 共享内存key
        * \param nSize 共享内存大小
        * \return 成功返回true，否则返回false
        */
        bool Create(int32 nKey, uint32 nSize);

        /**
        * \brief 释放
        */
        void Release();

        /**
        * \brief 挂载共享内存
        * \param nKey 共享内存key
        * \param nSize 共享内存大小
        * \return 成功返回true，否则返回false
        */
        bool Attach(int32 nKey, uint32 nSize);

        /**
        * \brief 卸载共享内存
        */
        void Detach();

        /**
        * \brief 获取共享内存数据
        * \return 返回共享内存数据
        */
        void * GetData();

        /**
        * \brief 获取共享内存数据大小
        * \return 返回共享内存数据大小
        */
        uint32 GetSize();

    public:
        /**
        * \brief 创建共享内存
        * \param nKey 共享内存主键
        * \param nSize 共享内存大小
        * return 返回共享内存ID
        */
        static shmid CreateShm( int32 nKey, uint32 nSize );

        /**
        * \brief 获得共享内存
        * \param nKey 共享内存主键
        * \param nSize 共享内存大小
        * return 返回共享内存ID
        */
        static shmid GetShm( int32 nKey, uint32 nSize );

        /**
        * \brief 挂载共享内存
        * \param nShmID 共享内存ID
        * return 返回共享内存数据
        */
        static void * AttachShm( shmid nShmID );

        /**
        * \brief 卸载共享内存
        * \param pShm 共享内存数据
        */
        static void DetachShm( const void *pShm );

        /**
        * \brief 删除共享内存
        * \param nShmID 共享内存ID
        */
        static void DestroyShm( shmid nShmID );

    private:
        void    *m_pData;       ///< 共享内存数据
        uint32  m_nSize;        ///< 共享内存大小
        shmid   m_nShmID;       ///< 共享内存ID
    };
}

#endif // __SHM_H__