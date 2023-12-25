#ifndef _CELLTimestamp_hpp_
#define _CELLTimestamp_hpp_

#include <chrono>
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
	void update()
	{
		_begin = high_resolution_clock::now();
	}

	// 获取秒
	double getElapsedSend()
	{
		return getElapsedTimeInMicroSec() * 0.000001;
	}

	// 获取毫秒
	double getElapsedTimeInMilliSec()
	{
		return getElapsedTimeInMicroSec() * 0.001;
	}

	// 获取微秒
	long long getElapsedTimeInMicroSec()
	{
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
	}


protected:
	time_point<high_resolution_clock> _begin;
};

#endif

