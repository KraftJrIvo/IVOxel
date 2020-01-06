#include "RayVoxelMarcher.h"

#include <algorithm>

#include <types.h>

RayVoxelMarcher::RayVoxelMarcher() :
	_finished(false),
	_cubeSide(0),
	_curPos({0,0,0}),
	_direction({0,0,0}),
	_lastResult({0,0,0})
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

	pathX = pathX == 0 ? 0.0000001f : pathX;
	pathY = pathY == 0 ? 0.0000001f : pathY;
	pathZ = pathZ == 0 ? 0.0000001f : pathZ;

	std::vector<float> diffs = { _direction[X]/pathX, _direction[Y]/pathY, _direction[Z]/pathZ };

	float maxDiff = std::max(diffs[X], std::max(diffs[Y], diffs[Z]));

	std::vector<int8_t> result = {
		char((1 - 2 * xIsNegative) * (diffs[X] == maxDiff)),
		char((1 - 2 * yIsNegative) * (diffs[Y] == maxDiff)),
		char((1 - 2 * zIsNegative) * (diffs[Z] == maxDiff))
	};

	std::vector<float> intersection = {
		(diffs[X] == maxDiff) ? (float(!xIsNegative) - vecStart[X]) : ((diffs[Y] == maxDiff) ? (pathY * _direction[X] / _direction[Y]) : (pathZ * _direction[X] / _direction[Z])),
		(diffs[Y] == maxDiff) ? (float(!yIsNegative) - vecStart[Y]) : ((diffs[X] == maxDiff) ? (pathX * _direction[Y] / _direction[X]) : (pathZ * _direction[Y] / _direction[Z])),
		(diffs[Z] == maxDiff) ? (float(!zIsNegative) - vecStart[Z]) : ((diffs[X] == maxDiff) ? (pathX * _direction[Z] / _direction[X]) : (pathY * _direction[Z] / _direction[Y]))
	};

	for (uint8_t i = 0; i < DIMENSIONS; ++i)
	{
		_curPos[i] += intersection[i] * _cubeSide;
		_finished |= (_curPos[i] < maxMins[i].first || _curPos[i] > maxMins[i].second);
	}

	_lastResult = result;

	return result;
}

std::vector<float> RayVoxelMarcher::getCurEntryPoint()
{
	std::vector<float> result = {0,0,0};

	float dbg = floor(_curPos[Z] / _cubeSide);

	for (uint8_t i = 0; i < DIMENSIONS; ++i)
		result[i] = (_lastResult[i] == -1) ? 1.0f : (_curPos[i] - ((_curPos[i] > 0) ? floor(_curPos[i] / _cubeSide) : ceil(_curPos[i] / _cubeSide)));

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
