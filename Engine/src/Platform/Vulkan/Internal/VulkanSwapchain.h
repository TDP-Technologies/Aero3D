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
    VulkanSwapchain() = default;
    ~VulkanSwapchain() = default;

    void Init(const VulkanPhysicalDevice& physDevice, VkSurfaceKHR surface,
        SDL_Window* window, VkDevice device, int width, int height);
    void Shutdown();

    void Recreate(const VulkanPhysicalDevice& physDevice, VkSurfaceKHR surface,
        SDL_Window* window, VkDevice device, int width, int height);

    VkSwapchainKHR GetHandle() { return m_Swapchain; }
    VkSwapchainKHR* GetHandleAddr() { return &m_Swapchain; }
    VkFormat GetImageFormat() { return m_ImageFormat; }
    VkExtent2D GetExtent() { return m_Extent; }
    uint32_t GetNumImages() { return m_NumImages; }
    std::vector<VkImage>& GetImages() { return m_Images; }
    std::vector<VkImageView>& GetImageViews() { return m_ImageViews; }
    VkImage GetImage(uint32_t index) { return m_Images[index]; }
    VkImageView GetImageView(uint32_t index) { return m_ImageViews[index]; }

private:
    void CreateSwapchain(const VulkanPhysicalDevice& physDevice, int width, int height);
    void CreateImageViews();

private:
    VkDevice m_Device = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    SDL_Window* m_Window = nullptr;

    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    
    VkFormat m_ImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_Extent {};

    uint32_t m_NumImages = 0;
    std::vector<VkImage> m_Images {};
    std::vector<VkImageView> m_ImageViews {};

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANSWAPCHAIN_H_