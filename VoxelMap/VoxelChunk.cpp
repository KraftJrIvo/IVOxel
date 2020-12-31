#include "VoxelChunk.h"

VoxelChunk::VoxelChunk()
{
}

VoxelChunk::VoxelChunk(const std::vector<Voxel>& voxels, const VoxelMapFormat& format_) :
	format(format_)
{	
}

void VoxelChunk::_buildPyramid(const std::vector<Voxel>& voxels)
{
	pyramid = VoxelChunkPyramid(format);

	pyramid.build(voxels);
}

void VoxelChunk::changeVoxels(const std::vector<VoxelModifyData>& voxels)
{
	modified = true;

	std::vector<Voxel> curVoxels;

	std::vector<Voxel> newVoxels;

	//TODO

	_buildPyramid(newVoxels);
}

bool VoxelChunk::isEmpty()
{
	if (pyramid.power == 0)
		return getVoxel({0.5, 0.5, 0.5}).isEmpty();
	return false;
}

Voxel VoxelChunk::getVoxel(const std::vector<float>& chunkPos)
{
	Voxel vox;

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

	return format.unformatVoxel(ptr);
}
