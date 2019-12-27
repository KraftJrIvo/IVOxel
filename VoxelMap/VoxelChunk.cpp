#include "VoxelChunk.h"

#include <algorithm>

VoxelChunk::VoxelChunk() :
	_nPyramidPower(0),
	_nTypeBytes(1)
{
}

VoxelChunk::VoxelChunk(const std::vector<int>& size, VoxelMapType type)
{
	int maxDim = std::max(size[0], std::max(size[1], size[2]));
	_nPyramidPower = unsigned char(std::ceil(std::log2(maxDim)));

	switch (type)
	{
	case ONE_BYTE_RGB256:
		_nTypeBytes = 1;
		break;
	case TWO_BYTES_RGB256_AND_ALPHA:
	case TWO_BYTES_RGB256_AND_TYPE:
		_nTypeBytes = 2;
		break;
	case THREE_BYTES_RGB256_ALPHA_AND_TYPE:
	case THREE_BYTES_RGB:
		_nTypeBytes = 3;
		break;
	case FOUR_BYTES_RGB_AND_TYPE:
	case FOUR_BYTES_RGBA:
		_nTypeBytes = 4;
		break;
	case FIVE_BYTES_RGBA_AND_TYPE:
		_nTypeBytes = 5;
		break;
	default:
		_nTypeBytes = 1;
		break;
	}
}
