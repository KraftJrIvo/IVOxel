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

// orientaion
#define NO_DIR		0
#define LEFT		0
#define RIGHT		1
#define DOWN		2
#define UP			3
#define BACKWARD	4
#define FORWARD		5

// sides
#define BOTTOM		2
#define TOP			3
#define FRONT		4
#define BACK		5

enum class VoxelColorFormat
{
	NO_COLOR,					// 0 bytes
	GRAYSCALE,					// 1 byte
	RGB256,						// 1 byte
	RGB256_WITH_ALPHA,			// 2 bytes: 1 byte color + 1 byte alpha
	THREE_BYTES_RGB,			// 3 bytes (r g b)
	THREE_BYTES_RGB_WITH_ALPHA	// 4 bytes (r g b a)
};

enum class VoxelTypeFormat
{
	NO_TYPE,					// 0 bytes
	UINT8,						// 1 byte
	UINT8_WITH_ORIENTATION,		// 2 bytes: 1 byte type + 1 byte orientation (0 flipZ flipY flipX - high half, orientation - low half)
	UINT16,						// 2 bytes
	UINT16_WITH_ORIENTATION,	// 3 bytes: 2 bytes type + 1 byte orientation
};

enum class VoxelNeighbourInfoFormat
{
	NO_NEIGHBOUR_INFO,			// 0 bytes
	BINARY_6_DIR_INFO,			// 1 byte: 6 bits are used (from high to low: 0 0 l r d u b f)
	BINARY_26_DIR_INFO			// 4 bytes: 26 bits are used
};

class WordDelimitedBySpace : public std::string {};
std::istream& operator>>(std::istream& is, WordDelimitedBySpace& output);

std::vector<uint8_t> join(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b);
