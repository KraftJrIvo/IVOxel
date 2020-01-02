#include "RayVoxelMarcher.h"

#include <algorithm>

#include <types.h>

RayVoxelMarcher::RayVoxelMarcher() :
	_finished(false),
	_cubeSide(0),
	_curPos({0,0,0}),
	_direction({0,0,0})
{
}

void RayVoxelMarcher::setStart(const Ray& ray, float cubeSide)
{
	_cubeSide = cubeSide;
	_curPos = ray.start;
	_direction = ray.direction;
}

std::vector<int8_t> RayVoxelMarcher::marchAndGetNextDir(const std::vector<std::pair<int32_t, int32_t>>& maxMins)
{
	bool xIsNegative = (_direction[X] < 0);
	bool yIsNegative = (_direction[Y] < 0);
	bool zIsNegative = (_direction[Z] < 0);

	std::vector<float> vecStart = getCurEntryPoint();

	float pathX = xIsNegative ? -vecStart[X] : (1 - vecStart[X]);
	float pathY = yIsNegative ? -vecStart[Y] : (1 - vecStart[Y]);
	float pathZ = zIsNegative ? -vecStart[Z] : (1 - vecStart[Z]);

	std::vector<float> diffs = { _direction[X]/pathX, _direction[Y]/pathY, _direction[Z]/pathZ };

	float maxDiff = std::max(diffs[X], std::max(diffs[Y], diffs[Z]));

	std::vector<int8_t> result = {
		char((1 - 2 * xIsNegative) * (diffs[X] == maxDiff)),
		char((1 - 2 * yIsNegative) * (diffs[Y] == maxDiff)),
		char((1 - 2 * zIsNegative) * (diffs[Z] == maxDiff))
	};

	for (uint8_t i = 0; i < DIMENSIONS; ++i)
	{
		_curPos[i] += result[i] * _cubeSide;
		_finished |= (_curPos[i] < maxMins[i].first || _curPos[i] > maxMins[i].second);
	}

	return result;
}

std::vector<float> RayVoxelMarcher::getCurEntryPoint()
{
	std::vector<float> result = {0,0,0};

	for (uint8_t i = 0; i < DIMENSIONS; ++i)
		result[i] = _curPos[i] - floor(_curPos[i] / _cubeSide);

	return result;
}

std::vector<float> RayVoxelMarcher::getAbsPos()
{
	return _curPos;
}

bool RayVoxelMarcher::checkFinished()
{
	return _finished;
}
