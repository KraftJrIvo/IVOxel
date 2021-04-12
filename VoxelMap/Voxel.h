#pragma once

#include <vector>

#include "VoxelType.h"

class Voxel
{
public:
	Voxel(uint8_t size = 0, std::shared_ptr<VoxelShape> shape = nullptr, std::shared_ptr<VoxelMaterial> material = nullptr, VoxelOrientation orientation = { {0,0,0}, false }, const glm::vec4& rgba = { 0,0,0,0 });
	bool isEmpty() const;
	bool isTransparent() const;

	std::shared_ptr<VoxelShape> shape;
	std::shared_ptr<VoxelMaterial> material;

	VoxelOrientation orientation;
	glm::vec4 color;

	uint8_t size;
};

struct VoxelModifyData
{
	Voxel voxel;
	uint32_t x, y, z;
};

struct VoxelState
{
	bool full;
	uint32_t size;
	VoxelNeighbours neighs;
	std::vector<glm::uvec3> parals;
};

struct VoxelFormat
{
	VoxelFullnessFormat      fullness;
	VoxelSizeFormat	         size;
	VoxelShapeFormat         shape;
	VoxelMaterialFormat      material;
	VoxelOrientationFormat   orientation;
	VoxelColorFormat         color;
	VoxelNeighbourInfoFormat neighbour;
	ParalsInfoFormat         parals;

	VoxelTypeCoder* coder;
	VoxelTypeStorer* storer;

	uint32_t getSizeInBytes(bool alignToFourBytes = true) const;
	std::vector<uint8_t> formatVoxel(const Voxel& voxel, uint32_t size, const std::vector<uint8_t>& neighs, const std::vector<uint8_t>& parals, bool alignToFourBytes = true) const;
	Voxel unformatVoxel(const uint8_t* data) const;
	VoxelNeighbours unformatNeighs(const uint8_t* data) const;
};