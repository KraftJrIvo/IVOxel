#include "FPSLimiter.h"

#include <iostream>
#include <thread>

FPSLimiter::FPSLimiter(uint32_t fps) :
	_fps(fps)
{
	reset();
}

void FPSLimiter::reset()
{
	_timer = std::chrono::steady_clock();
	_firstTime = _timer.now();
	_lastTime = _firstTime;
	_frameCounter = _fps;
	_targetTime = std::chrono::nanoseconds(uint32_t((1000.0f / _fps) * 1e6));
	_timeToWait = std::chrono::nanoseconds(0);
}

void FPSLimiter::tick()
{
	if ((_lastTime - _firstTime).count() < 1000000000)
		_targetTime = std::chrono::nanoseconds(uint32_t(((1000000000 - (_lastTime - _firstTime).count()) / _frameCounter)));
	else
		reset();

	auto realTimeElapsed = _timer.now() - _lastTime;
	if (realTimeElapsed < _targetTime)
	{
		auto start = std::chrono::system_clock::now();
		auto len = (_targetTime - realTimeElapsed).count();
		bool sleep = true;
		while (sleep)
		{
			auto now = std::chrono::system_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start);
			if (elapsed.count() > len)
				sleep = false;
		}
	}

	_frameCounter--;
	_lastTime = _timer.now();
}

uint32_t FPSLimiter::getTimeToWait()
{
	return _timeToWait.count();
}
