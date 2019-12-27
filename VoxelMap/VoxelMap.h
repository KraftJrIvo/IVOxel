#pragma once

#include "VoxelChunk.h"

#include <map>

#include "types.h"
#include "Light.h"

class VoxelMap
{
public:
	VoxelMap();
	VoxelMap(VoxelMapType type);

	int optimize();

	virtual int save() = 0;
	virtual int load() = 0;

protected:
	VoxelMapType _type;

	std::vector<VoxelChunk> _chunks;
	std::map<int, std::map<int, std::map<int, size_t>>> _chunksIds;

	std::vector<Light> _lights;
};