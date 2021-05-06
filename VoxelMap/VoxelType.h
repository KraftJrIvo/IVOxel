#pragma once

#include <string>
#include <map>
#include <memory>

#include <glm/glm.hpp>
#include "types.hpp"

struct VoxelShape
{
	VoxelShape(const std::string& name_ = "none") : name(name_)
	{ }

	std::string name = "none";

	virtual bool raytrace(glm::vec3 start, glm::vec3 dir, VoxelNeighbours neighs, glm::vec3& hit, glm::vec3& normal) = 0;

	virtual std::string getRaytraceShaderCode() = 0;
};

struct VoxelMaterial
{
	VoxelMaterial(const std::string& name_ = "none", float opacity_ = 1, float reflectivity_ = 0, glm::vec3 tint_ = glm::vec3(1, 1, 1)) :
		name(name_),
		opacity(opacity_),
		reflectivity(reflectivity_),
		tint(tint_)
	{ }

	std::string name;
	float opacity;
	float reflectivity;
	glm::vec3 tint;

	virtual glm::vec3 shade(glm::vec3 curColor, glm::vec3 voxColor, glm::vec3 hitPoint, glm::vec3 normal, glm::vec3 lightDir, glm::vec3 lightColor) = 0;

	virtual std::string getShadeShaderCode() = 0;
};

struct VoxelOrientation
{
	glm::vec3 rotation;
	bool mirror;
};

class VoxelTypeCoder
{
public:
	virtual std::shared_ptr<VoxelShape> decodeShape(const uint8_t* data) = 0;
	virtual std::shared_ptr<VoxelMaterial> decodeMaterial(const uint8_t* data) = 0;
	virtual uint32_t encodeShape(std::shared_ptr<VoxelShape> shp) = 0;
	virtual uint32_t encodeMaterial(std::shared_ptr<VoxelMaterial> mat) = 0;
};

class VoxelTypeStorer
{
public:
	bool hasShape(uint32_t id) const;
	bool hasMaterial(uint32_t id) const;
	std::shared_ptr<VoxelShape> getShape(uint32_t id) const;
	std::shared_ptr<VoxelMaterial> getMaterial(uint32_t id) const;
	std::shared_ptr<VoxelShape> addShape(uint32_t id, std::shared_ptr<VoxelShape> shp);
	std::shared_ptr<VoxelMaterial> addMaterial(uint32_t id, std::shared_ptr<VoxelMaterial> mat);
private:
	std::map<uint32_t, std::shared_ptr<VoxelShape>> _shapes;
	std::map<uint32_t, std::shared_ptr<VoxelMaterial>> _materials;
};