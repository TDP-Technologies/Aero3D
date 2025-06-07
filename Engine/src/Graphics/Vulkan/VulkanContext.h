#ifndef AERO3D_GRAPHICS_VULKAN_VULKANCONTEXT_H_
#define AERO3D_GRAPHICS_VULKAN_VULKANCONTEXT_H_

#include <optional>
#include <vector>

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

#include "Graphics/Context.h"

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

class VulkanContext : public Context
{
public:
    VulkanContext(SDL_Window* window);
    ~VulkanContext();

    SDL_Window* GetWindow() { return m_Window; }
    VkInstance GetInstance() { return m_Instance; }
    VkSurfaceKHR GetSurface() { return m_Surface; }
    VkPhysicalDevice GetPhysDevice() { return m_PhysDevice; }
    VkDevice GetDevice() { return m_Device; }
    VkCommandPool GetCommandPool() { return m_CommandPool; }
    VkSemaphore GetRenderFinishedSemaphore() { return m_RenderFinishedSemaphore; }
    VkSemaphore GetImageAvailableSemaphore() { return m_ImageAvailableSemaphore; }

    uint32_t GetCurrentImageIndex() { return m_CurrentImageIndex; }
    uint32_t* GetCurrentImageAddress() { return &m_CurrentImageIndex; }
    VulkanPhysicalDeviceInfo GetPhysDeviceInfo() { return m_PhysDeviceInfo; }

private:
    void CreateInstance();
    void CreateSurface();
    void CreatePhysDevice();
    void CreateDevice();
    void CreateCommandPool();
    void CreateSemaphores();

private:
    SDL_Window* m_Window = nullptr;
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysDevice = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    VkSemaphore m_RenderFinishedSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_ImageAvailableSemaphore = VK_NULL_HANDLE;
    uint32_t m_CurrentImageIndex = 0;

    VulkanPhysicalDeviceInfo m_PhysDeviceInfo {};

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_VULKAN_VULKANCONTEXT_H_