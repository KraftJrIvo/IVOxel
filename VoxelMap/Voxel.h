#pragma once

#include "NeighbourConnections.h"

enum class VoxelType
{
	AIR,
	CUBE,
	SPHERE
};


class Voxel
{
public:
	Voxel();
	Voxel(uint8_t power, VoxelType type, const std::vector<uint8_t>& rgba, const NeighbourConnections& neighs);

	std::vector<uint32_t> getData();
	bool isEmpty();

	uint8_t power;
	VoxelType type;
	std::vector<uint8_t> color;
	NeighbourConnections neighs;
};

struct VoxelModifyData
{
	Voxel voxel;
	uint32_t x, y, z;
};