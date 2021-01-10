#include "VoxelMapFormat.h"

VoxelMapFormat::VoxelMapFormat(VoxelChunkFormat chunkFormat_, VoxelFormat voxelFormat_) :
	chunkFormat(chunkFormat_),
	voxelFormat(voxelFormat_)
{ }

uint32_t VoxelMapFormat::getSizeInBytes(uint32_t nVoxels, bool alignToFourBytes) const
{
	uint32_t res = chunkFormat.getSizeInBytes() + nVoxels * voxelFormat.getSizeInBytes();

	if (alignToFourBytes)
		return ceil(float(res) / 4.0f);

	return res;
}

VoxelChunk VoxelMapFormat::unformatChunk(const uint8_t* header, const uint8_t* data, bool alignToFourBytes) const
{
	std::vector<Voxel> voxels;

	switch (chunkFormat.fullness)
	{
	case ChunkFullnessFormat::UINT8:
		header++;
		break;
	case ChunkFullnessFormat::UINT16:
		header += 2;
		break;
	case ChunkFullnessFormat::UINT24:
		header += 3;
		break;
	case ChunkFullnessFormat::UINT32:
		header += 4;
		break;
	}

	uint32_t off = 0;
	switch (chunkFormat.offset)
	{
	case ChunkOffsetFormat::UINT8:
		off = *header++;
		break;
	case ChunkOffsetFormat::UINT16:
		off = *((uint16_t*)header);
		header += 2;
		break;
	case ChunkOffsetFormat::UINT24:
		off = (*((uint32_t*)header) >> 1) & 0x00ffffff;
		header += 3;
		break;
	case ChunkOffsetFormat::UINT32:
		off = *((uint32_t*)header);
		header += 4;
		break;
	default:
		break;
	}

	uint32_t side = 0;
	uint8_t power = 0;
	switch (chunkFormat.size)
	{
	case ChunkSizeFormat::UINT8:
		side = *header++;
		break;
	case ChunkSizeFormat::UINT16:
		side = *((uint16_t*)header);
		break;
	case ChunkSizeFormat::BASE_POWER_UINT8:
		side = pow(header[0], header[1]);
		power = header[1];
		break;
	case ChunkSizeFormat::UINT24:
		side = (*((uint32_t*)header) >> 1) & 0x00ffffff;
		break;
	case ChunkSizeFormat::UINT32:
		side = *((uint32_t*)header);
		break;
	default:
		break;
	}

	auto voxSz = voxelFormat.getSizeInBytes(alignToFourBytes);
	auto sideSq = side * side;
	for (uint32_t x = 0; x < side; ++x)
		for (uint32_t y = 0; y < side; ++y)
			for (uint32_t z = 0; z < side; ++z)
				voxels.push_back(voxelFormat.unformatVoxel(data + off + voxSz * (z * sideSq + y * side + x), power));

	return VoxelChunk(voxels, chunkFormat, voxelFormat);
}
