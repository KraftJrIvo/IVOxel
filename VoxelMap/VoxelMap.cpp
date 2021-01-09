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

		// TODO

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
				auto chunk = getChunk({x, y, z});
				auto parals = getChunkParals({ x, y, z });
				_format.formatChunkHeader(chunk, 123, parals, )

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

std::vector<uint8_t> VoxelMap::getChunkParals(const std::vector<int32_t>& pos)
{
	std::vector<uint8_t> res;
	uint8_t oct = 0;

	auto checkParal = [&](const std::vector<int16_t>& from, const std::vector<int16_t>& to)
	{
		for (int8_t x = from[0]; x <= to[0]; x++)
			for (int8_t y = from[1]; y <= to[1]; y++)
				for (int8_t z = from[2]; z <= to[2]; z++)
					if (!getChunk({x,y,z}).isEmpty())
						return false;
		return true;
	};

	for (int16_t xOff = -1; xOff <= 1; xOff += 2)
		for (int16_t yOff = -1; yOff <= 1; yOff += 2)
			for (int16_t zOff = -1; zOff <= 1; zOff += 2)
			{
				int16_t x, y, z;
				bool xGo = true, yGo = true, zGo = true, stop = false;
				for (x = 0; abs(x) < 256 && !stop; x += xOff * xGo)
					for (y = 0; abs(y) < 256 && !stop; y += yOff * yGo)
						for (z = 0; abs(z) < 256 && !stop; z += zOff * zGo)
						{
							xGo = checkParal({ short(x + xOff), 0, 0 }, { short(x + xOff), y, z });
							zGo = checkParal({ 0, 0, short(z + zOff) }, { xGo ? short(x + xOff) : x, y, short(z + zOff) });
							yGo = checkParal({ 0, short(y + yOff), 0 }, { xGo ? short(x + xOff) : x, short(y + yOff), zGo ? short(z + zOff) : z });

							if (format == ParalsInfoFormat::CUBIC_UINT8 || format == ParalsInfoFormat::CUBIC_FLOAT32)
								if (!xGo || !yGo || !zGo)
								{
									stop = true;

									if (format == ParalsInfoFormat::CUBIC_FLOAT32)
										res.push_back(x);
									else
										utils::appendBytes(res, x * offset);
								}
								else
									if (!xGo && !yGo && !zGo)
									{
										stop = true;
										if (format == ParalsInfoFormat::NON_CUBIC_FLOAT32)
										{
											utils::appendBytes(res, x * offset);
											utils::appendBytes(res, y * offset);
											utils::appendBytes(res, z * offset);
										}
										else
										{
											res.push_back(x);
											res.push_back(y);
											res.push_back(z);
										}
									}
						}
				oct++;
			}

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

