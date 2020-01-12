#include "RayVoxelMarcher.h"

#include <algorithm>

#include <types.h>

RayVoxelMarcher::RayVoxelMarcher() :
	_finished(false),
	_curPos({0,0,0}),
	_direction({0,0,0}),
	_lastResult({0,0,0})
{
}

void RayVoxelMarcher::setStart(const Ray& ray)
{
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

	pathX = (pathX == 0) ? 0.001f : pathX;
	pathY = (pathY == 0) ? 0.001f : pathY;
	pathZ = (pathZ == 0) ? 0.001f : pathZ;

	std::vector<float> diffs = { _direction[X]/pathX, _direction[Y]/pathY, _direction[Z]/pathZ };

	float maxDiff = std::max(diffs[X], std::max(diffs[Y], diffs[Z]));

	std::vector<int8_t> result = {
		char((1 - 2 * xIsNegative) * (fabs(maxDiff - diffs[X]) < 0.001f)),
		char((1 - 2 * yIsNegative) * (fabs(maxDiff - diffs[Y]) < 0.001f)),
		char((1 - 2 * zIsNegative) * (fabs(maxDiff - diffs[Z]) < 0.001f))
	};

	std::vector<float> intersection = {
		(result[X] != 0) ? (float(!xIsNegative) - vecStart[X]) : ((result[Y] != 0) ? (pathY * _direction[X] / _direction[Y]) : (pathZ * _direction[X] / _direction[Z])),
		(result[Y] != 0) ? (float(!yIsNegative) - vecStart[Y]) : ((result[X] != 0) ? (pathX * _direction[Y] / _direction[X]) : (pathZ * _direction[Y] / _direction[Z])),
		(result[Z] != 0) ? (float(!zIsNegative) - vecStart[Z]) : ((result[X] != 0) ? (pathX * _direction[Z] / _direction[X]) : (pathY * _direction[Z] / _direction[Y]))
	};

	for (uint8_t i = 0; i < DIMENSIONS; ++i)
	{
		_curPos[i] += intersection[i];
		_finished |= ((_curPos[i] <= maxMins[i].first && _direction[i] < 0) || _curPos[i] >= maxMins[i].second && _direction[i] > 0);
	}

	_lastResult = result;

	return result;
}

std::vector<float> RayVoxelMarcher::getCurEntryPoint()
{
	std::vector<float> result = {0,0,0};

	auto curPos = _curPos;

	for (uint8_t i = 0; i < DIMENSIONS; ++i)
	{
		while (curPos[i] < 0) curPos[i]++;
		result[i] = (_lastResult[i] == -1) ? 0.999f : (curPos[i] - floor(curPos[i]));
	}

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
