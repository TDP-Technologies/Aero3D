#include "Graphics/Vulkan/VulkanCommandList.h"

#include "Graphics/Vulkan/VulkanGraphicsDevice.h"
#include "Graphics/Vulkan/VulkanUtils.h"

namespace aero3d {

VulkanCommandList::VulkanCommandList(VulkanGraphicsDevice* gd)
{
    m_GraphicsDevice = gd;

    CreateCommandPool();
    CreateCommandList();
}

VulkanCommandList::~VulkanCommandList()
{
    vkDeviceWaitIdle(m_GraphicsDevice->device);
    if (m_CommandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(m_GraphicsDevice->device, m_CommandPool, nullptr);
        m_CommandPool = VK_NULL_HANDLE;
    }
}

void VulkanCommandList::Begin()
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;

    A3D_CHECK_VKRESULT(vkResetCommandBuffer(commandBuffer, 0));
    A3D_CHECK_VKRESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_GraphicsDevice->swapchain->extent.width);
    viewport.height = static_cast<float>(m_GraphicsDevice->swapchain->extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = { m_GraphicsDevice->swapchain->extent };

    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void VulkanCommandList::End()
{
    if (m_CurrentFramebuffer)
    {
        EndRendering();

        m_CurrentFramebuffer = nullptr;
    }

    A3D_CHECK_VKRESULT(vkEndCommandBuffer(commandBuffer));
}

void VulkanCommandList::SetFramebuffer(Ref<Framebuffer> framebuffer)
{
    Ref<VulkanFramebuffer> vfb = std::static_pointer_cast<VulkanFramebuffer>(framebuffer);

    if (m_CurrentFramebuffer != vfb && m_CurrentFramebuffer != nullptr)
    {
        EndRendering();
    }

    m_CurrentFramebuffer = vfb;

    BeginRendering();
}

void VulkanCommandList::SetPipeline(Ref<Pipeline> pipeline)
{
    Ref<VulkanPipeline> vulkanPipeline = std::static_pointer_cast<VulkanPipeline>(pipeline);
    m_CurrentPipeline = vulkanPipeline;
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->pipeline);
}

void VulkanCommandList::SetVertexBuffer(Ref<DeviceBuffer> buffer, uint32_t offset)
{
    Ref<VulkanDeviceBuffer> vulkanDeviceVulkan = std::static_pointer_cast<VulkanDeviceBuffer>(buffer);
    VkDeviceSize offsets[] = { offset };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vulkanDeviceVulkan->buffer, offsets);
}

static VkIndexType IndexFormatToVkIndexType(IndexFormat format)
{
    switch (format)
    {
        case IndexFormat::UnsignedShort: return VK_INDEX_TYPE_UINT16;
        case IndexFormat::UnsignedInt: return VK_INDEX_TYPE_UINT32;
        default: return VK_INDEX_TYPE_UINT32;
    }
}

void VulkanCommandList::SetIndexBuffer(Ref<DeviceBuffer> buffer, IndexFormat format, uint32_t offset)
{
    Ref<VulkanDeviceBuffer> vulkanDeviceVulkan = std::static_pointer_cast<VulkanDeviceBuffer>(buffer);

    vkCmdBindIndexBuffer(commandBuffer, vulkanDeviceVulkan->buffer, offset, IndexFormatToVkIndexType(format));
}

void VulkanCommandList::SetResourceSet(uint32_t slot, Ref<ResourceSet> resourceSet)
{
    Ref<VulkanResourceSet> vulkanResourceSet = std::static_pointer_cast<VulkanResourceSet>(resourceSet);

    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_CurrentPipeline->pipelineLayout,
        slot,
        1,
        &vulkanResourceSet->descriptorSet,
        0, nullptr
    );
}

void VulkanCommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, 
    uint32_t firstVertex, uint32_t firstInstance)
{
    vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount,
    uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandList::ClearRenderTargets(float r, float g, float b, float a)
{
    std::vector<VkClearAttachment> clearAttachments;

    for (uint32_t i = 0; i < m_CurrentFramebuffer->imageViews.size(); ++i)
    {
        VkClearAttachment clearAttachment{};
        clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        clearAttachment.colorAttachment = i;
        clearAttachment.clearValue.color = {{r, g, b, a}};
        clearAttachments.push_back(clearAttachment);
    }

    VkClearRect clearRect{};
    clearRect.rect.offset = {0, 0};
    clearRect.rect.extent = m_CurrentFramebuffer->renderArea;
    clearRect.baseArrayLayer = 0;
    clearRect.layerCount = 1;

    vkCmdClearAttachments(commandBuffer, clearAttachments.size(), clearAttachments.data(), 1, &clearRect);
}

void VulkanCommandList::ClearDepthStencil()
{
    VkClearAttachment clearAttachment{};
    clearAttachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    clearAttachment.clearValue.depthStencil = {1.0f, 0};

    VkClearRect clearRect{};
    clearRect.rect.offset = {0, 0};
    clearRect.rect.extent = m_CurrentFramebuffer->renderArea;
    clearRect.baseArrayLayer = 0;
    clearRect.layerCount = 1;

    vkCmdClearAttachments(commandBuffer, 1, &clearAttachment, 1, &clearRect);
}

void VulkanCommandList::CreateCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_GraphicsDevice->graphicsQueueIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    A3D_CHECK_VKRESULT(vkCreateCommandPool(m_GraphicsDevice->device, &poolInfo, nullptr, &m_CommandPool));
}

void VulkanCommandList::CreateCommandList()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    A3D_CHECK_VKRESULT(vkAllocateCommandBuffers(m_GraphicsDevice->device, &allocInfo, &commandBuffer));
}

void VulkanCommandList::BeginRendering()
{
    std::vector<VkRenderingAttachmentInfo> colorAttachments;
    colorAttachments.resize(m_CurrentFramebuffer->frames.size());

    for (int i = 0; i < m_CurrentFramebuffer->frames.size(); i++)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_CurrentFramebuffer->frames[i]->image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        VkRenderingAttachmentInfo colorAttachment = {};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = m_CurrentFramebuffer->imageViews[i];
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        colorAttachments[i] = colorAttachment;
    }

    VkRenderingInfo renderingInfo = {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = {{0, 0}, m_CurrentFramebuffer->renderArea };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = colorAttachments.size();
    renderingInfo.pColorAttachments = colorAttachments.data();

    VkRenderingAttachmentInfo depthAttachment = {};
    if (m_CurrentFramebuffer->depthStencil)
    {
        VkImageMemoryBarrier depthBarrier{};
        depthBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        depthBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        depthBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        depthBarrier.image = m_CurrentFramebuffer->depthStencil->image;
        depthBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        depthBarrier.subresourceRange.baseMipLevel = 0;
        depthBarrier.subresourceRange.levelCount = 1;
        depthBarrier.subresourceRange.baseArrayLayer = 0;
        depthBarrier.subresourceRange.layerCount = 1;
        depthBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        depthBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &depthBarrier);

        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = m_CurrentFramebuffer->depthStencilImageView;
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        renderingInfo.pDepthAttachment = &depthAttachment;
    }

    vkCmdBeginRenderingKHR(commandBuffer, &renderingInfo);
}

void VulkanCommandList::EndRendering()
{
    vkCmdEndRenderingKHR(commandBuffer);

    for (int i = 0; i < m_CurrentFramebuffer->frames.size(); i++)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_CurrentFramebuffer->frames[i]->image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    if (m_CurrentFramebuffer->depthStencil)
    {
        VkImageMemoryBarrier depthBarrierEnd{};
        depthBarrierEnd.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        depthBarrierEnd.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthBarrierEnd.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthBarrierEnd.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        depthBarrierEnd.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        depthBarrierEnd.image = m_CurrentFramebuffer->depthStencil->image;
        depthBarrierEnd.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        depthBarrierEnd.subresourceRange.baseMipLevel = 0;
        depthBarrierEnd.subresourceRange.levelCount = 1;
        depthBarrierEnd.subresourceRange.baseArrayLayer = 0;
        depthBarrierEnd.subresourceRange.layerCount = 1;
        depthBarrierEnd.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        depthBarrierEnd.dstAccessMask = 0;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &depthBarrierEnd);
    }

    m_CurrentFramebuffer = nullptr;
}

} // namespace aero3d
