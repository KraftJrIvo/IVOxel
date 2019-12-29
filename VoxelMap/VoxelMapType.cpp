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

	sizeInBytesType = 0;
	switch (typeFormat)
	{
	case VoxelTypeFormat::UINT8:
		sizeInBytesType = sizeInBytes += 1;
		break;
	case VoxelTypeFormat::UINT8_WITH_ORIENTATION:
	case VoxelTypeFormat::UINT16:
		sizeInBytesType = sizeInBytes += 2;
		break;
	case VoxelTypeFormat::UINT16_WITH_ORIENTATION:
		sizeInBytesType = sizeInBytes += 3;
		break;
	case VoxelTypeFormat::NO_TYPE:
	default:
		break;
	}

	sizeInBytesColor = 0;
	switch (colorFormat)
	{
	case VoxelColorFormat::GRAYSCALE:
	case VoxelColorFormat::RGB256:
		sizeInBytesColor = sizeInBytes += 1;
		break;
	case VoxelColorFormat::RGB256_WITH_ALPHA:
		sizeInBytesColor = sizeInBytes += 2;
		break;
	case VoxelColorFormat::THREE_BYTES_RGB:
		sizeInBytesColor = sizeInBytes += 3;
		break;
	case VoxelColorFormat::THREE_BYTES_RGB_WITH_ALPHA:
		sizeInBytesColor = sizeInBytes += 4;
		break;
	case VoxelColorFormat::NO_COLOR:
	default:
		break;
	}

	sizeInBytesNeighbourInfo = 0;
	switch (neightInfoFormat)
	{
	case VoxelNeighbourInfoFormat::BINARY_6_DIR_INFO:
		sizeInBytesNeighbourInfo = 1;
		break;
	case VoxelNeighbourInfoFormat::BINARY_26_DIR_INFO:
		sizeInBytesNeighbourInfo = 4;
		break;
	case VoxelNeighbourInfoFormat::NO_NEIGHBOUR_INFO:
	default:
		break;
	}

	return sizeInBytes;
}

std::vector<uint8_t> VoxelMapType::formatType(int32_t type, uint8_t orientation, bool flipX, bool flipY, bool flipZ)
{
	//TODO flip

	uint8_t* ptr;

	switch (typeFormat)
	{
	case VoxelTypeFormat::UINT8:
		return { uint8_t(type) };
	case VoxelTypeFormat::UINT8_WITH_ORIENTATION:
		return { uint8_t(type), orientation };
	case VoxelTypeFormat::UINT16:
		ptr = (uint8_t*)(&type);
		return { ptr[1], ptr[0] };
	case VoxelTypeFormat::UINT16_WITH_ORIENTATION:
		ptr = (uint8_t*)(&type);
		return { ptr[1], ptr[0], orientation };
	case VoxelTypeFormat::NO_TYPE:
	default:
		return {};
	}
}

std::vector<uint8_t> VoxelMapType::formatColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	uint8_t rgb8;

	switch (colorFormat)
	{
	case VoxelColorFormat::GRAYSCALE:
		return { r };
	case VoxelColorFormat::RGB256:
		rgb8 = ((r << 5) | (g << 2) | b);
		return { rgb8 };
	case VoxelColorFormat::RGB256_WITH_ALPHA:
		rgb8 = ((r << 5) | (g << 2) | b);
		return { rgb8, a };
	case VoxelColorFormat::THREE_BYTES_RGB:
		return { r, g, b };
	case VoxelColorFormat::THREE_BYTES_RGB_WITH_ALPHA:
		return { r, g, b, a };
	case VoxelColorFormat::NO_COLOR:
	default:
		break;
	}

	//256 back to rgb
	//byte R = (byte)((rgb8 & 0xE0) >> 5);     // rgb8 & 1110 0000  >> 5
	//byte G = (byte)((rgb8 & 0x1C) >> 2);     // rgb8 & 0001 1100  >> 2
	//byte B = (byte)(rgb8 & 0x03);            // rgb8 & 0000 0011
}

std::vector<uint8_t> VoxelMapType::formatNeighbours(const std::vector<int32_t>& neighsTypes, int32_t type)
{
	//TODO

	return {};
}
