#pragma once

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

	std::vector<uint8_t> formatVoxel(const Voxel& voxel, const std::vector<uint8_t>& neighs, const std::vector<uint8_t>& parals, bool alignToFourBytes = true) const;
	Voxel unformatVoxel(const uint8_t* data, uint8_t power) const;

	std::vector<uint8_t> formatChunkHeader(const VoxelChunk& chunk, uint32_t voxDataOffset, const std::vector<uint8_t>& parals, bool alignToFourBytes = true) const;
	std::vector<uint8_t> formatChunk(const VoxelChunk& chunk, bool alignToFourBytes = true) const;
	VoxelChunk unformatChunk(const uint8_t* header, const uint8_t* data, bool alignToFourBytes = true) const;
};