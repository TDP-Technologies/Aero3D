#include "Platform/Vulkan/Internal/VulkanCore.h"

#include <vector>
#include <algorithm>

#include <SDL3/SDL_vulkan.h>

#include "Utils/Log.h"
#include "Utils/Assert.h"
#include "Utils/Common.h"
#include "Platform/Vulkan/Internal/VulkanUtils.h"

namespace aero3d {

std::unique_ptr<VulkanCore> g_VulkanCore = std::make_unique<VulkanCore>();

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

static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, SDL_Window* window) 
{
    if (capabilities.currentExtent.width != UINT32_MAX) 
    {
        return capabilities.currentExtent;
    }
    else 
    {
        int width, height;
        SDL_GetWindowSizeInPixels(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width);

        actualExtent.height = std::clamp(actualExtent.height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VulkanCore::VulkanCore()
    : m_Window(nullptr), m_Instance(VK_NULL_HANDLE), m_Surface(VK_NULL_HANDLE),
    m_Swapchain(VK_NULL_HANDLE), m_CurrentPhysDevice(0), m_SwapchainImageFormat(VK_FORMAT_UNDEFINED),
    m_SwapchainExtent(0, 0), m_RenderPass(VK_NULL_HANDLE), m_CommandPool(VK_NULL_HANDLE),
    m_ImageAvailableSemaphore(VK_NULL_HANDLE), m_RenderFinishedSemaphore(VK_NULL_HANDLE), m_InFlightFence(VK_NULL_HANDLE)
{
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
    A3D_CHECK_INIT(CreatePhysicalDevices());
    A3D_CHECK_INIT(m_Device.Init());
    A3D_CHECK_INIT(CreateSwapchain());
    A3D_CHECK_INIT(CreateImageViews());
    A3D_CHECK_INIT(CreateRenderPass());
    A3D_CHECK_INIT(CreateFramebuffers());
    A3D_CHECK_INIT(CreateCommandBuffersAndCommandPool());
    A3D_CHECK_INIT(CreateSyncObjects());

    return true;
}

void VulkanCore::Shutdown()
{
    vkDestroyFence(m_Device.GetDevice(), m_InFlightFence, nullptr);
    vkDestroySemaphore(m_Device.GetDevice(), m_RenderFinishedSemaphore, nullptr);
    vkDestroySemaphore(m_Device.GetDevice(), m_ImageAvailableSemaphore, nullptr);
    vkDestroyCommandPool(m_Device.GetDevice(), m_CommandPool, nullptr);
    for (auto framebuffer : m_SwapchainFramebuffers) {
        vkDestroyFramebuffer(m_Device.GetDevice(), framebuffer, nullptr);
    }
    vkDestroyRenderPass(m_Device.GetDevice(), m_RenderPass, nullptr);
    for (auto imageView : m_SwapchainImageViews) {
        vkDestroyImageView(m_Device.GetDevice(), imageView, nullptr);
    }
    vkDestroySwapchainKHR(m_Device.GetDevice(), m_Swapchain, nullptr);
    m_Device.Shutdown();
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}

void VulkanCore::SwapBuffers()
{
    vkWaitForFences(m_Device.GetDevice(), 1, &m_InFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_Device.GetDevice(), 1, &m_InFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(m_Device.GetDevice(), m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    A3D_CHECK_VKRESULT(vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &submitInfo, m_InFlightFence));

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = { m_Swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(m_Device.GetPresentQueue(), &presentInfo);
}

bool VulkanCore::CreateInstance()
{
    uint32_t extensionCount;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    if (!extensions) {
        LogErr(ERROR_INFO, "Could not get Vulkan instance extensions: %s", SDL_GetError());
        return false;
    }

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
    createInfo.enabledLayerCount = 0;

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

bool VulkanCore::CreatePhysicalDevices()
{
    uint32_t physDeviceCount;
    A3D_CHECK_VKRESULT(vkEnumeratePhysicalDevices(m_Instance, &physDeviceCount, nullptr));

    if (!physDeviceCount)
    {
        LogErr(ERROR_INFO, "Failed to Enum PhysDevices.");
        return false;
    }

    std::vector<VkPhysicalDevice> vkDevices(physDeviceCount);
    m_PhysDevices.reserve(physDeviceCount);

    A3D_CHECK_VKRESULT(vkEnumeratePhysicalDevices(m_Instance, &physDeviceCount, vkDevices.data()));

    for (auto& vkDevice : vkDevices) {
        VulkanPhysicalDevice physDev{};
        physDev.Device = vkDevice;

        vkGetPhysicalDeviceProperties(vkDevice, &physDev.Properties);
        vkGetPhysicalDeviceFeatures(vkDevice, &physDev.Features);
        vkGetPhysicalDeviceMemoryProperties(vkDevice, &physDev.MemoryProperties);

        uint32_t queueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vkDevice, &queueCount, nullptr);

        physDev.QueueFamilyProperties.resize(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(vkDevice, &queueCount, physDev.QueueFamilyProperties.data());

        for (uint32_t i = 0; i < physDev.QueueFamilyProperties.size(); i++)
        {
            if (physDev.QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) 
            {
                physDev.QueueFamilyIndices.GraphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(vkDevice, i, m_Surface, &presentSupport);

            if (presentSupport) 
            {
                physDev.QueueFamilyIndices.PresentFamily = i;
            }
        }

        if (physDev.QueueFamilyIndices.IsComplete())
        {
            m_PhysDevices.emplace_back(physDev);
        }
    }

    Assert(ERROR_INFO, !m_PhysDevices.empty(), "Unnable to Find Suitable Physical Device.");

    return true;
}

bool VulkanCore::CreateSwapchain()
{
    VulkanPhysicalDevice& physDevice = m_PhysDevices[m_CurrentPhysDevice];
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physDevice.Device, m_Surface);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.Capabilities, m_Window);

    uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
    if (swapChainSupport.Capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.Capabilities.maxImageCount) 
    {
        imageCount = swapChainSupport.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_Surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = physDevice.QueueFamilyIndices;
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

    A3D_CHECK_VKRESULT(vkCreateSwapchainKHR(m_Device.GetDevice(), &createInfo, nullptr, &m_Swapchain));
    if (!m_Swapchain)
    {
        LogErr(ERROR_INFO, "Failed to create Swapchain.");
        return false;
    }

    vkGetSwapchainImagesKHR(m_Device.GetDevice(), m_Swapchain, &imageCount, nullptr);
    m_SwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_Device.GetDevice(), m_Swapchain, &imageCount, m_SwapchainImages.data());

    m_SwapchainImageFormat = surfaceFormat.format;
    m_SwapchainExtent = extent;

    return true;
}

bool VulkanCore::CreateImageViews()
{
    m_SwapchainImageViews.resize(m_SwapchainImages.size());

    for (size_t i = 0; i < m_SwapchainImages.size(); i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_SwapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_SwapchainImageFormat;

        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        A3D_CHECK_VKRESULT(vkCreateImageView(m_Device.GetDevice(), &viewInfo, nullptr, &m_SwapchainImageViews[i]));
    }

    return true;
}

bool VulkanCore::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_SwapchainImageFormat;
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

    A3D_CHECK_VKRESULT(vkCreateRenderPass(m_Device.GetDevice(), &renderPassInfo, nullptr, &m_RenderPass));
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

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_RenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_SwapchainExtent.width;
        framebufferInfo.height = m_SwapchainExtent.height;
        framebufferInfo.layers = 1;

        A3D_CHECK_VKRESULT(vkCreateFramebuffer(m_Device.GetDevice(), &framebufferInfo, nullptr, &m_SwapchainFramebuffers[i]));
    }

    return true;
}

bool VulkanCore::CreateCommandBuffersAndCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_PhysDevices[m_CurrentPhysDevice].QueueFamilyIndices.GraphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    A3D_CHECK_VKRESULT(vkCreateCommandPool(m_Device.GetDevice(), &poolInfo, nullptr, &m_CommandPool));
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

    A3D_CHECK_VKRESULT(vkAllocateCommandBuffers(m_Device.GetDevice(), &allocInfo, m_CommandBuffers.data()));

    for (size_t i = 0; i < m_CommandBuffers.size(); ++i) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        A3D_CHECK_VKRESULT(vkBeginCommandBuffer(m_CommandBuffers[i], &beginInfo));

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_RenderPass;
        renderPassInfo.framebuffer = m_SwapchainFramebuffers[i];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_SwapchainExtent;

        VkClearValue clearColor = { {{0.0f, 1.0f, 0.0f, 1.0f}} };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdEndRenderPass(m_CommandBuffers[i]);

        A3D_CHECK_VKRESULT(vkEndCommandBuffer(m_CommandBuffers[i]));
    }

    return true;
}

bool VulkanCore::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    A3D_CHECK_VKRESULT(vkCreateSemaphore(m_Device.GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore));
    A3D_CHECK_VKRESULT(vkCreateSemaphore(m_Device.GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore));
    A3D_CHECK_VKRESULT(vkCreateFence(m_Device.GetDevice(), &fenceInfo, nullptr, &m_InFlightFence));

    return true;
}

} // namespace aero3d