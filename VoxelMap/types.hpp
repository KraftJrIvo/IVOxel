#pragma once

#include <vector>
#include <string>
#include <istream>

struct VoxelNeighbours
{
	bool l, r, d, u, b, f;
	bool ld, lu, lb, lf, rd, ru, rb, rf, db, df, ub, uf;
	bool ldb, ldf, lub, luf, rdb, rdf, rub, ruf;
};

enum class VoxelFullnessFormat
{
	UINT8,							// 1 byte
	UINT16,							// 2 bytes
	UINT24,							// 3 bytes
	UINT32							// 4 bytes
};
uint8_t getSizeInBytes(VoxelFullnessFormat vff);

enum class ChunkFullnessFormat
{
	UINT8,							// 1 byte
	UINT16,							// 2 bytes
	UINT24,							// 3 bytes
	UINT32							// 4 bytes
};
uint8_t getSizeInBytes(ChunkFullnessFormat cff);

enum class ChunkOffsetFormat
{
	UINT8,							// 1 byte
	UINT16,							// 2 bytes
	UINT24,							// 3 bytes
	UINT32							// 4 bytes
};
uint8_t getSizeInBytes(ChunkOffsetFormat cof);

enum class VoxelShapeFormat
{
	NO_TYPE,						// 0 bytes
	UINT8,							// 1 byte
	UINT16,							// 2 bytes
	UINT32,							// 4 bytes
};
uint8_t getSizeInBytes(VoxelShapeFormat vsf);

enum class VoxelMaterialFormat
{
	NO_TYPE,						// 0 bytes
	UINT8,							// 1 byte
	UINT16,							// 2 bytes
	UINT32							// 4 bytes
};
uint8_t getSizeInBytes(VoxelMaterialFormat vmf);

enum class VoxelOrientationFormat
{
	NO_ORIENTATION,					// 0 bytes
	UINT8,							// 1 byte:   mirror 0 rx rx ry ry rz rz (only orthagonal rotations encoded in 2 bits)
	UINT32,							// 4 bytes:  mirror rx ry rz (byte per axis, integer number of pi/120 angle parts)
	FULL_16BYTES					// 16 bytes: mirror rx ry rz (full float value per axis)
};
uint8_t getSizeInBytes(VoxelOrientationFormat vof);

enum class ChunkSizeFormat
{
	NO_SIZE,						// 0 bytes
	UINT8,							// 1 byte
	UINT16,							// 2 bytes
	BASE_POWER_UINT8,				// 2 bytes
	UINT24,							// 3 bytes
	UINT32							// 4 bytes
};
uint8_t getSizeInBytes(ChunkSizeFormat csf);

enum class VoxelSizeFormat
{
	NO_SIZE,						// 0 bytes
	UINT8,							// 1 byte
	UINT32							// 4 bytes
};
uint8_t getSizeInBytes(VoxelSizeFormat vsf);

enum class VoxelColorFormat
{
	NO_COLOR,						// 0 bytes
	GRAYSCALE,						// 1 byte
	RGB256,							// 1 byte
	RGB256_WITH_ALPHA,				// 2 bytes: 1 byte color + 1 byte alpha
	RGB_THREE_BYTES,				// 3 bytes (r g b)
	RGBA_FOUR_BYTES		            // 4 bytes (r g b a)
};
uint8_t getSizeInBytes(VoxelColorFormat vcf);

enum class VoxelNeighbourInfoFormat
{
	NO_NEIGHBOUR_INFO,				// 0 bytes
	SIX_DIRS_ONE_BYTE,				// 1 byte: 6 bits are used (from high to low: 0 0 l r d u b f)
	TWENTY_SIX_DIRS_FOUR_BYTES		// 4 bytes: 26 bits are used (from high to low: 0 0 0 0 0 0 l ld lu lb lf ldb ldf lub luf r rd ru rb rf rdb rdf rub ruf d db df u ub uf b f)
};
uint8_t getSizeInBytes(VoxelNeighbourInfoFormat vnif);

enum class ParalsInfoFormat
{
	NO_PARALS,						// 0 bytes
	CUBIC_UINT8,					// 8 bytes
	NON_CUBIC_UINT8,				// 24 bytes
	CUBIC_FLOAT32,					// 24 bytes
	NON_CUBIC_FLOAT32				// 72 bytes
};
uint8_t getSizeInBytes(ParalsInfoFormat pif);

namespace utils
{
	class WordDelimitedBySpace : public std::string {};
	std::istream& operator>>(std::istream& is, WordDelimitedBySpace& output);

	std::vector<uint8_t> joinVectors(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b);

	uint8_t encodeRGB(uint8_t r, uint8_t g, uint8_t b);
	std::vector<uint8_t> decodeRGB(uint8_t rgb256);

	float calculateDist(const std::vector<float>& start, const std::vector<float>& end, float div = 1.0f);

	template <typename T>
	void appendBytes(std::vector<uint8_t>& bytes, T val)
	{
		const uint8_t* pVal = (const uint8_t*)&val;
		size_t sizeInBytes = sizeof(T);
		bytes.insert(bytes.end(), pVal, pVal + sizeInBytes);
	}

	template <typename T>
	void appendBytes(std::vector<uint8_t>& bytes, T val, size_t customSize)
	{
		const uint8_t* pVal = (const uint8_t*)&val;
		size_t sizeInBytes = customSize;
		bytes.insert(bytes.end(), pVal, pVal + sizeInBytes);
	}

	void appendBytes(std::vector<uint8_t>& bytes, std::vector<uint8_t> extraBytes);

	uint8_t packByte(const bool& a7, const bool& a6, const bool& a5, const bool& a4, const bool& a3, const bool& a2, const bool& a1, const bool& a0);
	void unpackByte(const uint8_t* byte, bool& a7, bool& a6, bool& a5, bool& a4, bool& a3, bool& a2, bool& a1, bool& a0);
}
