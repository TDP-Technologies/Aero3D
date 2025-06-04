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

Scope<VulkanCore> g_VulkanCore = std::make_unique<VulkanCore>();

void VulkanDevice::Init(VkInstance instance, VkSurfaceKHR surface)
{
    m_Instance = instance;

    SearchForPhysDevices(surface);
    CreateLogicalDevice();
}

void VulkanDevice::Shutdown()
{
    vkDestroyDevice(m_Device, nullptr);
}

void VulkanDevice::SearchForPhysDevices(VkSurfaceKHR surface)
{
    uint32_t physDeviceCount;
    A3D_CHECK_VKRESULT(vkEnumeratePhysicalDevices(m_Instance, &physDeviceCount, nullptr));

    std::vector<VkPhysicalDevice> vkDevices(physDeviceCount);
    m_PhysDevices.reserve(physDeviceCount);

    A3D_CHECK_VKRESULT(vkEnumeratePhysicalDevices(m_Instance, &physDeviceCount, vkDevices.data()));

    for (auto& vkDevice : vkDevices) 
    {
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
            const auto& queueFamily = physDev.QueueFamilyProperties[i];

            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                physDev.QueueFamilyIndices.GraphicsFamily = i;
            }

            if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                physDev.QueueFamilyIndices.TransferFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(vkDevice, i, surface, &presentSupport);

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
}

void VulkanDevice::CreateLogicalDevice()
{
    VulkanPhysicalDevice& physDevice = m_PhysDevices[m_CurrentPhysDevice];

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = 
    {
        physDevice.QueueFamilyIndices.TransferFamily.value(),
        physDevice.QueueFamilyIndices.GraphicsFamily.value(), 
        physDevice.QueueFamilyIndices.PresentFamily.value()
    };

    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) 
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    const std::vector<const char*> deviceExtensions = 
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    createInfo.enabledLayerCount = 0;

    A3D_CHECK_VKRESULT(vkCreateDevice(physDevice.Device, &createInfo, nullptr, &m_Device));
}

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

bool VulkanCore::Init(SDL_Window* window, int width, int height)
{
    m_Window = window;

    if (!SDL_Vulkan_LoadLibrary(nullptr)) 
    {
        LogErr(ERROR_INFO, "Failed to load Vulkan library: %s", SDL_GetError());
        return false;
    }

    CreateInstance(m_Instance);
    CreateSurface(m_Instance, m_Window, m_Surface);

    m_Device.Init(m_Instance, m_Surface);

    m_TransferQueue.Init(m_Device.GetHandle(),
        m_Device.GetPhysicalDevice().QueueFamilyIndices.TransferFamily.value());

    m_GraphicsQueue.Init(m_Device.GetHandle(),
        m_Device.GetPhysicalDevice().QueueFamilyIndices.GraphicsFamily.value());

    m_PresentQueue.Init(m_Device.GetHandle(), 
        m_Device.GetPhysicalDevice().QueueFamilyIndices.PresentFamily.value());

    m_Swapchain.Init(m_Device.GetPhysicalDevice(), m_Surface, m_Window,
        m_Device.GetHandle(), width, height);

    CreateRenderPass(m_Device.GetHandle(), m_Swapchain.GetImageFormat(), m_RenderPass);

    CreateFramebuffers();
    CreateCommandBuffersAndCommandPools();
    CreateSyncObjects();
    CreateDescriptors();
    CreatePipelineLayout();

    return true;
}

