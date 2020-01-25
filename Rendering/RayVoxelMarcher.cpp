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

std::vector<float> RayVoxelMarcher::marchAndGetNextDir(const std::vector<std::pair<int32_t, int32_t>>& maxMins, float side)
{
	std::vector<int8_t> isNegative(3);
	for (uint8_t i = 0; i < DIMENSIONS; ++i)
		isNegative[i] = (_direction[i] < 0);

	std::vector<float> vecStart = getCurEntryPoint(side);

	std::vector<float> path(3);
	for (uint8_t i = 0; i < DIMENSIONS; ++i)
	{
		path[i] = isNegative[i] ? -vecStart[i] : (1 - vecStart[i]);
		path[i] = (path[i] == 0) ? 0.001f : path[i];
	}

	std::vector<float> diffs(3);
	for (uint8_t i = 0; i < DIMENSIONS; ++i)
		diffs[i] = _direction[i] / path[i];

	float maxDiff = std::max(diffs[X], std::max(diffs[Y], diffs[Z]));

	std::vector<float> result(3, 0);
	for (uint8_t i = 0; i < DIMENSIONS; ++i)
		result[i] = char((1 - 2 * isNegative[i]) * (fabs(maxDiff - diffs[i]) < 0.001f));

	std::vector<float> intersection(3);
	for (uint8_t i = 0; i < DIMENSIONS; ++i)
	{
		uint8_t otherCoord1 = (i == X) ? Y : X;
		uint8_t otherCoord2 = (i == Z) ? Y : Z;
		intersection[i] = (result[i] != 0) ? 
			(float(!isNegative[i]) - vecStart[i]) : 
			((result[otherCoord1] != 0) ? 
				(path[otherCoord1] * _direction[i] / _direction[otherCoord1]) : 
				(path[otherCoord2] * _direction[i] / _direction[otherCoord2]));
		_curPos[i] += intersection[i] * side;
		_finished |= ((_curPos[i] <= maxMins[i].first && _direction[i] < 0) || _curPos[i] >= maxMins[i].second && _direction[i] > 0);
	}

	for (uint8_t i = 0; i < DIMENSIONS; ++i)
	{
		uint32_t voxStart = floor(vecStart[i] * side);
		if (result[i] == 0) result[i] = (floor((vecStart[i] + intersection[i]) * side) - voxStart) / side;
	}

	_lastResult = result;

	return result;
}

std::vector<float> RayVoxelMarcher::getCurEntryPoint(float side)
{
	std::vector<float> result = {0,0,0};

	std::vector<float> curPos = { _curPos[0]/side, _curPos[1]/side, _curPos[2]/side };

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
