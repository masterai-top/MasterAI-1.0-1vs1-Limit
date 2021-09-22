
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <sstream>



#ifndef __SH_ASYN_LOG_DEF__
#define __SH_ASYN_LOG_DEF__

namespace shstd
{
	const static unsigned int LOG_BUF_SIZE = 8*1024*1024; //8M
	const static unsigned int ALARM_SIZE = 6*1024*1024; //6M

	struct SS_LOG
	{
		std::stringstream ss;
		unsigned int cnt;
	public:
		void Clear()
		{
			ss.str("");	
			ss.clear();
			cnt = 0;
		}
		SS_LOG(const SS_LOG &al)
		{
			Clear();
			ss<<al.ss.str();
			cnt = al.cnt;
		}
		SS_LOG()
		{
			Clear();
		}
	};
	struct BUF_LOG
	{
		unsigned int cnt;
		unsigned int nSize;
		char logbuf[LOG_BUF_SIZE];
	public:
		BUF_LOG() {memset(this, 0, sizeof(BUF_LOG));}
		void Clear() {memset(this, 0, sizeof(BUF_LOG));}
	};

	struct ASYN_LOG
	{
		SS_LOG sslog;
		BUF_LOG buflog;
		void Clear()
		{
			sslog.Clear();
			buflog.Clear();
		}
		unsigned int GetTotalLogCnt()
		{
			return sslog.cnt + buflog.cnt;
		}
		//溢出
		void CheckOverflow() //检查数量溢出以及大小溢出问题
		{
			if (buflog.nSize > ALARM_SIZE) //超出警戒值，移到流中
			{
				sslog.ss << buflog.logbuf;
				sslog.cnt += buflog.cnt;
				buflog.Clear();
			}
		}
	};


}


#endif


