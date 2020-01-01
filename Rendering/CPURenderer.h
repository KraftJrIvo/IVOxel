#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <opencv2/opencv.hpp>

#include "IVoxelRenderer.h"

class CPURenderer
{
public:
	CPURenderer();
	virtual void render(const VoxelMap& map, const Camera& cam);
private:
	std::vector<uint8_t> _renderPixel(const VoxelMap& map, const Camera& cam, uint32_t x, uint32_t y);
	//Eigen _renderPixel(const VoxelMap& map, const Camera& cam, uint32_t x, uint32_t y);
};