#pragma once

#include <Eigen/Core>

class Sphere
{
public:
	Sphere() = default;

	static float rayTrace(const Eigen::Vector3f& orig, const Eigen::Vector3f& dir, Eigen::Vector3f& hit, Eigen::Vector3f& normal);
	static float getIntersectionDist(const Eigen::Vector3f& orig, const Eigen::Vector3f& dir);
	static bool solveQuadratic(const float& a, const float& b, const float& c, float& x0, float& x1);
};