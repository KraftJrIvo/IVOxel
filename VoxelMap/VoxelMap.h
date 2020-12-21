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

	virtual void save() {}
	virtual void load() {}

	bool checkIfChunkIsPossible(const std::vector<float>& pos, const Eigen::Vector3f& dir) const;
	const std::vector<std::pair<int32_t, int32_t>>& getMinMaxChunks() const;
	VoxelChunk* getChunk(const std::vector<int32_t>& pos) const;
	VoxelMapType getType() const;
	void moveLight(uint32_t chunkID, uint32_t lightID, const std::vector<float>& pos);

	const std::vector<std::vector<Light>>& getLightsByChunks() const;

protected:
	VoxelMapType _type;

	std::vector<VoxelChunk> _chunks;
	std::map<int32_t, std::map<int32_t, std::map<int32_t, uint32_t>>> _chunksIds;
	std::vector<std::pair<int32_t, int32_t>> _maxMinChunk;

	std::vector<std::vector<Light>> _lightsByChunks;

	VoxelChunk& _addChunk(const std::vector<int32_t>& pos, const std::vector<uint32_t>& sz);
};