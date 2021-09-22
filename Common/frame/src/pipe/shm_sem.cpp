/**
* \file shm_sem.cpp
* \brief 共享内存信号量函数的实现
*/

#include "pch.h"
#include "pipe/shm_sem.h"

/*
struct sembuf {
    short semnum; -----  信号量集合中的信号量编号，0代表第1个信号量,1代表第2.....

    short val;    -----  若val>0进行V操作信号量值加val，表示进程释放控制的资源
                         若val<0进行P操作信号量值减val，若(semval-val)<0（semval为该信号量值），则调用进程阻塞，直到资源可用；若设置IPC_NOWAIT不会睡眠，
                         进程直接返回EAGAIN错误
                         若val==0时阻塞等待信号量为0，调用进程进入睡眠状态，直到信号值为0；若设置IPC_NOWAIT，进程不会睡眠，直接返回EAGAIN错误

    short flag;   -----  0 设置信号量的默认操作
                         IPC_NOWAIT 设置信号量操作不等待
                         SEM_UNDO  选项会让内核记录一个与调用进程相关的UNDO记录，如果该进程崩溃，则根据这个进程的UNDO记录自动恢复相应信号量的计数值
};
*/

namespace frame
{
#ifndef WIN32
#   if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
    /* union semun is defined by including <sys/sem.h> */
#   else
    /* according to X/OPEN we have to define it ourselves */
    union semun
    {
        int val;                /*SETVAL用的值*/
        struct semid_ds *buf;   /*IPC_STAT、IPC_SET用的semid_ds结构*/
        unsigned short *array;  /*SETALL、GETALL用的数组值*/
        struct seminfo *__buf;  /*为控制IPC_INFO提供的缓存*/
        void *__pad;
    };
#   endif
#endif

    /**
    * \brief 构造函数
    */
    ShmSem::ShmSem(void)
    {
        m_key = -1;
        m_sem = -1;
    }

    /**
    * \brief 析构函数
    */
    ShmSem::~ShmSem(void)
    {
        m_key = -1;
        m_sem = -1;
    }

    /**
    * \brief 初始化共享内存信号
    * \param nKey 信号主键
    * \return 初始化信号集时的进程数量，失败返回-1
    */ 
    int32 ShmSem::Init(int32 nKey)
    {
        int32 nCount = -1;

#ifndef WIN32

        int32 nFlag = IPC_CREAT|0666;   // 不能IPC_EXCL，若有该信号集会报错
        const int32 NUM_SEMS = 6;       // 信号量个数，必须作为一组5个信号量创建，否则将无法访问信号量

        // 生成信号量集
        m_sem = semget(nKey, NUM_SEMS, nFlag);
        if (m_sem == -1)
        {
            return -1;
        }

        // 信号0[SYSVSEM_WAIT], 信号1[SYSVSEM_USAGE], 信号2[SYSVSEM_SETVAL], 信号3[SYSVSEM_READ], 信号4[SYSVSEM_WRITE]
        // 等待信号SYSVSEM_SETVAL变为0：{SYSVSEM_SETVAL, 0, 0}
        // 并且把信号SYSVSEM_SETVAL加1：{SYSVSEM_SETVAL, 1, SEM_UNDO}
        // 再把信号SYSVSEM_USAGE加1：{SYSVSEM_USAGE, 1, SEM_UNDO}
        struct sembuf sops[3];
        sops[0] = {SYSVSEM_SETVAL, 0, 0};
        sops[1] = {SYSVSEM_SETVAL, 1, SEM_UNDO};
        sops[2] = {SYSVSEM_USAGE, 1, SEM_UNDO};

        // 等待信号SYSVSEM_SETVAL
        int32 ret = -1;
        do
        {
            ret = semop(m_sem, &sops[0], 3);
        } while ((ret == -1) && (errno == EINTR));

        if (ret == -1)
        {
            return -1;
        }

        // 获取使用该信号集的进程数量
        nCount = semctl(m_sem, SYSVSEM_USAGE, GETVAL, NULL);
        if (nCount == -1)
        {
            return -1;
        }

        // 使用该信号集的进程只有自己，则初始化信号SYSVSEM_WAIT和信号SYSVSEM_READ/SYSVSEM_WRITE
        if (nCount == 1)
        {
            // 将信号SYSVSEM_WAIT设置为0
            union semun arg;
            arg.val = 0;
            if (semctl(m_sem, SYSVSEM_SEM, SETVAL, arg) == -1)
            {
                return -1;
            }
            if (semctl(m_sem, SYSVSEM_WAIT, SETVAL, arg) == -1)
            {
                return -1;
            }
            // 将信号SYSVSEM_READ设置为1
            arg.val = 1;
            if (semctl(m_sem, SYSVSEM_READ, SETVAL, arg) == -1)
            {
                return -1;
            }
            // 将信号SYSVSEM_READ设置为1
            arg.val = 1;
            if (semctl(m_sem, SYSVSEM_WRITE, SETVAL, arg) == -1)
            {
                return -1;
            }
        }

        // 将信号SYSVSEM_SETVAL还原为0
        sops[0] = {SYSVSEM_SETVAL, -1, SEM_UNDO};

        ret = -1;
        do
        {
            ret = semop(m_sem, &sops[0], 1);
        } while ((ret == -1) && (errno == EINTR));

        if (ret == -1)
        {
            return -1;
        }

        m_key = nKey;

#endif

        return nCount;
    }

