#pragma once

#include <vector>

struct Light
{
	Light();
	Light(const std::vector<float>& position, const std::vector<unsigned char>& rgba);

	std::vector<float> position;
	std::vector<unsigned char> rgba;
};
