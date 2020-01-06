#pragma once

#include <Eigen/Geometry>
#include <opencv2/opencv.hpp>

#include "IVoxelRenderer.h"
#include "RayVoxelMarcher.h"
#include "Voxel.h"

class CPURenderer : public IVoxelRenderer
{
public:
	CPURenderer();
	virtual void render(const VoxelMap& map, const Camera& cam);
private:
	std::vector<uint8_t> _renderPixel(const VoxelMap& map, const Camera& cam, uint32_t x, uint32_t y);
	std::vector<uint8_t> _rayTraceMap(const VoxelMap& map, Ray& ray);
	std::vector<uint8_t> _rayTraceChunk(const VoxelMap& map, const VoxelChunk& chunk, Ray& ray);
	std::vector<uint8_t> _rayTraceVoxel(const Voxel& vox, const Ray& ray);
	std::vector<uint8_t> mixRayColor(const std::vector<uint8_t>& color, const Ray& ray);
	Voxel getVoxelData(const VoxelMap& map, const VoxelPyramid& pyram, const std::vector<uint32_t>& pos);
};