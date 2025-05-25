#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANUTILS_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANUTILS_H_

#include <vulkan/vulkan.h>

#include "Utils/Log.h"

#define A3D_CHECK_VKRESULT(res) \
    if (res != VK_SUCCESS) { \
        LogErr(ERROR_INFO, "VkResult Failed: %s (%d)", VkResultToString(res), res); \
    }

namespace aero3d {

const char* VkResultToString(VkResult result);

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANUTILS_H_