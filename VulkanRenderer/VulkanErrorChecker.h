#pragma once

#include "VulkanTypes.h"

class VulkanErrorChecker
{
public:
	VulkanErrorChecker() = default;
	bool check(const VkResult& res);

private:
	
};