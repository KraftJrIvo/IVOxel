#include "VoxelMap.h"

#include <algorithm>

VoxelMap::VoxelMap(const VoxelMapFormat& format, const VoxelChunkGenerator& generator, uint32_t loadRadius) :
	_format(format),
	_generator(generator),
	_loadRadius(loadRadius),
	_loadDiameter(loadRadius * 2 + 1),
	_curAbsPos({0,0,0})
{ 
	_chunks.resize(pow(_loadDiameter, 3));
}

VoxelChunk& VoxelMap::getChunk(const std::vector<int32_t>& pos)
{
	std::vector<int32_t> absPos = { _curAbsPos[0] + pos[0], _curAbsPos[1] + pos[1], _curAbsPos[2] + pos[2] };
	
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
		if (lpos[0] <= pos[0] + radius && lpos[0] >= pos[0] - radius &&
			lpos[1] <= pos[1] + radius && lpos[1] >= pos[1] - radius &&
			lpos[2] <= pos[2] + radius && lpos[2] >= pos[2] - radius)
			lights.push_back(l.second);
	}

	return lights;
}

bool VoxelMap::checkLoadNeeded(const std::vector<int32_t>& pos)
{
	if (_curAbsPos[0] != pos[0] || _curAbsPos[1] != pos[1] || _curAbsPos[2] != pos[2])
	{
		std::vector<int32_t> diff = { pos[0] - _curAbsPos[0], pos[1] - _curAbsPos[1], pos[2] - _curAbsPos[2] };

		for (int16_t x = 0; x < _loadDiameter; x++)
			for (int16_t y = 0; y < _loadDiameter; y++)
				for (int16_t z = 0; z < _loadDiameter; z++)
				{
					if (x + diff[0] < _loadDiameter && x + diff[0] > 0 &&
						y + diff[1] < _loadDiameter && y + diff[1] > 0 &&
						z + diff[2] < _loadDiameter && z + diff[2] > 0)
					{
						std::vector<int32_t> cpos = {x, y, z};
						auto idx1 = _getIdx(cpos);
						auto idx2 = _getIdx({ x + diff[0], y + diff[1], z + diff[2] });
						_chunks[idx2] = _chunks[idx1];
						if (!_storer.loadChunk(cpos, &_chunks[idx1]))
							_chunks[idx1] = _generator.generate(_format, _loadDiameter, cpos);
					}
				}

		_curAbsPos = pos;

		return true;
	}
	return false;
}

uint32_t VoxelMap::_getIdx(const std::vector<int32_t>& pos) const
{
	return pos[2] * _loadDiameter * _loadDiameter + pos[1] * _loadDiameter + pos[0];;
}

bool VoxelMap::_checkParal(const std::vector<int16_t>& from, const std::vector<int16_t>& to)
{
	for (int8_t x = from[0]; x <= to[0]; x++)
		for (int8_t y = from[1]; y <= to[1]; y++)
			for (int8_t z = from[2]; z <= to[2]; z++)
				if (!getChunk({ x,y,z }).isEmpty())
					return false;
	return true;
}

bool VoxelMap::_checkParalDist(const std::vector<int16_t>& from, const std::vector<int16_t>& to, const std::vector<int8_t>& dir)
{
	float dist = 999999999.0f;

	for (int8_t x = from[0]; x <= to[0]; x++)
		for (int8_t y = from[1]; y <= to[1]; y++)
			for (int8_t z = from[2]; z <= to[2]; z++)
				dist = fmin(getChunk({ x,y,z }).getClosestSidePointDistance(dir), dist);
	return dist;
}

