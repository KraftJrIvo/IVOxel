#pragma once

#include "VoxelChunkPyramid.h"
#include "Voxel.h"

class VoxelChunk
{
public:
	VoxelChunk()
	{ }
	
	VoxelChunk(const std::vector<Voxel>& voxels, const VoxelChunkFormat& format);

	bool modified = false;
	uint32_t side;
	float minOffset;

	VoxelChunkFormat format;
	VoxelChunkPyramid pyramid;

	void changeVoxels(const std::vector<VoxelModifyData>& voxels);
	Voxel getVoxel(const std::vector<float>& voxInChunkPos) const;
	std::vector<uint8_t> getNeighbours(const Voxel& vox, const std::vector<float>& voxInChunkPos, const VoxelNeighbourInfoFormat& format, std::vector<VoxelType> connectableTypes = {}) const;
	std::vector<uint8_t> getVoxParals(const Voxel& vox, const std::vector<float>& voxInChunkPos, const ParalsInfoFormat& format, std::vector<VoxelType> transparentTypes = {}) const;
	bool isEmpty() const;

private:

	void _buildPyramid(const std::vector<Voxel>& voxels);
};