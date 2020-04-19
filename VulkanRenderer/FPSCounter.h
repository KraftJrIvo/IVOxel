#pragma once

#include <chrono>

class FPSCounter
{
public:
	FPSCounter();
	void tellFPS(uint32_t rateInMillis);

private:
	std::chrono::steady_clock timer;
	std::chrono::time_point<std::chrono::steady_clock> last_time;
	uint64_t frame_counter;
	uint64_t fps;
};
