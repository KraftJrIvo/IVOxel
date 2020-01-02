#pragma once

#include <map>

#include <Eigen/Core>

#include "VoxelChunk.h"
#include "Light.h"

class VoxelMap
{
public:
	VoxelMap();
	VoxelMap(const VoxelMapType& type);

	void buildPyramid();

	virtual void save() = 0;
	virtual void load() = 0;

	bool checkIfChunkIsPossible(const std::vector<float>& pos, const Eigen::Vector3f& dir) const;

protected:
	VoxelMapType _type;

	std::vector<VoxelChunk> _chunks;
	std::map<int32_t, std::map<int32_t, std::map<int32_t, uint32_t>>> _chunksIds;
	std::vector<std::pair<int32_t, int32_t>> _maxMinChunk;

	std::vector<std::vector<Light>> _lightsByChunks;

	VoxelChunk& _addChunk(const std::vector<int32_t>& pos, const std::vector<uint32_t>& sz);
};