#include "VoxelChunkPyramid.h"

#include <map>
#include <algorithm>
#include <functional>

VoxelChunkPyramid::VoxelChunkPyramid() :
	base(0),
	power(0),
	side(0),
	nVoxBytes(0),
	alignToFourBytes(false)
{ }

VoxelChunkPyramid::VoxelChunkPyramid(const VoxelFormat& format_, uint32_t side, bool alignToFourBytes_) :
	format(format_),
	alignToFourBytes(alignToFourBytes_)
{
	base = (side % 3 == 0) ? 3 : 2;
	power = uint32_t(std::ceil(std::log2(side) / std::log2(base)));
	side = std::pow(base, power);
	nVoxBytes = format.getSizeInBytes(alignToFourBytes);
}

void VoxelChunkPyramid::build(const std::vector<Voxel>& voxels)
{
	std::vector<std::vector<uint32_t>> leavesOnLayers;
	std::vector<std::vector<int32_t>> offsets;

	uint8_t nLayers = power + 1;

	leavesOnLayers.resize(nLayers);
	offsets.resize(nLayers);

	std::vector<uint32_t> size = {side, side, side};

	uint32_t sideSqr = size[0] * size[1];
	uint32_t sideCub = sideSqr * size[2];

	std::vector<uint16_t> bytesForLayers;
	for (uint32_t i = 0; i < nLayers; ++i)
	{
		uint32_t vol = std::pow(std::pow(base, i), 3);
		uint16_t bytesForThis = uint16_t(std::ceil(std::log2(vol) / 8.0f));
		bytesForThis = (bytesForThis == 1 || bytesForThis == 2) ? bytesForThis : (bytesForThis == 0 ? 1 : 4);
		bytesForLayers.push_back(bytesForThis);
	}

	auto checkIfHomogenous = [&](uint32_t x, uint32_t y, uint32_t z, uint32_t step)
	{
		if (z >= size[2] || y >= size[1] || x > size[0])
			return true;

		uint32_t offset1 = (sideSqr * z + side * y + x);
		for (uint32_t zz = 0; zz < step; ++zz)
		{
			if (z + zz >= size[2])
				return false;
			for (uint32_t yy = 0; yy < step; ++yy)
			{
				if (y + yy >= size[1])
					return false;
				for (uint32_t xx = 0; xx < step; ++xx)
				{
					uint32_t offset2 = (sideSqr * (z + zz) + side * (y + yy) + (x + xx));
					if (voxels[offset1].type != voxels[offset2].type)
						return false;
				}
			}
		}

		return true;
	};

	uint32_t nSteps;
	uint32_t nVoxels = 0;
	uint32_t nComplex = 0;
	uint32_t nOffsetBytes = 0;

	Voxel emptyVox;

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
		data = utils::joinVectors(data, offsetBytes);
		nOffsetBytes += bytesForThis;
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

						bool oob = (curZ >= size[2] || curY >= size[1] || curX > size[0]);
						uint32_t offset = (sideSqr * curZ + side * curY + curX);

						leavesOnLayers[pwr].push_back(voxID);
						int32_t curOffset = -int32_t(leavesOnLayers[pwr].size());
						offsets[pwr].push_back(curOffset);

						std::vector<uint8_t> allData = oob ? format.formatVoxel(emptyVox, {}, {}, alignToFourBytes) : format.formatVoxel(voxels[offset], {}, {}, alignToFourBytes);

						tempData[pwr] = utils::joinVectors(tempData[pwr], allData);
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

	data.resize(sizeof(uint32_t) + 3 * sizeof(uint8_t) + nLayers * sizeof(uint32_t));
	auto dataPtr = data.data() + sizeof(uint32_t);
	std::memcpy(dataPtr, &base, sizeof(uint8_t));
	std::memcpy(dataPtr + 1, &power, sizeof(uint8_t));
	uint8_t sizeInBytes = format.getSizeInBytes();
	std::memcpy(dataPtr + 2, &sizeInBytes, sizeof(uint8_t));
	dataPtr += 3 * sizeof(uint8_t);
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
	std::memcpy(data.data(), &nOffsetBytes, sizeof(uint32_t));
	for (int i = 0; i < nLayers; ++i)
		data = utils::joinVectors(data, tempData[i]);
}

uint8_t VoxelChunkPyramid::getPyramLayerBytesCount(uint8_t base, uint8_t power)
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

	uint32_t vol = std::pow(std::pow(base, power), 3);
	uint16_t bytesForThis = uint16_t(std::ceil(std::log2(vol) / 8.0f));
	bytesForThis = (bytesForThis == 1 || bytesForThis == 2) ? bytesForThis : (bytesForThis == 0 ? 1 : 4);
	auto result = bytesForPyramLayers[base][power] = bytesForThis;

	return result;
}
