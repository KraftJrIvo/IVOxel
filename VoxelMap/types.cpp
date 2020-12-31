#include "types.h"
#include <map>

namespace utils
{

	std::istream& operator>>(std::istream& is, utils::WordDelimitedBySpace& output)
	{
		std::getline(is, output, ' ');
		return is;
	}

	std::vector<uint8_t> joinVectors(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b)
	{
		std::vector<uint8_t> ab;
		ab.reserve(a.size() + b.size());
		ab.insert(ab.end(), a.begin(), a.end());
		ab.insert(ab.end(), b.begin(), b.end());
		return ab;
	}

	uint8_t encodeRGB(uint8_t r, uint8_t g, uint8_t b)
	{
		return ((r / 32 << 5) | (g / 32 << 2) | b / 64);
	}

	std::vector<uint8_t> decodeRGB(uint8_t rgb256)
	{
		return {
			(uint8_t)(32 * ((rgb256 & 0xE0) >> 5)),
			(uint8_t)(32 * ((rgb256 & 0x1C) >> 2)),
			(uint8_t)(64 * (rgb256 & 0x03))
		};
	}

	float calculateDist(const std::vector<float>& start, const std::vector<float>& end, float div)
	{
		std::vector<float> pos = { end[X] - start[X], end[Y] - start[Y], end[Z] - start[Z] };

		float val = sqrt((pos[X] * pos[X]) + (pos[Y] * pos[Y]) + (pos[Z] * pos[Z]));

		if (div != 1.0f)
			val /= div;

		return  val;
	}

}

uint32_t VoxelFormat::getSizeInBytes(bool alignToFourBytes)
{
	uint32_t res = 0;

	switch (fullness)
	{
	case VoxelFullnessFormat::UINT8:
		res++;
		break;
	case VoxelFullnessFormat::UINT16:
		res += 2;
		break;	
	case VoxelFullnessFormat::UINT24:
		res += 3;
		break;
	case VoxelFullnessFormat::UINT32:
		res += 4;
		break;
	default:
		break;
	}

	switch (type)
	{
	case VoxelTypeFormat::UINT8:
		res++;
		break;
	case VoxelTypeFormat::UINT16:
		res += 2;
		break;
	default:
		break;
	}

	switch (orientation)
	{
	case VoxelOrientationFormat::UINT8:
		res += 1;
		break;
	default:
		break;
	}

	switch (color)
	{
	case VoxelColorFormat::GRAYSCALE:
		res++;
		break;
	case VoxelColorFormat::RGB256:
		res++;
		break;
	case VoxelColorFormat::RGB256_WITH_ALPHA:
		res += 2;
		break;
	case VoxelColorFormat::RGB_THREE_BYTES:
		res += 3;
		break;
	case VoxelColorFormat::RGBA_FOUR_BYTES:
		res += 4;
		break;
	default:
		break;
	}

	switch (neighbour)
	{
	case VoxelNeighbourInfoFormat::SIX_DIRS_ONE_BYTE:
		res++;
		break;
	case VoxelNeighbourInfoFormat::TWENTY_SIX_DIRS_FOUR_BYTES:
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

uint32_t ChunkFormat::getSizeInBytes(bool alignToFourBytes)
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

	if (alignToFourBytes)
		return ceil(float(res) / 4.0f);

	return res;
}
