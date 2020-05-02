#pragma once

#include <chrono>

class FPSCounter
{
public:
	FPSCounter();
	void tellFPS(uint32_t rateInMillis);

private:
	std::chrono::steady_clock _timer;
	std::chrono::time_point<std::chrono::steady_clock> _lastTime;
	uint64_t _frameCounter;
};
