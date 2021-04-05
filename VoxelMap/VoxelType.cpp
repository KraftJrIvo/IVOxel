#include "VoxelType.h"

bool VoxelTypeStorer::hasShape(uint32_t id) const
{
	return _shapes.count(id);
}

bool VoxelTypeStorer::hasMaterial(uint32_t id) const 
{
	return _materials.count(id);
}

std::shared_ptr<VoxelShape> VoxelTypeStorer::getShape(uint32_t id) const
{
	if (!_shapes.count(id)) return nullptr;
	return _shapes.at(id);
}

std::shared_ptr<VoxelMaterial> VoxelTypeStorer::getMaterial(uint32_t id) const
{
	if (!_materials.count(id)) return nullptr;
	return _materials.at(id);
}

std::shared_ptr<VoxelShape> VoxelTypeStorer::addShape(uint32_t id, std::shared_ptr<VoxelShape> shp)
{
	_shapes[id] = shp;
	return _shapes[id];
}

std::shared_ptr<VoxelMaterial> VoxelTypeStorer::addMaterial(uint32_t id, std::shared_ptr<VoxelMaterial> mat)
{
	_materials[id] = mat;
	return _materials[id];
}
