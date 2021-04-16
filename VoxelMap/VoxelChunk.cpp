#include "VoxelChunk.h"
#include <iostream>
#include <algorithm>

VoxelChunk::VoxelChunk(const std::vector<Voxel>& voxels, const VoxelChunkFormat& format_, const VoxelFormat& voxFormat_, bool alignToFourBytes) :
	format(format_),
	voxFormat(voxFormat_),
	voxFormatForPyr(voxFormat_),
	empty(true)
{ 
	voxFormatForPyr.parals = ParalsInfoFormat::NO_PARALS;
	voxFormatForPyr.neighbour = VoxelNeighbourInfoFormat::NO_NEIGHBOUR_INFO;

	for (auto& v : voxels)
	{
		if (v.shape)
		{
			empty = false;
			break;
		}
	}

	_buildPyramid(voxels, alignToFourBytes);
}

void VoxelChunk::_buildPyramid(const std::vector<Voxel>& voxels, bool alignToFourBytes)
{
	side = pow(voxels.size(), 1.0f / 3.0f);
	minOffset = 1.0f / side;

	pyramid = VoxelChunkPyramid(voxFormat, side, alignToFourBytes);
	pyramid.build(voxels);
}

bool VoxelChunk::_checkParal(const std::vector<float>& from, const std::vector<float>& to, float offset) const
{
	for (int i = 0; i < 3; ++i)
	{
		auto fromC = std::clamp(from[i], 0.0f, 1.0f);
		auto toC = std::clamp(to[i], 0.0f, 1.0f);

		if (fromC != from[i] || toC != to[i])
			return false;
	}

	std::vector<float> diff = { to[0] - from[0], to[1] - from[1], to[2] - from[2] };

	for (float x = from[0]; true; x += ((diff[0] != 0) ? ((diff[0]) / abs(diff[0])) : 0) * offset)
	{
		for (float y = from[1]; true; y += ((diff[1] != 0) ? ((diff[1]) / abs(diff[1])) : 0) * offset)
		{
			for (float z = from[2]; true; z += ((diff[2] != 0) ? ((diff[2]) / abs(diff[2])) : 0) * offset)
			{
				std::vector<float> pos = { x, y, z };
				if (getVoxel(pos).shape)
					return false;
				if (z == to[2] || diff[2] == 0)
					break;
			}
			if (y == to[1] || diff[1] == 0)
				break;
		}
		if (x == to[0] || diff[0] == 0)
			break;
	}
	return true;
}

void VoxelChunk::changeVoxels(const std::vector<VoxelModifyData>& voxelsMod)
{
	modified = true;

	std::vector<Voxel> voxels(pow(side, 3));
	for (uint8_t x = 0; x < side; ++x)
		for (uint8_t y = 0; y < side; ++y)
			for (uint8_t z = 0; z < side; ++z)
			{
				auto idx = z * side * side + y * side + x;
				voxels[idx] = getVoxel({ x * minOffset, y * minOffset, z * minOffset });
			}

	for (auto& mod : voxelsMod)
	{
		auto idx = mod.z * side * side + mod.y * side + mod.x;
		voxels[idx] = mod.voxel;
	}

	_buildPyramid(voxels);
}

bool VoxelChunk::isEmpty() const
{
	return empty;//getVoxel({0.5, 0.5, 0.5}).isEmpty();
}

