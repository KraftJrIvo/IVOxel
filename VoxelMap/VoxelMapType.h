#pragma once


#include "types.h"

struct VoxelMapType
{
	VoxelMapType();
	VoxelMapType(VoxelTypeFormat tf, VoxelColorFormat cf, VoxelNeighbourInfoFormat nif);

	VoxelTypeFormat typeFormat;
	VoxelColorFormat colorFormat;
	VoxelNeighbourInfoFormat neightInfoFormat;

	uint8_t sizeInBytes;

	uint8_t computeSizeInBytes();
};