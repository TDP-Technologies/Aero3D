#include "Graphics/Vulkan/VulkanCommandBuffer.h"

namespace aero3d {

VulkanCommandBuffer::VulkanCommandBuffer(Ref<VulkanContext> context, Ref<VulkanViewport> viewport)
    : m_Context(context), m_Viewport(viewport)
{
    CreateCommandBuffer();
    CreateRenderPass();
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    vkDeviceWaitIdle(m_Context->GetDevice());

    vkDestroyRenderPass(m_Context->GetDevice(), m_RenderPass, nullptr);
    for (auto& frameBuffer : m_Framebuffers)
    {
        vkDestroyFramebuffer(m_Context->GetDevice(), frameBuffer, nullptr);
    }
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

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_RenderPass;
    renderPassInfo.framebuffer = m_Framebuffers[m_Context->GetCurrentImageIndex()];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_Viewport->GetExtent();

    VkClearValue clearColor { {0.0f, 1.0f, 0.0f, 1.0f} };

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(m_CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanCommandBuffer::End()
{
    vkCmdEndRenderPass(m_CommandBuffer);
    vkEndCommandBuffer(m_CommandBuffer);
}

void VulkanCommandBuffer::Clear()
{

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

void VulkanCommandBuffer::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_Viewport->GetFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    vkCreateRenderPass(m_Context->GetDevice(), &renderPassInfo, nullptr, &m_RenderPass);

    m_Framebuffers.resize(m_Viewport->GetNumImages());
    for (int i = 0; i < m_Viewport->GetNumImages(); i++)
    {
        VkImageView attachments[] = 
        {
            m_Viewport->GetImageView(i)
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_RenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_Viewport->GetExtent().width;
        framebufferInfo.height = m_Viewport->GetExtent().height;
        framebufferInfo.layers = 1;

        vkCreateFramebuffer(m_Context->GetDevice(), &framebufferInfo, nullptr, &m_Framebuffers[i]);
    }
}

} // namespace aero3d