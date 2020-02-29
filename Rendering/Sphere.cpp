#include "Sphere.h"

#include <types.h>

float Sphere::rayTrace(const Eigen::Vector3f& orig, const Eigen::Vector3f& dir, Eigen::Vector3f& hit, Eigen::Vector3f& normal)
{
	float len = getIntersectionDist(orig, dir);
	hit = orig + dir * len;
	Eigen::Vector3f center = { 0.5f, 0.5f, 0.5f };
	normal = hit - center;

	return len;
}

float Sphere::getIntersectionDist(const Eigen::Vector3f& orig, const Eigen::Vector3f& dir)
{
	float t0, t1; // solutions for t if the ray intersects 

	Eigen::Vector3f center = { 0.5f, 0.5f, 0.5f };
	float radius2 = 0.49f * 0.49f;

	Eigen::Vector3f L = orig - center;
	float a = dir.dot(dir);
	float b = 2 * dir.dot(L);
	float c = L.dot(L) - radius2;
	if (!solveQuadratic(a, b, c, t0, t1)) return -1;

	if (t0 > t1) std::swap(t0, t1);

	if (t0 < 0) {
		t0 = t1; // if t0 is negative, let's use t1 instead 
		if (t0 < 0) return -1; // both t0 and t1 are negative 
	}

	float t = t0;

	return t;
}

bool Sphere::solveQuadratic(const float& a, const float& b, const float& c, float& x0, float& x1)
{
	float discr = b * b - 4 * a * c;
	if (discr < 0) return false;
	else if (discr == 0) x0 = x1 = -0.5 * b / a;
	else {
		float q = (b > 0) ?
			-0.5 * (b + sqrt(discr)) :
			-0.5 * (b - sqrt(discr));
		x0 = q / a;
		x1 = c / q;
	}
	if (x0 > x1) std::swap(x0, x1);

	return true;
}
