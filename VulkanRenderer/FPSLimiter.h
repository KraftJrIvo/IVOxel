#pragma once

#include <chrono>

class FPSLimiter
{
public:
	FPSLimiter(uint32_t fps);
	void reset();
	void tick();

	uint32_t getTimeToWait();

private:
	std::chrono::steady_clock _timer;
	std::chrono::time_point<std::chrono::steady_clock> _firstTime;
	std::chrono::time_point<std::chrono::steady_clock> _lastTime;
	std::chrono::nanoseconds _oneFrameTime;
	std::chrono::nanoseconds _targetTime;
	std::chrono::nanoseconds _timeToWait;
	uint64_t _frameCounter;
	uint64_t _fps;
};
