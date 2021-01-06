#pragma once

#include <vector>
#include <string>
#include <istream>

// 3d coords
#define DIMENSIONS 3
#define X 0
#define Y 1
#define Z 2

// color
#define RGB 3
#define RGBA 4
#define R 0
#define G 1
#define B 2
#define A 3

enum class VoxelFullnessFormat 
{
	UINT8,							// 1 byte
	UINT16,							// 2 bytes
	UINT24,							// 3 bytes
	UINT32							// 4 bytes
};

enum class ChunkFullnessFormat
{
	UINT8,							// 1 byte
	UINT16,							// 2 bytes
	UINT24,							// 3 bytes
	UINT32							// 4 bytes
};

enum class ChunkOffsetFormat
{
	UINT8,							// 1 byte
	UINT16,							// 2 bytes
	UINT24,							// 3 bytes
	UINT32							// 4 bytes
};

enum class VoxelTypeFormat
{
	NO_TYPE,						// 0 bytes
	UINT8,							// 1 byte
	UINT16							// 2 bytes
};

enum class VoxelOrientationFormat
{
	NO_ORIENTATION,					// 0 bytes
	UINT8							// 1 byte (see VoxelOrientation type)
};

enum class ChunkSizeFormat
{
	NO_SIZE,						// 0 bytes
	UINT8,							// 1 byte
	UINT16,							// 2 bytes
	BASE_POWER_UINT8,				// 2 bytes
	UINT24,							// 3 bytes
	UINT32							// 4 bytes
};

enum class VoxelColorFormat
{
	NO_COLOR,						// 0 bytes
	GRAYSCALE,						// 1 byte
	RGB256,							// 1 byte
	RGB256_WITH_ALPHA,				// 2 bytes: 1 byte color + 1 byte alpha
	RGB_THREE_BYTES,				// 3 bytes (r g b)
	RGBA_FOUR_BYTES		            // 4 bytes (r g b a)
};

enum class VoxelNeighbourInfoFormat
{
	NO_NEIGHBOUR_INFO,				// 0 bytes
	SIX_DIRS_ONE_BYTE,				// 1 byte: 6 bits are used (from high to low: 0 0 l r d u b f)
	TWENTY_SIX_DIRS_FOUR_BYTES		// 4 bytes: 26 bits are used (from high to low: 0 0 0 0 0 0 l ld lu lb lf ldb ldf lub luf r rd ru rb rf rdb rdf rub ruf d db df u ub uf b f)
};

enum class ParalsInfoFormat
{
	NO_PARALS,						// 0 bytes
	CUBIC_UINT8,					// 8 bytes
	NON_CUBIC_UINT8,				// 24 bytes
	CUBIC_FLOAT32,					// 24 bytes
	NON_CUBIC_FLOAT32				// 72 bytes
};

struct VoxelFormat
{
	VoxelFullnessFormat      fullness;
	VoxelTypeFormat          type;
	VoxelOrientationFormat   orientation;
	VoxelColorFormat         color;
	VoxelNeighbourInfoFormat neighbour;
	ParalsInfoFormat         parals;

	uint32_t getSizeInBytes(bool alignToFourBytes = true) const;
};

struct VoxelChunkFormat
{
	ChunkFullnessFormat fullness;
	ChunkOffsetFormat   offset;
	ChunkSizeFormat     size;
	ParalsInfoFormat    parals;

	uint32_t getSizeInBytes(bool alignToFourBytes = true) const;
};

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
		const uint8_t* pVal = (const uint8_t*)val;
		size_t sizeInBytes = customSize;
		bytes.insert(bytes.end(), pVal, pVal + sizeInBytes);
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
}
