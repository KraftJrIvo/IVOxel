#include "Cube.h"

#include <types.h>

float Cube::rayTrace(const Eigen::Vector3f& orig, const Eigen::Vector3f& dir, Eigen::Vector3f& hit, Eigen::Vector3f& normal)
{
	Eigen::Vector3f g = orig - Eigen::Vector3f(0.5f, 0.5f, 0.5f);
	Eigen::Vector3f gabs = { fabs(g[X]), fabs(g[Y]), fabs(g[Z]) };
	float maxG = std::max(gabs[X], std::max(gabs[Y], gabs[Z]));
	for (uint8_t i = 0; i < DIMENSIONS; ++i)
		if (gabs[i] == maxG)
		{
			normal[i] = g[i] / 0.5f;
			normal[(i + 1) % DIMENSIONS] = 0;
			normal[(i + 2) % DIMENSIONS] = 0;
			break;
		}
	hit = orig;

	return 0;
}
