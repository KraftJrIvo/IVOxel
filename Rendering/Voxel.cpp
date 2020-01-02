#include "Voxel.h"

Voxel::Voxel() :
	_power(0),
	_type(-1),
	_color({ 0,0,0,0 })
{
}

Voxel::Voxel(uint8_t power, int32_t type, const std::vector<uint8_t>& rgba, const NeighbourConnections& neighs) :
	_power(power),
	_type(type),
	_color(rgba),
	_neighs(neighs)
{
}

std::vector<Ray> Voxel::reflect(const Ray& entryRay)
{
	//TODO

	return {};
}
