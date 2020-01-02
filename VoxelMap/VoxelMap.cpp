#include "VoxelMap.h"

#include <algorithm>

VoxelMap::VoxelMap()
{
}

VoxelMap::VoxelMap(const VoxelMapType& type) :
	_type(type)
{
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
		notFail &= (dir[i] > 0 || pos[i] > _maxMinChunk[i].first) && (dir[i] < 0 || pos[i] < _maxMinChunk[X].second);

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
