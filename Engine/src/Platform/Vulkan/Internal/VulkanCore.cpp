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
}

VulkanCore::~VulkanCore()
{
}

bool VulkanCore::Init(SDL_Window* window, int width, int height)
{
    m_Window = window;

    if (!SDL_Vulkan_LoadLibrary(nullptr)) 
    {
        LogErr(ERROR_INFO, "Failed to load Vulkan library: %s", SDL_GetError());
        return false;
    }

    CreateInstance(&m_Instance);
    CreateSurface(m_Instance, m_Window, &m_Surface);

    m_Device.Init(m_Instance, m_Surface);

    m_GraphicsQueue.Init(m_Device.GetHandle(),
     m_Device.GetPhysicalDevice().QueueFamilyIndices.GraphicsFamily.value());

    m_PresentQueue.Init(m_Device.GetHandle(), 
    m_Device.GetPhysicalDevice().QueueFamilyIndices.PresentFamily.value());

    m_Swapchain.Init(m_Device.GetPhysicalDevice(), m_Surface, m_Window,
        m_Device.GetHandle(), width, height);

    CreateRenderPass(m_Device.GetHandle(), m_Swapchain.GetImageFormat(), &m_RenderPass);

    CreateFramebuffers();
    CreateCommandBuffersAndCommandPool();
    CreateSyncObjects();

    return true;
}

void VulkanCore::Shutdown()
{
    vkDeviceWaitIdle(m_Device.GetHandle());

    vkDestroyFence(m_Device.GetHandle(), m_InFlightFence, nullptr);
    vkDestroySemaphore(m_Device.GetHandle(), m_RenderFinishedSemaphore, nullptr);
    vkDestroySemaphore(m_Device.GetHandle(), m_ImageAvailableSemaphore, nullptr);

    vkDestroyCommandPool(m_Device.GetHandle(), m_CommandPool, nullptr);

    for (auto framebuffer : m_SwapchainFramebuffers) 
    {
        vkDestroyFramebuffer(m_Device.GetHandle(), framebuffer, nullptr);
    }

    vkDestroyRenderPass(m_Device.GetHandle(), m_RenderPass, nullptr);

    m_Swapchain.Shutdown();
    m_Device.Shutdown();

    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}

void VulkanCore::ResizeBuffers(int width, int height)
{
    vkDeviceWaitIdle(m_Device.GetHandle());
    for (auto framebuffer : m_SwapchainFramebuffers) 
    {
        vkDestroyFramebuffer(m_Device.GetHandle(), framebuffer, nullptr);
    }

    m_Swapchain.Recreate(m_Device.GetPhysicalDevice(), m_Surface, m_Window, 
        m_Device.GetHandle(), width, height);
    CreateFramebuffers();
}

void VulkanCore::RecordCommands()
{
    vkWaitForFences(m_Device.GetHandle(), 1, &m_InFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_Device.GetHandle(), 1, &m_InFlightFence);

    vkAcquireNextImageKHR(m_Device.GetHandle(), m_Swapchain.GetHandle(),
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
    renderPassInfo.renderArea.extent = m_Swapchain.GetExtent();

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &m_ClearColor;

    vkCmdBeginRenderPass(m_CommandBuffers[m_CurrentImage], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(m_CommandBuffers[m_CurrentImage], 0, 1, &m_Viewport);
    vkCmdSetScissor(m_CommandBuffers[m_CurrentImage], 0, 1, &m_Scissor);
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

    m_GraphicsQueue.Submit(&submitInfo, m_InFlightFence);

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = { m_Swapchain.GetHandle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &m_CurrentImage;

    vkQueuePresentKHR(m_PresentQueue.GetHandle(), &presentInfo);
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
        m_Swapchain.GetImage(m_CurrentImage),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        &m_ClearColor.color,
        1,
        &range
    );
}

void VulkanCore::CreateFramebuffers()
{
    m_SwapchainFramebuffers.resize(m_Swapchain.GetNumImageViews());

    VkExtent2D swapchainExtent = m_Swapchain.GetExtent();
        
    m_Viewport.width = static_cast<float>(swapchainExtent.width);
    m_Viewport.height = static_cast<float>(swapchainExtent.height);
    m_Scissor.extent.width = swapchainExtent.width;
    m_Scissor.extent.height = swapchainExtent.height;

    for (size_t i = 0; i < m_Swapchain.GetNumImageViews(); i++)
    {
        CreateFramebuffer(m_Device.GetHandle(), m_Swapchain.GetImageView(i), 
            swapchainExtent, m_RenderPass, &m_SwapchainFramebuffers[i]);
    }
}

void VulkanCore::CreateCommandBuffersAndCommandPool()
{
    CreateCommandPool(m_Device.GetHandle(), m_Device.GetPhysicalDevice().QueueFamilyIndices.GraphicsFamily.value(),
        &m_CommandPool);

    m_CommandBuffers.resize(m_SwapchainFramebuffers.size());

    CreateCommandBuffers(m_Device.GetHandle(), m_CommandPool, 
        static_cast<uint32_t>(m_CommandBuffers.size()), m_CommandBuffers.data());
}

void VulkanCore::CreateSyncObjects()
{

    CreateSemaphore(m_Device.GetHandle(), &m_ImageAvailableSemaphore);
    CreateSemaphore(m_Device.GetHandle(), &m_RenderFinishedSemaphore);
    CreateFence(m_Device.GetHandle(), &m_InFlightFence);
}

} // namespace aero3d