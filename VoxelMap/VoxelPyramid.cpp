#include "VoxelPyramid.h"

#include <algorithm>
#include <functional>

VoxelPyramid::VoxelPyramid() :
	base(0),
	power(0),
	side(0),
	nVoxBytes(0)
{	
}

VoxelPyramid::VoxelPyramid(const std::vector<uint32_t>& size, const VoxelMapType& _type) :
	type(_type)
{
	uint32_t maxDim = std::max(size[0], std::max(size[1], size[2]));
	base = (maxDim % 3 == 0) ? 3 : 2;
	power = uint32_t(std::ceil(std::log2(maxDim) / std::log2(base)));

	side = std::pow(base, power);

	nVoxBytes = type.sizeInBytes;
}

void VoxelPyramid::build(const std::vector<uint32_t>& size, const std::vector<uint8_t>& types, const std::vector<uint8_t>& colors, const std::vector<uint8_t>& neighbours)
{
	std::vector<std::vector<uint32_t>> leavesOnLayers;
	std::vector<uint32_t> layerSizes;
	std::vector<std::vector<uint8_t>> offsets;

	uint8_t nLayers = power + 1;

	leavesOnLayers.resize(nLayers);
	layerSizes.resize(nLayers);
	offsets.resize(nLayers);
	data.resize(nLayers);

	uint32_t sideSqr = side * side;
	uint32_t sideCub = sideSqr * side;
	uint32_t rowLen = type.sizeInBytesType * side;
	uint32_t layerLen = type.sizeInBytesType * sideSqr;

	std::vector<uint8_t> emptyElem = type.formatType(-1);
	std::vector<uint8_t> paddedTypes(type.sizeInBytesType * sideCub);

	// padding in case original chunk size is not cubic
	for (uint32_t z = 0; z < side; ++z)
	{
		for (uint32_t y = 0; y < side; ++y)
			for (uint32_t x = 0; x < side; ++x)
				if (x < size[X] && y < size[Y] && z < size[Z])
					for (uint8_t b = 0; b < type.sizeInBytesType; ++b)
						paddedTypes[type.sizeInBytesType * (sideSqr * z + side * y + x) + b] =
						types[type.sizeInBytesType * (size[Y] * size[X] * z + size[X] * y + x) + b];
				else
					for (uint8_t b = 0; b < type.sizeInBytesType; ++b)
						paddedTypes[type.sizeInBytesType * (sideSqr * z + side * y + x) + b] = emptyElem[b];
	}

	auto checkIfHomogenous = [&](uint32_t x, uint32_t y, uint32_t z, uint32_t step)
	{
		uint32_t offset1 = type.sizeInBytesType * (sideSqr * z + side * y + x);
		for (uint32_t zz = 0; zz < step; ++zz)
			for (uint32_t yy = 0; yy < step; ++yy)
				for (uint32_t xx = 0; xx < step; ++xx)
					for (uint8_t bb = 0; bb < type.sizeInBytesType; ++bb)
					{
						uint32_t offset2 = type.sizeInBytesType * (sideSqr * (z + zz) + side * (y + yy) + (x + xx)) + bb;
						if (paddedTypes[offset1] != paddedTypes[offset2])
							return false;
					}

		return true;
	};

	uint32_t nSteps;

	std::function<void(uint32_t, uint32_t, uint32_t, uint32_t)> recursivePyramid = [&](uint32_t pwr, uint32_t sx, uint32_t sy, uint32_t sz)
	{
		uint32_t nSteps = std::pow(base, pwr);
		uint32_t step = side / nSteps;

		uint32_t to = (pwr == 0) ? 1 : base;

		uint32_t nLeaves = 0;

		for (int z = 0; z < to; ++z)
		{
			for (int y = 0; y < to; ++y)
				for (int x = 0; x < to; ++x)
				{
					bool homogenous = checkIfHomogenous(sx + x * step, sy + y * step, sz + z * step, step);

					if (homogenous)
					{
						nLeaves++;
						leavesOnLayers[pwr].push_back(base * base * z + base * y + x);
					}
					else
						recursivePyramid(pwr + 1, sx + x * step, sy + y * step, sz + z * step);
				}
		}

		layerSizes[pwr] = nSteps * nSteps * nSteps - nLeaves;
	};

	recursivePyramid(0, 0, 0, 0);
}
