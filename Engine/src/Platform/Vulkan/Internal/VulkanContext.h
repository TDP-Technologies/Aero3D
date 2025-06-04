#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCONTEXT_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCONTEXT_H_

#include <vector>
#include <optional>

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>

namespace aero3d {

struct DeviceQueueFamilyIndices
{
    std::optional<uint32_t> GraphicsFamily;
    std::optional<uint32_t> PresentFamily;
    std::optional<uint32_t> TransferFamily;

    bool IsComplete() {
        return GraphicsFamily.has_value() && PresentFamily.has_value() && TransferFamily.has_value();
    }
};

struct VulkanPhysicalDeviceInfo
{
    VkPhysicalDevice Device;
    VkPhysicalDeviceProperties Properties;
    VkPhysicalDeviceFeatures Features;
    VkPhysicalDeviceMemoryProperties MemoryProperties;
    std::vector<VkQueueFamilyProperties> QueueFamilyProperties;
    DeviceQueueFamilyIndices QueueFamilyIndices;
};

class VulkanContext
{
public:
    SDL_Window* Window = nullptr;
    VkInstance Instance = VK_NULL_HANDLE;
    VkSurfaceKHR Surface = VK_NULL_HANDLE;
    VkDevice Device = VK_NULL_HANDLE;
    VkPhysicalDevice PhysDevice = VK_NULL_HANDLE;
    VulkanPhysicalDeviceInfo PhysDeviceInfo {};

public:
    void Init(SDL_Window* window);
    void Shutdown();

private:
    void CreateInstance();
    void CreateSurface();
    void CreatePhysDevice();
    void CreateDevice();

};

extern VulkanContext g_VulkanContext;

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCONTEXT_H_