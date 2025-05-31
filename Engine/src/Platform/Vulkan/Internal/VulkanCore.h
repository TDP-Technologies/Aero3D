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

class VulkanCore
{
public:
    VulkanCore() = default;
    ~VulkanCore() = default;

    bool Init(SDL_Window* window, int width, int height);
    void Shutdown();

    void ResizeBuffers(int width, int height);

    void RecordCommands();
    void EndCommands();

    void Draw(size_t count);
    void DrawIndexed(size_t count);

    void SwapBuffers();

    void SetClearColor(float r, float g, float b, float a);
    void Clear();

    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    VkInstance GetInstance() { return m_Instance; }
    VkSurfaceKHR GetSurface() { return m_Surface; }

    VkDevice GetDeviceHandle() { return m_Device.GetHandle(); }
    VkPhysicalDevice GetPhysDeviceHandle() { return m_Device.GetPhysicalDevice().Device; }

    const VulkanDevice* GetDevice() const { return &m_Device; }
    const VulkanSwapchain* GetSwapchain() const { return &m_Swapchain; }

    VkRenderPass GetRenderPass() { return m_RenderPass; }

    VkCommandBuffer GetCommandBuffer() { return m_GraphicsCommandBuffers[m_CurrentImage]; }

private:
    void CreateFramebuffers();
    void CreateCommandBuffersAndCommandPools();
    void CreateSyncObjects();

private:
    SDL_Window* m_Window = nullptr;

    VkInstance m_Instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

    VulkanDevice m_Device;
    VulkanQueue m_TransferQueue;
    VulkanQueue m_GraphicsQueue;
    VulkanQueue m_PresentQueue;

    VulkanSwapchain m_Swapchain;

    uint32_t m_CurrentImage = 0;

    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_SwapchainFramebuffers {};

    VkCommandPool m_GraphicsCommandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_GraphicsCommandBuffers {};

    VkCommandPool m_CopyCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_CopyCommandBuffer = VK_NULL_HANDLE;

    VkSemaphore m_ImageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_RenderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence m_InFlightFence = VK_NULL_HANDLE;

    VkClearValue m_ClearColor { {0.0f, 1.0f, 0.0f, 1.0f} };
    VkViewport m_Viewport { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
    VkRect2D m_Scissor { {0, 0}, {0, 0} };

};

extern Scope<VulkanCore> g_VulkanCore;

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANCORE_H_