    /**
    * \brief 销毁共享内存信号
    * \return 成功返回true，否则返回false
    * 无需主动销毁，所有进程销毁后该信号集会自动销毁
    */ 
    bool ShmSem::Destroy()
    {
#ifndef WIN32
        int32 nFlag = 0666;
        const int32 NUM_SEMS = 6;       // 信号量个数，必须作为一组5个信号量创建，否则将无法访问信号量

        //获取信号量集
        int32 sem = semget(m_key, NUM_SEMS, nFlag);
        if ( sem == -1 || sem != m_sem)
        {
            return false;
        }

        union semun arg;
        // 立即删除信号量集合，唤醒所有因调用 semop（） 
        // 阻塞在该信号量集合里的所有进程(相应调用会返回错误且 errno 被设置为 EIDRM)。
        // 调用进程的有效用户ID必须匹配信号量集合的创建者或所有者，或者调用者必须有特权。
        // 参数 semnum 被忽略。
        semctl(m_sem, 0, IPC_RMID, arg);

        m_key = -1;
        m_sem = -1;
#endif

        return true;
    }
    
    /**
    * \brief 等待信号通知
    */
    void ShmSem::Wait()
    {
#ifndef WIN32
        // 信号0[SYSVSEM_SEM], 信号1[SYSVSEM_USAGE], 信号2[SYSVSEM_SETVAL], 信号3[SYSVSEM_READ], 信号4[SYSVSEM_WRITE]，信号5[SYSVSEM_WAIT]
        // 先将SYSVSEM_WAIT信号量加1：{SYSVSEM_WAIT, 1, SEM_UNDO}
        struct sembuf sops[2];
        sops[0] = {SYSVSEM_WAIT, 1, SEM_UNDO};

        int32 ret = -1;

        do
        {
            ret = semop(m_sem, &sops[0], 1);
        } while ((ret == -1) && (errno == EINTR));

        // 再把SYSVSEM_SEM信号量减1：{SYSVSEM_SEM, -1, 0}
        // 把SYSVSEM_WAIT信号量还原：{SYSVSEM_WAIT, -1, SEM_UNDO}
        sops[0] = {SYSVSEM_SEM, -1, 0};
        sops[1] = {SYSVSEM_WAIT, -1, SEM_UNDO};

        ret = -1;

        do
        {
            ret = semop(m_sem, &sops[0], 2);
        } while ((ret == -1) && (errno == EINTR));
#endif
    }

    /**
    * \brief 发送信号通知
    */
    void ShmSem::Post()
    {
#ifndef WIN32
        // 信号0[SYSVSEM_SEM], 信号1[SYSVSEM_USAGE], 信号2[SYSVSEM_SETVAL], 信号3[SYSVSEM_READ], 信号4[SYSVSEM_WRITE]，信号5[SYSVSEM_WAIT]
        // 把SYSVSEM_SEM信号量加1：{SYSVSEM_SEM, 1, 0}
        struct sembuf sops[1] = {{SYSVSEM_SEM, 1, 0}};

        int32 ret = -1;

        do
        {
            ret = semop(m_sem, &sops[0], 1);
        } while ((ret == -1) && (errno == EINTR));
#endif
    }

    /**
    * \brief 广播信号通知
    */
    void ShmSem::Broadcast()
    {
#ifndef WIN32
        // 获取使用该信号集的进程数量
        int16 nCount = semctl(m_sem, SYSVSEM_WAIT, GETVAL, NULL);

        // 没有进程在等
        if (nCount <= 0)
        {
            return;
        }

        // 信号0[SYSVSEM_SEM], 信号1[SYSVSEM_USAGE], 信号2[SYSVSEM_SETVAL], 信号3[SYSVSEM_READ], 信号4[SYSVSEM_WRITE]，信号5[SYSVSEM_WAIT]
        // 把SYSVSEM_SEM信号量加nCount：{SYSVSEM_SEM, nCount, 0}
        struct sembuf sops[1] = {{SYSVSEM_SEM, nCount, 0}};

        int32 ret = -1;

        do
        {
            ret = semop(m_sem, &sops[0], 1);
        } while ((ret == -1) && (errno == EINTR));
#endif
    }
    
