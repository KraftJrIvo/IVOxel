#include "Voxel.h"

Voxel::Voxel(uint8_t power_, VoxelType type_, VoxelOrientation orientation_, const std::vector<uint8_t>& rgba) :
	power(power_),
	type(type_),
	orientation(orientation_),
	color(rgba)
{ }

std::vector<uint32_t> Voxel::getData()
{
	std::vector<uint32_t> res(24, 0);

	auto ptr = res.data();
	bool empty = isEmpty();
	std::memcpy(ptr, &type, sizeof(uint32_t)); ptr += sizeof(uint32_t);
	std::memcpy(ptr, &type, sizeof(uint32_t)); ptr += sizeof(uint32_t);

	return res;
}

bool Voxel::isEmpty() const
{
	return type == VoxelType::AIR;
}

uint8_t Voxel::getOrientation() const
{
	switch (orientation)
	{
	case VoxelOrientation::NONE:
		return -1;
	case VoxelOrientation::DEFAULT:
		return 0;
	case VoxelOrientation::LEFT:
		return 1;
	case VoxelOrientation::RIGHT:
		return 2;
	case VoxelOrientation::DOWN:
		return 3;
	case VoxelOrientation::UP:
		return 4;
	case VoxelOrientation::BACK:
		return 5;
	case VoxelOrientation::FRONT:
		return 6;
	default:
		return -1;
	}
}

void Voxel::setOrientation(uint8_t orient)
{
	switch (orient)
	{
	case 255:
		orientation = VoxelOrientation::NONE;
		break;
	case 0:
		orientation = VoxelOrientation::DEFAULT;
		break;
	case 1:
		orientation = VoxelOrientation::LEFT;
		break;
	case 2:
		orientation = VoxelOrientation::RIGHT;
		break;
	case 3:
		orientation = VoxelOrientation::DOWN;
		break;
	case 4:
		orientation = VoxelOrientation::UP;
		break;
	case 5:
		orientation = VoxelOrientation::BACK;
		break;
	case 6:
		orientation = VoxelOrientation::FRONT;
		break;
	}
}