Voxel VoxelChunk::getVoxel(const std::vector<float>& chunkPos) const
{
	std::vector<uint32_t> pos = { uint32_t(floor(side * chunkPos[0])), uint32_t(floor(side * chunkPos[1])), uint32_t(floor(side * chunkPos[2])) };

	uint32_t nLeavesBeforeCurrent = 0;
	uint32_t curPwr = 0;
	uint32_t curLayerLen = 1;
	std::vector<uint32_t> leavesOnLayers;
	uint32_t nOffsetBytes = *((uint32_t*)pyramid.data.data());
	uint8_t base = pyramid.data[4];
	uint8_t power = pyramid.data[5];
	uint8_t voxSizeInBytes = pyramid.data[6];
	uint8_t* ptr = (uint8_t*)pyramid.data.data() + sizeof(uint32_t) + 3 * sizeof(uint8_t);

	for (uint8_t i = 0; i < power + 1; ++i)
	{
		leavesOnLayers.push_back(*((uint32_t*)ptr));
		ptr += sizeof(uint32_t);
	}

	uint32_t curPwrLayerPos = 0;

	uint32_t totalWidth = std::pow(base, power);
	uint32_t zLayerLen = std::pow(base, 3 - 1);
	uint32_t yRowLen = std::pow(base, 3 - 2);

	uint8_t bytesForThisLayer = VoxelChunkPyramid::getPyramLayerBytesCount(base, curPwr);

	bool offsetIsFinal = false;
	while (!offsetIsFinal)
	{
		int8_t* offset = (int8_t*)ptr;
		int32_t val;

		if (bytesForThisLayer == 1)
			val = *(offset);
		else if (bytesForThisLayer == 2)
			val = *((int16_t*)offset);
		else
			val = *((int32_t*)offset);

		bool offsetIsFinal = val < 0;

		val = offsetIsFinal ? (-val - 1) : val;
		nLeavesBeforeCurrent += (offsetIsFinal ? val : leavesOnLayers[curPwr]);

		if (offsetIsFinal)
			break;

		ptr += bytesForThisLayer * uint32_t(curLayerLen - curPwrLayerPos); // skipping to the end of current layer

		uint32_t curSide = totalWidth / std::pow(base, curPwr);
		uint32_t sidePart = curSide / base;
		curPwrLayerPos = ((pos[2] % curSide) / sidePart) * zLayerLen + ((pos[1] % curSide) / sidePart) * yRowLen + ((pos[0] % curSide) / sidePart);

		curLayerLen -= leavesOnLayers[curPwr];
		curLayerLen *= std::pow(base, 3);

		curPwr++;
		bytesForThisLayer = VoxelChunkPyramid::getPyramLayerBytesCount(base, curPwr);
		ptr += bytesForThisLayer * uint32_t(val * std::pow(base, 3) + curPwrLayerPos); // skipping to the value of interest
	}

	ptr = (uint8_t*)pyramid.data.data() + sizeof(uint32_t) + 3 * sizeof(uint8_t) +
		leavesOnLayers.size() * sizeof(uint32_t) + nOffsetBytes;

	ptr += voxSizeInBytes * nLeavesBeforeCurrent;

	return voxFormatForPyr.unformatVoxel(ptr);
}

std::vector<uint8_t> VoxelChunk::getNeighbours(const Voxel& vox, const std::vector<float>& chunkPos, const VoxelNeighbourInfoFormat& format) const
{
	uint32_t voxSide = pow(pyramid.base, vox.size);
	float offset = 1.0f / float(voxSide);
	std::vector<float> pos = { floor(chunkPos[0] / offset) + offset / 2.0f, floor(chunkPos[1] / offset) + offset / 2.0f, floor(chunkPos[2] / offset) + offset / 2.0f };

	bool l = getVoxel({ pos[0] - offset, pos[1], pos[2] }).shape == vox.shape;
	bool r = getVoxel({ pos[0] + offset, pos[1], pos[2] }).shape == vox.shape;
	bool d = getVoxel({ pos[0],          pos[1] - offset, pos[2] }).shape == vox.shape;
	bool u = getVoxel({ pos[0],          pos[1] + offset, pos[2] }).shape == vox.shape;
	bool b = getVoxel({ pos[0],          pos[1],          pos[2] - offset }).shape == vox.shape;
	bool f = getVoxel({ pos[0],          pos[1],          pos[2] + offset }).shape == vox.shape;

	if (format == VoxelNeighbourInfoFormat::SIX_DIRS_ONE_BYTE)
		return { utils::packByte(0, 0, l, r, d, u, b, f) };
	
	bool ld  = getVoxel({ pos[0] - offset, pos[1] - offset, pos[2]          }).shape == vox.shape;
	bool lu  = getVoxel({ pos[0] - offset, pos[1] + offset, pos[2]          }).shape == vox.shape;
	bool lb  = getVoxel({ pos[0] - offset, pos[1],          pos[2] - offset }).shape == vox.shape;
	bool lf  = getVoxel({ pos[0] - offset, pos[1],          pos[2] + offset }).shape == vox.shape;
	bool ldb = getVoxel({ pos[0] - offset, pos[1] - offset, pos[2] - offset }).shape == vox.shape;
	bool lub = getVoxel({ pos[0] - offset, pos[1] + offset, pos[2] - offset }).shape == vox.shape;
	bool ldf = getVoxel({ pos[0] - offset, pos[1] - offset, pos[2] + offset }).shape == vox.shape;
	bool luf = getVoxel({ pos[0] - offset, pos[1] + offset, pos[2] + offset }).shape == vox.shape;
	bool rd  = getVoxel({ pos[0] + offset, pos[1] - offset, pos[2]          }).shape == vox.shape;
	bool ru  = getVoxel({ pos[0] + offset, pos[1] + offset, pos[2]          }).shape == vox.shape;
	bool rb  = getVoxel({ pos[0] + offset, pos[1],          pos[2] - offset }).shape == vox.shape;
	bool rf  = getVoxel({ pos[0] + offset, pos[1],          pos[2] + offset }).shape == vox.shape;
	bool rdb = getVoxel({ pos[0] + offset, pos[1] - offset, pos[2] - offset }).shape == vox.shape;
	bool rub = getVoxel({ pos[0] + offset, pos[1] + offset, pos[2] - offset }).shape == vox.shape;
	bool rdf = getVoxel({ pos[0] + offset, pos[1] - offset, pos[2] + offset }).shape == vox.shape;
	bool ruf = getVoxel({ pos[0] + offset, pos[1] + offset, pos[2] + offset }).shape == vox.shape;
	
	bool db = getVoxel({ pos[0], pos[1] - offset, pos[2] - offset }).shape == vox.shape;
	bool df = getVoxel({ pos[0], pos[1] - offset, pos[2] + offset }).shape == vox.shape;
	bool ub = getVoxel({ pos[0], pos[1] + offset, pos[2] - offset }).shape == vox.shape;
	bool uf = getVoxel({ pos[0], pos[1] + offset, pos[2] + offset }).shape == vox.shape;

	//0 0 0 0 0 0 l ld lu lb lf ldb ldf lub luf r rd ru rb rf rdb rdf rub ruf d db df u ub uf b f
	return { utils::packByte(0, 0, 0, 0, 0, 0, l, ld), utils::packByte(lu, lb, lf, ldb, ldf, lub, luf, r), utils::packByte(r, rd, ru, rb, rf, rdb, rdf, rub), utils::packByte(d, db, df, u, ub, uf, b, f) };
}

