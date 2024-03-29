#pragma once

#include "VoxelChunkPyramid.h"
#include "Voxel.h"

class VoxelChunk;

struct VoxelChunkFormat
{
	ChunkFullnessFormat fullness;
	ChunkOffsetFormat   offset;
	ChunkSizeFormat     size;
	ParalsInfoFormat    parals;

	uint32_t getSizeInBytes(bool alignToFourBytes = true) const;
	std::vector<uint8_t> formatChunkHeader(const VoxelChunk& chunk, uint32_t voxDataOffset, const std::vector<uint8_t>& parals, bool alignToFourBytes = true) const;
	std::vector<uint8_t> formatChunk(const VoxelChunk& chunk, bool alignToFourBytes = true) const;
};

struct VoxelChunkState
{
	uint8_t fullness;
	uint32_t voxOffset;
	uint8_t base;
	uint8_t power;
	uint8_t side;
	std::vector<glm::uvec3> parals;
};

class VoxelChunk
{
public:
	VoxelChunk()
	{ }
	
	VoxelChunk(const std::vector<Voxel>& voxels, const VoxelChunkFormat& format, const VoxelFormat& voxFormat, bool alignToFourBytes = true);
	
	bool modified = false;
	uint32_t side;
	float minOffset;
	bool empty;

	std::vector<uint8_t> parals;
	std::vector<uint8_t> data;

	VoxelChunkFormat format;
	VoxelFormat voxFormat, voxFormatForPyr;
	VoxelChunkPyramid pyramid;

	void changeVoxels(const std::vector<VoxelModifyData>& voxelsMod);
	Voxel getVoxel(const std::vector<float>& voxInChunkPos) const;
	std::vector<uint8_t> getNeighbours(const Voxel& vox, const std::vector<float>& voxInChunkPos, const VoxelNeighbourInfoFormat& format) const;
	std::vector<uint8_t> getVoxParals(const Voxel& vox, const std::vector<float>& voxInChunkPos, const ParalsInfoFormat& format) const;
	float getClosestSidePointDistance(const std::vector<int8_t>& dir);
	bool isEmpty() const;

private:

	void _buildPyramid(const std::vector<Voxel>& voxels, bool alignToFourBytes = true);
	bool _checkParal(const std::vector<float>& from, const std::vector<float>& to, float offset) const;
};