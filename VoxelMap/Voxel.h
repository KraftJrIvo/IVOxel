#pragma once

#include "NeighbourConnections.h"

class Voxel
{
public:
	Voxel();
	Voxel(uint8_t power, int32_t type, const std::vector<uint8_t>& rgba, const NeighbourConnections& neighs);

	uint8_t power;
	int32_t type;
	std::vector<uint8_t> color;
	NeighbourConnections neighs;
};
