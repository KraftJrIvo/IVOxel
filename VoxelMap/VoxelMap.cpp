#include "VoxelMap.h"

#include <algorithm>
#include <iostream>

VoxelMap::VoxelMap(const VoxelMapFormat& format, VoxelChunkGenerator& generator, uint32_t chunkSide, uint32_t loadRadius) :
	_format(format),
	_generator(generator),
	_chunkSide(chunkSide),
	_loadRadius(loadRadius),
	_loadDiameter(loadRadius * 2 + 1),
	_curAbsPos({0,0,0})
{ 
	_chunks.resize(pow(_loadDiameter, 3));
	_loadChunks();
}

VoxelChunk& VoxelMap::getChunk(const std::vector<int32_t>& relPos)
{
	std::vector<int32_t> pos = { relPos[0] + (int32_t)_loadRadius,
								 relPos[1] + (int32_t)_loadRadius,
								 relPos[2] + (int32_t)_loadRadius };
	uint32_t idx = _getIdx(pos);

	if (idx < _chunks.size())
		return *_chunks[idx];

	return _emptyChunk;
}

VoxelMapFormat VoxelMap::getFormat() const
{
	return _format;
}

VoxelTypeStorer& VoxelMap::getVoxelTypeStorer()
{
	return _voxTypeStorer;
}

uint32_t VoxelMap::getLoadRadius()
{
	return _loadRadius;
}

bool VoxelMap::checkLoadNeeded(const std::vector<int32_t>& pos)
{
	if (_curAbsPos[0] != pos[0] || _curAbsPos[1] != pos[1] || _curAbsPos[2] != pos[2])
	{
		std::vector<int32_t> diff = { pos[0] - _curAbsPos[0], pos[1] - _curAbsPos[1], pos[2] - _curAbsPos[2] };

		_curAbsPos = pos;

		auto temp = _chunks;

		std::vector<int32_t> xyz(3);
		for (xyz[0] = 0; xyz[0] < _loadDiameter; xyz[0]++)
			for (xyz[1] = 0; xyz[1] < _loadDiameter; xyz[1]++)
				for (xyz[2] = 0; xyz[2] < _loadDiameter; xyz[2]++)
				{
					if (xyz[0] + diff[0] < _loadDiameter && xyz[0] + diff[0] >= 0 &&
						xyz[1] + diff[1] < _loadDiameter && xyz[1] + diff[1] >= 0 &&
						xyz[2] + diff[2] < _loadDiameter && xyz[2] + diff[2] >= 0)
					{
						auto idx1 = _getIdx(xyz);
						auto idx2 = _getIdx({ xyz[0] + diff[0], xyz[1] + diff[1], xyz[2] + diff[2] });
						_chunks[idx1] = temp[idx2];
					}
					else
					{
						std::vector<int32_t> cAbsPos = _getAbsPos(xyz);
						auto id = _getIdx(xyz);
						_loadChunk(cAbsPos, id);
					}
				}

		return true;
	}
	return false;
}

std::vector<uint8_t> VoxelMap::getChunksDataAt(const std::vector<int32_t>& absPos, uint8_t radius, bool alignToFourBytes)
{
	uint32_t cubeSide = 2 * radius + 1;
	uint32_t nChunks = cubeSide * cubeSide * cubeSide;

	uint32_t nChunkBytes = _format.chunkFormat.getSizeInBytes(alignToFourBytes);
	uint32_t nTotalChunkBytes = nChunks * nChunkBytes;
	std::vector<uint8_t> chunkData(nTotalChunkBytes, 0);
	auto chunkDataPtr = chunkData.data();

	std::vector<uint8_t> voxData;

	for (int32_t x = -radius; x <= radius; ++x)
		for (int32_t y = -radius; y <= radius; ++y)
			for (int32_t z = - radius; z <= radius; ++z)
			{
				auto chunk = getChunk({ x, y, z });
				auto header = _format.chunkFormat.formatChunkHeader(chunk, chunkData.size() + voxData.size(), getChunkParals({ x, y, z }), alignToFourBytes);
				auto voxels = _format.chunkFormat.formatChunk(chunk, alignToFourBytes);

				memcpy(chunkDataPtr, header.data(), nChunkBytes);
				chunkDataPtr += nChunkBytes;

				voxData = utils::joinVectors(voxData, voxels);
			}

	return utils::joinVectors(chunkData, voxData);
}

