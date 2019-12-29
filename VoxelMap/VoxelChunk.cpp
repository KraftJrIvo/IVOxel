#include "VoxelChunk.h"

VoxelChunk::VoxelChunk()
{
}

VoxelChunk::VoxelChunk(const std::vector<uint32_t>& _size, const VoxelMapType& _type) :
	size(_size),
	type(_type)
{	
}

void VoxelChunk::buildPyramid()
{
	pyramid = VoxelPyramid(size, type);

	pyramid.build(size, vTypes, vColors, vNeighbours);
}