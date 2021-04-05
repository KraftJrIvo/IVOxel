#include "VoxelTypes.h"

std::shared_ptr<VoxelShape> TestVoxelTypeCoder::decodeShape(const uint8_t* data)
{
	auto id = ((uint32_t*)data)[0];
	return _vts.getShape(id);
}

std::shared_ptr<VoxelMaterial> TestVoxelTypeCoder::decodeMaterial(const uint8_t* data)
{
	auto id = ((uint32_t*)data)[0];
	return _vts.getMaterial(id);
}

uint32_t TestVoxelTypeCoder::encodeShape(std::shared_ptr<VoxelShape> shp)
{
	if (shp)
	{
		if (shp->name == "cube") return 1;
		return 2;
	}
	return 0;
}

uint32_t TestVoxelTypeCoder::encodeMaterial(std::shared_ptr<VoxelMaterial> mat)
{
	if (mat)
		return 1;
	return 0;
}

bool ShapeCube::raytrace(glm::vec3 start, glm::vec3 dir, VoxelNeighbours neighs, glm::vec3& hit, glm::vec3& normal)
{
	return false;
}
bool ShapeSphere::raytrace(glm::vec3 start, glm::vec3 dir, VoxelNeighbours neighs, glm::vec3& hit, glm::vec3& normal)
{
	return false;
}

glm::vec3 MaterialDefault::shade(glm::vec3 curColor, glm::vec3 hitPoint, glm::vec3 normal, glm::vec3 lightDir, glm::vec4 lightColor)
{
	return glm::vec3();
}

std::string ShapeCube::getRaytraceShaderCode()
{
	return std::string();
}

std::string ShapeSphere::getRaytraceShaderCode()
{
	return std::string();
}

std::string MaterialDefault::getShadeShaderCode()
{
	return std::string();
}