std::vector<uint8_t> VoxelMap::getChunksDataAt(const std::vector<int32_t>& absPos, uint8_t radius, bool alignToFourBytes)
{
	uint32_t cubeSide = radius + 1;
	uint32_t nChunks = cubeSide * cubeSide * cubeSide;

	uint32_t nChunkBytes = _format.chunkFormat.getSizeInBytes(alignToFourBytes);
	uint32_t nTotalChunkBytes = nChunks * nChunkBytes;
	std::vector<uint8_t> chunkData(nTotalChunkBytes, 0);
	auto chunkDataPtr = chunkData.data();

	std::vector<uint8_t> voxData;

	std::vector<int32_t> pos = { absPos[0] - _curAbsPos[0], absPos[1] - _curAbsPos[1], absPos[2] - _curAbsPos[2] };

	for (int32_t x = pos[0] - radius; ++x; x <= pos[0] + radius)
		for (int32_t y = pos[0] - radius; ++y; y <= pos[0] + radius)
			for (int32_t z = pos[0] - radius; ++z; z <= pos[0] + radius)
			{
				auto chunk = getChunk({ x, y, z });
				auto header = _format.chunkFormat.formatChunkHeader(chunk, chunkData.size() + voxData.size(), getChunkParals({ x, y, z }), alignToFourBytes);
				auto voxels = _format.chunkFormat.formatChunk(chunk, alignToFourBytes);

				memcpy(chunkDataPtr, header.data(), nChunkBytes);
				chunkDataPtr += nChunkBytes;

				utils::joinVectors(voxData, voxels);
			}

	return utils::joinVectors(chunkData, voxData);
}

std::vector<uint8_t> VoxelMap::getChunkParals(const std::vector<int32_t>& pos)
{
	std::vector<uint8_t> res;
	uint8_t oct = 0;

	for (int8_t xOff = -1; xOff <= 1; xOff += 2)
		for (int8_t yOff = -1; yOff <= 1; yOff += 2)
			for (int8_t zOff = -1; zOff <= 1; zOff += 2)
			{
				int16_t x, y, z;
				float xf, yf, zf;
				bool xGo = true, yGo = true, zGo = true, stop = false;
				for (x = 0; abs(x) < 256 && !stop; x += xOff * xGo)
					for (y = 0; abs(y) < 256 && !stop; y += yOff * yGo)
						for (z = 0; abs(z) < 256 && !stop; z += zOff * zGo)
						{
							xGo = _checkParal({ short(x + xOff), 0, 0 }, { short(x + xOff), y, z });
							zGo = _checkParal({ 0, 0, short(z + zOff) }, { xGo ? short(x + xOff) : x, y, short(z + zOff) });
							yGo = _checkParal({ 0, short(y + yOff), 0 }, { xGo ? short(x + xOff) : x, short(y + yOff), zGo ? short(z + zOff) : z });

							if (_format.chunkFormat.parals == ParalsInfoFormat::CUBIC_UINT8 || _format.chunkFormat.parals == ParalsInfoFormat::CUBIC_FLOAT32)
								if (!xGo || !yGo || !zGo)
								{
									stop = true;

									if (_format.chunkFormat.parals == ParalsInfoFormat::CUBIC_FLOAT32)
										utils::appendBytes(res, x);
									else
										res.push_back(x);
								}
								else
									if (!xGo && !yGo && !zGo)
									{
										float xDist = _checkParalDist({ short(x + xOff), 0, 0 }, { short(x + xOff), y, z }, { xOff, 0, 0 });
										float zDist = _checkParalDist({ 0, 0, short(z + zOff) }, { xGo ? short(x + xOff) : x, y, short(z + zOff) }, { 0, 0, zOff });
										float yDist = _checkParalDist({ 0, short(y + yOff), 0 }, { xGo ? short(x + xOff) : x, short(y + yOff), zGo ? short(z + zOff) : z }, { 0, yOff, 0 });

										stop = true;
										if (_format.chunkFormat.parals == ParalsInfoFormat::NON_CUBIC_FLOAT32)
										{
											utils::appendBytes(res, x + xDist);
											utils::appendBytes(res, y + yDist);
											utils::appendBytes(res, z + zDist);
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

