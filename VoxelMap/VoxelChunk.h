#pragma once

#include <vector>

class VoxelChunk
{
public:
	VoxelChunk();

private:
	unsigned char _nPyramidPower;
	unsigned char _nTypeBytes;
	std::vector<std::vector<unsigned char>> _vNLeavesOnLayers;
	std::vector<std::vector<unsigned char>> _vLayerSizes;
	std::vector<std::vector<unsigned char>> _vLayers;
	std::vector<std::vector<unsigned char>> _vTypes;
};