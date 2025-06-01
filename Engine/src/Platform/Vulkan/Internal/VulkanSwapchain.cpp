#include "Platform/Vulkan/Internal/VulkanSwapchain.h"

#include <algorithm>

#include "Utils/Log.h"
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

void VulkanSwapchain::Init(const VulkanPhysicalDevice& physDevice, VkSurfaceKHR surface,
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

void VulkanSwapchain::Recreate(const VulkanPhysicalDevice& physDevice, VkSurfaceKHR surface,
    SDL_Window* window, VkDevice device, int width, int height)
{
    Shutdown();
    Init(physDevice, surface, window, device, width, height);
}

void VulkanSwapchain::CreateSwapchain(const VulkanPhysicalDevice& physDevice, int width, int height)
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physDevice.Device, m_Surface);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
    VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

    m_NumFrames = swapChainSupport.Capabilities.minImageCount + 1;
    if (swapChainSupport.Capabilities.maxImageCount > 0 &&
        m_NumFrames > swapChainSupport.Capabilities.maxImageCount)
    {
        m_NumFrames = swapChainSupport.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_Surface;

    createInfo.minImageCount = m_NumFrames;
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

    vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &m_NumFrames, nullptr);
    m_Images.resize(m_NumFrames);
    vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &m_NumFrames, m_Images.data());

    m_ImageFormat = surfaceFormat.format;
    m_Extent = extent;
}

void VulkanSwapchain::CreateImageViews()
{
    m_ImageViews.resize(m_NumFrames);

    for (size_t i = 0; i < m_Images.size(); i++) 
    {
        CreateImageView(m_Device, m_Images[i], m_ImageFormat, m_ImageViews[i]);
    }
}

} // namespace aero3d