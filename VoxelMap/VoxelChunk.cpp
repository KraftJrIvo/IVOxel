#include "VoxelChunk.h"

VoxelChunk::VoxelChunk(const std::vector<Voxel>& voxels, const VoxelChunkFormat& format_, const VoxelFormat& voxFormat_, bool alignToFourBytes) :
	format(format_),
	voxFormat(voxFormat_),
	voxFormatForPyr(voxFormat_)
{ 
	voxFormatForPyr.parals = ParalsInfoFormat::NO_PARALS;
	voxFormatForPyr.neighbour = VoxelNeighbourInfoFormat::NO_NEIGHBOUR_INFO;

	_buildPyramid(voxels, alignToFourBytes);
}

void VoxelChunk::_buildPyramid(const std::vector<Voxel>& voxels, bool alignToFourBytes)
{
	side = pow(voxels.size(), 1.0f / 3.0f);

	pyramid = VoxelChunkPyramid(voxFormat, side, alignToFourBytes);
	pyramid.build(voxels);
}

bool VoxelChunk::_checkParal(const std::vector<int16_t>& from, const std::vector<int16_t>& to, float offset) const
{
	for (int8_t x = from[0]; x <= to[0]; x++)
		for (int8_t y = from[1]; y <= to[1]; y++)
			for (int8_t z = from[2]; z <= to[2]; z++)
			{
				std::vector<float> pos = { x * offset + offset / 2.0f, y * offset + offset / 2.0f, z * offset + offset / 2.0f };
				if (getVoxel(pos).shape)
					return false;
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
	if (pyramid.power == 0)
		return getVoxel({0.5, 0.5, 0.5}).isEmpty();
	return false;
}

Voxel VoxelChunk::getVoxel(const std::vector<float>& chunkPos) const
{
	std::vector<uint32_t> pos = { uint32_t(floor(chunkPos[0])), uint32_t(floor(chunkPos[1])), uint32_t(floor(chunkPos[2])) };

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

std::vector<uint8_t> VoxelChunk::getVoxParals(const Voxel& vox, const std::vector<float>& voxInChunkPos, const ParalsInfoFormat& format) const
{
	std::vector<uint8_t> res;
	uint8_t oct = 0;

	uint32_t voxSide = pow(pyramid.base, vox.size);
	float offset = 1.0f / float(voxSide);

	for (int16_t zOff = -1; zOff <= 1; zOff += 2)
		for (int16_t yOff = -1; yOff <= 1; yOff += 2)
			for (int16_t xOff = -1; xOff <= 1; xOff += 2)
			{
				int16_t x, y, z;
				bool xGo = true, yGo = true, zGo = true, stop = false;
				for (x = 0; abs(x) < 256 && !stop; x += xOff * xGo)
					for (y = 0; abs(y) < 256 && !stop; y += yOff * yGo)
						for (z = 0; abs(z) < 256 && !stop; z += zOff * zGo)
						{
							xGo = _checkParal({ short(x + xOff), 0, 0 }, { short(x + xOff), y, z }, offset);
							zGo = _checkParal({ 0, 0, short(z + zOff) }, { xGo ? short(x + xOff) : x, y, short(z + zOff) }, offset);
							yGo = _checkParal({ 0, short(y + yOff), 0 }, { xGo ? short(x + xOff) : x, short(y + yOff), zGo ? short(z + zOff) : z }, offset);

							if (format == ParalsInfoFormat::CUBIC_UINT8 || format == ParalsInfoFormat::CUBIC_FLOAT32)
								if (!xGo || !yGo || !zGo)
								{
									stop = true;
									
									if (format == ParalsInfoFormat::CUBIC_FLOAT32)
										utils::appendBytes(res, x * offset);
									else
										res.push_back(x);
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

float VoxelChunk::getClosestSidePointDistance(const std::vector<int8_t>& dir)
{
	float dist = 0;

	uint8_t axis = (dir[0] != 0) ? 0 : (dir[1] != 0) ? 1 : (dir[2] != 0) ? 2 : -1;
	float start = dir[axis] < 0 ? 1.0f : 0.0f;
	float finish = start + dir[axis];
	float offset = minOffset * dir[axis];
	for (float a = start; a < finish; a += offset)
	{
		bool layerOk = _checkParal({ short(axis == 0 ? a : 0), short(axis == 1 ? a : 0), short(axis == 2 ? a : 0) }, 
			                       { short(axis == 0 ? a : side - 1), short(axis == 1 ? a : side - 1), short(axis == 2 ? a : side - 1) }, minOffset);
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
		return ceil(float(sz) / 4.0f);

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

	return res;
}