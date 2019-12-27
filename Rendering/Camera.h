#pragma once

#include <vector>

#include "types.h"

class Camera
{
public:
	Camera();
	Camera(float fov, const std::vector<size_t>& resolution);

	void move(const std::vector<float>& delta);
	void rotate(const std::vector<float>& delta);

private:
	float _fov;
	std::vector<size_t> _resolution;
	std::vector<float> _translation;
	std::vector<float> _rotation;
};
