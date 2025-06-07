#include "Graphics/Vulkan/VulkanQueue.h"

#include "Graphics/Vulkan/VulkanCommandBuffer.h"

namespace aero3d {

VulkanQueue::VulkanQueue(Ref<VulkanContext> context)
    : m_Context(context)
{
    CreateQueue();
}

VulkanQueue::~VulkanQueue()
{
    vkDeviceWaitIdle(m_Context->GetDevice());

    vkDestroyFence(m_Context->GetDevice(), m_Fence, nullptr);
}

void VulkanQueue::Execute(Ref<CommandBuffer> buffer)
{
    Ref<VulkanCommandBuffer> vulkanBuffer = std::static_pointer_cast<VulkanCommandBuffer>(buffer);

    VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkCommandBuffer cmb = vulkanBuffer->GetCommandBuffer();
    VkSemaphore imageAvailebleSemaphore = m_Context->GetImageAvailableSemaphore();
    VkSemaphore renderFinishedSemaphore = m_Context->GetRenderFinishedSemaphore();

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailebleSemaphore;
	submitInfo.pWaitDstStageMask = &waitFlags;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmb;
	submitInfo.signalSemaphoreCount = 1;			
	submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

	vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_Fence);

    vkWaitForFences(m_Context->GetDevice(), 1, &m_Fence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_Context->GetDevice(), 1, &m_Fence);
}

void VulkanQueue::CreateQueue()
{
    vkGetDeviceQueue(m_Context->GetDevice(),
        m_Context->GetPhysDeviceInfo().QueueFamilyIndices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
    
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    vkCreateFence(m_Context->GetDevice(), &fenceInfo, nullptr, &m_Fence);
    vkResetFences(m_Context->GetDevice(), 1, &m_Fence);
}

} // namespace aero3d