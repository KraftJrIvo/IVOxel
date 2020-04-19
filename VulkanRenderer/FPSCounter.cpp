#include "FPSCounter.h"

#include <iostream>

FPSCounter::FPSCounter()
{
	std::chrono::steady_clock timer = std::chrono::steady_clock();
	std::chrono::time_point<std::chrono::steady_clock> last_time = timer.now();
	uint64_t frame_counter = 0;
	uint64_t fps = 0;
}

void FPSCounter::tellFPS(uint32_t rateInMillis)
{
	++frame_counter;
	if (last_time + std::chrono::milliseconds(rateInMillis) < timer.now()) {
		last_time = timer.now();
		fps = frame_counter;
		frame_counter = 0;
		std::cout << "FPS: " << fps << std::endl;
	}
}
