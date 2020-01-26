#include "Ray.h"

#include <algorithm>

#include <types.h>

Ray::Ray() :
	start({0,0,0}),
	direction({1,1,1}),
	strength(0),
	color({0,0,0}),
	bouncesLeft(0),
	length(0),
	lightRay(false)
{
}

Ray::Ray(uint8_t nBounces, const std::vector<float>& _start, const Eigen::Vector3f& dir, float str, bool _lightRay, std::vector<uint8_t> _color) :
	start(_start),
	direction(dir),
	strength(str),
	color(_color),
	bouncesLeft(nBounces),
	length(0),
	lightRay(_lightRay)
{
	normalize();
}

void Ray::normalize()
{
	float hypot = sqrt(direction[X] * direction[X] + direction[Y] * direction[Y] + direction[Z] * direction[Z]);
	direction = { direction[X] / hypot, direction[Y] / hypot, direction[Z] / hypot };
}

void Ray::mixColor(const std::vector<uint8_t>& _color)
{
	for (uint8_t i = 0; i < RGB; ++i)
		color[i] = _color[i] * strength;
}

void Ray::bounce(const std::vector<uint8_t>& surfaceColor, const std::vector<float>& surfaceNormal, float strengthToll)
{
	bouncesLeft--;
	mixColor(surfaceColor);
	strength -= strength * strengthToll;

	//TODO
	//r=d−2(d⋅n)n
}