void VoxelMap::setAbsPos(const std::vector<int32_t>& absPos)
{
	_curAbsPos = absPos;
}

void VoxelMap::_loadChunks()
{
	std::vector<int32_t> xyz(3);
	for (xyz[0] = 0; xyz[0] < _loadDiameter; xyz[0]++)
		for (xyz[1] = 0; xyz[1] < _loadDiameter; xyz[1]++)
			for (xyz[2] = 0; xyz[2] < _loadDiameter; xyz[2]++)
			{
				_loadChunk(_getAbsPos(xyz), _getIdx(xyz));
			}
}

void VoxelMap::_loadChunk(const std::vector<int32_t>& pos, uint32_t id)
{
	if (!_storer.loadChunk(pos, _chunks[id].get()))
		_chunks[id] = _generator.generateChunk(_format, _chunkSide, pos);
}

uint32_t VoxelMap::_getIdx(const std::vector<int32_t>& pos) const
{
	return pos[2] * _loadDiameter * _loadDiameter + pos[1] * _loadDiameter + pos[0];;
}

std::vector<int32_t> VoxelMap::_getAbsPos(const std::vector<int32_t>& pos) const
{
	return { _curAbsPos[0] + pos[0] - (int)_loadRadius, _curAbsPos[1] + pos[1] - (int)_loadRadius, _curAbsPos[2] + pos[2] - (int)_loadRadius };
}

bool VoxelMap::_checkParal(std::vector<int16_t> from, std::vector<int16_t> to)
{
	for (int i = 0; i < 3; ++i)
	{
		auto fromC = std::clamp(from[i], (int16_t)(-((int16_t)_loadRadius)), (int16_t)_loadRadius);
		auto toC = std::clamp(to[i], (int16_t)(-((int16_t)_loadRadius)), (int16_t)_loadRadius);

		if (fromC != from[i] || toC != to[i])
			return false;
	}

	std::vector<int32_t> diff = { to[0] - from[0], to[1] - from[1], to[2] - from[2] };
	
	for (int8_t x = from[0]; true; x += (diff[0] != 0) ? ((diff[0]) / abs(diff[0])) : 0)
	{
		for (int8_t y = from[1]; true; y += (diff[1] != 0) ? ((diff[1]) / abs(diff[1])) : 0)
		{
			for (int8_t z = from[2]; true; z += (diff[2] != 0) ? ((diff[2]) / abs(diff[2])) : 0)
				if (!getChunk({ x,y,z }).isEmpty())
					return false;
				else if (z == to[2] || diff[2] == 0)
					break;
			if (y == to[1] || diff[1] == 0)
				break;
		}
		if (x == to[0] || diff[0] == 0)
			break;
	}
	return true;
}

bool VoxelMap::_checkParalDist(std::vector<int16_t> from, std::vector<int16_t> to, const std::vector<int8_t>& dir)
{
	float dist = 999999999.0f;

	for (int i = 0; i < 3; ++i)
	{
		from[i] = std::clamp(from[i], (int16_t)(-((int16_t)_loadRadius)), (int16_t)_loadRadius);
		to[i] = std::clamp(to[i], (int16_t)(-((int16_t)_loadRadius)), (int16_t)_loadRadius);
	}

	std::vector<int32_t> diff = { to[0] - from[0], to[1] - from[1], to[2] - from[2] };
	if (!diff[0] && !diff[1] && !diff[2]) return 0;

	for (int8_t x = from[0]; abs(x) <= abs(to[0]); x += (diff[0] != 0) ? ((diff[0]) / abs(diff[0])) : 0)
	{
		for (int8_t y = from[1]; abs(y) <= abs(to[1]); y += (diff[1] != 0) ? ((diff[1]) / abs(diff[1])) : 0)
		{
			for (int8_t z = from[2]; abs(z) <= abs(to[2]); z += (diff[2] != 0) ? ((diff[2]) / abs(diff[2])) : 0)
			{
				dist = fmin(getChunk({ x,y,z }).getClosestSidePointDistance(dir), dist);
				if (z == to[2] || diff[1] == 0)
					break;
			}
			if (y == to[1] || diff[1] == 0)
				break;
		}
		if (x == to[0] || diff[1] == 0)
			break;
	}
	return dist;
}

