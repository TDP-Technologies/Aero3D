#include "Graphics/Vulkan/VulkanSwapchain.h"

#include "Graphics/Vulkan/VulkanGraphicsDevice.h"
#include "Graphics/Vulkan/VulkanUtils.h"

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

static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& format : availableFormats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }
    return availableFormats[0];
}

static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& mode : availablePresentModes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VulkanSwapchain::VulkanSwapchain(VulkanGraphicsDevice* gd)
{
    m_GraphicsDevice = gd;
    
    CreateLocks();
    Create();
}

VulkanSwapchain::~VulkanSwapchain()
{
    if (imageAvailableFence != VK_NULL_HANDLE)
    {
        vkDestroyFence(m_GraphicsDevice->device, imageAvailableFence, nullptr);
        imageAvailableFence = VK_NULL_HANDLE;
    }
    Destroy();
}

void VulkanSwapchain::Resize()
{
    Destroy();
    Create();
}

Ref<Framebuffer> VulkanSwapchain::GetFramebuffer()
{
    return frameBuffers[currentImageIndex];
}

uint32_t VulkanSwapchain::AcquireNextImage()
{
    VkResult result = vkAcquireNextImageKHR(m_GraphicsDevice->device, swapchain,
        UINT64_MAX, VK_NULL_HANDLE, imageAvailableFence, &currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        Resize();
    } 
    else
    {
        A3D_CHECK_VKRESULT(result);
    }

    A3D_CHECK_VKRESULT(vkWaitForFences(m_GraphicsDevice->device, 1, &imageAvailableFence, VK_TRUE, UINT64_MAX));
    A3D_CHECK_VKRESULT(vkResetFences(m_GraphicsDevice->device, 1, &imageAvailableFence));

    return currentImageIndex;
}

void VulkanSwapchain::CreateLocks()
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    A3D_CHECK_VKRESULT(vkCreateFence(m_GraphicsDevice->device, &fenceInfo, nullptr, &imageAvailableFence));
    A3D_CHECK_VKRESULT(vkResetFences(m_GraphicsDevice->device, 1, &imageAvailableFence));
}

void VulkanSwapchain::CreateSwapchain()
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_GraphicsDevice->physDevice, m_GraphicsDevice->surface);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
    extent = swapChainSupport.Capabilities.currentExtent;

    numImages = swapChainSupport.Capabilities.minImageCount + 1;
    if (swapChainSupport.Capabilities.maxImageCount > 0 &&
        numImages > swapChainSupport.Capabilities.maxImageCount)
    {
        numImages = swapChainSupport.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_GraphicsDevice->surface;

    createInfo.minImageCount = numImages;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = { m_GraphicsDevice->graphicsQueueIndex, m_GraphicsDevice->presentQueueIndex };

    if (queueFamilyIndices[0] != queueFamilyIndices[1])
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

    A3D_CHECK_VKRESULT(vkCreateSwapchainKHR(m_GraphicsDevice->device, &createInfo, nullptr, &swapchain));

    std::vector<VkImage> swapchainImages;
    A3D_CHECK_VKRESULT(vkGetSwapchainImagesKHR(m_GraphicsDevice->device, swapchain, &numImages, nullptr));
    swapchainImages.resize(numImages);
    A3D_CHECK_VKRESULT(vkGetSwapchainImagesKHR(m_GraphicsDevice->device, swapchain, &numImages, swapchainImages.data()));

    imageFormat = surfaceFormat.format;

    frames.clear();
    frames.resize(numImages);

    for (uint32_t i = 0; i < numImages; ++i)
    {
        TextureDesc desc{};
        desc.width = extent.width;
        desc.height = extent.height;
        desc.format = FromVkFormat(surfaceFormat.format);
        desc.usage = TextureUsage::RENDERTARGET;

        frames[i] = std::make_shared<VulkanTexture>(m_GraphicsDevice, desc, swapchainImages[i]);
    }

    TextureDesc depthDesc{};
    depthDesc.width = extent.width;
    depthDesc.height = extent.height;
    depthDesc.format = TextureFormat::D24S8;
    depthDesc.usage = TextureUsage::DEPTHSTENCIL;

    depthStencil = std::make_shared<VulkanTexture>(m_GraphicsDevice, depthDesc);
}

void VulkanSwapchain::CreateFramebuffer()
{
    frameBuffers.resize(numImages);

    for (size_t i = 0; i < frames.size(); i++) 
    {
        std::vector<Ref<Texture>> colorTargets = { frames[i] };
        FramebufferDesc desc;
        desc.colorTargets = colorTargets;
        desc.depthTarget = depthStencil;
        frameBuffers[i] = std::make_shared<VulkanFramebuffer>(m_GraphicsDevice, desc);
    }
}

void VulkanSwapchain::Create()
{
    CreateSwapchain();
    CreateFramebuffer();
    AcquireNextImage();
}

void VulkanSwapchain::Destroy()
{
    vkDeviceWaitIdle(m_GraphicsDevice->device);

    frameBuffers.clear();
    frames.clear();
    depthStencil = nullptr;

    if (swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(m_GraphicsDevice->device, swapchain, nullptr);
        swapchain = VK_NULL_HANDLE;
    }
}

} // namespace aero3d
