#include "Camera.h"

Camera::Camera() :
	fov(90),
	resolution({ 640, 480 }),
	translation({ 0,0,0 }),
	rotation({ 0,0,0 })
{
}

Camera::Camera(float fov, const std::vector<uint16_t>& resolution) :
	fov(fov),
	resolution(resolution),
	translation({ 0,0,0 }),
	rotation({ 0,0,0 })
{
}

void Camera::move(const std::vector<float>& delta)
{
	for (uint8_t i = 0; i < 3; ++i)
		translation[i] += delta[i];
}

void Camera::rotate(const std::vector<float>& delta)
{
	for (uint8_t i = 0; i < 3; ++i)
		rotation[i] += delta[i];
}
