#pragma once

#include "IReflectiveObject.h"
#include <NeighbourConnections.h>

class Voxel : IReflectiveObject
{
public:
	Voxel();
	Voxel(uint8_t power, int32_t type, const std::vector<uint8_t>& rgba, const NeighbourConnections& neighs);
	std::vector<Ray> reflect(const Ray& entryRay);
private:
	uint8_t _power;
	int32_t _type;
	std::vector<uint8_t> _color;
	NeighbourConnections _neighs;
};
