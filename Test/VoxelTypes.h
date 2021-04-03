#pragma once

#include <VoxelType.h>

struct ShapeCube : public VoxelShape
{
	ShapeCube() : VoxelShape("cube")
	{ }

	virtual bool raytrace(glm::vec3 start, glm::vec3 dir, VoxelNeighbours neighs, glm::vec3& hit, glm::vec3& normal) override;

	virtual std::string getRaytraceShaderCode() override;
};

struct ShapeSphere : public VoxelShape
{
	ShapeSphere() : VoxelShape("sphere")
	{ }

	virtual bool raytrace(glm::vec3 start, glm::vec3 dir, VoxelNeighbours neighs, glm::vec3& hit, glm::vec3& normal) override;

	virtual std::string getRaytraceShaderCode() override;
};

struct MaterialDefault : public VoxelMaterial
{
	MaterialDefault() : 
		VoxelMaterial("default")
	{ }

	virtual glm::vec3 shade(glm::vec3 curColor, glm::vec3 hitPoint, glm::vec3 normal, glm::vec3 lightDir, glm::vec4 lightColor) override;

	virtual std::string getShadeShaderCode() override;
};

class TestVoxelTypeCoder : public VoxelTypeCoder
{
public:
	TestVoxelTypeCoder(VoxelTypeStorer& vts) : _vts(vts)
	{ }

	std::shared_ptr<VoxelShape> decodeShape(const uint8_t* data) override;
	std::shared_ptr<VoxelMaterial> decodeMaterial(const uint8_t* data) override;
	uint32_t encodeShape(std::shared_ptr<VoxelShape> shp) override;
	uint32_t encodeMaterial(std::shared_ptr<VoxelMaterial> mat) override;

private:
	VoxelTypeStorer& _vts;
};