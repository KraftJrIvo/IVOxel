#pragma once

#include <vector>

#include "Ray.h"

class RayVoxelMarcher
{
public:
	RayVoxelMarcher();

	void setStart(const Ray& startRay);
	std::vector<float> marchAndGetNextDir(const std::vector<std::pair<int32_t, int32_t>>& maxMins, float side = 1.0f);
	std::vector<float> getCurEntryPoint(float side = 1.0f);
	std::vector<float> getAbsPos();
	bool checkFinished();
private:
	bool _finished;
	std::vector<float> _curPos;
	std::vector<float> _lastResult;
	Eigen::Vector3f _direction;
};