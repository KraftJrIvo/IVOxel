#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "types.h"
#include "VoxelType.h"

class Voxel
{
public:
	Voxel(std::shared_ptr<VoxelShape> shape = nullptr, std::shared_ptr<VoxelMaterial> material = nullptr, uint8_t power = 0, VoxelOrientation orientation = VoxelOrientation::DEFAULT, const std::vector<uint8_t>& rgba = {0,0,0,0});
	bool isEmpty() const;
	bool isTransparent() const;

	std::shared_ptr<VoxelShape> shape;
	std::shared_ptr<VoxelMaterial> material;

	uint8_t power;
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
	VoxelPowerFormat	     power;
	VoxelShapeFormat         shape;
	VoxelMaterialFormat      material;
	VoxelOrientationFormat   orientation;
	VoxelColorFormat         color;
	VoxelNeighbourInfoFormat neighbour;
	ParalsInfoFormat         parals;

	VoxelTypeCoder* coder;

	uint32_t getSizeInBytes(bool alignToFourBytes = true) const;
	std::vector<uint8_t> formatVoxel(const Voxel& voxel, const std::vector<uint8_t>& neighs, const std::vector<uint8_t>& parals, bool alignToFourBytes = true) const;
	Voxel unformatVoxel(VoxelTypeStorer& vts, const uint8_t* data) const;
};