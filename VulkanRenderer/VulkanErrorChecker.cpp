#include <iostream>

#include "VulkanErrorChecker.h"

bool VulkanErrorChecker::check(const VkResult& res)
{
    bool success = (res == VK_SUCCESS);

    if (!success)
    {
        std::cout << "Therre was an ERROR: ";
        switch (res)
        {
        case VK_SUCCESS:
            std::cout << "VK_SUCCESS";
            break;
        case VK_NOT_READY:
            std::cout << "VK_NOT_READY";
            break;
        case VK_TIMEOUT:
            std::cout << "VK_TIMEOUT";
            break;
        case VK_EVENT_SET:
            std::cout << "VK_EVENT_SET";
            break;
        case VK_EVENT_RESET:
            std::cout << "VK_EVENT_RESET";
            break;
        case VK_INCOMPLETE:
            std::cout << "VK_INCOMPLETE";
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY";
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY";
            break;
        case VK_ERROR_INITIALIZATION_FAILED:
            std::cout << "VK_ERROR_INITIALIZATION_FAILED";
            break;
        case VK_ERROR_DEVICE_LOST:
            std::cout << "VK_ERROR_DEVICE_LOST";
            break;
        case VK_ERROR_MEMORY_MAP_FAILED:
            std::cout << "VK_ERROR_MEMORY_MAP_FAILED";
            break;
        case VK_ERROR_LAYER_NOT_PRESENT:
            std::cout << "VK_ERROR_LAYER_NOT_PRESENT";
            break;
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            std::cout << "VK_ERROR_EXTENSION_NOT_PRESENT";
            break;
        case VK_ERROR_FEATURE_NOT_PRESENT:
            std::cout << "VK_ERROR_FEATURE_NOT_PRESENT";
            break;
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            std::cout << "VK_ERROR_INCOMPATIBLE_DRIVER";
            break;
        case VK_ERROR_TOO_MANY_OBJECTS:
            std::cout << "VK_ERROR_TOO_MANY_OBJECTS";
            break;
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            std::cout << "VK_ERROR_FORMAT_NOT_SUPPORTED";
            break;
        case VK_ERROR_FRAGMENTED_POOL:
            std::cout << "VK_ERROR_FRAGMENTED_POOL";
            break;
        case VK_ERROR_UNKNOWN:
            std::cout << "VK_ERROR_UNKNOWN";
            break;
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            std::cout << "VK_ERROR_OUT_OF_POOL_MEMORY";
            break;
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            std::cout << "VK_ERROR_INVALID_EXTERNAL_HANDLE";
            break;
        case VK_ERROR_FRAGMENTATION:
            std::cout << "VK_ERROR_FRAGMENTATION";
            break;
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
            std::cout << "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
            break;
        case VK_ERROR_SURFACE_LOST_KHR:
            std::cout << "VK_ERROR_SURFACE_LOST_KHR";
            break;
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            std::cout << "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
            break;
        case VK_SUBOPTIMAL_KHR:
            std::cout << "VK_SUBOPTIMAL_KHR";
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
            std::cout << "VK_ERROR_OUT_OF_DATE_KHR";
            break;
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            std::cout << "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
            break;
        case VK_ERROR_VALIDATION_FAILED_EXT:
            std::cout << "VK_ERROR_VALIDATION_FAILED_EXT";
            break;
        case VK_ERROR_INVALID_SHADER_NV:
            std::cout << "VK_ERROR_INVALID_SHADER_NV";
            break;
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
            std::cout << "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
            break;
        case VK_ERROR_NOT_PERMITTED_EXT:
            std::cout << "VK_ERROR_NOT_PERMITTED_EXT";
            break;
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
            std::cout << "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
            break;
        case VK_RESULT_RANGE_SIZE:
            std::cout << "VK_RESULT_RANGE_SIZE";
            break;
        case VK_RESULT_MAX_ENUM:
            std::cout << "VK_RESULT_MAX_ENUM";
            break;
        default:
            std::cout << "Unknown";
            break;
        }
        std::cout << std::endl;
    }

	return success;
}
