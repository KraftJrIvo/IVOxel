#include "Voxel.h"

Voxel::Voxel() :
	power(0),
	type(-1),
	color({ 0,0,0,0 })
{
}

Voxel::Voxel(uint8_t _power, VoxelType _type, const std::vector<uint8_t>& rgba, const NeighbourConnections& _neighs) :
	power(_power),
	type(_type),
	color(rgba),
	neighs(_neighs)
{
}

std::vector<uint32_t> Voxel::getData()
{
	std::vector<uint32_t> res(24, 0);

	auto ptr = res.data();
	bool empty = isEmpty();
	std::memcpy(ptr, &type, sizeof(uint32_t)); ptr += sizeof(uint32_t);
	std::memcpy(ptr, &type, sizeof(uint32_t)); ptr += sizeof(uint32_t);

	return res;
}

bool Voxel::isEmpty()
{
	return type == VoxelType::AIR;
}
