#ifndef __TIME_PIECE__
#define __TIME_PIECE__

#include <sys/time.h>
#include <cstdlib>

class CTimePiece {
private:
	struct timeval start_tv_;
	int total_;
public:
	CTimePiece(){
		gettimeofday(&start_tv_, NULL);
		total_ = 0;
	}

	void Restart() {
		gettimeofday(&start_tv_, NULL);
		total_ = 0;
	}

	//计算构造开始的时间
	int ToNow() const {
		struct timeval tn;
		gettimeofday(&tn, NULL);
		return (int)((tn.tv_sec  - start_tv_.tv_sec ) * 1000 + (tn.tv_usec - start_tv_.tv_usec) / 1000);
	}
	
	unsigned int uToNow() const {
		struct timeval tn;
		gettimeofday(&tn, NULL);
		return (unsigned int)((tn.tv_sec  - start_tv_.tv_sec ) * 1000000 + (tn.tv_usec - start_tv_.tv_usec));
	}

	//计算和制定值的差距
	static int ToNow(struct timeval & tv)
	{
		struct timeval tn;
		gettimeofday(&tn, NULL);
		return (int)((tn.tv_sec  - tv.tv_sec ) * 1000 + (tn.tv_usec - tv.tv_usec) / 1000);
	}

	static unsigned int uToNow(struct timeval & tv)
	{
		struct timeval tn;
		gettimeofday(&tn, NULL);
		return (unsigned int)((tn.tv_sec  - tv.tv_sec ) * 1000000 + (tn.tv_usec - tv.tv_usec));
	}
    
	static int Diff(struct timeval & tvLeft, struct timeval & tvRight)
	{
		return (int)((tvLeft.tv_sec  - tvRight.tv_sec ) * 1000 + (tvLeft.tv_usec - tvRight.tv_usec) / 1000);
	}

	//暂停及时
	void Stop() {total_ += ToNow();	}

	//从新的时刻继续
	void Continue() { gettimeofday(&start_tv_, NULL); }

	//总共的时间
	int Total() { return total_;}
}; 

#endif



