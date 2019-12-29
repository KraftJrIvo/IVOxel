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
	std::vector<std::vector<int32_t>> offsets;

	uint8_t nLayers = power + 1;

	leavesOnLayers.resize(nLayers);
	offsets.resize(nLayers);

	uint32_t sideSqr = size[X] * size[Y];
	uint32_t sideCub = sideSqr * size[Z];
	uint32_t rowLen = type.sizeInBytesType * side;
	uint32_t layerLen = type.sizeInBytesType * sideSqr;

	std::vector<uint8_t> emptyType = type.formatType(-1);
	std::vector<uint8_t> emptyColor = type.formatColor(0,0,0,0);
	std::vector<uint8_t> emptyNeigh = type.formatNeighbours({}, -1);

	std::vector<uint16_t> bytesForLayers;
	for (uint32_t i = 0; i < nLayers; ++i)
	{
		uint32_t vol = std::pow(std::pow(base, i), DIMENSIONS);
		uint16_t bytesForThis = uint16_t(std::ceil(std::log2(vol) / std::log2(base) / 8));
		bytesForThis = (bytesForThis == 1 || bytesForThis == 2) ? bytesForThis : (bytesForThis == 0 ? 1 : 4);
		bytesForLayers.push_back(bytesForThis);
	}

	auto checkIfHomogenous = [&](uint32_t x, uint32_t y, uint32_t z, uint32_t step)
	{
		if (z >= size[Z] || y >= size[Y] || x > size[X])
			return true;

		uint32_t offset1 = type.sizeInBytesType * (sideSqr * z + side * y + x);
		for (uint32_t zz = 0; zz < step; ++zz)
		{
			if (z + zz >= size[Z])
				return false;
			for (uint32_t yy = 0; yy < step; ++yy)
			{
				if (y + yy >= size[Y])
					return false;
				for (uint32_t xx = 0; xx < step; ++xx)
				{
					for (uint8_t bb = 0; bb < type.sizeInBytesType; ++bb)
					{
						uint32_t offset2 = type.sizeInBytesType * (sideSqr * (z + zz) + side * (y + yy) + (x + xx)) + bb;
						if (types[offset1] != types[offset2])
							return false;
					}
				}
			}
		}

		return true;
	};

	uint32_t nSteps;
	uint32_t nVoxels = 0;
	uint32_t nComplex = 0;

	std::vector<std::vector<uint8_t>> tempData(nLayers);

	auto pushOffset = [&](uint8_t bytesForThis, int32_t curOffset, std::vector<uint8_t>& offsetBytes)
	{
		if (bytesForThis == 1)
		{
			int8_t off = curOffset;
			std::memcpy(offsetBytes.data(), &off, bytesForThis);
		}
		else if (bytesForThis == 2)
		{
			int16_t off = curOffset;
			std::memcpy(offsetBytes.data(), &off, bytesForThis);
		}
		else
		{
			int32_t off = curOffset;
			std::memcpy(offsetBytes.data(), &off, sizeof(int32_t));
		}
		data = join(data, offsetBytes);
	};

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
					uint32_t curX = sx + x * step;
					uint32_t curY = sy + y * step;
					uint32_t curZ = sz + z * step;
					bool homogenous = checkIfHomogenous(sx + x * step, sy + y * step, sz + z * step, step);

					uint32_t voxID = base * base * z + base * y + x;

					if (homogenous)
					{
						nLeaves++;
						nVoxels++;

						bool oob = (curZ >= size[Z] || curY >= size[Y] || curX > size[X]);
						uint32_t offset = (sideSqr * curZ + side * curY + curX);

						leavesOnLayers[pwr].push_back(voxID);
						int32_t curOffset = -int32_t(leavesOnLayers[pwr].size());
						offsets[pwr].push_back(curOffset);

						std::vector<uint8_t> typeData = oob ? emptyType : std::vector<uint8_t>(types.begin() + type.sizeInBytesType * offset, types.begin() + type.sizeInBytesType * (offset + 1));
						std::vector<uint8_t> colorData = oob ? emptyColor : std::vector<uint8_t>(colors.begin() + type.sizeInBytesColor * offset, colors.begin() + type.sizeInBytesColor * (offset + 1));
						std::vector<uint8_t> neighData = oob ? emptyNeigh : std::vector<uint8_t>(neighbours.begin() + type.sizeInBytesNeighbourInfo * offset, neighbours.begin() + type.sizeInBytesNeighbourInfo * (offset + 1));
						std::vector<uint8_t> allData = join(typeData, join(colorData, neighData));

						tempData[pwr] = join(tempData[pwr], allData);
					}
					else
					{
						nComplex++;
						int32_t curOffset = voxID - nLeaves;
						offsets[pwr].push_back(curOffset);

						recursivePyramid(pwr + 1, sx + x * step, sy + y * step, sz + z * step);
					}
				}
		}
	};

	recursivePyramid(0, 0, 0, 0);

	data.resize(sizeof(uint8_t) * 3 + offsets.size() * sizeof(uint32_t));
	auto dataPtr = data.data();
	std::memcpy(dataPtr, &nLayers, sizeof(uint8_t));
	dataPtr += sizeof(uint8_t);
	std::memcpy(dataPtr, &type.sizeInBytes, sizeof(uint8_t));
	dataPtr += sizeof(uint8_t);
	for (int i = 0; i < nLayers; ++i)
	{
		uint32_t sz = leavesOnLayers[i].size();
		std::memcpy(dataPtr, &sz, sizeof(uint32_t));
		dataPtr += sizeof(uint32_t);
	}
	for (int i = 0; i < nLayers; ++i)
	{
		const size_t& sz = offsets[i].size();
		std::vector<uint8_t> offsetBytes(bytesForLayers[i]);
		for (int j = 0; j < sz; ++j)
			pushOffset(bytesForLayers[i], offsets[i][j], offsetBytes);
	}
	for (int i = 0; i < nLayers; ++i)
		data = join(data, tempData[i]);
}
