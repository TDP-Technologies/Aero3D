#include "Platform/Vulkan/Internal/VulkanWrappers.h"

#include <vector>
#include <algorithm>
#include <set>

#include <SDL3/SDL_vulkan.h>

#include "Utils/Log.h"
#include "Utils/Assert.h"
#include "Utils/Common.h"
#include "Platform/Vulkan/Internal/VulkanUtils.h"

namespace aero3d {

static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.Formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.Formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.PresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.PresentModes.data());
    }

    return details;
}

void VulkanSwapchain::Init(const VulkanPhysicalDeviceInfo& physDevice, VkSurfaceKHR surface,
    SDL_Window* window, VkDevice device, int width, int height)
{
    m_Device = device;
    m_Surface = surface;
    m_Window = window;

    CreateSwapchain(physDevice, width, height);
    CreateImageViews();
}

void VulkanSwapchain::Shutdown()
{
    for (auto imageView : m_ImageViews)
    {
        vkDestroyImageView(m_Device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
}

void VulkanSwapchain::Recreate(const VulkanPhysicalDeviceInfo& physDevice, VkSurfaceKHR surface,
    SDL_Window* window, VkDevice device, int width, int height)
{
    Shutdown();
    Init(physDevice, surface, window, device, width, height);
}

void VulkanSwapchain::CreateSwapchain(const VulkanPhysicalDeviceInfo& physDevice, int width, int height)
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physDevice.Device, m_Surface);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
    VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

    m_NumImages = swapChainSupport.Capabilities.minImageCount + 1;
    if (swapChainSupport.Capabilities.maxImageCount > 0 &&
        m_NumImages > swapChainSupport.Capabilities.maxImageCount)
    {
        m_NumImages = swapChainSupport.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_Surface;

    createInfo.minImageCount = m_NumImages;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    DeviceQueueFamilyIndices indices = physDevice.QueueFamilyIndices;
    uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

    if (indices.GraphicsFamily != indices.PresentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    A3D_CHECK_VKRESULT(vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_Swapchain));

    vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &m_NumImages, nullptr);
    m_Images.resize(m_NumImages);
    vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &m_NumImages, m_Images.data());

    m_ImageFormat = surfaceFormat.format;
    m_Extent = extent;
}

void VulkanSwapchain::CreateImageViews()
{
    m_ImageViews.resize(m_NumImages);

    for (size_t i = 0; i < m_Images.size(); i++) 
    {
        CreateImageView(m_Device, m_Images[i], m_ImageFormat, m_ImageViews[i]);
    }
}

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