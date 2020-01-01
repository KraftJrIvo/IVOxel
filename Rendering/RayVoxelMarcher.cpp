#include "RayVoxelMarcher.h"

#include <types.h>

RayVoxelMarcher::RayVoxelMarcher() :
	_curPos({0,0,0}),
	_direction({0,0,0})
{
}

void RayVoxelMarcher::setStart(const std::vector<int32_t>& start)
{
	_curPos = start;
}

void RayVoxelMarcher::setDirection(const std::vector<int32_t>& dir)
{
	_direction = dir;
}

std::vector<int8_t> RayVoxelMarcher::getNextVoxelDir()
{
	bool xIsNegative = (_direction[X] < 0);
	bool yIsNegative = (_direction[Y] < 0);
	bool zIsNegative = (_direction[Z] < 0);

	std::vector<float> vecStart = {
		(float)fabs(_curPos[X]) - uint64_t(floor(fabs(_curPos[X]))),
		(float)fabs(_curPos[Y]) - uint64_t(floor(fabs(_curPos[Y]))),
		(float)fabs(_curPos[Z]) - uint64_t(floor(fabs(_curPos[Z])))
	};

	float pathX = xIsNegative ? -vecStart[X] : (1 - vecStart[X]);
	float pathY = yIsNegative ? -vecStart[Y] : (1 - vecStart[Y]);
	float pathZ = zIsNegative ? -vecStart[Z] : (1 - vecStart[Z]);

	float intersectionX = vecStart[X] + (_direction[X] / _direction[Y]) * pathY;
	bool xIsFasterThanY = (intersectionX < 0 || intersectionX > 1);

	float intersectionZ = vecStart[Z] +
		(xIsFasterThanY ?
			((_direction[Z] / _direction[X]) * pathX) :
			((_direction[Z] / _direction[Y]) * pathY)
		);
	bool zIsEvenFaster = (intersectionZ < 0 || intersectionZ > 1);

	std::vector<int8_t> result = {
		char((1 - 2 * xIsNegative) * (xIsFasterThanY && !zIsEvenFaster)),
		char((1 - 2 * yIsNegative) * (!xIsFasterThanY && !zIsEvenFaster)),
		char((1 - 2 * zIsNegative) * zIsEvenFaster)
	};

	for (uint8_t i = 0; i < DIMENSIONS; ++i)
		_curPos[i] += result[i];

	return result;
}
