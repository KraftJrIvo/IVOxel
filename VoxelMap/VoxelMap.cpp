#include "VoxelMap.h"

#include <algorithm>

VoxelMap::VoxelMap(const VoxelMapFormat& format, const VoxelChunkGenerator& generator, uint32_t loadRadius) :
	_format(format),
	_generator(generator),
	_loadRadius(loadRadius),
	_loadDiameter(loadRadius * 2 + 1),
	_curAbsPos({0,0,0})
{ }

VoxelChunk& VoxelMap::getChunk(const std::vector<int32_t>& pos)
{
	std::vector<int32_t> absPos = { _curAbsPos[X] + pos[X], _curAbsPos[Y] + pos[Y], _curAbsPos[Z] + pos[Z] };
	
	uint32_t idx = _getIdx(absPos);

	if (idx < _chunks.size())
		return _chunks[idx];

	return _emptyChunk;
}

VoxelMapFormat VoxelMap::getFormat() const
{
	return _format;
}

std::vector<Light> VoxelMap::getLightsByChunk(const std::vector<int32_t>& pos, uint32_t radius) const
{
	std::vector<Light> lights;

	for (auto l : _lights)
	{
		auto& lpos = l.second.position;
		if (lpos[X] <= pos[X] + radius && lpos[X] >= pos[X] - radius &&
			lpos[Y] <= pos[Y] + radius && lpos[Y] >= pos[Y] - radius &&
			lpos[Z] <= pos[Z] + radius && lpos[Z] >= pos[Z] - radius)
			lights.push_back(l.second);
	}

	return lights;
}

bool VoxelMap::checkLoadNeeded(const std::vector<int32_t>& pos)
{
	if (_curAbsPos[X] != pos[X] || _curAbsPos[Y] != pos[Y] || _curAbsPos[Z] != pos[Z])
	{
		auto diff = { pos[X] - _curAbsPos[X], pos[Y] - _curAbsPos[Y], pos[Z] - _curAbsPos[Z] };



		_curAbsPos = pos;

		return true;
	}
	return false;
}

uint32_t VoxelMap::_getIdx(const std::vector<int32_t>& pos) const
{
	return pos[Z] * _loadDiameter * _loadDiameter + pos[Y] * _loadDiameter + pos[X];;
}

std::vector<uint8_t> VoxelMap::getChunksDataAt(const std::vector<int32_t>& absPos, uint8_t radius)
{
	uint32_t cubeSide = radius + 1;
	uint32_t nChunks = cubeSide * cubeSide * cubeSide;

	uint32_t currVoxPyrOffset = 0;
	uint32_t CHUNK_SZ = 40;
	uint32_t VOX_SZ = 24;

	std::vector<uint8_t> chunkData(nChunks * CHUNK_SZ, 0);
	auto chunkDataPtr = chunkData.data();

	std::vector<uint8_t> voxData;
	auto voxDataPtr = voxData.data();

	std::vector<int32_t> pos = { absPos[X] - _curAbsPos[X], absPos[Y] - _curAbsPos[Y], absPos[Z] - _curAbsPos[Z] };

	for (int32_t x = pos[0] - radius; ++x; x <= pos[0] + radius)
		for (int32_t y = pos[0] - radius; ++y; y <= pos[0] + radius)
			for (int32_t z = pos[0] - radius; ++z; z <= pos[0] + radius)
			{
				// empty? 1 byte
				uint8_t empty = getChunk(pos).isEmpty();
				std::memcpy(chunkDataPtr, &empty, sizeof(uint8_t));
				chunkDataPtr += sizeof(uint8_t);

				// base 1 byte
				uint8_t base = getChunk(pos).pyramid.base;
				std::memcpy(chunkDataPtr, &base, sizeof(uint8_t));
				chunkDataPtr += sizeof(uint8_t);

				// power 1 byte
				uint8_t power = getChunk(pos).pyramid.power;
				std::memcpy(chunkDataPtr, &power, sizeof(uint8_t));
				chunkDataPtr += sizeof(uint8_t);

				// extra 1 byte
				uint8_t extra = 0;
				std::memcpy(chunkDataPtr, &extra, sizeof(uint8_t));
				chunkDataPtr += sizeof(uint8_t);

				// voxel pyramid offset 4 bytes
				std::memcpy(chunkDataPtr, &currVoxPyrOffset, sizeof(uint32_t));
				uint32_t curVoxPyrSize = pow(base, power) * VOX_SZ;
				currVoxPyrOffset += curVoxPyrSize;
				chunkDataPtr += sizeof(uint32_t);

				// chunk parals 32 bytes
				std::vector<float> parals = getChunkParals(pos);
				std::memcpy(chunkDataPtr, parals.data(), 4 * sizeof(float));
				chunkDataPtr += 4 * sizeof(float);

				voxData.resize(voxData.size() + curVoxPyrSize, 0);

			}



	return utils::joinVectors(chunkData, voxData);
}

std::vector<float> VoxelMap::getChunkParals(const std::vector<int32_t>& pos)
{
	std::vector<float> res;

	return res;
}

uint32_t VoxelMap::addLight(const Light& l)
{
	_lights[++_lastLightId] = l;
	return _lastLightId;
}

void VoxelMap::moveLight(uint32_t lightID, const std::vector<float>& absPos)
{
	_lights[lightID].position = absPos;
}

void VoxelMap::removeLight(uint32_t lightID)
{
	auto l = _lights.find(lightID);
	if (l != _lights.end())
		_lights.erase(lightID);
}

