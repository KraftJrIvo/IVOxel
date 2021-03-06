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