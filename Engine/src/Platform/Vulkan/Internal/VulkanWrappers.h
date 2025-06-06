#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANWRAPPERS_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANWRAPPERS_H_

#include <memory>
#include <vector>
#include <optional>

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

#include "Utils/Common.h"
#include "Platform/Vulkan/Internal/VulkanDescriptors.h"

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

    void Init(const VulkanPhysicalDeviceInfo& physDevice, VkSurfaceKHR surface,
        SDL_Window* window, VkDevice device, int width, int height);
    void Shutdown();

    void Recreate(const VulkanPhysicalDeviceInfo& physDevice, VkSurfaceKHR surface,
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
    void CreateSwapchain(const VulkanPhysicalDeviceInfo& physDevice, int width, int height);
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

class VulkanQueue 
{
public:
    VulkanQueue() = default;
    ~VulkanQueue() = default;

    void Init(VkDevice device, uint32_t queueFamilyIndex);
    void Shutdown();

    void SubmitSync(VkCommandBuffer* pCmdBuff);
	void SubmitAsync(VkCommandBuffer* pCmdBuff, VkSemaphore* pWaitSem,
        VkSemaphore* pSigSem, VkFence pFence);

    void Present(VkSemaphore* pWaitSem, VkSwapchainKHR* pSwapchain, uint32_t imageIndex);
    void WaitIdle();

    VkQueue GetHandle() const { return m_Queue; }
    uint32_t GetQueueFamilyIndex() const { return m_QueueFamilyIndex; }

private:
    VkDevice m_Device = VK_NULL_HANDLE;

    VkQueue m_Queue = VK_NULL_HANDLE;
    uint32_t m_QueueFamilyIndex = 0;

    VkFence m_Fence = VK_NULL_HANDLE;

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANWRAPPERS_H_