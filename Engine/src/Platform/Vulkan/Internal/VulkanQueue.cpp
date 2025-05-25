#include "Platform/Vulkan/Internal/VulkanQueue.h"

#include "Platform/Vulkan/Internal/VulkanUtils.h"

namespace aero3d {

VulkanQueue::VulkanQueue(VkQueue queue, uint32_t queueFamilyIndex)
    : m_Queue(queue), m_QueueFamilyIndex(queueFamilyIndex)
{
}

void VulkanQueue::Submit(const VkSubmitInfo* submitInfo, VkFence fence) 
{
    A3D_CHECK_VKRESULT(vkQueueSubmit(m_Queue, 1, submitInfo, fence));
}

void VulkanQueue::WaitIdle() 
{
    A3D_CHECK_VKRESULT(vkQueueWaitIdle(m_Queue));
}

} // namespace aero3d