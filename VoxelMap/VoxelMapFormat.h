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

	VoxelChunk unformatChunk(const VoxelTypeStorer& vts, const uint8_t* header, const uint8_t* data, bool alignToFourBytes = true) const;
	VoxelChunkHeader unformatChunkHeader(const uint8_t* data, bool alignToFourBytes = true) const;
};