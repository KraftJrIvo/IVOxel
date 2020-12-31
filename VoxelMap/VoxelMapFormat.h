#pragma once

#include <tuple>

#include "types.h"

struct VoxelMapFormat
{
	VoxelMapFormat();
	VoxelMapFormat(ChunkFormat chunkFormat, VoxelFormat voxelFormat);

	VoxelFormat voxelFormat;
	ChunkFormat chunkFormat;

	uint8_t sizeInBytes;
	uint8_t sizeInBytesType;
	uint8_t sizeInBytesColor;
	uint8_t sizeInBytesNeighbourInfo;

	uint32_t getSizeInBytes(bool alignToFourBytes = true);

	std::vector<uint8_t> formatVoxel(const Voxel& chunk);
	Voxel unformatVoxel(const uint8_t* data);

	std::vector<uint8_t> formatChunkHeader(const VoxelChunk& chunk, uint32_t voxDataOffset);
	std::vector<uint8_t> formatChunk(const VoxelChunk& chunk);
	VoxelChunk unformatChunk(const uint8_t* header, const uint8_t* data);
};