#pragma once

#include <vector>

struct Light
{
	Light();
	Light(const std::vector<float>& position, const std::vector<uint8_t>& rgba);

	std::vector<float> position;
	std::vector<uint8_t> rgba;
};
