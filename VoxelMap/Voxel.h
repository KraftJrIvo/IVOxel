#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "types.h"
#include "VoxelType.h"

class Voxel
{
public:
	Voxel(std::shared_ptr<VoxelShape> shape = nullptr, std::shared_ptr<VoxelMaterial> material = nullptr, VoxelOrientation orientation = VoxelOrientation::DEFAULT, const std::vector<uint8_t>& rgba = {0,0,0,0});
	bool isEmpty() const;
	bool isTransparent() const;

	std::shared_ptr<VoxelShape> shape;
	std::shared_ptr<VoxelMaterial> material;

	VoxelOrientation orientation;
	std::vector<uint8_t> color;
};

struct VoxelModifyData
{
	Voxel voxel;
	uint32_t x, y, z;
};

struct VoxelNeighbours
{
	bool l, r, d, u, b, f;
	bool ld, lu, lb, lf, rd, ru, rb, rf, db, df, ub, uf;
	bool ldb, ldf, lub, luf, rdb, rdf, rub, ruf;
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

	uint32_t getSizeInBytes(bool alignToFourBytes = true) const;
	std::vector<uint8_t> formatVoxel(const Voxel& voxel, uint32_t size, const std::vector<uint8_t>& neighs, const std::vector<uint8_t>& parals, bool alignToFourBytes = true) const;
	VoxelState getVoxelState(const uint8_t* data) const;
	Voxel unformatVoxel(VoxelTypeStorer& vts, const uint8_t* data) const;
	std::vector<glm::uvec3> unformatParals(const uint8_t* data) const;
	VoxelNeighbours unformatNeighs(const uint8_t* data) const;
};