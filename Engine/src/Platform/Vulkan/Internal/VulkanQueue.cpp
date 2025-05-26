#include "Platform/Vulkan/Internal/VulkanQueue.h"

#include "Platform/Vulkan/Internal/VulkanUtils.h"

namespace aero3d {

VulkanQueue::VulkanQueue()
{
}

VulkanQueue::~VulkanQueue()
{
}

bool VulkanQueue::Init(VkDevice device, uint32_t queueFamilyIndex)
{
    m_QueueFamilyIndex = queueFamilyIndex;
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &m_Queue);

    return true;
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