#pragma once

#include <vector>

#include "Ray.h"

class RayVoxelMarcher
{
public:
	RayVoxelMarcher();

	void setStart(const Ray& startRay, float cubeSide);
	std::vector<int8_t> marchAndGetNextDir(const std::vector<std::pair<int32_t, int32_t>>& maxMins);
	std::vector<float> getCurEntryPoint();
	std::vector<float> getAbsPos();
	bool checkFinished();
private:
	bool _finished;
	float _cubeSide;
	std::vector<float> _curPos;
	std::vector<int8_t> _lastResult;
	Eigen::Vector3f _direction;
};