std::vector<uint8_t> VoxelMap::getChunkParals(const std::vector<int32_t>& pos)
{
	std::vector<uint8_t> res;
	uint8_t oct = 0;

	if (!getChunk(pos).isEmpty()) {
		return std::vector<uint8_t>(getSizeInBytes(_format.chunkFormat.parals), 0);
	}

	std::vector<float> dir;

	for (int8_t zOff = -1; zOff <= 1; zOff += 2)
		for (int8_t yOff = -1; yOff <= 1; yOff += 2)
			for (int8_t xOff = -1; xOff <= 1; xOff += 2)
			{
				int16_t x = pos[0], y = pos[1], z = pos[2];
				bool xGo = true, yGo = true, zGo = true, stop = false;
				while (abs(x) <= _loadRadius && !stop)
				{
					while (abs(y) <= _loadRadius && !stop)
					{
						while (abs(z) <= _loadRadius && !stop)
						{
							if (xGo) xGo = _checkParal({ short(x + xOff), (short)pos[1], (short)pos[2] }, { short(x + xOff), y, z });
							if (zGo) zGo = _checkParal({ (short)pos[0], (short)pos[1], short(z + zOff) }, { xGo ? short(x + xOff) : x, y, short(z + zOff) });
							if (yGo) yGo = _checkParal({ (short)pos[0], short(y + yOff), (short)pos[2] }, { xGo ? short(x + xOff) : x, short(y + yOff), zGo ? short(z + zOff) : z });

							if (_format.chunkFormat.parals == ParalsInfoFormat::CUBIC_UINT8 || _format.chunkFormat.parals == ParalsInfoFormat::CUBIC_FLOAT32)
							{
								if (!xGo || !yGo || !zGo)
								{
									stop = true;

									if (_format.chunkFormat.parals == ParalsInfoFormat::CUBIC_FLOAT32)
										utils::appendBytes(res, fabs(x - pos[0]));
									else
										res.push_back(abs(x - pos[0]));
								}
							}
							else if (!xGo && !yGo && !zGo)
							{
								float xDist = (abs(x + xOff) < _loadRadius) ? _checkParalDist({ short(x + xOff), (short)pos[1], (short)pos[2] }, { short(x + xOff), y, z }, { xOff, 0, 0 }) : 0;
								float zDist = (abs(y + yOff) < _loadRadius) ? _checkParalDist({ (short)pos[0], (short)pos[1], short(z + zOff) }, { xGo ? short(x + xOff) : x, y, short(z + zOff) }, { 0, 0, zOff }) : 0;
								float yDist = (abs(z + zOff) < _loadRadius) ? _checkParalDist({ (short)pos[0], short(y + yOff), (short)pos[2] }, { xGo ? short(x + xOff) : x, short(y + yOff), zGo ? short(z + zOff) : z }, { 0, yOff, 0 }) : 0;

								stop = true;
								if (_format.chunkFormat.parals == ParalsInfoFormat::NON_CUBIC_FLOAT32)
								{
									utils::appendBytes(res, abs(x - pos[0]) + xDist);
									utils::appendBytes(res, abs(y - pos[1]) + yDist);
									utils::appendBytes(res, abs(z - pos[2]) + zDist);
									dir.push_back(xOff);
									dir.push_back(yOff);
									dir.push_back(zOff);
								}
								else
								{
									res.push_back(abs(x - pos[0]) + xDist);
									res.push_back(abs(y - pos[1]) + yDist);
									res.push_back(abs(z - pos[2]) + zDist);
								}
							}

							x += xOff * xGo;
							y += yOff * yGo;
							z += zOff * zGo;

							if (!zGo) break;
						}
						if (!yGo) break;
					}
					if (!xGo) break;
				}
				oct++;
			}

	std::vector<float>* ptr = (std::vector<float>*)&res;

	return res;
}

std::vector<uint8_t> VoxelMap::getLightDataAt(const std::vector<int32_t>& absPos, uint8_t radius) const
{
	auto lights = _generator.generateLights(absPos, radius);

	std::vector<uint8_t> vals;
	for (auto& light : lights)
	{
		utils::appendBytes(vals, 1.0f);
		utils::appendBytes(vals, light.position[0]);
		utils::appendBytes(vals, light.position[1]);
		utils::appendBytes(vals, light.position[2]);
		utils::appendBytes(vals, (float)light.rgba[0] / 255.0f);
		utils::appendBytes(vals, (float)light.rgba[1] / 255.0f);
		utils::appendBytes(vals, (float)light.rgba[2] / 255.0f);
		utils::appendBytes(vals, (float)light.rgba[3] / 255.0f);
	}

	return vals;
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

