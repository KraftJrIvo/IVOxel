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
		sizeInBytes += sizeInBytesColor += 1;
		break;
	case VoxelColorFormat::RGB256_WITH_ALPHA:
		sizeInBytes += sizeInBytesColor += 2;
		break;
	case VoxelColorFormat::THREE_BYTES_RGB:
		sizeInBytes += sizeInBytesColor += 3;
		break;
	case VoxelColorFormat::THREE_BYTES_RGB_WITH_ALPHA:
		sizeInBytes += sizeInBytesColor += 4;
		break;
	case VoxelColorFormat::NO_COLOR:
	default:
		break;
	}

	sizeInBytesNeighbourInfo = 0;
	switch (neightInfoFormat)
	{
	case VoxelNeighbourInfoFormat::BINARY_6_DIR_INFO:
		sizeInBytes += sizeInBytesNeighbourInfo += 1;
		break;
	case VoxelNeighbourInfoFormat::BINARY_26_DIR_INFO:
		sizeInBytes += sizeInBytesNeighbourInfo += 4;
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
}

std::vector<uint8_t> VoxelMapType::formatNeighbours(const std::vector<int32_t>& neighsTypes, int32_t type)
{
	//TODO

	return {};
}

VoxelData VoxelMapType::unformatVoxelData(const uint8_t* data)
{
	//TODO orient

	VoxelData voxData;

	uint8_t* ptr = (uint8_t*)data;

	switch (typeFormat)
	{
	case VoxelTypeFormat::UINT8:
	case VoxelTypeFormat::UINT8_WITH_ORIENTATION:
		std::get<0>(voxData) = *ptr;
		ptr += sizeof(uint8_t);
		break;
	case VoxelTypeFormat::UINT16:
	case VoxelTypeFormat::UINT16_WITH_ORIENTATION:
		std::get<0>(voxData) = *((uint16_t*)ptr);
		ptr += sizeof(uint16_t);
		break;
	}

	auto& color = std::get<1>(voxData);
	color = {0, 0, 0, 255};
	uint8_t rgb256;
	switch (colorFormat)
	{
	case VoxelColorFormat::GRAYSCALE:
		color[R] = color[G] = color[B] = *ptr;
		ptr += sizeof(uint8_t);
		break;
	case VoxelColorFormat::RGB256:
		rgb256 = *ptr;
		color[R] = (uint8_t)((rgb256 & 0xE0) >> 5);
		color[G] = (uint8_t)((rgb256 & 0x1C) >> 2);
		color[B] = (uint8_t)(rgb256 & 0x03);
		ptr += sizeof(uint8_t);
		break;
	case VoxelColorFormat::RGB256_WITH_ALPHA:
		rgb256 = *ptr;
		color[R] = (uint8_t)((rgb256 & 0xE0) >> 5);
		color[G] = (uint8_t)((rgb256 & 0x1C) >> 2);
		color[B] = (uint8_t)(rgb256 & 0x03);
		ptr += sizeof(uint8_t);
		color[A] = *ptr;
		ptr += sizeof(uint8_t);
		break;
	case VoxelColorFormat::THREE_BYTES_RGB:
		color[R] = *(ptr++);
		color[G] = *(ptr++);
		color[B] = *(ptr++);
		break;
	case VoxelColorFormat::THREE_BYTES_RGB_WITH_ALPHA:
		color[R] = *(ptr++);
		color[G] = *(ptr++);
		color[B] = *(ptr++);
		color[A] = *(ptr++);
		break;
	case VoxelColorFormat::NO_COLOR:
	default:
		break;
	}

	//TODO unformat neighbours

	return voxData;
}
