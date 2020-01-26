#include "VoxelMap.h"

#include <algorithm>

VoxelMap::VoxelMap()
{
	_maxMinChunk.resize(DIMENSIONS, { -1,1 });
}

VoxelMap::VoxelMap(const VoxelMapType& type) :
	_type(type)
{
	_maxMinChunk.resize(DIMENSIONS, { -1,1 });
}

void VoxelMap::buildPyramid()
{
	for (auto& chunk : _chunks)
		chunk.buildPyramid();
}

bool VoxelMap::checkIfChunkIsPossible(const std::vector<float>& pos, const Eigen::Vector3f& dir) const
{
	bool notFail = true;

	for (uint8_t i = 0; i < DIMENSIONS && notFail; ++i)
		notFail &= (dir[i] > 0 && pos[i] < _maxMinChunk[i].second) || (pos[i] > _maxMinChunk[X].first);

	return notFail;
}

VoxelChunk& VoxelMap::_addChunk(const std::vector<int32_t>& pos, const std::vector<uint32_t>& sz)
{
	for (uint8_t j = 0; j < DIMENSIONS; ++j)
	{
		_maxMinChunk[j].first = std::min(_maxMinChunk[j].first, pos[j]);
		_maxMinChunk[j].second = std::max(_maxMinChunk[j].second, pos[j]);
	}

	_chunksIds[pos[X]][pos[Y]][pos[Z]] = _chunks.size();
	_chunks.push_back(VoxelChunk(sz, _type));
	return _chunks[_chunks.size() - 1];
}

const std::vector<std::pair<int32_t, int32_t>>& VoxelMap::getMinMaxChunks() const
{
	return _maxMinChunk;
}

VoxelChunk* VoxelMap::getChunk(const std::vector<int32_t>& pos) const
{
	if (_chunksIds.count(pos[X]))
		if (_chunksIds.at(pos[X]).count(pos[Y]))
			if (_chunksIds.at(pos[X]).at(pos[Y]).count(pos[Z]))
				return (VoxelChunk*)&_chunks[_chunksIds.at(pos[X]).at(pos[Y]).at(pos[Z])];
	return nullptr;
}

VoxelMapType VoxelMap::getType() const
{
	return _type;
}

const std::vector<std::vector<Light>>& VoxelMap::getLightsByChunks() const
{
	return _lightsByChunks;
}

