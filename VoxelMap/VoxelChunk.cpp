#include "VoxelChunk.h"

VoxelChunk::VoxelChunk(const std::vector<Voxel>& voxels, const VoxelChunkFormat& format_) :
	format(format_)
{ }

void VoxelChunk::_buildPyramid(const std::vector<Voxel>& voxels)
{
	pyramid = VoxelChunkPyramid(format);

	pyramid.build(voxels);

	side = pow(pyramid.base, pyramid.power);
	minOffset = 1.0f / float(side);
}

void VoxelChunk::changeVoxels(const std::vector<VoxelModifyData>& voxels)
{
	modified = true;

	std::vector<Voxel> curVoxels;

	std::vector<Voxel> newVoxels;

	//TODO

	_buildPyramid(newVoxels);
}

bool VoxelChunk::isEmpty() const
{
	if (pyramid.power == 0)
		return getVoxel({0.5, 0.5, 0.5}).isEmpty();
	return false;
}

Voxel VoxelChunk::getVoxel(const std::vector<float>& chunkPos) const
{
	std::vector<uint32_t> pos = { uint32_t(floor(chunkPos[X])), uint32_t(floor(chunkPos[Y])), uint32_t(floor(chunkPos[Z])) };

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
	uint32_t zLayerLen = std::pow(base, DIMENSIONS - 1);
	uint32_t yRowLen = std::pow(base, DIMENSIONS - 2);

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
		curPwrLayerPos = ((pos[Z] % curSide) / sidePart) * zLayerLen + ((pos[Y] % curSide) / sidePart) * yRowLen + ((pos[X] % curSide) / sidePart);

		curLayerLen -= leavesOnLayers[curPwr];
		curLayerLen *= std::pow(base, DIMENSIONS);

		curPwr++;
		bytesForThisLayer = VoxelChunkPyramid::getPyramLayerBytesCount(base, curPwr);
		ptr += bytesForThisLayer * uint32_t(val * std::pow(base, DIMENSIONS) + curPwrLayerPos); // skipping to the value of interest
	}

	ptr = (uint8_t*)pyramid.data.data() + sizeof(uint32_t) + 3 * sizeof(uint8_t) +
		leavesOnLayers.size() * sizeof(uint32_t) + nOffsetBytes;

	ptr += voxSizeInBytes * nLeavesBeforeCurrent;

	return format.unformatVoxel(ptr, curPwr);
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

	auto checkParal = [&](const std::vector<int16_t>& from, const std::vector<int16_t>& to)
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
