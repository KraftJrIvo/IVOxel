#pragma once

#include <vector>

class RayVoxelMarcher
{
public:
	RayVoxelMarcher();

	void setStart(const std::vector<int32_t>& start);
	void setDirection(const std::vector<int32_t>& dir);
	std::vector<int8_t> getNextVoxelDir();
private:
	std::vector<int32_t> _curPos;
	std::vector<int32_t> _direction;
};