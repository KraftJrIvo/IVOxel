#pragma once

#include <Eigen/Core>

#include <types.h>

struct Ray
{
	Ray();
	Ray(uint8_t nBounces, const std::vector<float>& _start, const Eigen::Vector3f& dir, float str, std::vector<uint8_t> _color = {0, 0, 0});

	void normalize();
	void mixColor(const std::vector<uint8_t>& color);
	void bounce(const std::vector<uint8_t>& surfaceColor, const std::vector<float>& surfaceNormal, float strengthToll);

	uint8_t bouncesLeft;
	float strength;
	float length;
	Eigen::Vector3f direction;
	std::vector<float> start;
	std::vector<uint8_t> color;
};
