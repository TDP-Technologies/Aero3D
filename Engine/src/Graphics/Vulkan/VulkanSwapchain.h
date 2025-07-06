#ifndef AERO3D_GRAPHICS_VULKAN_VULKANSWAPCHAIN_H_
#define AERO3D_GRAPHICS_VULKAN_VULKANSWAPCHAIN_H_

#include <vector>

#include <volk.h>

#include "Graphics/Swapchain.h"
#include "Graphics/Vulkan/VulkanResources.h"

namespace aero3d {

class VulkanGraphicsDevice;

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR Capabilities {};
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;
};

class VulkanSwapchain : public Swapchain
{
public:
    VulkanSwapchain(VulkanGraphicsDevice* gd);
    ~VulkanSwapchain();

    virtual void Resize() override;

    virtual Ref<Framebuffer> GetFramebuffer() override;

    uint32_t AcquireNextImage();

public:
    VkFence imageAvailableFence = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat imageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D extent;

    uint32_t numImages = 0;
    uint32_t currentImageIndex = 0;
    std::vector<Ref<VulkanTexture>> frames;
    std::vector<Ref<VulkanFramebuffer>> frameBuffers;
    Ref<VulkanTexture> depthStencil = nullptr;

private:
    void CreateLocks();
    void CreateSwapchain();
    void CreateFramebuffer();

    void Create();
    void Destroy();

private:
    VulkanGraphicsDevice* m_GraphicsDevice = nullptr;

};

} // namespace aero3d

#endif // AERO3D_GRAPHICS_VULKAN_VULKANSWAPCHAIN_H_