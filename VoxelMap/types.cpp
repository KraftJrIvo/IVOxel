#include "types.h"
#include <map>

std::istream& operator>>(std::istream& is, WordDelimitedBySpace& output)
{
	std::getline(is, output, ' ');
	return is;
}

std::vector<uint8_t> join(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b)
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

uint8_t getPyramLayerBytesCount(uint8_t base, uint8_t power)
{
	static std::map<uint8_t, std::map<uint8_t, uint8_t>> bytesForPyramLayers;

	auto it1 = bytesForPyramLayers.find(base);
	bool found = (it1 != bytesForPyramLayers.end());

	if (found)
	{
		auto it2 = it1->second.find(power);
		found = (it2 != bytesForPyramLayers[base].end());
		
		if (found)
			return it2->second;
	}

	uint32_t vol = std::pow(std::pow(base, power), DIMENSIONS);
	uint16_t bytesForThis = uint16_t(std::ceil(std::log2(vol) / 8.0f));
	bytesForThis = (bytesForThis == 1 || bytesForThis == 2) ? bytesForThis : (bytesForThis == 0 ? 1 : 4);
	auto result = bytesForPyramLayers[base][power] = bytesForThis;

	return result;
}
