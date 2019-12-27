#include "types.h"

std::istream& operator>>(std::istream& is, WordDelimitedBySpace& output)
{
	std::getline(is, output, ' ');
	return is;
}