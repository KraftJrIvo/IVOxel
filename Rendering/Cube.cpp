#include "Cube.h"

#include <types.h>

float Cube::rayTrace(const Eigen::Vector3f& orig, const Eigen::Vector3f& dir, Eigen::Vector3f& hit, Eigen::Vector3f& normal)
{
	Eigen::Vector3f g = orig - Eigen::Vector3f(0.5f, 0.5f, 0.5f);
	Eigen::Vector3f gabs = { fabs(g[0]), fabs(g[1]), fabs(g[2]) };
	float maxG = std::max(gabs[0], std::max(gabs[1], gabs[2]));
	for (uint8_t i = 0; i < 3; ++i)
		if (gabs[i] == maxG)
		{
			normal[i] = g[i] / 0.5f;
			normal[(i + 1) % 3] = 0;
			normal[(i + 2) % 3] = 0;
			break;
		}
	hit = orig;

	return 0;
}
