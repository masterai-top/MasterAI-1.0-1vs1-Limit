
/*! @file 
@brief 一般函数库头文件

提供一些常用函数功能，包括系统的启动停止规范。
   
*/


#ifndef __SH_STD_FUN__
#define __SH_STD_FUN__
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <stdarg.h>
#include <string.h>



#pragma pack(push,1)

/**
* shstd库
*/
namespace shstd
{
    /*!
    @brief shstd库之工具函数库
    */
    namespace fun
    {    
        /*! @fn char * cp2ap(char *path)
        @brief 从相对路径到绝对路径的转换
        @param [in] path 相对路径
        @return 返回绝对路径，失败返回NULL
        */
        char * cp2ap(char *path); //相对路径到绝对路径


        //进行自监控, 连续拉起最小间隔nSleepSeconds
        void MonitorSelf(unsigned int nSleepSeconds = 3); //监控自己
		void MonitorSelf(int argc, const char **argv, unsigned int nSleepSeconds = 3); //监控自己


        /*! @fn bool KillTree(pid_t pid, unsigned int nMaxWaitSeconds = 60)
        @brief 杀掉进程树，先向树里的节点发SIGINT， 然后等待进程的退出，最多等待nMaxWaitSeconds, 再向没退出的进程发送SIGKILL
        @param [in] pid 树根pid
        @param [in]  nMaxWaitSeconds 等待推出秒数
		@return 进程树中的进程都不存在了，成功返回true，失败返回false
        */
        bool KillTree(pid_t pid, unsigned int nMaxWaitSeconds = 45);

        //切换用户

        /*! @fn bool ChangeRunUser(const char *szUser)
        @brief 切换运行用户身份
        @param [in] szUser 目的切换用户
        @return 成功返回true，失败返回false
        */
        bool ChangeRunUser(const char *szUser);

        //argc， argv为main函数的透传， szModuleName为模块名(比如lda,ms,mi 不带svr后缀的), szUser：填充生成pid路径对应的用户, nochdir 是否改变目录到/, noclose 是否将标准输入输出映射到/dev/null
        /*! @fn bool SvrStdInit(int argc, const char **argv, const char *szModuleName, const char *szUser, int nochdir = 1, int noclose = 0);
        @brief 对服务进行规范的初始化，包括启动停止规范，pid文件。 --stop的时候，会发送SIGINT信号给进程，通知要准备退出进程，默认最长等待20秒，要是进程还没退出，发送SIGKILL信号到进程。
        @param [in] argc 程序启动的参数，透传值
        @param [in] argv 程序启动的参数，透传值，当参数中包含 --debug时， 函数不做任何动作并返回true，以便进行单进程调试
        @param [in] szModuleName 模块名(比如lda,ms,mi 不带svr后缀的)
        @param [in] szUser 填充生成pid路径对应的用户
        @param [in] nochdir 是否改变目录到/, 1:不切换 0：切换
        @param [in] noclose 是否将标准输入输出映射到/dev/null 1：不切换， 0：切换
        @return 成功返回true，失败返回false
        */
        bool SvrStdInit(int argc, const char **argv, const char *szModuleName, const char *szUser, int nochdir = 1, int noclose = 0);

		/*! @fn const char * GetWorkHomePath(bool bPrintErr = true);
        @brief 取得工作的根目录，即进程可执行程序的上一级目录，尾部不带'/'
        @param [in] bPrintErr, 当出错时，是否打印错误提示
        @return 如果没按规范部署，会返回NULL， 否则返回正确的路径。
        */
		const char * GetWorkHomePath(bool bPrintErr = true);


		/*! @fn const char * GetRunUser();
        @brief 取得运行系统的用户名
        @param [in] 无
        @return 失败返回NULL, 成功返回系统存在的用户名
        */
		const char * GetRunUser();


		/*! 
        \brief 该结构数组存放域名解析后的ip
        */
		struct IP_INFO
		{
			char ip[64];
			IP_INFO()
			{
				memset(this, 0, sizeof(IP_INFO));
			}
		};

        /*! @fn int ParseDns(const char * szDns, IP_INFO  ipinfos[], unsigned int nIpInfoSize, unsigned short port = 0);
        @brief 解析dns，得到域名对应的ip，可能会有多个ip，如果同时存在ipv4和ipv6，ipv4在返回数组的前部
        @param [in] szDns 需要解析的域名
		@param [in] ipinfos 返回的ip信息数组的缓存
		@param [in] nIpInfoSize 传入缓存的IP_INFO结构数组元素的数量
		@param [in] port 测试连接的端口,如果不为0，则表示需要测试该端口的连通性，可能会引起程序阻塞一段时间，返回结果为可以连通的
        @return 失败返回负值，成功返回域名对应的ip填充到数组中的数量，当得到的ip数量比nIpInfoSize大时，数组会被填满
        */
		int ParseDns(const char * szDns, IP_INFO  ipinfos[], unsigned int nIpInfoSize, unsigned short port = 0);


		bool IsIpV4(const char * ip); //是否是ipv4
		bool IsIpV6(const char * ip); //是否是ipv6


		const char * GetModuleName();


        //取得工作子进程被拉起的数量，对于非正常的进程，可以看出coredump的次数
        unsigned int GetForkCnt();
        time_t GetParentProgressStartup(); //父进程启动时间
        time_t GetWorkProgressStartup(); //子进程启动时间
		pid_t GetMonitorParentPid(); //取得监控进程pid


		/*! @fn bool SetMonitorParentPid(pid_t ppid); 
        @brief 设置监控父进程的进程id,只能设置一次，使用SvrStdInit函数的服务无需调用此接口，没有使用SvrStdInit接口的服务进程不调用此接口不能成功设置日志异步模式。
        @param [in] ppid 监控父进程的pid
        @return 成功状态，true为成功，false为失败
        */
		bool SetMonitorParentPid(pid_t ppid); 


        /*! @fn char * TrimUsername(char *szUser);
        @brief 对用户名进行整理，去掉前后空格和双引号、单引号
        @param [in] szUser 需要整理的用户名
        @return szUser的地址
        */
        char * TrimUsername(char *szUser);

		char * TrimMobile(char *szMobile);
       

		/*! @fn const char * DecodeDBPass(char *szEnPass, char * szDePass, int key=19); 
        @brief 对数据库加密后的密码进行解密，解密后的结果存放在原始字符中
        @param [in] szEnPass 加密后的密码
		@param [in] szDePass 解密后的密码，其长度不得小于szEnPass的2/3
		@param [in] key 加密的可以，默认为19，加解密要一致
        @return 失败返回NULL， 成功返回解密后的密码
        */
		const char * DecodeDBPass(char *szEnPass, char * szDePass, int key=19); 

		//url 编解码
		std::string URLencode(const std::string & strSrc);
		std::string URLdecode(const std::string & strSrc);

		//base64编解码
		int Base64Dec(unsigned char *buf,const unsigned char*text,int size);
		int Base64Enc(unsigned char *buf,const unsigned char*text,int size); 


		//字符串替代
        void replace_all(std::string &str, const std::string &old_value, const std::string &new_value);

        //获取当前日期
        std::string CurrentDate();

        /*从字符串获取指定key值的内容
        *    内容以delimiter分隔
        *    格式：key1=value1 key2=value2 ...
        */
        std::string GetValueBykey(const char *szBuf, const char *key, const char* delimiter);

    }
}


#pragma pack(pop)

#endif


