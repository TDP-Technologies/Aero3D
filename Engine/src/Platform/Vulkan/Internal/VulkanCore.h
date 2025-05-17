#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCORE_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCORE_H_

#include <memory>
#include <vector>

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

namespace aero3d {

struct VulkanPhysicalDevice
{
    VkPhysicalDevice Device;
    VkPhysicalDeviceProperties Properties;
    VkPhysicalDeviceFeatures Features;
    VkPhysicalDeviceMemoryProperties MemoryProperties;
    std::vector<VkQueueFamilyProperties> QueueFamilyProperties;
};

class VulkanCore
{
public:
    VulkanCore();
    ~VulkanCore();

    bool Init(SDL_Window* window);
    void Shutdown();

private:
    bool CreateInstance();
    bool CreateSurface();
    bool CreatePhysicalDevices();

private:
    SDL_Window* m_Window;

    VkInstance m_Instance;
    VkSurfaceKHR m_Surface;

    unsigned int m_PhysDeviceCount;
    unsigned int m_CurrentPhysDevice;
    std::vector<VulkanPhysicalDevice> m_PhysDevices;

};

extern std::unique_ptr<VulkanCore> g_VulkanCore;

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCORE_H_