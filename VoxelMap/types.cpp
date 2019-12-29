#include "types.h"

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
