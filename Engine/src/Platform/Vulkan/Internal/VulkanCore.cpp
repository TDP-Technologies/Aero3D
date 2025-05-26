#include "Platform/Vulkan/Internal/VulkanCore.h"

#include <vector>
#include <algorithm>

#include <SDL3/SDL_vulkan.h>

#include "Utils/Log.h"
#include "Utils/Assert.h"
#include "Utils/Common.h"
#include "Platform/Vulkan/Internal/VulkanUtils.h"

namespace aero3d {

Scope<VulkanCore> g_VulkanCore = std::make_unique<VulkanCore>();

VulkanCore::VulkanCore()
{
    m_Device = std::make_shared<VulkanDevice>();
    m_Swapchain = std::make_shared<VulkanSwapchain>();
}

VulkanCore::~VulkanCore()
{
}

bool VulkanCore::Init(SDL_Window* window)
{
    m_Window = window;

    if (!SDL_Vulkan_LoadLibrary(nullptr)) {
        LogErr(ERROR_INFO, "Failed to load Vulkan library: %s", SDL_GetError());
        return false;
    }

    A3D_CHECK_INIT(CreateInstance());
    A3D_CHECK_INIT(CreateSurface());
    A3D_CHECK_INIT(m_Device->Init(m_Instance, m_Surface));
    m_GraphicsQueue = m_Device->GetGraphicsQueue();
    m_PresentQueue = m_Device->GetPresentQueue();
    A3D_CHECK_INIT(m_Swapchain->Init(m_Device->GetPhysicalDevice(), m_Surface, m_Window, m_Device->GetHandle()));
    A3D_CHECK_INIT(CreateImageViews());
    A3D_CHECK_INIT(CreateRenderPass());
    A3D_CHECK_INIT(CreateFramebuffers());
    A3D_CHECK_INIT(CreateCommandBuffersAndCommandPool());
    A3D_CHECK_INIT(CreateSyncObjects());

    return true;
}

void VulkanCore::Shutdown()
{
    vkDeviceWaitIdle(m_Device->GetHandle());

    vkDestroyFence(m_Device->GetHandle(), m_InFlightFence, nullptr);
    vkDestroySemaphore(m_Device->GetHandle(), m_RenderFinishedSemaphore, nullptr);
    vkDestroySemaphore(m_Device->GetHandle(), m_ImageAvailableSemaphore, nullptr);

    vkDestroyCommandPool(m_Device->GetHandle(), m_CommandPool, nullptr);

    for (auto framebuffer : m_SwapchainFramebuffers) {
        vkDestroyFramebuffer(m_Device->GetHandle(), framebuffer, nullptr);
    }

    vkDestroyRenderPass(m_Device->GetHandle(), m_RenderPass, nullptr);
    for (auto imageView : m_SwapchainImageViews) {
        vkDestroyImageView(m_Device->GetHandle(), imageView, nullptr);
    }

    A3D_SHUTDOWN(m_Swapchain);
    A3D_SHUTDOWN(m_Device);

    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}

void VulkanCore::SwapBuffers()
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentImage];

    VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    m_GraphicsQueue->Submit(&submitInfo, m_InFlightFence);

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = { m_Swapchain->GetHandle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &m_CurrentImage;

    vkQueuePresentKHR(m_PresentQueue->GetHandle(), &presentInfo);
}

void VulkanCore::SetViewport(int x, int y, int width, int height)
{
    m_Viewport.x = static_cast<float>(x);
    m_Viewport.y = static_cast<float>(y);

    m_Viewport.width = static_cast<float>(width);
    m_Viewport.height = static_cast<float>(height);
}

void VulkanCore::SetClearColor(float r, float g, float b, float a)
{
    m_ClearColor.color = { r, g, b, a };
}

void VulkanCore::Clear()
{
    VkImageSubresourceRange range = {};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;

    vkCmdClearColorImage(
        m_CommandBuffers[m_CurrentImage],
        m_Swapchain->GetSwapchainImages()[m_CurrentImage],
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        &m_ClearColor.color,
        1,
        &range
    );
}

