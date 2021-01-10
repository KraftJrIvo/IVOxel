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
	side = pow(pyramid.base, pyramid.power);
	minOffset = 1.0f / float(side);

	pyramid = VoxelChunkPyramid(voxFormat, side, alignToFourBytes);
	pyramid.build(voxels);
}

bool VoxelChunk::_checkParal(const std::vector<int16_t>& from, const std::vector<int16_t>& to, float offset, std::vector<VoxelType> transparentTypes) const
{
	for (int8_t x = from[0]; x <= to[0]; x++)
		for (int8_t y = from[1]; y <= to[1]; y++)
			for (int8_t z = from[2]; z <= to[2]; z++)
			{
				std::vector<float> pos = { x * offset + offset / 2.0f, y * offset + offset / 2.0f, z * offset + offset / 2.0f };
				if (std::find(transparentTypes.begin(), transparentTypes.end(), getVoxel(pos).type) == transparentTypes.end())
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

	return voxFormatForPyr.unformatVoxel(ptr, curPwr);
}

std::vector<uint8_t> VoxelChunk::getNeighbours(const Voxel& vox, const std::vector<float>& chunkPos, const VoxelNeighbourInfoFormat& format, std::vector<VoxelType> connectableTypes) const
{
	if (connectableTypes.size() == 0)
		connectableTypes.push_back(vox.type);
	
	uint32_t voxSide = pow(pyramid.base, vox.power);
	float offset = 1.0f / float(voxSide);
	std::vector<float> pos = { floor(chunkPos[0] / offset) + offset / 2.0f, floor(chunkPos[1] / offset) + offset / 2.0f, floor(chunkPos[2] / offset) + offset / 2.0f };

	bool l = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] - offset, pos[1], pos[2] }).type) != connectableTypes.end();
	bool r = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] + offset, pos[1], pos[2] }).type) != connectableTypes.end();
	bool d = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0],          pos[1] - offset, pos[2] }).type) != connectableTypes.end();
	bool u = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0],          pos[1] + offset, pos[2] }).type) != connectableTypes.end();
	bool b = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0],          pos[1],          pos[2] - offset }).type) != connectableTypes.end();
	bool f = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0],          pos[1],          pos[2] + offset }).type) != connectableTypes.end();

	if (format == VoxelNeighbourInfoFormat::SIX_DIRS_ONE_BYTE)
		return { utils::packByte(0, 0, l, r, d, u, b, f) };
	
	bool ld  = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] - offset, pos[1] - offset, pos[2]          }).type) != connectableTypes.end();
	bool lu  = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] - offset, pos[1] + offset, pos[2]          }).type) != connectableTypes.end();
	bool lb  = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] - offset, pos[1],          pos[2] - offset }).type) != connectableTypes.end();
	bool lf  = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] - offset, pos[1],          pos[2] + offset }).type) != connectableTypes.end();
	bool ldb = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] - offset, pos[1] - offset, pos[2] - offset }).type) != connectableTypes.end();
	bool lub = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] - offset, pos[1] + offset, pos[2] - offset }).type) != connectableTypes.end();
	bool ldf = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] - offset, pos[1] - offset, pos[2] + offset }).type) != connectableTypes.end();
	bool luf = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] - offset, pos[1] + offset, pos[2] + offset }).type) != connectableTypes.end();
	bool rd  = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] + offset, pos[1] - offset, pos[2]          }).type) != connectableTypes.end();
	bool ru  = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] + offset, pos[1] + offset, pos[2]          }).type) != connectableTypes.end();
	bool rb  = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] + offset, pos[1],          pos[2] - offset }).type) != connectableTypes.end();
	bool rf  = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] + offset, pos[1],          pos[2] + offset }).type) != connectableTypes.end();
	bool rdb = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] + offset, pos[1] - offset, pos[2] - offset }).type) != connectableTypes.end();
	bool rub = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] + offset, pos[1] + offset, pos[2] - offset }).type) != connectableTypes.end();
	bool rdf = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] + offset, pos[1] - offset, pos[2] + offset }).type) != connectableTypes.end();
	bool ruf = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0] + offset, pos[1] + offset, pos[2] + offset }).type) != connectableTypes.end();
	
	bool db = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0], pos[1] - offset, pos[2] - offset }).type) != connectableTypes.end();
	bool df = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0], pos[1] - offset, pos[2] + offset }).type) != connectableTypes.end();
	bool ub = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0], pos[1] + offset, pos[2] - offset }).type) != connectableTypes.end();
	bool uf = std::find(connectableTypes.begin(), connectableTypes.end(), getVoxel({ pos[0], pos[1] + offset, pos[2] + offset }).type) != connectableTypes.end();

	//0 0 0 0 0 0 l ld lu lb lf ldb ldf lub luf r rd ru rb rf rdb rdf rub ruf d db df u ub uf b f
	return { utils::packByte(0, 0, 0, 0, 0, 0, l, ld), utils::packByte(lu, lb, lf, ldb, ldf, lub, luf, r), utils::packByte(r, rd, ru, rb, rf, rdb, rdf, rub), utils::packByte(d, db, df, u, ub, uf, b, f) };
}

