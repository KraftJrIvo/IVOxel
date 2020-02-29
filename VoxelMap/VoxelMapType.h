#pragma once

#include <tuple>

#include "types.h"

struct VoxelMapType
{
	VoxelMapType();
	VoxelMapType(VoxelTypeFormat tf, VoxelColorFormat cf, VoxelNeighbourInfoFormat nif);

	VoxelTypeFormat typeFormat;
	VoxelColorFormat colorFormat;
	VoxelNeighbourInfoFormat neightInfoFormat;

	uint8_t sizeInBytes;
	uint8_t sizeInBytesType;
	uint8_t sizeInBytesColor;
	uint8_t sizeInBytesNeighbourInfo;

	uint8_t computeSizeInBytes();

	std::vector<uint8_t> formatType(int32_t type, uint8_t orientation = NO_DIR, bool flipX = false, bool flipY = false, bool flipZ = false);
	std::vector<uint8_t> formatColor(uint8_t r, uint8_t g = 0, uint8_t b = 0, uint8_t a = 0);
	std::vector<uint8_t> formatNeighbours(const std::vector<int32_t>& neighsTypes, int32_t type);
	
	utils::VoxelData unformatVoxelData(const uint8_t* data);
};