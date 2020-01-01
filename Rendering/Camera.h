#pragma once

#include "types.h"

class Camera
{
public:
	Camera();
	Camera(float fov, const std::vector<uint16_t>& resolution);

	void move(const std::vector<float>& delta);
	void rotate(const std::vector<float>& delta);

	float fov;
	std::vector<uint16_t> resolution;
	std::vector<float> translation;
	std::vector<float> rotation;

};
