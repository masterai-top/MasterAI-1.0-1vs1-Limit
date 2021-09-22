/**
* \file conf.h
* \brief 配置
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __CONF__
#define __CONF__

#include "script.h"
#include "public_conf.h"
#include "GlobalConf.h"

#include <string>

class CConf : public dtscript::IScriptHandler
{
private:
    /**
    * \brief 构造函数
    */
    CConf(void);
    
public:
    static CConf* Instance()
    {
        static CConf m_instance;
        return &m_instance;
    }
    
    /**
    * \brief 析构函数
    */
    virtual ~CConf(void);

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

    std::string GetConfigInfo() const;

public:
    uint32              m_nTaskThreadCnt;       ///< 线程池初始线程数量
    int32               m_nPacketNum;           //数据包数
    uint8               m_nLimitedLogEnable;    //限量日志开关 1:打开(限制部分日志输出) 0:关闭
    uint32              m_nMaxFd;               //最大支持的fd
    uint32              m_nNetThreadCnt;        //网络线程数[不含Accept线程]
    
    DefServer           m_oServer;              //服务配置
    CLogConf            m_oLogConf;             //日志配置项

    std::string         m_strCfrDir;
    std::string         m_strStaticDir;
    std::string         m_strCardParamPath;
    std::string         m_strBettingParamPath;
    std::string         m_strCfrParamPath;
    std::string         m_strGameParamPath;

    std::vector<bool>   m_vecSumprobFlag;
    std::vector<int>    m_vecIter;

    std::string         m_strBettingVer;
    std::string         m_strBucketVer;
    std::string         m_strCfrVer;

    bool                m_bRandomFlag;

    std::string         m_strFinalVersion;
};

#endif // __CONF__


