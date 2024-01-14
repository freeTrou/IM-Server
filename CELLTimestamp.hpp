#ifndef _CELLTIMESTAMP_HPP_
#define _CELLTIMESTAMP_HPP_

#include <chrono>//定时器头文件
using namespace std::chrono;

class CELLTimestamp
{
public:
	CELLTimestamp()
	{
		update();
	}
	~CELLTimestamp()
	{

	}
	//刷新
	void update()
	{
		_begin = high_resolution_clock::now();
	}
	//获取当前秒
	double getElapsedSecond()
	{
		return getElaosedTimeInMicroSec() * 0.000001;
	}
	//获取毫秒
	double getElapsedTineInMillisec()
	{
		return getElaosedTimeInMicroSec() * 0.001;
	}
	//获取微秒
	long long getElaosedTimeInMicroSec()
	{
		//high_resolution_clock::now()-_begin 经过的时间
		return duration_cast<microseconds>(high_resolution_clock::now()-_begin).count();//转成微秒
	}
protected:
	time_point<high_resolution_clock> _begin;//高进度计时器开始
};

#endif // !_CELLTIMESTAMP_HPP_

