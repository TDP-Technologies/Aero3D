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

struct VulkanPhysicalDevice
{
    VkPhysicalDevice Device;
    VkPhysicalDeviceProperties Properties;
    VkPhysicalDeviceFeatures Features;
    VkPhysicalDeviceMemoryProperties MemoryProperties;
    std::vector<VkQueueFamilyProperties> QueueFamilyProperties;
    DeviceQueueFamilyIndices QueueFamilyIndices;
};

class VulkanDevice
{
public:
    VulkanDevice() = default;
    ~VulkanDevice() = default;

    void Init(VkInstance instance, VkSurfaceKHR surface);
    void Shutdown();

    const VulkanPhysicalDevice& GetPhysicalDevice() const { return m_PhysDevices[m_CurrentPhysDevice]; }
    unsigned int GetCurrentPhysDeviceIndex() { return m_CurrentPhysDevice; }

    VkDevice GetHandle() const { return m_Device; }
    
    void SetPhysicalDevice(unsigned int index) { m_CurrentPhysDevice = index; }
    
private:
    void SearchForPhysDevices(VkSurfaceKHR surface);
    void CreateLogicalDevice();

private:
    VkInstance m_Instance =VK_NULL_HANDLE;

    unsigned int m_CurrentPhysDevice = 0;
    std::vector<VulkanPhysicalDevice> m_PhysDevices {};

    VkDevice m_Device = VK_NULL_HANDLE;

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

class VulkanQueue {
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
    VkCommandBuffer GetCommandBuffer() { return m_GraphicsCommandBuffers[m_CurrentFrame]; }
    std::vector<VkDescriptorSet>& GetDescriptorSets() { return m_DescriptorSets; }
    VulkanDescriptorSetLayout* GetDescriptorSetLayout() { return &m_DescriptorSetLayout; }
    VulkanDescriptorPool* GetDescriptorPool() { return &m_DescriptorPool; }
    VkPipelineLayout GetPipelineLayout() { return m_PipelineLayout; }
    uint32_t GetNumFrames() { return m_Swapchain.GetNumImages(); }
    uint32_t GetCurrentFrame() { return m_CurrentFrame; }

private:
    void CreateFramebuffers();
    void CreateCommandBuffersAndCommandPools();
    void CreateSyncObjects();
    void CreateDescriptors();
    void CreatePipelineLayout();

private:
    SDL_Window* m_Window = nullptr;

    VkInstance m_Instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

    VulkanDevice m_Device;
    VulkanQueue m_TransferQueue;
    VulkanQueue m_GraphicsQueue;
    VulkanQueue m_PresentQueue;

    VulkanSwapchain m_Swapchain;

    VulkanDescriptorSetLayout m_DescriptorSetLayout;
    VulkanDescriptorPool m_DescriptorPool;
    std::vector<VkDescriptorSet> m_DescriptorSets {};

    uint32_t m_CurrentFrame = 0;
    uint32_t m_CurrentImage = 0;

    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_SwapchainFramebuffers {};

    VkCommandPool m_GraphicsCommandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_GraphicsCommandBuffers {};

    VkCommandPool m_CopyCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_CopyCommandBuffer = VK_NULL_HANDLE;

    std::vector<VkSemaphore> m_ImageAvailableSemaphores {};
    std::vector<VkSemaphore> m_RenderFinishedSemaphores {};
    std::vector<VkFence> m_InFlightFences {};

    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

    VkClearValue m_ClearColor { {0.0f, 1.0f, 0.0f, 1.0f} };
    VkViewport m_Viewport { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
    VkRect2D m_Scissor { {0, 0}, {0, 0} };

};

extern Scope<VulkanCore> g_VulkanCore;

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANWRAPPERS_H_