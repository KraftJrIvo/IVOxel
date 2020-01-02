#pragma once

#include <vector>

#include "Ray.h"

class IReflectiveObject 
{
	virtual std::vector<Ray> reflect(const Ray& entryRay) = 0;
};