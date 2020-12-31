#pragma once

#include "VoxelChunkPyramid.h"
#include "Voxel.h"

class VoxelChunk
{
public:
	VoxelChunk();
	VoxelChunk(const std::vector<Voxel>& voxels, const VoxelMapFormat& format);

	bool modified = false;
	uint32_t side;

	VoxelMapFormat format;
	VoxelChunkPyramid pyramid;

	void changeVoxels(const std::vector<VoxelModifyData>& voxels);
	Voxel getVoxel(const std::vector<float>& chunkPos);
	bool isEmpty();

private:

	void _buildPyramid(const std::vector<Voxel>& voxels);
};