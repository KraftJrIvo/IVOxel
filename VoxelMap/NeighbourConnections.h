#pragma once

#include <vector>

struct NeighbourConnections
{
	NeighbourConnections();

	bool sameType;
	std::vector<std::vector<int8_t>> vals;
};
