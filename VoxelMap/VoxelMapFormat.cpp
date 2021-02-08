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

VoxelChunk VoxelMapFormat::unformatChunk(VoxelTypeStorer& vts, const uint8_t* header, const uint8_t* data, bool alignToFourBytes) const
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
				voxels.push_back(voxelFormat.unformatVoxel(vts, data + off + voxSz * (z * sideSq + y * side + x)));

	return VoxelChunk(voxels, chunkFormat, voxelFormat);
}

VoxelChunkState VoxelMapFormat::getChunkState(const uint8_t* data, bool alignToFourBytes) const
{
	VoxelChunkState vch;

	const uint8_t* dataStart = data;

	switch (chunkFormat.fullness)
	{
	case ChunkFullnessFormat::UINT8:
		vch.fullness = data[0];
		data++;
		break;
	case ChunkFullnessFormat::UINT16:
		vch.fullness = *(uint16_t*)data;
		data += 2;
		break;
	case ChunkFullnessFormat::UINT24:
		vch.fullness = (*((uint32_t*)data) >> 1) & 0x00ffffff;
		data += 3;
		break;
	case ChunkFullnessFormat::UINT32:
		vch.fullness = *(uint32_t*)data;
		data += 4;
		break;
	}

	switch (chunkFormat.offset)
	{
	case ChunkOffsetFormat::UINT8:
		vch.voxOffset = *data++;
		break;
	case ChunkOffsetFormat::UINT16:
		vch.voxOffset = *((uint16_t*)data);
		data += 2;
		break;
	case ChunkOffsetFormat::UINT24:
		vch.voxOffset = (*((uint32_t*)data) >> 1) & 0x00ffffff;
		data += 3;
		break;
	case ChunkOffsetFormat::UINT32:
		vch.voxOffset = *((uint32_t*)data);
		data += 4;
		break;
	default:
		break;
	}

	switch (chunkFormat.size)
	{
	case ChunkSizeFormat::UINT8:
		vch.side = *data++;
		break;
	case ChunkSizeFormat::UINT16:
		vch.side = *((uint16_t*)data);
		data += 2;
		break;
	case ChunkSizeFormat::BASE_POWER_UINT8:
		vch.base = data[0];
		vch.power = data[1];
		vch.side = pow(data[0], data[1]);
		data += 2;
		break;
	case ChunkSizeFormat::UINT24:
		vch.side = (*((uint32_t*)data) >> 1) & 0x00ffffff;
		data += 3;
		break;
	case ChunkSizeFormat::UINT32:
		vch.side = *((uint32_t*)data);
		data += 4;
		break;
	default:
		break;
	}

	vch.parals = unformatParals(chunkFormat.parals, data);
}

std::vector<glm::uvec3> VoxelMapFormat::unformatParals(ParalsInfoFormat format, const uint8_t* data) const
{
	std::vector<glm::uvec3> res(8);

	switch (format)
	{
	case ParalsInfoFormat::CUBIC_UINT8:
		for (int i = 0; i < 8; ++i)
			res[i][0] = res[i][1] = res[i][2] = data[i];
		break;
	case ParalsInfoFormat::NON_CUBIC_UINT8:
		for (int i = 0; i < 8; ++i)
		{
			res[i][0] = data[i * 3];
			res[i][1] = data[i * 3 + 1];
			res[i][2] = data[i * 3 + 2];
		}
		break;
	case ParalsInfoFormat::CUBIC_FLOAT32:
		for (int i = 0; i < 8; ++i)
			res[i][0] = res[i][1] = res[i][2] = ((float*)data)[i];
		break;
	case ParalsInfoFormat::NON_CUBIC_FLOAT32:
		for (int i = 0; i < 8; ++i)
		{
			res[i][0] = ((float*)data)[i * 3];
			res[i][1] = ((float*)data)[i * 3 + 1];
			res[i][2] = ((float*)data)[i * 3 + 2];
		}
		break;
	}

	return res;
}

VoxelState VoxelMapFormat::getVoxelState(const uint8_t* data) const
{
	VoxelState state;

	switch (voxelFormat.fullness)
	{
	case VoxelFullnessFormat::UINT8:
		state.full = data[0];
		break;
	case VoxelFullnessFormat::UINT16:
		state.full = ((uint16_t*)data)[0];
		break;
	case VoxelFullnessFormat::UINT24:
		state.full = ((uint32_t*)data)[0];
		break;
	case VoxelFullnessFormat::UINT32:
		state.full = ((uint32_t*)data)[0];
		break;
	}

	data += ::getSizeInBytes(voxelFormat.fullness);

	switch (voxelFormat.size)
	{
	case VoxelSizeFormat::UINT8:
		state.size = data[0];
		break;
	}

	data += ::getSizeInBytes(voxelFormat.size) + ::getSizeInBytes(voxelFormat.shape) + ::getSizeInBytes(voxelFormat.material) + ::getSizeInBytes(voxelFormat.orientation) + ::getSizeInBytes(voxelFormat.color);

	state.neighs = voxelFormat.unformatNeighs(data);
	data += ::getSizeInBytes(voxelFormat.neighbour);

	state.parals = unformatParals(voxelFormat.parals, data);

	return state;
}
