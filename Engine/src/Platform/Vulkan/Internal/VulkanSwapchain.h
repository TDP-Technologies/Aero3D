#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANSWAPCHAIN_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANSWAPCHAIN_H_

#include <vector>

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

#include "Platform/Vulkan/Internal/VulkanDevice.h"

namespace aero3d {

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR Capabilities {};
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;
};

class VulkanSwapchain
{
public:
    VulkanSwapchain();
    ~VulkanSwapchain();

    bool Init(const VulkanPhysicalDevice& physDevice, VkSurfaceKHR surface,
        SDL_Window* window, VkDevice device);
    void Shutdown();

    VkSwapchainKHR GetSwapchain() { return m_Swapchain; }
    VkFormat GetImageFormat() { return m_ImageFormat; }
    VkExtent2D GetExtent() { return m_Extent; }
    std::vector<VkImage>& GetSwapchainImages() { return m_Images; }

private:
    void CreateSwapchain(const VulkanPhysicalDevice& physDevice);

private:
    VkDevice m_Device;
    VkSurfaceKHR m_Surface;
    SDL_Window* m_Window;

    VkSwapchainKHR m_Swapchain;
    VkFormat m_ImageFormat;
    VkExtent2D m_Extent;
    std::vector<VkImage> m_Images;

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANSWAPCHAIN_H_