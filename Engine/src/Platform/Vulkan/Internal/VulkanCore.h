#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCORE_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCORE_H_

#include <memory>
#include <vector>

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

#include "Platform/Vulkan/Internal/VulkanDevice.h"
#include "Platform/Vulkan/Internal/VulkanSwapchain.h"

namespace aero3d {

constexpr int FRAMES = 3;

struct FrameSyncObjects {
    VkSemaphore ImageAvailableSemaphore;
    VkSemaphore RenderFinishedSemaphore;
    VkFence InFlightFence;
};

class VulkanCore
{
public:
    VulkanCore();
    ~VulkanCore();

    bool Init(SDL_Window* window);
    void Shutdown();

    void SwapBuffers();

    void SetViewport(int x, int y, int width, int height);
    void SetClearColor(float r, float g, float b, float a);
    void Clear();

    VkInstance GetInstance() { return m_Instance; }
    VkSurfaceKHR GetSurface() { return m_Surface; }

    const VulkanDevice& GetDevice() { return m_Device; }
    const VulkanSwapchain& GetSwapchain() { return m_Swapchain; }

    VkRenderPass GetRenderPass() { return m_RenderPass; }

    VkCommandBuffer GetCommandBuffer() { return m_CommandBuffers[m_CurrentImage]; }

private:
    bool CreateInstance();
    bool CreateSurface();
    bool CreateImageViews();
    bool CreateRenderPass();
    bool CreateFramebuffers();
    bool CreateCommandBuffersAndCommandPool();
    bool CreateSyncObjects();

    void RecordCommands();
    void EndCommands();

private:
    SDL_Window* m_Window;

    VkInstance m_Instance;
    VkSurfaceKHR m_Surface;

    VulkanDevice m_Device;
    VulkanSwapchain m_Swapchain;

    uint32_t m_CurrentImage;
    std::vector<VkImageView> m_SwapchainImageViews;

    VkRenderPass m_RenderPass;
    std::vector<VkFramebuffer> m_SwapchainFramebuffers;

    VkCommandPool m_CommandPool;
    std::vector<VkCommandBuffer> m_CommandBuffers;

    int m_CurrentFrame;
    FrameSyncObjects m_SyncObjects[FRAMES];

    VkClearValue m_ClearColor;
    VkViewport m_Viewport;

};

extern std::unique_ptr<VulkanCore> g_VulkanCore;

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCORE_H_