std::vector<uint8_t> VoxelChunk::getVoxParals(const Voxel& vox, const std::vector<float>& pose, const ParalsInfoFormat& format) const
{
	std::vector<uint8_t> res;
	uint8_t oct = 0;

	if (!vox.isEmpty()) {
		return std::vector<uint8_t>(getSizeInBytes(format), 0);
	}

	uint32_t voxSide = pow(pyramid.base, vox.size);
	float offset = 1.0f / float(voxSide);

	std::vector<float> dir;
	std::vector<float> pos = { pose[0] + minOffset / 2.0f, pose[1] + minOffset / 2.0f, pose[2] + minOffset / 2.0f };

	for (float zOff = -minOffset; zOff <= minOffset; zOff += 2 * minOffset)
		for (float yOff = -minOffset; yOff <= minOffset; yOff += 2 * minOffset)
			for (float xOff = -minOffset; xOff <= minOffset; xOff += 2 * minOffset)
			{
				float x = pos[0], y = pos[1], z = pos[2];
				bool xGo = true, yGo = true, zGo = true, stop = false;
				while (abs(x) < 1.0f && abs(x) > 0.0f && !stop)
				{
					while (abs(y) < 1.0f && abs(y) > 0.0f && !stop)
					{
   						while (abs(z) < 1.0f && abs(z) > 0.0f && !stop)
						{
							if (xGo) xGo = _checkParal({ x + xOff, pos[1], pos[2] }, { x + xOff, y, z }, minOffset);
							if (zGo) zGo = _checkParal({ pos[0], pos[1], z + zOff }, { xGo ? (x + xOff) : x, y, z + zOff }, minOffset);
							if (yGo) yGo = _checkParal({ pos[0], y + yOff, pos[2] }, { xGo ? (x + xOff) : x, y + yOff, zGo ? (z + zOff) : z }, minOffset);


							if (!xGo && !yGo && !zGo) {

								if (format == ParalsInfoFormat::CUBIC_UINT8 || format == ParalsInfoFormat::CUBIC_FLOAT32)
								{
									stop = true;

									if (format == ParalsInfoFormat::CUBIC_FLOAT32)
										utils::appendBytes(res, side * (x - pos[0]));
									else
										res.push_back(side * (x - pos[0]));
								}
								else
								{
									stop = true;
									if (format == ParalsInfoFormat::NON_CUBIC_FLOAT32)
									{
										utils::appendBytes(res, side * abs(x - pos[0]));
										utils::appendBytes(res, side * abs(y - pos[1]));
										utils::appendBytes(res, side * abs(z - pos[2]));
									}
									else
									{
										res.push_back(side * abs(x - pos[0]));
										res.push_back(side * abs(y - pos[1]));
										res.push_back(side * abs(z - pos[2]));
										dir.push_back(xOff);
										dir.push_back(yOff);
										dir.push_back(zOff);
									}
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

	return res;
}

float VoxelChunk::getClosestSidePointDistance(const std::vector<int8_t>& dir)
{
	float dist = 0;

	uint8_t axis = (dir[0] != 0) ? 0 : (dir[1] != 0) ? 1 : (dir[2] != 0) ? 2 : -1;
	float start = dir[axis] < 0 ? 1.0f : 0.0f;
	float finish = start + dir[axis];
	float offset = minOffset * dir[axis];
	for (float a = start; abs(a) != abs(finish); a += offset)
	{
		bool layerOk = _checkParal({ (axis == 0 ? a : 0), (axis == 1 ? a : 0), (axis == 2 ? a : 0) }, 
			                       { (axis == 0 ? a : side - 1), (axis == 1 ? a : side - 1), (axis == 2 ? a : side - 1) }, minOffset);
		if (layerOk)
			dist += minOffset;
		else
			break;
	}
	
	return dist;
}

uint32_t VoxelChunkFormat::getSizeInBytes(bool alignToFourBytes) const
{
	uint32_t sz = ::getSizeInBytes(fullness) + ::getSizeInBytes(offset) + ::getSizeInBytes(size) + ::getSizeInBytes(parals);
	
	if (alignToFourBytes)
		return 4 * ceil(float(sz) / 4.0f);

	return sz;
}

std::vector<uint8_t> VoxelChunkFormat::formatChunkHeader(const VoxelChunk& chunk, uint32_t voxDataOffset, const std::vector<uint8_t>& parals, bool alignToFourBytes) const
{
	std::vector<uint8_t> res;
	res.reserve(getSizeInBytes(alignToFourBytes));

	switch (fullness)
	{
	case ChunkFullnessFormat::UINT8:
		utils::appendBytes(res, chunk.isEmpty());
		break;
	case ChunkFullnessFormat::UINT16:
		utils::appendBytes(res, uint16_t(chunk.isEmpty()));
		break;
	case ChunkFullnessFormat::UINT24:
		utils::appendBytes(res, uint32_t(chunk.isEmpty()), 3);
		break;
	case ChunkFullnessFormat::UINT32:
		utils::appendBytes(res, uint32_t(chunk.isEmpty()));
		break;
	default:
		break;
	}

	switch (offset)
	{
	case ChunkOffsetFormat::UINT8:
		utils::appendBytes(res, uint8_t(voxDataOffset));
		break;
	case ChunkOffsetFormat::UINT16:
		utils::appendBytes(res, uint16_t(voxDataOffset));
		break;
	case ChunkOffsetFormat::UINT24:
		utils::appendBytes(res, voxDataOffset, 3);
		break;
	case ChunkOffsetFormat::UINT32:
		utils::appendBytes(res, voxDataOffset);
		break;
	default:
		break;
	}

	switch (size)
	{
	case ChunkSizeFormat::UINT8:
		utils::appendBytes(res, uint8_t(chunk.side));
		break;
	case ChunkSizeFormat::UINT16:
		utils::appendBytes(res, uint16_t(chunk.side));
		break;
	case ChunkSizeFormat::BASE_POWER_UINT8:
		utils::appendBytes(res, chunk.pyramid.base);
		utils::appendBytes(res, chunk.pyramid.power);
		break;
	case ChunkSizeFormat::UINT24:
		utils::appendBytes(res, uint32_t(chunk.side), 3);
		break;
	case ChunkSizeFormat::UINT32:
		utils::appendBytes(res, uint32_t(chunk.side));
		break;
	default:
		break;
	}

	utils::appendBytes(res, parals);

	res.resize(getSizeInBytes(alignToFourBytes), 0);

	return res;
}

std::vector<uint8_t> VoxelChunkFormat::formatChunk(const VoxelChunk& chunk, bool alignToFourBytes) const
{
	std::vector<uint8_t> res;
	uint32_t size = chunk.voxFormat.getSizeInBytes(alignToFourBytes) * pow(chunk.side, 3);
	res.reserve(size);

	for (uint32_t x = 0; x < chunk.side; ++x)
		for (uint32_t y = 0; y < chunk.side; ++y)
			for (uint32_t z = 0; z < chunk.side; ++z)
			{
				auto voxPos = { x * chunk.minOffset, y * chunk.minOffset, z * chunk.minOffset };
				auto vox = chunk.getVoxel(voxPos);
				auto neighs = chunk.getNeighbours(vox, voxPos, chunk.voxFormat.neighbour);
				auto parals = chunk.getVoxParals(vox, voxPos, chunk.voxFormat.parals);
				utils::appendBytes(res, chunk.voxFormat.formatVoxel(vox, vox.size, neighs, parals, alignToFourBytes));
			}

	res.resize(size, 0);

	return res;
}