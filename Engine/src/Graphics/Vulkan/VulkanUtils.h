#ifndef AERO3D_GRAPHICS_VULKAN_VULKANUTILS_H_
#define AERO3D_GRAPHICS_VULKAN_VULKANUTILS_H_

#include <volk.h>

#include "Utils/Log.h"

#define A3D_CHECK_VKRESULT(res) \
    if (res != VK_SUCCESS) { \
        LogErr(ERROR_INFO, "VkResult Failed: %s (%d)", VkResultToString(res), res); \
    }

namespace aero3d {

const char* VkResultToString(VkResult result);

void BeginCommandBuffer(VkCommandBuffer& buffer, VkCommandBufferUsageFlags flags);

} // namespace aero3d

#endif // AERO3D_GRAPHICS_VULKAN_VULKANUTILS_H_