/**
* \brief 处理来自datasvr的业务
*
* Copyright (c) 2019
* All rights reserved.
*
*/

#ifndef __DO_DATA_TASK_H__
#define __DO_DATA_TASK_H__

#include "typedef.h"
#include "CQUEUE_List.h"

class CDataTask
{
private:
    CDataTask(void);

public:
    static CDataTask* Instance()
    {
        static CDataTask m_instance;
        return &m_instance;
    }
       
    ~CDataTask(void);
    bool Init();
    void Restore();

    void DataKeepAliveResp(LISTSMG * const p);
    void DataRegisterResp(LISTSMG * const p);
    void DataQueryUserStatResp(LISTSMG * const p);
};

#endif 


