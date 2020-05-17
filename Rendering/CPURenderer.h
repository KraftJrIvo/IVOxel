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
	std::vector<uint8_t> _rayTraceMap(const VoxelMap& map, Ray& ray) const;
	std::vector<uint8_t> _rayTraceChunk(const VoxelMap& map, const VoxelChunk& chunk, Ray& ray, const std::vector<int32_t>& curChunkPos) const;
	std::vector<uint8_t> _rayTraceVoxel(const VoxelMap& map, const VoxelChunk& chunk, const Voxel& vox, Ray& ray, const std::vector<float>& absPose, float voxSide) const;
	Voxel getVoxelData(const VoxelMap& map, const std::vector<uchar>& pyramData, const std::vector<uint32_t>& pos) const;
};