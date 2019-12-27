#pragma once

#include <string>
#include <istream>

#define DIMENSIONS 3
#define X 0
#define Y 1
#define Z 2
#define RGB 3
#define RGBA 4
#define R 0
#define G 1
#define B 2
#define A 3

enum VoxelMapType
{
	ONE_BYTE_RGB256,
	TWO_BYTES_RGB256_AND_ALPHA,
	TWO_BYTES_RGB256_AND_TYPE,
	THREE_BYTES_RGB256_ALPHA_AND_TYPE,
	THREE_BYTES_RGB,
	FOUR_BYTES_RGBA,
	FOUR_BYTES_RGB_AND_TYPE,
	FIVE_BYTES_RGBA_AND_TYPE
};

class WordDelimitedBySpace : public std::string {};
std::istream& operator>>(std::istream& is, WordDelimitedBySpace& output);
