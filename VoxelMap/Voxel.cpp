#include "Voxel.h"

Voxel::Voxel(uint8_t power_, VoxelType type_, VoxelOrientation orientation_, const std::vector<uint8_t>& rgba) :
	power(power_),
	type(type_),
	orientation(orientation_),
	color(rgba)
{ }

std::vector<uint32_t> Voxel::getData()
{
	std::vector<uint32_t> res(24, 0);

	auto ptr = res.data();
	bool empty = isEmpty();
	std::memcpy(ptr, &type, sizeof(uint32_t)); ptr += sizeof(uint32_t);
	std::memcpy(ptr, &type, sizeof(uint32_t)); ptr += sizeof(uint32_t);

	return res;
}

bool Voxel::isEmpty() const
{
	return type == VoxelType::AIR;
}

uint8_t Voxel::getOrientation() const
{
	switch (orientation)
	{
	case VoxelOrientation::NONE:
		return -1;
	case VoxelOrientation::DEFAULT:
		return 0;
	case VoxelOrientation::LEFT:
		return 1;
	case VoxelOrientation::RIGHT:
		return 2;
	case VoxelOrientation::DOWN:
		return 3;
	case VoxelOrientation::UP:
		return 4;
	case VoxelOrientation::BACK:
		return 5;
	case VoxelOrientation::FRONT:
		return 6;
	default:
		return -1;
	}
}

void Voxel::setOrientation(uint8_t orient)
{
	switch (orient)
	{
	case 255:
		orientation = VoxelOrientation::NONE;
		break;
	case 0:
		orientation = VoxelOrientation::DEFAULT;
		break;
	case 1:
		orientation = VoxelOrientation::LEFT;
		break;
	case 2:
		orientation = VoxelOrientation::RIGHT;
		break;
	case 3:
		orientation = VoxelOrientation::DOWN;
		break;
	case 4:
		orientation = VoxelOrientation::UP;
		break;
	case 5:
		orientation = VoxelOrientation::BACK;
		break;
	case 6:
		orientation = VoxelOrientation::FRONT;
		break;
	}
}

uint32_t VoxelFormat::getSizeInBytes(bool alignToFourBytes) const
{
	uint32_t res = 0;

	switch (fullness)
	{
	case VoxelFullnessFormat::UINT8:
		res++;
		break;
	case VoxelFullnessFormat::UINT16:
		res += 2;
		break;
	case VoxelFullnessFormat::UINT24:
		res += 3;
		break;
	case VoxelFullnessFormat::UINT32:
		res += 4;
		break;
	default:
		break;
	}

	switch (type)
	{
	case VoxelTypeFormat::UINT8:
		res++;
		break;
	case VoxelTypeFormat::UINT16:
		res += 2;
		break;
	default:
		break;
	}

	switch (orientation)
	{
	case VoxelOrientationFormat::UINT8:
		res += 1;
		break;
	default:
		break;
	}

	switch (color)
	{
	case VoxelColorFormat::GRAYSCALE:
		res++;
		break;
	case VoxelColorFormat::RGB256:
		res++;
		break;
	case VoxelColorFormat::RGB256_WITH_ALPHA:
		res += 2;
		break;
	case VoxelColorFormat::RGB_THREE_BYTES:
		res += 3;
		break;
	case VoxelColorFormat::RGBA_FOUR_BYTES:
		res += 4;
		break;
	default:
		break;
	}

	switch (neighbour)
	{
	case VoxelNeighbourInfoFormat::SIX_DIRS_ONE_BYTE:
		res++;
		break;
	case VoxelNeighbourInfoFormat::TWENTY_SIX_DIRS_FOUR_BYTES:
		res += 4;
		break;
	default:
		break;
	}

	switch (parals)
	{
	case ParalsInfoFormat::CUBIC_UINT8:
		res += 8;
		break;
	case ParalsInfoFormat::NON_CUBIC_UINT8:
		res += 3 * 8;
		break;
	case ParalsInfoFormat::CUBIC_FLOAT32:
		res += sizeof(float) * 8;
		break;
	case ParalsInfoFormat::NON_CUBIC_FLOAT32:
		res += sizeof(float) * 3 * 8;
		break;
	default:
		break;
	}

	if (alignToFourBytes)
		return ceil(float(res) / 4.0f);

	return res;
}

std::vector<uint8_t> VoxelFormat::formatVoxel(const Voxel& voxel, const std::vector<uint8_t>& neighs, const std::vector<uint8_t>& parals, bool alignToFourBytes) const
{
	std::vector<uint8_t> res;
	res.reserve(getSizeInBytes(alignToFourBytes));

	switch (fullness)
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

	switch (type)
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

	switch (orientation)
	{
	case VoxelOrientationFormat::UINT8:
		utils::appendBytes(res, voxel.getOrientation());
		break;
	default:
		break;
	}

	uint8_t rgb8;
	switch (color)
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

Voxel VoxelFormat::unformatVoxel(const uint8_t* data, uint8_t power) const
{
	Voxel voxel(power);

	switch (fullness)
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

	switch (type)
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

	switch (orientation)
	{
	case VoxelOrientationFormat::UINT8:
		voxel.setOrientation(*data++);
		break;
	default:
		break;
	}

	std::vector<uint8_t> rgb;
	switch (color)
	{
	case VoxelColorFormat::GRAYSCALE:
		voxel.color[0] = voxel.color[1] = voxel.color[2] = data[0];
		voxel.color[3] = 255;
		break;
	case VoxelColorFormat::RGB256:
		rgb = utils::decodeRGB(data[0]);
		voxel.color = { rgb[0], rgb[1], rgb[2], 255 };
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