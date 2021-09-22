
/*! @file 
@brief 日志库头文件

提供服务日志的基本功能，包括命名、文件切换和部分日志规范内容。

*/

#ifndef __SH_LOG__
#define __SH_LOG__


/*! \def LOG
\brief 写日志的宏，简化编码

可以用这个宏代替shstd::log::Log， 便于维护。比如日志头不需要事务id的DEBUG日志，可以这样
LOG(LT_DEBUG, "SEND_MAIL| ip=%s| port=%u|", param1, param2);
写带事务id的INFO日志，可以这样
LOG(LT_INFO_TRANS, szTransId, "SEND_MAIL| mid=%s|", param2);

//带扩展参数,暂时支持值为0和1，当为1时，支持日志内容写入多个文件
LOG(LT_DEBUG, 1, "SEND_MAIL| ip=%s| port=%u|", param1, param2);
LOG(LT_INFO_TRANS, szTransId, 1, "SEND_MAIL| mid=%s|", param2);

*/
#define LOG  shstd::log::Log

//no deal time, no trans id
#define LT_DEBUG shstd::log::LOG_TYPE_DEBUG,__FILE__,__LINE__,-1,NULL
#define LT_INFO  shstd::log::LOG_TYPE_INFO,__FILE__,__LINE__,-1,NULL
#define LT_WARN  shstd::log::LOG_TYPE_WARN,__FILE__,__LINE__,-1,NULL
#define LT_ERROR shstd::log::LOG_TYPE_ERROR,__FILE__,__LINE__,-1,NULL

//no deal time, have trans id
#define LT_DEBUG_TRANS shstd::log::LOG_TYPE_DEBUG,__FILE__,__LINE__,-1
#define LT_INFO_TRANS  shstd::log::LOG_TYPE_INFO,__FILE__,__LINE__,-1
#define LT_WARN_TRANS  shstd::log::LOG_TYPE_WARN,__FILE__,__LINE__,-1
#define LT_ERROR_TRANS shstd::log::LOG_TYPE_ERROR,__FILE__,__LINE__,-1

//have deal time, have trans id
#define LT_DEBUG_ALL shstd::log::LOG_TYPE_DEBUG,__FILE__,__LINE__
#define LT_INFO_ALL  shstd::log::LOG_TYPE_INFO,__FILE__,__LINE__
#define LT_WARN_ALL  shstd::log::LOG_TYPE_WARN,__FILE__,__LINE__
#define LT_ERROR_ALL shstd::log::LOG_TYPE_ERROR,__FILE__,__LINE__

/**
* shstd库
*/
namespace shstd
{
    /*! 
    @brief shstd库之日志库
    */
    namespace log
    {
        static const int LOG_TYPE_NULL = 0;
        static const int LOG_TYPE_DEBUG = 1;
        static const int LOG_TYPE_INFO = 2;
        static const int LOG_TYPE_WARN = 4;
        static const int LOG_TYPE_ERROR = 8;
        static const int LOG_TYPE_PRINT = 1024;


        /*! @fn int Init(const char *pszDir, const char *szModuleName, const char *szLogTypes);
        @brief 对日志库进行初始化, 只能调用一次
        @param [in] pszDir 日志存放路径
        @param [in] szModuleName 日志模块的名称
        @param [in] szLogTypes 日志模块的输出类型，debug，info，warn，error的组合串。
		@param [in] szOwner 生成的日志文件所有者
		@param [in] nExtType 扩展类型，如果不等于0，则多写一个日志文件, 暂时此值只能为0或1, 默认为0
        @return 成功返回0,失败返回其它值
        */
        int Init(const char *pszDir, const char *szModuleName, const char *szLogTypes, const char *szOwner);
        int Init(const char *pszDir, const char *szModuleName, const char *szLogTypes, unsigned int nExtType, const char *szOwner);

		/*! @fn int IsOpen();
		@brief 检查日志库是否已经成功初始化
        @return 成功返回0:标识日志初始化成功, 其它表示初始化失败或还没有初始化
        */
		int IsOpen();


		/*! @fn bool SetAsynMode(unsigned int nAsynFlushLogCnt, unsigned int nAsynFlushSeconds)
        @brief 将上报日志库设成异步模式,在工作进程调用才能成功，初始化成功后调用此接口。在缓存有日志的情况下，两参数只要达到任何一个条件，即会写文件。
		在调用此接口之前没有调用SvrStdInit，请查看函数shstd::fun::SetMonitorParentPid。
		设成异步模式之后，将不能再切换到同步模式。接口重复调用，参数会生效。
		@param [in] nAsynFlushLogCnt 当日志缓存数量，达到此值会引起写文件操作。默认值为100， 范围1～10000, 超出范围会被设置为默认值
        @param [in] nAsynFlushSeconds 刷文件周期，单位秒,默认每秒至少写一次文件，在有缓存日志的情况下。默认值为1，范围1～120。超出范围会被设置为默认值
        @return 返回是否是异步模式，true为异步模式，false为同步模式。
        */
		bool SetAsynMode(unsigned int nAsynFlushLogCnt = 100, unsigned int nAsynFlushSeconds = 1);  

