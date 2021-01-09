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

std::vector<uint8_t> VoxelMapFormat::formatVoxel(const Voxel& voxel, const std::vector<uint8_t>& neighs, const std::vector<uint8_t>& parals, bool alignToFourBytes) const
{
	std::vector<uint8_t> res;
	res.reserve(voxelFormat.getSizeInBytes(alignToFourBytes));

	switch (voxelFormat.fullness)
	{
	case VoxelFullnessFormat::UINT8:
		utils::appendBytes(res, uint8_t(voxel.isEmpty()));
		break;
	case VoxelFullnessFormat::UINT16:
		utils::appendBytes(res, uint16_t(voxel.isEmpty()), 2);
		break;
	case VoxelFullnessFormat::UINT24:
		utils::appendBytes(res, uint32_t(voxel.isEmpty()), 3);
		break;
	case VoxelFullnessFormat::UINT32:
		utils::appendBytes(res, uint32_t(voxel.isEmpty()), 4);
		break;
	default:
		break;
	}

	switch (voxelFormat.type)
	{
	case VoxelTypeFormat::UINT8:
		utils::appendBytes(res, uint8_t(voxel.type), 1);
		break;
	case VoxelTypeFormat::UINT16:
		utils::appendBytes(res, uint16_t(voxel.type), 2);
		break;
	default:
		break;
	}

	switch (voxelFormat.orientation)
	{
	case VoxelOrientationFormat::UINT8:
		utils::appendBytes(res, voxel.getOrientation());
		break;
	default:
		break;
	}

	uint8_t rgb8;
	switch (voxelFormat.color)
	{
	case VoxelColorFormat::GRAYSCALE:
		utils::appendBytes(res, voxel.color[0]);
		break;
	case VoxelColorFormat::RGB256:
		rgb8 = utils::encodeRGB(voxel.color[0], voxel.color[1], voxel.color[2]);
		utils::appendBytes(res, rgb8);
		break;
	case VoxelColorFormat::RGB256_WITH_ALPHA:
		rgb8 = utils::encodeRGB(voxel.color[0], voxel.color[1], voxel.color[2]);
		utils::appendBytes(res, rgb8);
		utils::appendBytes(res, voxel.color[3]);
		break;
	case VoxelColorFormat::RGB_THREE_BYTES:
		utils::appendBytes(res, voxel.color[0]);
		utils::appendBytes(res, voxel.color[1]);
		utils::appendBytes(res, voxel.color[2]);
		break;
	case VoxelColorFormat::RGBA_FOUR_BYTES:
		utils::appendBytes(res, voxel.color[0]);
		utils::appendBytes(res, voxel.color[1]);
		utils::appendBytes(res, voxel.color[2]);
		utils::appendBytes(res, voxel.color[3]);
		break;
	}

	utils::appendBytes(res, neighs);

	utils::appendBytes(res, parals);
}

Voxel VoxelMapFormat::unformatVoxel(const uint8_t* data, uint8_t power) const
{
	Voxel voxel(power);

	switch (voxelFormat.fullness)
	{
	case VoxelFullnessFormat::UINT8:
		data += 1;
		break;
	case VoxelFullnessFormat::UINT16:
		data += 2;
		break;
	case VoxelFullnessFormat::UINT24:
		data += 3;
		break;
	case VoxelFullnessFormat::UINT32:
		data += 4;
		break;
	}

	switch (voxelFormat.type)
	{
	case VoxelTypeFormat::UINT8:
		voxel.type = VoxelType(*data);
		data++;
		break;
	case VoxelTypeFormat::UINT16:
		voxel.type = VoxelType(*(uint16_t*)data);
		data += 2;
		break;
	default:
		break;
	}

	switch (voxelFormat.orientation)
	{
	case VoxelOrientationFormat::UINT8:
		voxel.setOrientation(*data++);
		break;
	default:
		break;
	}

	std::vector<uint8_t> rgb;
	switch (voxelFormat.color)
	{
	case VoxelColorFormat::GRAYSCALE:
		voxel.color[0] = voxel.color[1] = voxel.color[2] = data[0];
		voxel.color[3] = 255;
		break;
	case VoxelColorFormat::RGB256:
		rgb = utils::decodeRGB(data[0]);
		voxel.color = {rgb[0], rgb[1], rgb[2], 255};
		break;
	case VoxelColorFormat::RGB256_WITH_ALPHA:
		rgb = utils::decodeRGB(data[0]);
		voxel.color = { rgb[0], rgb[1], rgb[2], data[1] };
		break;
	case VoxelColorFormat::RGB_THREE_BYTES:
		voxel.color = { data[0], data[1], data[2], 255 };
		break;
	case VoxelColorFormat::RGBA_FOUR_BYTES:
		voxel.color = { data[0], data[1], data[2], data[3] };
		break;
	}
}

