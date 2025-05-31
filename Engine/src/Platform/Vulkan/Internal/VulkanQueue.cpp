#include "Platform/Vulkan/Internal/VulkanQueue.h"

#include "Platform/Vulkan/Internal/VulkanUtils.h"

namespace aero3d {

void VulkanQueue::Init(VkDevice device, uint32_t queueFamilyIndex)
{
    m_QueueFamilyIndex = queueFamilyIndex;
	m_Device = device;

    vkGetDeviceQueue(device, queueFamilyIndex, 0, &m_Queue);

	CreateFence(device, m_Fence);
}

void VulkanQueue::Shutdown()
{
	vkDestroyFence(m_Device, m_Fence, nullptr);
}

void VulkanQueue::SubmitSync(VkCommandBuffer* pCmdBuff)
{
	vkResetFences(m_Device, 1, &m_Fence);

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = VK_NULL_HANDLE;
	submitInfo.pWaitDstStageMask = VK_NULL_HANDLE;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = pCmdBuff;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = VK_NULL_HANDLE;

	A3D_CHECK_VKRESULT(vkQueueSubmit(m_Queue, 1, &submitInfo, m_Fence));

	vkWaitForFences(m_Device, 1, &m_Fence, VK_TRUE, UINT64_MAX);
}

void VulkanQueue::SubmitAsync(VkCommandBuffer* pCmdBuff, VkSemaphore* pWaitSem,
    VkSemaphore* pSigSem, VkFence pFence)
{
	VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = pWaitSem;
	submitInfo.pWaitDstStageMask = &waitFlags;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = pCmdBuff;
	submitInfo.signalSemaphoreCount = 1;			
	submitInfo.pSignalSemaphores = pSigSem;

	A3D_CHECK_VKRESULT(vkQueueSubmit(m_Queue, 1, &submitInfo, pFence));
}

void VulkanQueue::Present(VkSemaphore* pWaitSem, VkSwapchainKHR* pSwapchain, uint32_t imageIndex)
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = pWaitSem;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = pSwapchain;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(m_Queue, &presentInfo);
}

void VulkanQueue::WaitIdle() 
{
    A3D_CHECK_VKRESULT(vkQueueWaitIdle(m_Queue));
}

} // namespace aero3d