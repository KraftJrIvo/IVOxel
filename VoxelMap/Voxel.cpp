#include "Voxel.h"

Voxel::Voxel(uint8_t size, std::shared_ptr<VoxelShape> shape_, std::shared_ptr<VoxelMaterial> material_, VoxelOrientation orientation_, const glm::vec4& rgba) :
	shape(shape_),
	material(material_),
	orientation(orientation_),
	color(rgba)
{ }

bool Voxel::isEmpty() const
{
	return shape == nullptr;
}

bool Voxel::isTransparent() const
{
	return material->opacity < 1.0f;
}

uint32_t VoxelFormat::getSizeInBytes(bool alignToFourBytes) const
{
	uint32_t sz = ::getSizeInBytes(fullness) + ::getSizeInBytes(size) + ::getSizeInBytes(shape) + ::getSizeInBytes(material) + 
		::getSizeInBytes(orientation) + ::getSizeInBytes(color) + ::getSizeInBytes(neighbour) + ::getSizeInBytes(parals);
	
	if (alignToFourBytes)
		return 4 * ceil(float(sz) / 4.0f);

	return sz;
}

std::vector<uint8_t> VoxelFormat::formatVoxel(const Voxel& voxel, uint32_t size_, const std::vector<uint8_t>& neighs, const std::vector<uint8_t>& parals, bool alignToFourBytes) const
{
	std::vector<uint8_t> res;
	res.reserve(getSizeInBytes(alignToFourBytes));

	switch (fullness)
	{
	case VoxelFullnessFormat::UINT8:
		utils::appendBytes(res, uint8_t(!voxel.isEmpty()));
		break;
	case VoxelFullnessFormat::UINT16:
		utils::appendBytes(res, uint16_t(!voxel.isEmpty()), 2);
		break;
	case VoxelFullnessFormat::UINT24:
		utils::appendBytes(res, uint32_t(!voxel.isEmpty()), 3);
		break;
	case VoxelFullnessFormat::UINT32:
		utils::appendBytes(res, uint32_t(!voxel.isEmpty()), 4);
		break;
	default:
		break;
	}

	switch (size)
	{
	case VoxelSizeFormat::UINT8:
		utils::appendBytes(res, uint8_t(size_));
		break;
	case VoxelSizeFormat::UINT32:
		utils::appendBytes(res, size_);
		break;
	default:
		break;
	}

	switch (shape)
	{
	case VoxelShapeFormat::UINT8:
		utils::appendBytes(res, (uint8_t)coder->encodeShape(voxel.shape));
		break;
	case VoxelShapeFormat::UINT16:
		utils::appendBytes(res, (uint16_t)coder->encodeShape(voxel.shape));
		break;
	case VoxelShapeFormat::UINT32:
		utils::appendBytes(res, (uint32_t)coder->encodeShape(voxel.shape));
		break;
	default:
		break;
	}

	switch (material)
	{
	case VoxelMaterialFormat::UINT8:
		utils::appendBytes(res, (uint8_t)coder->encodeMaterial(voxel.material));
		break;
	case VoxelMaterialFormat::UINT16:
		utils::appendBytes(res, (uint16_t)coder->encodeMaterial(voxel.material));
		break;
	case VoxelMaterialFormat::UINT32:
		utils::appendBytes(res, (uint32_t)coder->encodeMaterial(voxel.material));
		break;
	default:
		break;
	}

	auto encodeOri1 = [&]() {
		uint8_t rx = voxel.orientation.rotation[0] / 3.14159 / 2.0;
		uint8_t ry = voxel.orientation.rotation[1] / 3.14159 / 2.0;
		uint8_t rz = voxel.orientation.rotation[2] / 3.14159 / 2.0;
		return (((voxel.orientation.mirror ? 1 : 0) << 7) & rx << 4 & ry << 2 & rz);
	};
	auto encodeOri4 = [&]() {
		uint8_t rx = voxel.orientation.rotation[0] / (3.14159 / 120.0f);
		uint8_t ry = voxel.orientation.rotation[1] / (3.14159 / 120.0f);
		uint8_t rz = voxel.orientation.rotation[2] / (3.14159 / 120.0f);
		std::vector<uint8_t> ori = { (uint8_t)voxel.orientation.mirror, rx, ry, rz };
		return ori;
	};
	auto encodeOri16 = [&]() {
		std::vector<float> ori = { voxel.orientation.mirror ? 1.0f : 0.0f, voxel.orientation.rotation[0], voxel.orientation.rotation[1], voxel.orientation.rotation[2] };
		return ori;
	};

	switch (orientation)
	{
	case VoxelOrientationFormat::UINT8:
		utils::appendBytes(res, encodeOri1());
		break;
	case VoxelOrientationFormat::UINT32:
		utils::appendBytes(res, encodeOri4());
		break;
	case VoxelOrientationFormat::FULL_16BYTES:
		utils::appendBytes(res, encodeOri16());
		break;
	break;	default:
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
		utils::appendBytes(res, (uint8_t)voxel.color[0]);
		utils::appendBytes(res, (uint8_t)voxel.color[1]);
		utils::appendBytes(res, (uint8_t)voxel.color[2]);
		break;
	case VoxelColorFormat::RGBA_FOUR_BYTES:
		utils::appendBytes(res, (uint8_t)voxel.color[0]);
		utils::appendBytes(res, (uint8_t)voxel.color[1]);
		utils::appendBytes(res, (uint8_t)voxel.color[2]);
		utils::appendBytes(res, (uint8_t)voxel.color[3]);
		break;
	}

	utils::appendBytes(res, neighs.size() ? neighs : std::vector<uint8_t>(0, ::getSizeInBytes(neighbour)));

	utils::appendBytes(res, parals.size() ? parals : std::vector<uint8_t>(0, ::getSizeInBytes(this->parals)));

	res.resize(getSizeInBytes(alignToFourBytes), 0);

	return res;
}

