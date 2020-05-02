#include "FPSCounter.h"

#include <iostream>

FPSCounter::FPSCounter()
{
	_timer = std::chrono::steady_clock();
	_lastTime = _timer.now();
	_frameCounter = 0;
}

void FPSCounter::tellFPS(uint32_t rateInMillis)
{
	++_frameCounter;
	if (_lastTime + std::chrono::milliseconds(rateInMillis) < _timer.now()) {
		_lastTime = _timer.now();
		std::cout << "FPS: " << _frameCounter << std::endl;
		_frameCounter = 0;
	}
}
