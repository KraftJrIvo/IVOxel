#pragma once

#include <Eigen/Geometry>
#include <opencv2/opencv.hpp>

#include <Voxel.h>

#include "IVoxelRenderer.h"
#include "RayVoxelMarcher.h"

class CPURenderer : public IVoxelRenderer
{
public:
	CPURenderer();
	virtual void render(const VoxelMap& map, Camera& cam);
	virtual void renderVideo(VoxelMap& map, Camera& cam);
private:
	std::vector<uint8_t> _renderPixel(const VoxelMap& map, const Camera& cam, uint32_t x, uint32_t y) const;
};