		/*! @fn bool GetAsynOption(unsigned int * pnAsynFlushLogCnt, unsigned int * pnAsynFlushSeconds);
        @brief 取得异步参数，当为同步模式时，函数将会失败
        @param [in] pnAsynFlushLogCnt 返回flush条数值
		@param [in] pnAsynFlushSeconds 返回flush时间值（单位秒)
        @return 成功返回true，失败返回false
        */
		bool GetAsynOption(unsigned int * pnAsynFlushLogCnt, unsigned int * pnAsynFlushSeconds);


		/*! @fn void SetSyncFlushOption(unsigned int nSyncFlushLogCnt, unsigned int nSyncFlushSecond)
        @brief 设置同步模式下的刷新条数或刷新周期, 两个条件满足任意一个都会flush
        @param [in] nSyncFlushLogCnt 未刷条数达到这个数量，会导致flush日志缓存数据到文件。0或1会每次都flush, 默认为1
		@param [in] nSyncFlushSecond 未刷新时间达到这个秒数，会导致flush日志缓存数据到文件，默认为3600
        @return 成功返回true，失败返回false
        */
		bool SetSyncFlushOption(unsigned int nSyncFlushLogCnt=1, unsigned int nSyncFlushSecond = 3600);

		/*! @fn bool GetSyncFlushOption(unsigned int *pnSyncFlushLogCnt, unsigned int *pnSyncFlushSecond)
        @brief 取得同步flush参数，如果是异步模式，此函数将失败.
        @param [in] pnSyncFlushLogCnt 返回flush条数值
		@param [in] pnSyncFlushSecond 返回flush时间值(单位秒)
        @return 成功返回true，失败返回false
        */
		bool GetSyncFlushOption(unsigned int *pnSyncFlushLogCnt, unsigned int *pnSyncFlushSecond);


        /*! @fn void Log(int nLogType, const char *szFileName, int nLineNum, int nMilliSecs,const char *szTransId, const char *fmt, ...);
        @brief 写日志
        @param [in] nLogType 日志类型
        @param [in] szFileName 源文件名
        @param [in] nLineNum 源文件行号
        @param [in] nMilliSecs 处理时间
        @param [in] szTransId 事务id
        @param [in] fmt 格式化串
        */
        void Log(int nLogType, const char *szFileName, int nLineNum, int nMilliSecs,const char *szTransId, const char *fmt, ...);

		/*! @fn void Log(int nLogType, const char *szFileName, int nLineNum, int nMilliSecs,const char *szTransId, unsigned int nExtType, const char *fmt, ...);
        @brief 按特定条件写日志
        @param [in] nLogType 日志类型
        @param [in] szFileName 源文件名
        @param [in] nLineNum 源文件行号
        @param [in] nMilliSecs 处理时间
        @param [in] szTransId 事务id
        @param [in] nExtType 扩展类型，如果参数不为0，则多写一个日志文件, 暂时此值只能为0或1
        @param [in] fmt 格式化串
        */
        void Log(int nLogType, const char *szFileName, int nLineNum, int nMilliSecs,const char *szTransId, unsigned int nExtType, const char *fmt, ...);
		
        /******************************************************\
        * 函 数 名：SetLogType
        * 函数功能：改变日志输出类型
        * 参    数：[in] szLogTypes :日志模块的输出类型, 为debug,info,warn,error四个单词的任意组合
        * 返 回 值：无
        * 备   注 ：
        \********************************************************/ 

        /*! @fn void SetLogType(const char *szLogTypes)
        @brief 改变日志输出类型
        @param [in] szLogTypes :日志模块的输出类型, 为debug,info,warn,error四个单词的任意组合
        */
        void SetLogType(const char *szLogTypes);  

        /*! @fn const char * GetLogType()
        @brief 取得当前日志输出类型
        @return 当前日志的输出类型
        */
        const char * GetLogType();

        /******************************************************\
        * 函 数 名：SetLogMaxSize
        * 函数功能：设置日志文件的最大大小
        * 参    数：[in] nMaxSize :日志文件最大大小
        * 返 回 值：返回实际大小
        * 备   注 ：默认2G-100M
        \********************************************************/ 

        /*! @fn unsigned long SetLogMaxSize(unsigned long nMaxSize = (2 * 1024 - 100) * 1024 * 1024L); 
        @brief 设置日志文件的最大大小
        @param [in] nMaxSize 日志文件最大大小，最大大小不超过10G
        @return 返回实际大小
        */
        unsigned long SetLogMaxSize(unsigned long nMaxSize = (2 * 1024 - 100) * 1024 * 1024L); 

        
    }
}

#endif



#ifdef __DEBUG__

#define DEBUG_LOG(x) do \
{ \
	LOG x; \
} while(0); 

#else

#define DEBUG_LOG(x) ;

#endif




