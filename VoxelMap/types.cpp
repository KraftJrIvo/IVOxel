#include "types.hpp"

#include "Voxel.h"

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
		std::vector<float> pos = { end[0] - start[0], end[1] - start[1], end[2] - start[2] };

		float val = sqrt((pos[0] * pos[0]) + (pos[1] * pos[1]) + (pos[2] * pos[2]));

		if (div != 1.0f)
			val /= div;

		return  val;
	}

	void appendBytes(std::vector<uint8_t>& bytes, std::vector<uint8_t> extraBytes) 
	{
		auto prevSz = bytes.size();
		bytes.resize(bytes.size() + extraBytes.size());
		memcpy(bytes.data() + prevSz, extraBytes.data(), extraBytes.size());
	}

	uint8_t packByte(const bool& a7, const bool& a6, const bool& a5, const bool& a4, const bool& a3, const bool& a2, const bool& a1, const bool& a0)
	{
		return (a7 ? 128 : 0) | (a6 ? 64 : 0) | (a5 ? 32 : 0) | (a4 ? 16 : 0) | (a3 ? 8 : 0) | (a2 ? 4 : 0) | (a1 ? 2 : 0) | (a0 ? 1 : 0);
	}

	void unpackByte(const uint8_t* byte, bool& a7, bool& a6, bool& a5, bool& a4, bool& a3, bool& a2, bool& a1, bool& a0)
	{
		a7 = (*byte) & 128;
		a6 = (*byte) & 64;
		a5 = (*byte) & 32;
		a4 = (*byte) & 16;
		a3 = (*byte) & 8;
		a2 = (*byte) & 4;
		a1 = (*byte) & 2;
		a0 = (*byte) & 1;
	}
}

uint8_t getSizeInBytes(VoxelFullnessFormat vff)
{
	switch (vff)
	{
	case VoxelFullnessFormat::UINT8:
		return 1;
	case VoxelFullnessFormat::UINT16:
		return 2;
	case VoxelFullnessFormat::UINT24:
		return 3;
	case VoxelFullnessFormat::UINT32:
		return 4;
	}
	return 0;
}

uint8_t getSizeInBytes(ChunkFullnessFormat cff)
{
	switch (cff)
	{
	case ChunkFullnessFormat::UINT8:
		return 1;
	case ChunkFullnessFormat::UINT16:
		return 2;
	case ChunkFullnessFormat::UINT24:
		return 3;
	case ChunkFullnessFormat::UINT32:
		return 4;
	}
	return 0;
}

uint8_t getSizeInBytes(ChunkOffsetFormat cof)
{
	switch (cof)
	{
	case ChunkOffsetFormat::UINT8:
		return 1;
	case ChunkOffsetFormat::UINT16:
		return 2;
	case ChunkOffsetFormat::UINT24:
		return 3;
	case ChunkOffsetFormat::UINT32:
		return 4;
	}
	return 0;
}

uint8_t getSizeInBytes(VoxelShapeFormat vtf)
{
	switch (vtf)
	{
	case VoxelShapeFormat::UINT8:
		return 1;
	case VoxelShapeFormat::UINT16:
		return 2;
	}
	return 0;
}

uint8_t getSizeInBytes(VoxelMaterialFormat vtf)
{
	switch (vtf)
	{
	case VoxelMaterialFormat::UINT8:
		return 1;
	case VoxelMaterialFormat::UINT16:
		return 2;
	}
	return 0;
}

uint8_t getSizeInBytes(VoxelOrientationFormat vof)
{
	switch (vof)
	{
	case VoxelOrientationFormat::UINT8:
		return 1;
	}
	return 0;
}

uint8_t getSizeInBytes(ChunkSizeFormat csf)
{
	switch (csf)
	{
	case ChunkSizeFormat::UINT8:
		return 1;
	case ChunkSizeFormat::UINT16:
		return 2;
	case ChunkSizeFormat::BASE_POWER_UINT8:
		return 2;
	case ChunkSizeFormat::UINT24:
		return 3;
	case ChunkSizeFormat::UINT32:
		return 4;
	}
	return 0;
}

uint8_t getSizeInBytes(VoxelSizeFormat vsf)
{
	switch (vsf)
	{
	case VoxelSizeFormat::UINT8:
		return 1;	
	}
	return 0;
}

uint8_t getSizeInBytes(VoxelColorFormat vcf)
{
	switch (vcf)
	{
	case VoxelColorFormat::GRAYSCALE:
		return 1;
	case VoxelColorFormat::RGB256:
		return 1;
	case VoxelColorFormat::RGB256_WITH_ALPHA:
		return 2;
	case VoxelColorFormat::RGB_THREE_BYTES:
		return 3;
	case VoxelColorFormat::RGBA_FOUR_BYTES:
		return 4;
	}
	return 0;
}

uint8_t getSizeInBytes(VoxelNeighbourInfoFormat vnif)
{
	switch (vnif)
	{
	case VoxelNeighbourInfoFormat::SIX_DIRS_ONE_BYTE:
		return 1;
	case VoxelNeighbourInfoFormat::TWENTY_SIX_DIRS_FOUR_BYTES:
		return 4;
	}
	return 0;
}

uint8_t getSizeInBytes(ParalsInfoFormat pif)
{
	switch (pif)
	{
	case ParalsInfoFormat::CUBIC_UINT8:
		return 8;
	case ParalsInfoFormat::NON_CUBIC_UINT8:
		return 3 * 8;
	case ParalsInfoFormat::CUBIC_FLOAT32:
		return sizeof(float) * 8;
	case ParalsInfoFormat::NON_CUBIC_FLOAT32:
		return sizeof(float) * 3 * 8;
	}
	return 0;
}
