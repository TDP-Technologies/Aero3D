#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANQUEUE_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANQUEUE_H_

#include <vulkan/vulkan.h>

namespace aero3d {

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

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANQUEUE_H_