Voxel VoxelFormat::unformatVoxel(const uint8_t* data) const
{
	Voxel voxel;

	data += ::getSizeInBytes(fullness);

	switch (size)
	{
	case VoxelSizeFormat::UINT8:
		voxel.size = data[0];
		break;
	case VoxelSizeFormat::UINT32:
		voxel.size = ((uint32_t*)data)[0];
		break;
	default:
		break;
	}
	data += ::getSizeInBytes(size);

	uint32_t shId;
	switch (shape)
	{
	case VoxelShapeFormat::UINT8:
		shId = data[0];
		break;
	case VoxelShapeFormat::UINT16:
		shId = ((uint16_t*)data)[0];
		break;
	case VoxelShapeFormat::UINT32:
		shId = ((uint32_t*)data)[0];
		break;
	default:
		break;
	}
	data += ::getSizeInBytes(shape);
	voxel.shape = storer->hasShape(shId) ? storer->getShape(shId) : storer->addShape(shId, coder->decodeShape(data));

	uint32_t mtId;
	switch (material)
	{
	case VoxelMaterialFormat::UINT8:
		mtId = data[0];
		break;
	case VoxelMaterialFormat::UINT16:
		mtId = ((uint16_t*)data)[0];
		break;
	case VoxelMaterialFormat::UINT32:
		mtId = ((uint32_t*)data)[0];
		break;
	default:
		break;
	}
	data += ::getSizeInBytes(material);
	voxel.material = storer->hasMaterial(mtId) ? storer->getMaterial(mtId) : storer->addMaterial(mtId, coder->decodeMaterial(data));

	auto decodeOri1 = [&](const uint8_t* data) {
		VoxelOrientation ori;
		ori.mirror = (data[0] >> 7);
		ori.rotation[0] = ((data[0] >> 4) & 0x11) * 3.14159 / 2.0;
		ori.rotation[1] = ((data[0] >> 2) & 0x11) * 3.14159 / 2.0;
		ori.rotation[2] = (data[0] & 0x11) * 3.14159 / 2.0;
		return ori;
	};
	auto decodeOri4 = [&](const uint8_t* data) {
		VoxelOrientation ori;
		ori.mirror = data[0];
		ori.rotation[0] = data[1] * (3.14159 / 120.0f);
		ori.rotation[1] = data[2] * (3.14159 / 120.0f);
		ori.rotation[2] = data[3] * (3.14159 / 120.0f);
		return ori;
	};
	auto decodeOri16 = [&](const uint8_t* data) {
		VoxelOrientation ori;
		auto dataf = (float*)data;
		ori.mirror = dataf[0];
		ori.rotation[0] = dataf[1];
		ori.rotation[1] = dataf[2];
		ori.rotation[2] = dataf[3];
		return ori;
	};

	switch (orientation)
	{
	case VoxelOrientationFormat::UINT8:
		voxel.orientation = decodeOri1(data);
		break;
	case VoxelOrientationFormat::UINT32:
		voxel.orientation = decodeOri4(data);
		break;
	case VoxelOrientationFormat::FULL_16BYTES:
		voxel.orientation = decodeOri16(data);
		break;
	default:
		break;
	}
	data += ::getSizeInBytes(orientation);

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

	return voxel;
}

VoxelNeighbours VoxelFormat::unformatNeighs(const uint8_t* data) const
{
	VoxelNeighbours res;
	bool tmp;

	switch (neighbour)
	{
	case VoxelNeighbourInfoFormat::TWENTY_SIX_DIRS_FOUR_BYTES:
		utils::unpackByte(data, tmp, tmp, tmp, tmp, tmp, tmp, res.l, res.ld);
		utils::unpackByte(data, res.lu, res.lb, res.lf, res.ldb, res.ldf, res.lub, res.luf, res.r);
		utils::unpackByte(data, res.rd, res.ru, res.rb, res.rf, res.rdb, res.rdf, res.rub, res.ruf);
		utils::unpackByte(data, res.d, res.db, res.df, res.u, res.ub, res.uf, res.b, res.f);
		break;
	case VoxelNeighbourInfoFormat::SIX_DIRS_ONE_BYTE:
		bool tmp;
		utils::unpackByte(data, tmp, tmp, res.l, res.r, res.d, res.u, res.b, res.f);
		break;
	}

	return res;
}
