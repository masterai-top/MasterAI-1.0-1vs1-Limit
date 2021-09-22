/**
* \file shm.cpp
* \brief 共享内存类实现代码
*/

#include "pch.h"
#include "memory/shm.h"
#include "system.h"
#include "strutil.h"
using namespace dtutil;


namespace frame
{
    /**
    * \brief 构造函数
    */
    Shm::Shm(void)
    {
        m_pData = NULL;
        m_nSize = 0;
        m_nShmID = 0;
    }

    /**
    * \brief 析构函数
    */
    Shm::~Shm(void)
    {
        Release();
    }

    /**
    * \brief 创建共享内存
    * \param nKey 共享内存key
    * \param nSize 共享内存大小
    * \return 成功返回true，否则返回false
    */
    bool Shm::Create(int32 nKey, uint32 nSize)
    {
        m_nShmID = CreateShm(nKey, nSize);

        if (m_nShmID == 0)
        {
            return false;
        }

        m_pData = AttachShm(m_nShmID);

        if (NULL == m_pData)
        {
            return false;
        }

        m_nSize = nSize;

        memset(m_pData, 0, nSize);

        return true;
    }

    /**
    * \brief 释放
    */
    void Shm::Release()
    {
        if (m_pData)
        {
            DetachShm(m_pData);
            m_pData = NULL;
        }

        m_nShmID = 0;
        m_nSize = 0;
    }

    /**
    * \brief 挂载共享内存
    * \param nKey 共享内存key
    * \param nSize 共享内存大小
    * \return 成功返回true，否则返回false
    */
    bool Shm::Attach(int32 nKey, uint32 nSize)
    {
        m_nShmID = GetShm(nKey, nSize);

        if (m_nShmID == 0)
        {
            return false;
        }

        m_pData = AttachShm(m_nShmID);

        if (NULL == m_pData)
        {
            return false;
        }

        m_nSize = nSize;

        return true;
    }

    /**
    * \brief 卸载共享内存
    */
    void Shm::Detach()
    {
        if (m_pData)
        {
            DetachShm(m_pData);
            m_pData = NULL;
        }

        m_nShmID = 0;
        m_nSize = 0;
    }

    /**
    * \brief 获取共享内存数据
    * \return 返回共享内存数据
    */
    void * Shm::GetData()
    {
        return m_pData;
    }

    /**
    * \brief 获取共享内存数据大小
    * \return 返回共享内存数据大小
    */
    uint32 Shm::GetSize()
    {
        return m_nSize;
    }

#ifdef WIN32
    /**
    * \brief 创建共享内存
    * \param nKey 共享内存主键
    * \param nSize 共享内存大小
    * return 返回共享内存ID
    */
    shmid Shm::CreateShm( int32 nKey, uint32 nSize )
    {
        std::string strPath, strName;
        StrUtil::strformat(strPath, "shm_%s_%d", System::GetProcessName().c_str(), nKey);
        StrUtil::strformat(strName, "shm_%d", nKey);

        HANDLE hFile = CreateFileA( strPath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL );

        if (INVALID_HANDLE_VALUE == hFile)
        {
            return 0;
        }

        // 创建内存映射文件对象
        uint32 nMaxSizeHigh = 0;
        uint32 nMaxSizeLow = nSize;
        HANDLE hFileMap = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, nMaxSizeHigh, nMaxSizeLow, strName.c_str());
        if (NULL == hFileMap)
        {
            return 0;
        }

        // Close后能不能写文件？
        CloseHandle(hFile);

        return (shmid)hFileMap;
    }

    /**
    * \brief 获得共享内存
    * \param nKey 共享内存主键
    * \param nSize 共享内存大小
    * return 返回共享内存ID
    */
    shmid Shm::GetShm( int32 nKey, uint32 nSize )
    {
        std::string strName;
        StrUtil::strformat(strName, "shm_%d", nKey);

        // 首先试图打开一个命名的内存映射文件对象
        HANDLE hFileMap = OpenFileMappingA(FILE_MAP_READ | FILE_MAP_WRITE, false, strName.c_str());
        if (INVALID_HANDLE_VALUE == hFileMap)
        {
            return NULL;
        }

        return (shmid)hFileMap;
    }

    /**
    * \brief 挂载共享内存
    * \param nShmID 共享内存ID
    * return 返回共享内存数据
    */
    void * Shm::AttachShm( shmid nShmID )
    {
        // 映射对象的一个视图，得到指向共享内存的指针
        void* pShm = MapViewOfFile(HANDLE(nShmID), FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, NULL);
        if (NULL == pShm)
        {
            return NULL;
        }

        return pShm;
    }

    /**
    * \brief 卸载共享内存
    * \param pShm 共享内存数据
    */
    void Shm::DetachShm( const void *pShm )
    {
        UnmapViewOfFile(pShm);
    }

    /**
    * \brief 删除共享内存
    * \param nShmID 共享内存ID
    */
    void Shm::DestroyShm( shmid nShmID )
    {
        CloseHandle(HANDLE(nShmID));
    }

#else
    /**
    * \brief 创建共享内存
    * \param nKey 共享内存主键
    * \param nSize 共享内存大小
    * return 返回共享内存ID
    */
    shmid Shm::CreateShm( int32 nKey, uint32 nSize )
    {
        int32 nFlag = IPC_CREAT|IPC_EXCL|0666;

        int32 nShmID = shmget(nKey, nSize, nFlag);
        if (nShmID == -1)
        {
            return 0;
        }

        return (shmid)nShmID;
    }

    /**
    * \brief 获得共享内存
    * \param nKey 共享内存主键
    * \param nSize 共享内存大小
    * return 返回共享内存ID
    */
    shmid Shm::GetShm( int32 nKey, uint32 nSize )
    {
        int32 nFlag = 0666;

        int32 nShmID = shmget(nKey, nSize, nFlag);
        if (nShmID == -1)
        {
            return 0;
        }

        return (shmid)nShmID;
    }

    /**
    * \brief 挂载共享内存
    * \param nShmID 共享内存ID
    * return 返回共享内存数据
    */
    void * Shm::AttachShm( shmid nShmID )
    {
        void *pShm = shmat(nShmID, NULL, 0);
        if (((void *)-1) == pShm)
        {
            return NULL;
        }

        return pShm;
    }

    /**
    * \brief 卸载共享内存
    * \param pShm 共享内存数据
    */
    void Shm::DetachShm( const void *pShm )
    {
        shmdt(pShm);
    }

    /**
    * \brief 删除共享内存
    * \param nShmID 共享内存ID
    */
    void Shm::DestroyShm( shmid nShmID )
    {
        shmctl(nShmID, IPC_RMID, NULL);
    }

#endif
}