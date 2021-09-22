
#ifndef __SH_STD_FUN__IN_
#define __SH_STD_FUN__IN_
#include "sysheads.h"
using namespace std;
namespace shstd
{
    namespace fun
    {
        const char * GetPeerIp(int nSocket);
        unsigned long Hex2Int(const char *szHex, unsigned int nSize);
		bool DelDir(const char *szDir); //递归删除目录
        bool MakeDir(const char *szDstDir);
        bool IsDirExist(const char *szDir);

		bool DelFile(const char *szFile); //删除文件
        bool IsFileExist(const char *szFile);
		int GetFileSize(const char * file); //取得文件大小
        void MakeLower(char *szStr, unsigned int nLen = 0);
        void MakeUpper(char *szStr, unsigned int nLen = 0);
        string MakeUpper(const string & strSrc);
        string MakeLower(const string & strSrc);
        string CleanStr(const string & str); //去掉不可见字符
        bool ParseGetElems(const string &str, vector<string> &vecElem, char chSeparator = ',');
        bool SetPathOwner(const string &strPath, const string &strUser);
		bool GetPathOwner(const string &strPath, string &strUser);
        bool SetPathMod(const string & strPath, int nMode);

        int GetParamPos(int argc, const char **argv, const char *szFind);        

        bool GetPTree(pid_t pid, set<pid_t> & setPTree);
		const char * GetProgramPath();


        struct SVR_ACCESS_INFO
		{
			string strDns; //别人访问用
            string strIp; //服务自己绑定用
		};
		bool ParseDnsAndIp(const string & strAccessInfo, SVR_ACCESS_INFO &sai);
		bool IsIp(const string & strIp);
		bool IsConnected(const char *ip, unsigned short port); //测试ip和端口是否可以连通

    }
}

void shstd_printf(const char *fmt, ...);

#endif


