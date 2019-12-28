#include "VoxelMapType.h"

VoxelMapType::VoxelMapType()
{
	computeSizeInBytes();
}

VoxelMapType::VoxelMapType(VoxelTypeFormat _typeFormat, VoxelColorFormat _colorFormat, VoxelNeighbourInfoFormat _neightInfoFormat) :
	typeFormat(_typeFormat),
	colorFormat(_colorFormat),
	neightInfoFormat(_neightInfoFormat)
{
	computeSizeInBytes();
}

uint8_t VoxelMapType::computeSizeInBytes()
{
	sizeInBytes = 0;

	switch (typeFormat)
	{
	case VoxelTypeFormat::UINT8:
		sizeInBytes += 1;
		break;
	case VoxelTypeFormat::UINT8_WITH_ORIENTATION:
	case VoxelTypeFormat::UINT16:
		sizeInBytes += 2;
		break;
	case VoxelTypeFormat::UINT16_WITH_ORIENTATION:
		sizeInBytes += 3;
		break;
	case VoxelTypeFormat::NO_TYPE:
	default:
		break;
	}

	switch (colorFormat)
	{
	case VoxelColorFormat::GRAYSCALE:
	case VoxelColorFormat::RGB256:
		sizeInBytes += 1;
		break;
	case VoxelColorFormat::RGB256_WITH_ALPHA:
		sizeInBytes += 2;
		break;
	case VoxelColorFormat::THREE_BYTES_RGB:
		sizeInBytes += 3;
		break;
	case VoxelColorFormat::THREE_BYTES_RGB_WITH_ALPHA:
		sizeInBytes += 4;
		break;
	case VoxelColorFormat::NO_COLOR:
	default:
		break;
	}

	switch (neightInfoFormat)
	{
	case VoxelNeighbourInfoFormat::BINARY_6_DIR_INFO:
		sizeInBytes += 1;
		break;
	case VoxelNeighbourInfoFormat::BINARY_26_DIR_INFO:
		sizeInBytes += 4;
		break;
	case VoxelNeighbourInfoFormat::NO_NEIGHBOUR_INFO:
	default:
		break;
	}

	return sizeInBytes;
}
