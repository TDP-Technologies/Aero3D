#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCORE_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCORE_H_

#include <memory>
#include <vector>

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

#include "Utils/Common.h"
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

    void RecordCommands();
    void EndCommands();

    void Draw(size_t count);
    void DrawIndexed(size_t count);

    void SwapBuffers();

    void SetViewport(int x, int y, int width, int height);
    void SetClearColor(float r, float g, float b, float a);
    void Clear();

    VkInstance GetInstance() { return m_Instance; }
    VkSurfaceKHR GetSurface() { return m_Surface; }

    const Ref<VulkanDevice> GetDevice() const { return m_Device; }
    const Ref<VulkanSwapchain> GetSwapchain() const { return m_Swapchain; }

    VkRenderPass GetRenderPass() { return m_RenderPass; }

    VkCommandBuffer GetCommandBuffer() { return m_CommandBuffers[m_CurrentFrame]; }

private:
    bool CreateInstance();
    bool CreateSurface();
    bool CreateImageViews();
    bool CreateRenderPass();
    bool CreateFramebuffers();
    bool CreateCommandBuffersAndCommandPool();
    bool CreateSyncObjects();

private:
    SDL_Window* m_Window = nullptr;

    VkInstance m_Instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

    Ref<VulkanDevice> m_Device = nullptr;
    Ref<VulkanQueue> m_GraphicsQueue = nullptr;
    Ref<VulkanQueue> m_PresentQueue = nullptr;

    Ref<VulkanSwapchain> m_Swapchain = nullptr;

    uint32_t m_CurrentImage = 0;
    std::vector<VkImageView> m_SwapchainImageViews {};

    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_SwapchainFramebuffers {};

    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_CommandBuffers {};

    int m_CurrentFrame = 0;
    FrameSyncObjects m_SyncObjects[FRAMES] {};

    VkClearValue m_ClearColor { {0.0f, 1.0f, 0.0f, 1.0f} };
    VkViewport m_Viewport { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

};

extern Scope<VulkanCore> g_VulkanCore;

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCORE_H_