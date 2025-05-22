#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANQUEUE_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANQUEUE_H_

#include <vulkan/vulkan.h>

namespace aero3d {

class VulkanQueue {
public:
    VulkanQueue(VkQueue queue, uint32_t queueFamilyIndex);

    void Submit(const VkSubmitInfo* submitInfo, VkFence fence);
    void WaitIdle();

    VkQueue GetHandle() const { return m_Queue; }
    uint32_t GetQueueFamilyIndex() const { return m_QueueFamilyIndex; }

private:
    VkQueue m_Queue;
    uint32_t m_QueueFamilyIndex;

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANQUEUE_H_