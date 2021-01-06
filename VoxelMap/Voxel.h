#pragma once

#include "types.h"

enum class VoxelType
{
	AIR,
	CUBE,
	SPHERE
};

enum class VoxelOrientation
{
	NONE,
	DEFAULT,
	LEFT,
	RIGHT,
	DOWN,
	UP,
	BACK,
	FRONT
};

class Voxel
{
public:
	Voxel(const VoxelFormat& format, uint8_t power = 0, VoxelType type = VoxelType::AIR, VoxelOrientation orientation = VoxelOrientation::DEFAULT,
		const std::vector<uint8_t>& rgba = {0,0,0,0});

	std::vector<uint32_t> getData();
	bool isEmpty() const;
	uint8_t getOrientation() const;
	void setOrientation(uint8_t orientation);

	const VoxelFormat& format;
	uint8_t power;
	VoxelType type;
	VoxelOrientation orientation;
	std::vector<uint8_t> color;
};

struct VoxelModifyData
{
	Voxel voxel;
	uint32_t x, y, z;
};