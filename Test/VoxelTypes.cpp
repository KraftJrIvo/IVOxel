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
	glm::vec3 g = start - glm::vec3(0.5f, 0.5f, 0.5f);
	if (g.x == g.y == g.z == 0) {
		float maxD = std::max(dir[0], std::max(dir[1], dir[2]));
		for (uint8_t i = 0; i < 3; ++i)
			if (dir[i] == maxD)
			{
				normal[i] = -1.0f * dir[i]/abs(dir[i]);
				normal[(i + 1) % 3] = 0;
				normal[(i + 2) % 3] = 0;
				hit[i] = 0;
				hit[(i + 1) % 3] = 0;
				hit[(i + 2) % 3] = 0;
				return true;
			}
	}
	glm::vec3 gabs = glm::vec3(abs(g[0]), abs(g[1]), abs(g[2]));
	float maxG = std::max(gabs[0], std::max(gabs[1], gabs[2]));
	for (uint8_t i = 0; i < 3; ++i)
		if (gabs[i] == maxG)
		{
			normal[i] = g[i] / 0.5f;
			normal[(i + 1) % 3] = 0;
			normal[(i + 2) % 3] = 0;
			break;
		}
	hit = start;

	return true;
}

bool ShapeSphere::raytrace(glm::vec3 start, glm::vec3 dir, VoxelNeighbours neighs, glm::vec3& hit, glm::vec3& normal)
{
	auto getSphereIntersectionDist = [](glm::vec3 orig, glm::vec3 dir) {
		auto solveQuadratic = [](float a, float b, float c, float& x0, float& x1) {
			float discr = b * b - 4 * a * c;

			if (discr < 0)
				return false;

			else if (discr == 0)
				x0 = x1 = -0.5 * b / a;
			else
			{
				float q = (b > 0) ?
					-0.5 * (b + sqrt(discr)) :
					-0.5 * (b - sqrt(discr));
				x0 = q / a;
				x1 = c / q;
			}

			if (x0 > x1)
			{
				float temp = x0;
				x0 = x1;
				x1 = temp;
			}

			return true;
		};

		float t0, t1; // solutions for t if the ray intersects 

		glm::vec3 center = { 0.5, 0.5, 0.5 };
		float radius2 = 0.49 * 0.49;

		glm::vec3 L = orig - center;
		float a = dot(dir, dir);
		float b = 2 * dot(dir, L);
		float c = dot(L, L) - radius2;

		if (!solveQuadratic(a, b, c, t0, t1))
			return -1.0f;

		if (t0 > t1)
		{
			float temp = t0;
			t0 = t1;
			t1 = t0;
		}

		if (t0 < 0)
		{
			t0 = t1; // if t0 is negative, let's use t1 instead 
			if (t0 < 0)
				return -1.0f; // both t0 and t1 are negative 
		}

		return t0;
	};

	float len = getSphereIntersectionDist(start, dir);
	if (len < 0) return false;

	hit = start + dir * len;
	glm::vec3 center = { 0.5f, 0.5f, 0.5f };
	normal = hit - center;

	return true;
}

glm::vec3 MaterialDefault::shade(glm::vec3 curColor, glm::vec3 hitPoint, glm::vec3 normal, glm::vec3 lightDir, glm::vec3 lightColor)
{
	float dotVal = dot(normalize(lightDir), normal);
	float lightStr = (dotVal < 0) ? 0 : dotVal;

	return curColor * lightColor * lightStr;
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
