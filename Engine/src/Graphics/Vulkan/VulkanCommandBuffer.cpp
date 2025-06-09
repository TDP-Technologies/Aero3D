#include "Graphics/Vulkan/VulkanCommandBuffer.h"

#include "Graphics/Vulkan/VulkanResources.h"

namespace aero3d {

VulkanCommandBuffer::VulkanCommandBuffer(Ref<VulkanContext> context, Ref<VulkanViewport> viewport)
    : m_Context(context), m_Viewport(viewport)
{
    CreateCommandBuffers();
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

    vkResetCommandBuffer(m_GraphicsCB, 0);
    vkBeginCommandBuffer(m_GraphicsCB, &beginInfo);

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
        m_GraphicsCB,
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

    vkCmdBeginRenderingKHR(m_GraphicsCB, &renderingInfo);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width  = m_Viewport->GetExtent().width;
    viewport.height = m_Viewport->GetExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(m_GraphicsCB, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {m_Viewport->GetExtent().width, m_Viewport->GetExtent().height};

    vkCmdSetScissor(m_GraphicsCB, 0, 1, &scissor);
}

void VulkanCommandBuffer::End()
{
    vkCmdDraw(m_GraphicsCB, 3, 1, 0, 0);

    vkCmdEndRenderingKHR(m_GraphicsCB);

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
        m_GraphicsCB,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    vkEndCommandBuffer(m_GraphicsCB);
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
	submitInfo.pCommandBuffers = &m_GraphicsCB;
	submitInfo.signalSemaphoreCount = 1;			
	submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

	vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_Fence);

    vkWaitForFences(m_Context->GetDevice(), 1, &m_Fence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_Context->GetDevice(), 1, &m_Fence);
}

void VulkanCommandBuffer::BindBuffer(Ref<Buffer> buffer)
{
    Ref<VulkanBuffer> vulkanBuffer = std::static_pointer_cast<VulkanBuffer>(buffer);
    VkBuffer vkBuffer = vulkanBuffer->GetBuffer();
    if (vulkanBuffer->GetUsage() == Buffer::BufferType::VERTEX)
    {
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(m_GraphicsCB, 0, 1, &vkBuffer, offsets);
    }
    else
    {
        vkCmdBindIndexBuffer(m_GraphicsCB, vkBuffer, 0, vulkanBuffer->GetIndexType());
    }
}

void VulkanCommandBuffer::BindPipeline(Ref<Pipeline> pipeline)
{
    vkCmdBindPipeline(m_GraphicsCB, VK_PIPELINE_BIND_POINT_GRAPHICS,
        std::static_pointer_cast<VulkanPipeline>(pipeline)->GetPipeline());
}

Ref<Buffer> VulkanCommandBuffer::CreateBuffer(Buffer::Description desc)
{
    Ref<VulkanBuffer> buffer = std::make_shared<VulkanBuffer>(m_Context, desc);

    VkDeviceSize bufferSize = desc.Size;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(m_Context->GetDevice(), &bufferInfo, nullptr, &stagingBuffer);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_Context->GetDevice(), stagingBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = m_Context->FindMemoryType(memRequirements.memoryTypeBits, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkAllocateMemory(m_Context->GetDevice(), &allocInfo, nullptr, &stagingBufferMemory);

    vkBindBufferMemory(m_Context->GetDevice(), stagingBuffer, stagingBufferMemory, 0);

    void* data;
    vkMapMemory(m_Context->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, desc.Data, (size_t)bufferSize);
    vkUnmapMemory(m_Context->GetDevice(), stagingBufferMemory);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(m_TransferCB, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = bufferSize;
    vkCmdCopyBuffer(m_TransferCB, stagingBuffer, buffer->GetBuffer(), 1, &copyRegion);

    vkEndCommandBuffer(m_TransferCB);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_TransferCB;

    vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_GraphicsQueue);

    vkDestroyBuffer(m_Context->GetDevice(), stagingBuffer, nullptr);
    vkFreeMemory(m_Context->GetDevice(), stagingBufferMemory, nullptr);

    return buffer;
}

void VulkanCommandBuffer::CreateCommandBuffers()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_Context->GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(m_Context->GetDevice(), &allocInfo, &m_GraphicsCB);
    vkAllocateCommandBuffers(m_Context->GetDevice(), &allocInfo, &m_TransferCB);
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