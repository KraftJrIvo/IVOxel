#pragma once

#include <vector>

enum LightType : unsigned char {
	NONE,
	AMBIENT,
	GLOBAL,
	LOCAL
};

struct Light
{
	Light(LightType type = NONE, const std::vector<uint8_t> rgba = { 0,0,0,0 }, const std::vector<float> position = { 0,0,0 });

	LightType type;
	std::vector<float> position;
	std::vector<uint8_t> rgba;
};