std::vector<uint8_t> VoxelMapFormat::formatChunkHeader(const VoxelChunk& chunk, uint32_t voxDataOffset, const std::vector<uint8_t>& parals, bool alignToFourBytes) const
{
	std::vector<uint8_t> res;
	res.reserve(chunkFormat.getSizeInBytes(alignToFourBytes));

	switch (chunkFormat.fullness)
	{
	case ChunkFullnessFormat::UINT8:
		utils::appendBytes(res, chunk.isEmpty());
		break;
	case ChunkFullnessFormat::UINT16:
		utils::appendBytes(res, uint16_t(chunk.isEmpty()));
		break;
	case ChunkFullnessFormat::UINT24:
		utils::appendBytes(res, uint32_t(chunk.isEmpty()), 3);
		break;
	case ChunkFullnessFormat::UINT32:
		utils::appendBytes(res, uint32_t(chunk.isEmpty()));
		break;
	default:
		break;
	}

	switch (chunkFormat.offset)
	{
	case ChunkOffsetFormat::UINT8:
		utils::appendBytes(res, uint8_t(voxDataOffset));
		break;
	case ChunkOffsetFormat::UINT16:
		utils::appendBytes(res, uint16_t(voxDataOffset));
		break;
	case ChunkOffsetFormat::UINT24:
		utils::appendBytes(res, voxDataOffset, 3);
		break;
	case ChunkOffsetFormat::UINT32:
		utils::appendBytes(res, voxDataOffset);
		break;
	default:
		break;
	}

	switch (chunkFormat.size)
	{
	case ChunkSizeFormat::UINT8:
		utils::appendBytes(res, uint8_t(chunk.side));
		break;
	case ChunkSizeFormat::UINT16:
		utils::appendBytes(res, uint16_t(chunk.side));
		break;
	case ChunkSizeFormat::BASE_POWER_UINT8:
		utils::appendBytes(res, chunk.pyramid.base);
		utils::appendBytes(res, chunk.pyramid.power);
		break;
	case ChunkSizeFormat::UINT24:
		utils::appendBytes(res, uint32_t(chunk.side), 3);
		break;
	case ChunkSizeFormat::UINT32:
		utils::appendBytes(res, uint32_t(chunk.side));
		break;
	default:
		break;
	}

	utils::appendBytes(res, parals);

	return res;
}

std::vector<uint8_t> VoxelMapFormat::formatChunk(const VoxelChunk& chunk, bool alignToFourBytes) const
{
	std::vector<uint8_t> res;
	uint32_t size = voxelFormat.getSizeInBytes(alignToFourBytes) * pow(chunk.side, 3);
	res.reserve(size);

	for (uint32_t x = 0; x < chunk.side; ++x)
		for (uint32_t y = 0; y < chunk.side; ++y)
			for (uint32_t z = 0; z < chunk.side; ++z)
			{
				auto voxPos = { x * chunk.minOffset, y * chunk.minOffset, z * chunk.minOffset };
				auto vox = chunk.getVoxel(voxPos);
				auto neighs = chunk.getNeighbours(vox, voxPos, voxelFormat.neighbour);
				auto parals = chunk.getVoxParals(vox, voxPos, voxelFormat.parals);
				utils::appendBytes(res, formatVoxel(vox, neighs, parals, alignToFourBytes));
			}

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
		header+=2;
		break;
	case ChunkFullnessFormat::UINT24:
		header += 3;
		break;
	case ChunkFullnessFormat::UINT32:
		header += 4;
		break;
	}

	uint32_t offset = 0;
	switch (chunkFormat.offset)
	{
	case ChunkOffsetFormat::UINT8:
		offset = *header++;
		break;
	case ChunkOffsetFormat::UINT16:
		offset = *((uint16_t*)header);
		header += 2;
		break;
	case ChunkOffsetFormat::UINT24:
		offset = (*((uint32_t*)header) >> 1) & 0x00ffffff;
		header += 3;
		break;
	case ChunkOffsetFormat::UINT32:
		offset = *((uint32_t*)header);
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
				voxels.push_back(unformatVoxel(data + offset + voxSz * (z * sideSq + y * side + x), power));

	return VoxelChunk(voxels, chunkFormat);
}
