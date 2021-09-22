#ifndef __APGI_SVR__
#define __APGI_SVR__

#include "app/app.h"
#include "memory/singleton.h"
#include "apgi_conf.h"
#include "game_manager.h"
#include "server_define.h"
#include "redis_connector.h"
#include "define.h"
#include "cfr_task.h"
#include "apg_task.h"
#include "data_task.h"


/**
* \brief AI服务器
*/
class CAPGISvr : public frame::App
    , public frame::Singleton<CAPGISvr>
{
public:
    /**
    * \brief 构造函数
    */
    CAPGISvr(void);

    /**
    * \brief 析构函数
    */
    virtual ~CAPGISvr(void);

    /**
    * \brief 服务初始化
    * \return 初始化成功返回true，否则返回false
    */
    virtual bool OnInit();

    /**
    * \brief 服务释放
    */
    virtual void OnRestore();

    /**
    * \brief 信号事件
    * \param nSignalID 信号ID
    */
    virtual void OnSignal(int32 nSignalID);

    static void* TaskIt(void *);
};

#endif // __APGI_SVR__
