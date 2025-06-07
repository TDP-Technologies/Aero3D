#ifndef AERO3D_GRAPHICS_VULKAN_VULKANVIEWPORT_H_
#define AERO3D_GRAPHICS_VULKAN_VULKANVIEWPORT_H_

#include <vector>

#include <vulkan/vulkan.h>

#include "Utils/Common.h"
#include "Graphics/Viewport.h"
#include "Graphics/Vulkan/VulkanContext.h"

namespace aero3d {

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR Capabilities {};
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;
};

class VulkanViewport : public Viewport
{
public:
    VulkanViewport(Ref<VulkanContext> context, int width, int height);
    ~VulkanViewport();

    virtual void SwapBuffers() override;
    virtual void Resize() override;

    VkSwapchainKHR GetSwapchain() { return m_Swapchain; }
    VkFormat GetFormat() { return m_ImageFormat; }
    VkExtent2D GetExtent() { return m_Extent; }
    uint32_t GetNumImages() { return m_NumImages; }

private:
    void CreateSwapchain(int width, int height);
    void CreateQueue();
    void CreateImageViews();
    void CreateFrames();

private:
    Ref<VulkanContext> m_Context = nullptr;
    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    VkQueue m_PresentQueue = VK_NULL_HANDLE;
    VkFence m_Fence = VK_NULL_HANDLE;

    VkFormat m_ImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_Extent {};

    uint32_t m_NumImages = 0;
    std::vector<VkImage> m_Images {};
    std::vector<VkImageView> m_ImageViews {};

};
    
} // namespace aero3d

#endif // AERO3D_GRAPHICS_VULKAN_VULKANVIEWPORT_H_