std::vector<uint8_t> VoxelChunk::getVoxParals(const Voxel& vox, const std::vector<float>& voxInChunkPos, const ParalsInfoFormat& format, std::vector<VoxelType> transparentTypes) const
{
	std::vector<uint8_t> res;
	uint8_t oct = 0;

	if (transparentTypes.size() == 0)
		transparentTypes.push_back(VoxelType::AIR);

	uint32_t voxSide = pow(pyramid.base, vox.power);
	float offset = 1.0f / float(voxSide);

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
							xGo = _checkParal({ short(x + xOff), 0, 0 }, { short(x + xOff), y, z }, offset, transparentTypes);
							zGo = _checkParal({ 0, 0, short(z + zOff) }, { xGo ? short(x + xOff) : x, y, short(z + zOff) }, offset, transparentTypes);
							yGo = _checkParal({ 0, short(y + yOff), 0 }, { xGo ? short(x + xOff) : x, short(y + yOff), zGo ? short(z + zOff) : z }, offset, transparentTypes);

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

float VoxelChunk::getClosestSidePointDistance(const std::vector<int8_t>& dir, std::vector<VoxelType> transparentTypes)
{
	float dist = 0;

	if (transparentTypes.size() == 0)
		transparentTypes.push_back(VoxelType::AIR);

	uint8_t axis = (dir[0] != 0) ? 0 : (dir[1] != 0) ? 1 : (dir[2] != 0) ? 2 : -1;
	float start = dir[axis] < 0 ? 1.0f : 0.0f;
	float finish = start + dir[axis];
	float offset = minOffset * dir[axis];
	for (float a = start; a < finish; a += offset)
	{
		bool layerOk = _checkParal({ short(axis == 0 ? a : 0), short(axis == 1 ? a : 0), short(axis == 2 ? a : 0) }, 
			                       { short(axis == 0 ? a : side - 1), short(axis == 1 ? a : side - 1), short(axis == 2 ? a : side - 1) }, minOffset, transparentTypes);
		if (layerOk)
			dist += minOffset;
		else
			break;
	}
	
	return dist;
}

uint32_t VoxelChunkFormat::getSizeInBytes(bool alignToFourBytes) const
{
	uint32_t res = 0;

	ChunkFullnessFormat fullness;
	ChunkOffsetFormat   offset;
	ChunkSizeFormat     size;
	ParalsInfoFormat    parals;

	switch (fullness)
	{
	case ChunkFullnessFormat::UINT8:
		res++;
		break;
	case ChunkFullnessFormat::UINT16:
		res += 2;
		break;
	case ChunkFullnessFormat::UINT24:
		res += 3;
		break;
	case ChunkFullnessFormat::UINT32:
		res += 4;
		break;
	default:
		break;
	}

	switch (offset)
	{
	case ChunkOffsetFormat::UINT8:
		res++;
		break;
	case ChunkOffsetFormat::UINT16:
		res += 2;
		break;
	case ChunkOffsetFormat::UINT24:
		res += 3;
		break;
	case ChunkOffsetFormat::UINT32:
		res += 4;
		break;
	default:
		break;
	}

	switch (size)
	{
	case ChunkSizeFormat::UINT8:
		res++;
		break;
	case ChunkSizeFormat::UINT16:
		res += 2;
		break;
	case ChunkSizeFormat::BASE_POWER_UINT8:
		res += 2;
		break;
	case ChunkSizeFormat::UINT24:
		res += 3;
		break;
	case ChunkSizeFormat::UINT32:
		res += 4;
		break;
	default:
		break;
	}

	switch (parals)
	{
	case ParalsInfoFormat::CUBIC_UINT8:
		res += 8;
		break;
	case ParalsInfoFormat::NON_CUBIC_UINT8:
		res += 3 * 8;
		break;
	case ParalsInfoFormat::CUBIC_FLOAT32:
		res += sizeof(float) * 8;
		break;
	case ParalsInfoFormat::NON_CUBIC_FLOAT32:
		res += sizeof(float) * 3 * 8;
		break;
	default:
		break;
	}

	if (alignToFourBytes)
		return ceil(float(res) / 4.0f);

	return res;
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
				utils::appendBytes(res, chunk.voxFormat.formatVoxel(vox, neighs, parals, alignToFourBytes));
			}

	return res;
}