    /**
    * \brief 共享内存上锁
    */
    void ShmSem::Lock()
    {
#ifndef WIN32
        // 信号0[SYSVSEM_SEM], 信号1[SYSVSEM_USAGE], 信号2[SYSVSEM_SETVAL], 信号3[SYSVSEM_READ], 信号4[SYSVSEM_WRITE]，信号5[SYSVSEM_WAIT]
        // 把SYSVSEM_READ信号量减1：{SYSVSEM_READ, -1, SEM_UNDO}
        // 把SYSVSEM_WRITE信号量减1：{SYSVSEM_WRITE, -1, SEM_UNDO}
        struct sembuf sops[2];
        sops[0] = {SYSVSEM_READ, -1, SEM_UNDO};
        sops[1] = {SYSVSEM_WRITE, -1, SEM_UNDO};

        int32 ret = -1;

        do
        {
            ret = semop(m_sem, &sops[0], 2);
        } while ((ret == -1) && (errno == EINTR));
#endif
    }

    /**
    * \brief 共享内存解锁
    */
    void ShmSem::Unlock()
    {
#ifndef WIN32
        // 信号0[SYSVSEM_WAIT], 信号1[SYSVSEM_USAGE], 信号2[SYSVSEM_SETVAL], 信号3[SYSVSEM_READ], 信号4[SYSVSEM_WRITE]
        // 把SYSVSEM_READ信号量加1：{SYSVSEM_READ, 1, SEM_UNDO}
        // 把SYSVSEM_WRITE信号量加1：{SYSVSEM_WRITE, 1, SEM_UNDO}
        struct sembuf sops[2];
        sops[0] = {SYSVSEM_READ, 1, SEM_UNDO};
        sops[1] = {SYSVSEM_WRITE, 1, SEM_UNDO};

        int32 ret = -1;

        do
        {
            ret = semop(m_sem, &sops[0], 2);
        } while ((ret == -1) && (errno == EINTR));
#endif
    }

    /**
    * \brief 共享内存读上锁
    */
    void ShmSem::ReadLock()
    {
#ifndef WIN32
        // 信号0[SYSVSEM_WAIT], 信号1[SYSVSEM_USAGE], 信号2[SYSVSEM_SETVAL], 信号3[SYSVSEM_READ], 信号4[SYSVSEM_WRITE]
        // 把SYSVSEM_READ信号量减1：{SYSVSEM_READ, -1, SEM_UNDO}
        struct sembuf sops[1] = {{SYSVSEM_READ, -1, SEM_UNDO}};

        int32 ret = -1;

        do
        {
            ret = semop(m_sem, &sops[0], 1);
        } while ((ret == -1) && (errno == EINTR));
#endif
    }

    /**
    * \brief 共享内存读解锁
    */
    void ShmSem::ReadUnlock()
    {
#ifndef WIN32
        // 信号0[SYSVSEM_WAIT], 信号1[SYSVSEM_USAGE], 信号2[SYSVSEM_SETVAL], 信号3[SYSVSEM_READ], 信号4[SYSVSEM_WRITE]
        // 把SYSVSEM_READ信号量加1：{SYSVSEM_READ, 1, SEM_UNDO}
        struct sembuf sops[1] = {{SYSVSEM_READ, 1, SEM_UNDO}};

        int32 ret = -1;

        do
        {
            ret = semop(m_sem, &sops[0], 1);
        } while ((ret == -1) && (errno == EINTR));
#endif
    }

    /**
    * \brief 共享内存写上锁
    */
    void ShmSem::WriteLock()
    {
#ifndef WIN32
        // 信号0[SYSVSEM_WAIT], 信号1[SYSVSEM_USAGE], 信号2[SYSVSEM_SETVAL], 信号3[SYSVSEM_READ], 信号4[SYSVSEM_WRITE]
        // 把SYSVSEM_WRITE信号量减1：{SYSVSEM_WRITE, -1, SEM_UNDO}
        struct sembuf sops[1] = {{SYSVSEM_WRITE, -1, SEM_UNDO}};

        int32 ret = -1;

        do
        {
            ret = semop(m_sem, &sops[0], 1);
        } while ((ret == -1) && (errno == EINTR));
#endif
    }

    /**
    * \brief 共享内存写解锁
    */
    void ShmSem::WriteUnlock()
    {
#ifndef WIN32
        // 信号0[SYSVSEM_WAIT], 信号1[SYSVSEM_USAGE], 信号2[SYSVSEM_SETVAL], 信号3[SYSVSEM_READ], 信号4[SYSVSEM_WRITE]
        // 把SYSVSEM_WRITE信号量加1：{SYSVSEM_WRITE, 1, SEM_UNDO}
        struct sembuf sops[1] = {{SYSVSEM_WRITE, 1, SEM_UNDO}};

        int32 ret = -1;

        do
        {
            ret = semop(m_sem, &sops[0], 1);
        } while ((ret == -1) && (errno == EINTR));
#endif
    }
}