#pragma once

#include <map>

#include "VoxelChunk.h"
#include "Light.h"

class VoxelMap
{
public:
	VoxelMap();
	VoxelMap(const VoxelMapType& type);

	void buildPyramid(uint16_t power = -1);
	void optimizePyramid();

	virtual void save() = 0;
	virtual void load() = 0;

protected:
	VoxelMapType _type;

	std::vector<VoxelChunk> _chunks;
	std::map<int32_t, std::map<int32_t, std::map<int32_t, uint32_t>>> _chunksIds;

	std::vector<std::vector<Light>> _lightsByChunks;
};