void VulkanCore::Shutdown()
{
    vkDeviceWaitIdle(m_Device.GetHandle());

    vkDestroyPipelineLayout(m_Device.GetHandle(), m_PipelineLayout, nullptr);

    m_DescriptorSetLayout.Shutdown();
    m_DescriptorPool.Shutdown();
    
    for (int i = 0; i < m_Swapchain.GetNumImages(); i++)
    {
        vkDestroyFence(m_Device.GetHandle(), m_InFlightFences[i], nullptr);
        vkDestroySemaphore(m_Device.GetHandle(), m_RenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_Device.GetHandle(), m_ImageAvailableSemaphores[i], nullptr);
    }

    vkDestroyCommandPool(m_Device.GetHandle(), m_GraphicsCommandPool, nullptr);
    vkDestroyCommandPool(m_Device.GetHandle(), m_CopyCommandPool, nullptr);

    for (auto& framebuffer : m_SwapchainFramebuffers) 
    {
        vkDestroyFramebuffer(m_Device.GetHandle(), framebuffer, nullptr);
    }

    vkDestroyRenderPass(m_Device.GetHandle(), m_RenderPass, nullptr);

    m_PresentQueue.Shutdown();
    m_GraphicsQueue.Shutdown();
    m_TransferQueue.Shutdown();

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
    vkWaitForFences(m_Device.GetHandle(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(m_Device.GetHandle(), 1, &m_InFlightFences[m_CurrentFrame]);

    vkAcquireNextImageKHR(m_Device.GetHandle(), m_Swapchain.GetHandle(),
        UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &m_CurrentImage);

    BeginCommandBuffer(m_GraphicsCommandBuffers[m_CurrentFrame]);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_RenderPass;
    renderPassInfo.framebuffer = m_SwapchainFramebuffers[m_CurrentImage];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_Swapchain.GetExtent();

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &m_ClearColor;

    vkCmdBeginRenderPass(m_GraphicsCommandBuffers[m_CurrentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(m_GraphicsCommandBuffers[m_CurrentFrame], 0, 1, &m_Viewport);
    vkCmdSetScissor(m_GraphicsCommandBuffers[m_CurrentFrame], 0, 1, &m_Scissor);

    vkCmdBindDescriptorSets(m_GraphicsCommandBuffers[m_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, 
        m_PipelineLayout, 0, 1, &m_DescriptorSets[m_CurrentFrame], 0, nullptr);
}

void VulkanCore::EndCommands()
{
    vkCmdEndRenderPass(m_GraphicsCommandBuffers[m_CurrentFrame]);
    vkEndCommandBuffer(m_GraphicsCommandBuffers[m_CurrentFrame]);
}

void VulkanCore::Draw(size_t count)
{
    vkCmdDraw(m_GraphicsCommandBuffers[m_CurrentFrame], count, 1, 0, 0);
}

void VulkanCore::DrawIndexed(size_t count)
{
    vkCmdDrawIndexed(m_GraphicsCommandBuffers[m_CurrentFrame], count, 1, 0, 0, 0);
}

void VulkanCore::SwapBuffers()
{
    m_GraphicsQueue.SubmitAsync(&m_GraphicsCommandBuffers[m_CurrentFrame], &m_ImageAvailableSemaphores[m_CurrentFrame],
        &m_RenderFinishedSemaphores[m_CurrentFrame], m_InFlightFences[m_CurrentFrame]);

    m_PresentQueue.Present(&m_RenderFinishedSemaphores[m_CurrentFrame], m_Swapchain.GetHandleAddr(), m_CurrentImage);

    m_CurrentFrame = (m_CurrentFrame + 1) % m_Swapchain.GetNumImages();
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
        m_GraphicsCommandBuffers[m_CurrentImage],
        m_Swapchain.GetImage(m_CurrentImage),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        &m_ClearColor.color,
        1,
        &range
    );
}

void VulkanCore::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    BeginCommandBuffer(m_CopyCommandBuffer);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(m_CopyCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(m_CopyCommandBuffer);

    m_TransferQueue.SubmitSync(&m_CopyCommandBuffer);
    m_TransferQueue.WaitIdle();
}

void VulkanCore::CreateFramebuffers()
{
    m_SwapchainFramebuffers.resize(m_Swapchain.GetNumImages());

    VkExtent2D swapchainExtent = m_Swapchain.GetExtent();
        
    m_Viewport.width = static_cast<float>(swapchainExtent.width);
    m_Viewport.height = static_cast<float>(swapchainExtent.height);
    m_Scissor.extent.width = swapchainExtent.width;
    m_Scissor.extent.height = swapchainExtent.height;

    for (size_t i = 0; i < m_SwapchainFramebuffers.size(); i++)
    {
        CreateFramebuffer(m_Device.GetHandle(), m_Swapchain.GetImageView(i), 
            swapchainExtent, m_RenderPass, m_SwapchainFramebuffers[i]);
    }
}

void VulkanCore::CreateCommandBuffersAndCommandPools()
{
    CreateCommandPool(m_Device.GetHandle(), m_Device.GetPhysicalDevice().QueueFamilyIndices.TransferFamily.value(),
        m_CopyCommandPool);

    CreateCommandBuffers(m_Device.GetHandle(), m_CopyCommandPool, 
        1, &m_CopyCommandBuffer);

    CreateCommandPool(m_Device.GetHandle(), m_Device.GetPhysicalDevice().QueueFamilyIndices.GraphicsFamily.value(),
        m_GraphicsCommandPool);

    m_GraphicsCommandBuffers.resize(m_Swapchain.GetNumImages());

    CreateCommandBuffers(m_Device.GetHandle(), m_GraphicsCommandPool, 
        static_cast<uint32_t>(m_GraphicsCommandBuffers.size()), m_GraphicsCommandBuffers.data());
}

void VulkanCore::CreateSyncObjects()
{
    uint32_t numImages = m_Swapchain.GetNumImages();

    m_ImageAvailableSemaphores.resize(numImages);
    m_RenderFinishedSemaphores.resize(numImages);
    m_InFlightFences.resize(numImages);

    for (int i = 0; i < m_Swapchain.GetNumImages(); i++)
    {
        CreateSemaphore(m_Device.GetHandle(), m_ImageAvailableSemaphores[i]);
        CreateSemaphore(m_Device.GetHandle(), m_RenderFinishedSemaphores[i]);
        CreateFence(m_Device.GetHandle(), m_InFlightFences[i]);
    }
}

void VulkanCore::CreatePipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    VkDescriptorSetLayout layouts[] = { m_DescriptorSetLayout.GetHandle() };
    pipelineLayoutInfo.pSetLayouts = layouts;

    A3D_CHECK_VKRESULT(vkCreatePipelineLayout(m_Device.GetHandle(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout));
}

void VulkanCore::CreateDescriptors()
{
    uint32_t numImages = m_Swapchain.GetNumImages();

    m_DescriptorSetLayout.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    m_DescriptorSetLayout.Init();

    m_DescriptorPool.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, numImages);
    m_DescriptorPool.SetMaxSets(numImages);
    m_DescriptorPool.Init();

    m_DescriptorSets.resize(numImages);

    VulkanDescriptorWriter writter;
    writter.Init(&m_DescriptorSetLayout, &m_DescriptorPool);
    
    for (auto& descriptorSet : m_DescriptorSets)
    {
        writter.Build(descriptorSet); 
    }
}

} // namespace aero3d