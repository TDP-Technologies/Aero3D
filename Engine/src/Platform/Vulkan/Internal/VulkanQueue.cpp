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

void VulkanQueue::SubmitSync(VkCommandBuffer* pCmdBuff)
{
	VkSubmitInfo SubmitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = 0,
		.pWaitSemaphores = VK_NULL_HANDLE,
		.pWaitDstStageMask = VK_NULL_HANDLE,
		.commandBufferCount = 1,
		.pCommandBuffers = pCmdBuff,
		.signalSemaphoreCount = 0,
		.pSignalSemaphores = VK_NULL_HANDLE
	};

	A3D_CHECK_VKRESULT(vkQueueSubmit(m_Queue, 1, &SubmitInfo, nullptr));
}

void VulkanQueue::SubmitAsync(VkCommandBuffer* pCmdBuff, VkSemaphore* pWaitSem,
    VkSemaphore* pSigSem, VkFence pFence)
{
	VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo SubmitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = pWaitSem,
		.pWaitDstStageMask = &waitFlags,
		.commandBufferCount = 1,
		.pCommandBuffers = pCmdBuff,
		.signalSemaphoreCount = 1,				
		.pSignalSemaphores = pSigSem		
	};

	A3D_CHECK_VKRESULT(vkQueueSubmit(m_Queue, 1, &SubmitInfo, pFence));
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