void VulkanCore::RecordCommands()
{
    vkWaitForFences(m_Device->GetHandle(), 1, &m_InFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_Device->GetHandle(), 1, &m_InFlightFence);

    vkAcquireNextImageKHR(m_Device->GetHandle(), m_Swapchain->GetHandle(),
        UINT64_MAX, m_ImageAvailableSemaphore, VK_NULL_HANDLE, &m_CurrentImage);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    vkResetCommandBuffer(m_CommandBuffers[m_CurrentImage], 0);
    vkBeginCommandBuffer(m_CommandBuffers[m_CurrentImage], &beginInfo);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_RenderPass;
    renderPassInfo.framebuffer = m_SwapchainFramebuffers[m_CurrentImage];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_Swapchain->GetExtent();

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &m_ClearColor;

    vkCmdBeginRenderPass(m_CommandBuffers[m_CurrentImage], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(m_CommandBuffers[m_CurrentImage], 0, 1, &m_Viewport);
}

void VulkanCore::EndCommands()
{
    vkCmdEndRenderPass(m_CommandBuffers[m_CurrentImage]);
    vkEndCommandBuffer(m_CommandBuffers[m_CurrentImage]);
}

void VulkanCore::Draw(size_t count)
{
    vkCmdDraw(m_CommandBuffers[m_CurrentImage], count, 1, 0, 0);
}

void VulkanCore::DrawIndexed(size_t count)
{
    vkCmdDrawIndexed(m_CommandBuffers[m_CurrentImage], count, 1, 0, 0, 0);
}

bool VulkanCore::CreateInstance()
{
    uint32_t extensionCount;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    if (!extensions) {
        LogErr(ERROR_INFO, "Could not get Vulkan instance extensions: %s", SDL_GetError());
        return false;
    }

    const char* validationLayers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Aero3D";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = validationLayers;

    A3D_CHECK_VKRESULT(vkCreateInstance(&createInfo, nullptr, &m_Instance));

    return true;
}

bool VulkanCore::CreateSurface()
{
    if (!SDL_Vulkan_CreateSurface(m_Window, m_Instance, nullptr, &m_Surface)) {
        LogErr(ERROR_INFO, "Failed to create Vulkan surface: %s", SDL_GetError());
        return false;
    }

    return true;
}

bool VulkanCore::CreateImageViews()
{
    std::vector<VkImage> swapchainImages = m_Swapchain->GetSwapchainImages();
    m_SwapchainImageViews.resize(swapchainImages.size());

    for (size_t i = 0; i < swapchainImages.size(); i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_Swapchain->GetImageFormat();

        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        A3D_CHECK_VKRESULT(vkCreateImageView(m_Device->GetHandle(), &viewInfo, nullptr, &m_SwapchainImageViews[i]));
    }

    return true;
}

bool VulkanCore::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_Swapchain->GetImageFormat();
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

    A3D_CHECK_VKRESULT(vkCreateRenderPass(m_Device->GetHandle(), &renderPassInfo, nullptr, &m_RenderPass));
    if (!m_RenderPass)
    {
        LogErr(ERROR_INFO, "Failed to create RenderPass.");
        return false;
    }

    return true;
}

bool VulkanCore::CreateFramebuffers()
{
    m_SwapchainFramebuffers.resize(m_SwapchainImageViews.size());

    for (size_t i = 0; i < m_SwapchainImageViews.size(); i++) {
        VkImageView attachments[] = {
            m_SwapchainImageViews[i]
        };

        VkExtent2D swapchainExtent = m_Swapchain->GetExtent();
        
        m_Viewport.width = static_cast<float>(swapchainExtent.width);
        m_Viewport.height = static_cast<float>(swapchainExtent.height);

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_RenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
        framebufferInfo.layers = 1;

        A3D_CHECK_VKRESULT(vkCreateFramebuffer(m_Device->GetHandle(), &framebufferInfo, nullptr, &m_SwapchainFramebuffers[i]));
    }

    return true;
}

bool VulkanCore::CreateCommandBuffersAndCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_Device->GetPhysicalDevice().QueueFamilyIndices.GraphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    A3D_CHECK_VKRESULT(vkCreateCommandPool(m_Device->GetHandle(), &poolInfo, nullptr, &m_CommandPool));
    if (!m_CommandPool)
    {
        LogErr(ERROR_INFO, "Failed to create Command Pool.");
        return false;
    }

    m_CommandBuffers.resize(m_SwapchainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

    A3D_CHECK_VKRESULT(vkAllocateCommandBuffers(m_Device->GetHandle(), &allocInfo, m_CommandBuffers.data()));

    return true;
}

bool VulkanCore::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    A3D_CHECK_VKRESULT(vkCreateSemaphore(m_Device->GetHandle(), &semaphoreInfo, nullptr,
        &m_ImageAvailableSemaphore));
    A3D_CHECK_VKRESULT(vkCreateSemaphore(m_Device->GetHandle(), &semaphoreInfo, nullptr,
        &m_RenderFinishedSemaphore));
    A3D_CHECK_VKRESULT(vkCreateFence(m_Device->GetHandle(), &fenceInfo, nullptr,
        &m_InFlightFence));

    return true;
}

} // namespace aero3d