#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCONTEXT_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCONTEXT_H_

#include <vector>
#include <optional>

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>

#include "Platform/Vulkan/Internal/VulkanDescriptors.h"
#include "Platform/Vulkan/Internal/VulkanWrappers.h"

namespace aero3d {

class VulkanContext
{
public:
    static SDL_Window* Window;
    static VkInstance Instance;
    static VkSurfaceKHR Surface;
    static VkDevice Device;
    static VkPhysicalDevice PhysDevice;

    static VulkanSwapchain Swapchain;

    static VulkanDescriptorSetLayout DescriptorSetLayout;
    static VkPipelineLayout PipelineLayout;

    static VulkanPhysicalDeviceInfo PhysDeviceInfo;

public:
    static void Init(SDL_Window* window, int width, int height);
    static void Shutdown();

private:
    static void CreateInstance();
    static void CreateSurface();
    static void CreatePhysDevice();
    static void CreateDevice();

    static void InitWrappers(int width, int height);
    static void ConfigurePipeline();

};

extern VulkanContext g_VulkanContext;

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCONTEXT_H_