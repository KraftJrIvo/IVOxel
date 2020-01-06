#include "Voxel.h"

Voxel::Voxel() :
	power(0),
	type(-1),
	color({ 0,0,0,0 })
{
}

Voxel::Voxel(uint8_t _power, int32_t _type, const std::vector<uint8_t>& rgba, const NeighbourConnections& _neighs) :
	power(_power),
	type(_type),
	color(rgba),
	neighs(_neighs)
{
}

std::vector<Ray> Voxel::reflect(const Ray& entryRay)
{
	//TODO

	return {};
}
