#include "Camera.h"

Camera::Camera() :
	_fov(90),
	_resolution({ 640, 480 })
{
}

Camera::Camera(float fov, const std::vector<uint16_t>& resolution) :
	_fov(fov),
	_resolution(resolution)
{
}

void Camera::move(const std::vector<float>& delta)
{
	for (uint8_t i = 0; i < DIMENSIONS; ++i)
		_translation[i] += delta[i];
}

void Camera::rotate(const std::vector<float>& delta)
{
	for (uint8_t i = 0; i < DIMENSIONS; ++i)
		_rotation[i] += delta[i];
}
