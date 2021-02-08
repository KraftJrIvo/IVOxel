#pragma once

#include <vector>
#include <tuple>

#include "types.h"
#include "VoxelChunk.h"

struct VoxelMapFormat
{
	VoxelMapFormat()
	{}

	VoxelMapFormat(VoxelChunkFormat VoxelChunkFormat, VoxelFormat voxelFormat);

	VoxelFormat voxelFormat;
	VoxelChunkFormat chunkFormat;

	uint8_t sizeInBytes;
	uint8_t sizeInBytesType;
	uint8_t sizeInBytesColor;
	uint8_t sizeInBytesNeighbourInfo;

	uint32_t getSizeInBytes(uint32_t nVoxels, bool alignToFourBytes = true) const;

	VoxelChunk unformatChunk(VoxelTypeStorer& vts, const uint8_t* header, const uint8_t* data, bool alignToFourBytes = true) const;
	VoxelChunkState getChunkState(const uint8_t* data, bool alignToFourBytes = true) const;
	std::vector<glm::uvec3> unformatParals(ParalsInfoFormat format, const uint8_t* data) const;
	VoxelState getVoxelState(const uint8_t* data) const;
};