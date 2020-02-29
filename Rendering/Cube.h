#pragma once

#include <Eigen/Core>

class Cube
{
public:
	Cube() = default;

	static float rayTrace(const Eigen::Vector3f& orig, const Eigen::Vector3f& dir, Eigen::Vector3f& hit, Eigen::Vector3f& normal);
};