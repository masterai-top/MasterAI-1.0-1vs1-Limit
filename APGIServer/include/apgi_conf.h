#ifndef __APGI_CONF__
#define __APGI_CONF__

#include "script.h"
#include "define.h"
#include "Def.h"
#include <map>
using namespace std;
extern "C" {
    #include "game.h"
};

//接入APG定义算法类型
const int32 G_APG_AI_MODE_L1    = 1;    //模型1
const int32 G_APG_AI_MODE_L2    = 2;    //模型2
const int32 G_APG_AI_MODE_L3    = 3;    //混合模型



//cfr服务信息
struct CFRServer
{
    ModelParam oModelParam;         //算法参数
    uint32     nServerID;           //服务ID
    string     strVersion;          //版本信息
    
    string GetVersion() {
        return strVersion;
    }
};


//测试机器人的Action
#define MAX_ROBOT_ACTION_CNT (4)
struct TRobotAction
{
    uint64  nRobotID;
    Action  oAction0[MAX_ROBOT_ACTION_CNT];
    Action  oAction1[MAX_ROBOT_ACTION_CNT];
    Action  oAction2[MAX_ROBOT_ACTION_CNT];
    Action  oAction3[MAX_ROBOT_ACTION_CNT];
};

/**
* \brief AI服务器配置
*/
class CAPGIConf : public dtscript::IScriptHandler
{
private:    
    CAPGIConf(void);
    
public:
    static CAPGIConf* Instance()
    {
        static CAPGIConf m_instance;
        return &m_instance;
    }

    string      GetCfrVersion(const string &cfrModel);
    CFRServer*  GetCfrServer(const string &cfrModel);
    string      SelectCfrModel(int32 nAPGAiMode, uint32 nGameBeginTimes);
    string      SelectValidModel(const string &strInvalidModel);
    
    /**
    * \brief 析构函数
    */
    virtual ~CAPGIConf(void);

    /**
    * \brief 加载配置
    * \param szConfigFile 配置文件
    * \return 成功返回true，否则返回false
    */
    bool LoadConfig(const char *szConfigFile);

public:
    /**
    * \brief csv文件加载回调
    * \param szFileName 文件名
    * \param pFilePoint 文件数据指针
    * \return 成功处理回调返回true，否则返回false
    */
    virtual bool OnFileLoad(const char *szFileName, dtscript::ICsvIterator *pFilePoint);

    /**
    * \brief ini文件加载回调
    * \param szFileName 文件名
    * \param pFilePoint 文件数据指针
    * \return 成功处理回调返回true，否则返回false
    */
    virtual bool OnFileLoad(const char *szFileName, dtscript::IIniIterator *pFilePoint);

    /**
    * \brief xml文件加载回调
    * \param szFileName 文件名
    * \param pFilePoint 文件数据指针
    * \return 成功处理回调返回true，否则返回false
    */
    virtual bool OnFileLoad(const char *szFileName, dtscript::IXmlIterator *pFilePoint);

private:
    /**
    * \brief 加载服务器配置
    * \param szFileName 文件名
    * \param pFilePoint 文件数据指针
    * \return 成功返回true，否则返回false
    */
    bool LoadSvrConfig(const char *szFileName, dtscript::IIniIterator *pFilePoint);


    int LoadTRobotAction(dtscript::IIniIterator *pFilePoint);
    
public:
    uint32          m_nTaskThreadCnt;       ///< 线程池初始线程数量
    int32           m_nPacketNum;           //数据包数
    uint32          m_nTimerObjectNum;      // 定时器对象数量
    uint32          m_nRunGameCnt;          //同时在线的游戏对象数
    
    uint32          m_nNetThreadCnt;        //网络线程数[不含Accept线程]
    uint32          m_nDataTimeout;         //数据超时时间
    uint32          m_nTCPTimeOut;          //TCP超时时间
    uint32          m_nDataConnCnt;         //连接datasvr服务的连接数
    uint32          m_nCfrConnCnt;          //连接Cfr服务的连接数
    uint32          m_nMaxFd;               //最大fd
    uint32          m_nConnObjectCnt;       //预分配的连接对象数
    uint32          m_nRobotTimeout;        //机器人超时动作时间
    uint32          m_nPacketTimeout;       //超时丢弃报文[单位:毫秒]
    
    uint8           m_nLimitedLogEnable;    //限量日志开关 1:打开(限制部分日志输出) 0:关闭
    
    //BettingTree配置
    string          m_strBettingTreeFile;       //betting tree的Csv文件名
    uint32          m_nBettingTreeRecords;      //betting tree 的记录数
    string          m_strBettingTreeRedis;      //betting tree的Redis服务
    string          m_strBettingTreeRedisPass;  //betting tree的Redis服务的密码

    uint8           m_nUnsaveHistory;       //不保存历史记录
    string          m_strHistoryRedis;      //历史记录的redis服务
    
    DefServer       m_oServer;              //服务配置
    CLogConf        m_oLogConf;             //日志配置项
    

    //机器人相关的配置
    ModelParam      m_oTModelParam;         //临时对象.初始化使用.只会用到部分参数

    
    map<string, CFRServer*> m_mapCfrServer; //map, key=cfr模型
    vector<DefServer>       m_vCfrServer;   //所有的cfr服务
    vector<DefServer>       m_vDataServer;  //所有的datasvr服务
    

    TRobotAction    m_oTRobotAction;        //机器人测试Action信息
};


#endif // __BRAIN_CONF__

