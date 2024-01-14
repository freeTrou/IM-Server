#ifndef _CELLTIMESTAMP_HPP_
#define _CELLTIMESTAMP_HPP_

#include <chrono>//��ʱ��ͷ�ļ�
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
	//ˢ��
	void update()
	{
		_begin = high_resolution_clock::now();
	}
	//��ȡ��ǰ��
	double getElapsedSecond()
	{
		return getElaosedTimeInMicroSec() * 0.000001;
	}
	//��ȡ����
	double getElapsedTineInMillisec()
	{
		return getElaosedTimeInMicroSec() * 0.001;
	}
	//��ȡ΢��
	long long getElaosedTimeInMicroSec()
	{
		//high_resolution_clock::now()-_begin ������ʱ��
		return duration_cast<microseconds>(high_resolution_clock::now()-_begin).count();//ת��΢��
	}
protected:
	time_point<high_resolution_clock> _begin;//�߽��ȼ�ʱ����ʼ
};

#endif // !_CELLTIMESTAMP_HPP_

