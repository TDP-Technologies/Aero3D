#include "Graphics/Vulkan/VulkanCommandBuffer.h"

namespace aero3d {

VulkanCommandBuffer::VulkanCommandBuffer(Ref<VulkanContext> context, Ref<VulkanViewport> viewport)
    : m_Context(context), m_Viewport(viewport)
{
    CreateCommandBuffer();
    CreateQueue();
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    vkDeviceWaitIdle(m_Context->GetDevice());
    vkDestroyFence(m_Context->GetDevice(), m_Fence, nullptr);
}

void VulkanCommandBuffer::Record()
{
    vkAcquireNextImageKHR(m_Context->GetDevice(), m_Viewport->GetSwapchain(),
        UINT64_MAX, m_Context->GetImageAvailableSemaphore(), VK_NULL_HANDLE, m_Context->GetCurrentImageAddress());

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    vkResetCommandBuffer(m_CommandBuffer, 0);
    vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_Viewport->GetImage(m_Context->GetCurrentImageIndex());
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    vkCmdPipelineBarrier(
        m_CommandBuffer,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    VkRenderingAttachmentInfo colorAttachment = {};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = m_Viewport->GetImageView(m_Context->GetCurrentImageIndex());
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = {0.2f, 0.8f, 0.1f, 1.0f};

    VkRenderingInfo renderingInfo = {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = {{0, 0}, m_Viewport->GetExtent()};
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vkCmdBeginRenderingKHR(m_CommandBuffer, &renderingInfo);
}

void VulkanCommandBuffer::End()
{
    vkCmdEndRenderingKHR(m_CommandBuffer);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_Viewport->GetImage(m_Context->GetCurrentImageIndex());
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;

    vkCmdPipelineBarrier(
        m_CommandBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    vkEndCommandBuffer(m_CommandBuffer);
}

void VulkanCommandBuffer::Execute()
{
    VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSemaphore imageAvailebleSemaphore = m_Context->GetImageAvailableSemaphore();
    VkSemaphore renderFinishedSemaphore = m_Context->GetRenderFinishedSemaphore();

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailebleSemaphore;
	submitInfo.pWaitDstStageMask = &waitFlags;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffer;
	submitInfo.signalSemaphoreCount = 1;			
	submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

	vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_Fence);

    vkWaitForFences(m_Context->GetDevice(), 1, &m_Fence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_Context->GetDevice(), 1, &m_Fence);
}

void VulkanCommandBuffer::CreateCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_Context->GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(m_Context->GetDevice(), &allocInfo, &m_CommandBuffer);
}

void VulkanCommandBuffer::CreateQueue()
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