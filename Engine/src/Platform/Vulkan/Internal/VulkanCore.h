#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCORE_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCORE_H_

#include <memory>
#include <vector>
#include <optional>

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

#include "Platform/Vulkan/Internal/VulkanDevice.h"

namespace aero3d {

struct QueueFamilyIndices 
{
    std::optional<uint32_t> GraphicsFamily;
    std::optional<uint32_t> PresentFamily;

    bool IsComplete() {
        return GraphicsFamily.has_value() && PresentFamily.has_value();
    }
};

struct VulkanPhysicalDevice
{
    VkPhysicalDevice Device;
    VkPhysicalDeviceProperties Properties;
    VkPhysicalDeviceFeatures Features;
    VkPhysicalDeviceMemoryProperties MemoryProperties;
    std::vector<VkQueueFamilyProperties> QueueFamilyProperties;
    QueueFamilyIndices QueueFamilyIndices;
};

struct SwapChainSupportDetails 
{
    VkSurfaceCapabilitiesKHR Capabilities;
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;
};

class VulkanCore
{
public:
    VulkanCore();
    ~VulkanCore();

    bool Init(SDL_Window* window);
    void Shutdown();

    void SwapBuffers();

    void SetClearColor(float r, float g, float b, float a);

    VkInstance GetInstance() { return m_Instance; }
    VkSurfaceKHR GetSurface() { return m_Surface; }

    const VulkanPhysicalDevice& GetPhysicalDevice() { return m_PhysDevices[m_CurrentPhysDevice]; }
    unsigned int GetCurrentPhysDeviceIndex() { return m_CurrentPhysDevice; }
    void SetPhysicalDevice(unsigned int index) { m_CurrentPhysDevice = index; }

private:
    bool CreateInstance();
    bool CreateSurface();
    bool CreatePhysicalDevices();
    bool CreateSwapchain();
    bool CreateImageViews();
    bool CreateRenderPass();
    bool CreateFramebuffers();
    bool CreateCommandBuffersAndCommandPool();
    bool CreateSyncObjects();

private:
    SDL_Window* m_Window;

    VkInstance m_Instance;
    VkSurfaceKHR m_Surface;
    VkSwapchainKHR m_Swapchain;

    unsigned int m_CurrentPhysDevice;
    std::vector<VulkanPhysicalDevice> m_PhysDevices;

    VulkanDevice m_Device;

    std::vector<VkImage> m_SwapchainImages;
    std::vector<VkImageView> m_SwapchainImageViews;
    VkFormat m_SwapchainImageFormat;
    VkExtent2D m_SwapchainExtent;

    VkRenderPass m_RenderPass;
    std::vector<VkFramebuffer> m_SwapchainFramebuffers;

    VkCommandPool m_CommandPool;
    std::vector<VkCommandBuffer> m_CommandBuffers;

    VkSemaphore m_ImageAvailableSemaphore;
    VkSemaphore m_RenderFinishedSemaphore;
    VkFence m_InFlightFence;

    VkClearValue m_ClearColor;

};

extern std::unique_ptr<VulkanCore> g_VulkanCore;

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCORE_H_