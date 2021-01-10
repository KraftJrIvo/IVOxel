#pragma once

#include <vector>

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
	Voxel(uint8_t power = 0, VoxelType type = VoxelType::AIR, VoxelOrientation orientation = VoxelOrientation::DEFAULT,
		const std::vector<uint8_t>& rgba = {0,0,0,0});

	std::vector<uint32_t> getData();
	bool isEmpty() const;
	uint8_t getOrientation() const;
	void setOrientation(uint8_t orientation);

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

struct VoxelFormat
{
	VoxelFullnessFormat      fullness;
	VoxelTypeFormat          type;
	VoxelOrientationFormat   orientation;
	VoxelColorFormat         color;
	VoxelNeighbourInfoFormat neighbour;
	ParalsInfoFormat         parals;

	uint32_t getSizeInBytes(bool alignToFourBytes = true) const;
	std::vector<uint8_t> formatVoxel(const Voxel& voxel, const std::vector<uint8_t>& neighs, const std::vector<uint8_t>& parals, bool alignToFourBytes = true) const;
	Voxel unformatVoxel(const uint8_t* data, uint8_t power) const;
};