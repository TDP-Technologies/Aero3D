#include "Graphics/Vulkan/VulkanGraphicsDevice.h"

#include <set>

#include <SDL3/SDL_vulkan.h>

#include "Graphics/Vulkan/VulkanUtils.h"
#include "Graphics/Vulkan/VulkanCommandList.h"

namespace aero3d {

VulkanGraphicsDevice::VulkanGraphicsDevice(SDL_Window* sdl_window)
{
    LogMsg("Creating Vulkan Graphics Device...");

    window = sdl_window;

    volkInitialize();
    CreateInstance();
    CreateSurface();
    CreatePhysDevice();
    CreateDevice();
    CreateQueues();
    CreateCommandBuffers();
    CreateLocks();

    swapchain = new VulkanSwapchain(this);
    descriptorAllocator = new VulkanDescriptorAllocator(this);
    resourceFactory = new VulkanResourceFactory(this);
}

VulkanGraphicsDevice::~VulkanGraphicsDevice() 
{
    LogMsg("Shutdown Vulkan Graphics Device...");

    vkDeviceWaitIdle(device);

    if (resourceFactory != nullptr)
    {
        delete resourceFactory;
        resourceFactory = nullptr;
    }
    if (descriptorAllocator != nullptr)
    {
        delete descriptorAllocator;
        descriptorAllocator = nullptr;
    }
    if (swapchain != nullptr)
    {
        delete swapchain;
        swapchain = nullptr;
    }
    if (commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(device, commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
    }
    if (transferFinishedFence != VK_NULL_HANDLE)
    {
        vkDestroyFence(device, transferFinishedFence, nullptr);
        transferFinishedFence = VK_NULL_HANDLE;
    }
    if (renderFinishedFence != VK_NULL_HANDLE)
    {
        vkDestroyFence(device, renderFinishedFence, nullptr);
        renderFinishedFence = VK_NULL_HANDLE;
    }
    if (device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
    }
    if (surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
    }
    if (instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }
}

Ref<CommandList> VulkanGraphicsDevice::CreateCommandList() 
{
    return std::make_shared<VulkanCommandList>(this);
}

ResourceFactory* VulkanGraphicsDevice::GetResourceFactory() 
{
    return resourceFactory;
}

Swapchain* VulkanGraphicsDevice::GetSwapchain() 
{
    return swapchain;
}

void VulkanGraphicsDevice::SubmitCommands(Ref<CommandList> commandList) 
{
    Ref<VulkanCommandList> vcl = std::static_pointer_cast<VulkanCommandList>(commandList);

    VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vcl->commandBuffer;

	A3D_CHECK_VKRESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, renderFinishedFence));

    A3D_CHECK_VKRESULT(vkWaitForFences(device, 1, &renderFinishedFence, VK_TRUE, UINT64_MAX));
    A3D_CHECK_VKRESULT(vkResetFences(device, 1, &renderFinishedFence));
}

void VulkanGraphicsDevice::Present() 
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain->swapchain;
    presentInfo.pImageIndices = &swapchain->currentImageIndex;

    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        swapchain->Resize();
    } 
    else
    {
        A3D_CHECK_VKRESULT(result);
    }
}

void VulkanGraphicsDevice::UpdateBuffer(Ref<DeviceBuffer> buffer, void* data, size_t size, size_t offset)
{
    BufferDesc stagingBufferDescription;
    stagingBufferDescription.size = size;
    stagingBufferDescription.usage = USAGE_STAGING;

    Ref<VulkanDeviceBuffer> stagingBuffer = 
        std::static_pointer_cast<VulkanDeviceBuffer>(resourceFactory->CreateBuffer(stagingBufferDescription));

    void* mappedData;
    A3D_CHECK_VKRESULT(vkMapMemory(device, stagingBuffer->memory, 0, size, 0, &mappedData));
    memcpy(mappedData, data, (size_t)size);
    vkUnmapMemory(device, stagingBuffer->memory);

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    A3D_CHECK_VKRESULT(vkBeginCommandBuffer(transferCommandBuffer, &beginInfo));

    VkBufferCopy copyRegion;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(transferCommandBuffer, stagingBuffer->buffer,
        std::static_pointer_cast<VulkanDeviceBuffer>(buffer)->buffer, 1, &copyRegion);

    A3D_CHECK_VKRESULT(vkEndCommandBuffer(transferCommandBuffer));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &transferCommandBuffer;

    A3D_CHECK_VKRESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, transferFinishedFence));

    A3D_CHECK_VKRESULT(vkWaitForFences(device, 1, &transferFinishedFence, VK_TRUE, UINT64_MAX));
    A3D_CHECK_VKRESULT(vkResetFences(device, 1, &transferFinishedFence));
}

void VulkanGraphicsDevice::UpdateTexture(Ref<Texture> texture, void* data, size_t size)
{
    Ref<VulkanTexture> vulkanTexture = std::static_pointer_cast<VulkanTexture>(texture);

    BufferDesc stagingBufferDescription;
    stagingBufferDescription.size = size;
    stagingBufferDescription.usage = USAGE_STAGING;

    Ref<VulkanDeviceBuffer> stagingBuffer = 
        std::static_pointer_cast<VulkanDeviceBuffer>(resourceFactory->CreateBuffer(stagingBufferDescription));

    void* mappedData;
    A3D_CHECK_VKRESULT(vkMapMemory(device, stagingBuffer->memory, 0, size, 0, &mappedData));
    memcpy(mappedData, data, size);
    vkUnmapMemory(device, stagingBuffer->memory);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    A3D_CHECK_VKRESULT(vkBeginCommandBuffer(transferCommandBuffer, &beginInfo));

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = vulkanTexture->image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(
        transferCommandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr,
        1, &barrier
    );

    VkBufferImageCopy copyRegion{};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;
    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;
    copyRegion.imageOffset = { 0, 0, 0 };
    copyRegion.imageExtent = {
        vulkanTexture->width,
        vulkanTexture->height,
        1
    };

    vkCmdCopyBufferToImage(transferCommandBuffer, stagingBuffer->buffer, vulkanTexture->image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        transferCommandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr,
        1, &barrier
    );

    A3D_CHECK_VKRESULT(vkEndCommandBuffer(transferCommandBuffer));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &transferCommandBuffer;

    A3D_CHECK_VKRESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, transferFinishedFence));
    
    A3D_CHECK_VKRESULT(vkWaitForFences(device, 1, &transferFinishedFence, VK_TRUE, UINT64_MAX));
    A3D_CHECK_VKRESULT(vkResetFences(device, 1, &transferFinishedFence));
}

uint32_t VulkanGraphicsDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
    {
        if ((typeFilter & (1 << i)) && 
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
        {
            return i;
        }
    }

    return 0;
}

void VulkanGraphicsDevice::CreateInstance()
{
    Uint32 sdlExtensionCount = 0;
    const char* const* sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);
    if (!sdlExtensions) {
        LogErr(ERROR_INFO, "Could not get Vulkan instance extensions: %s", SDL_GetError());
    }

    std::vector<const char*> extensions(sdlExtensions, sdlExtensions + sdlExtensionCount);

    extensions.push_back("VK_EXT_debug_utils");

    std::vector<const char*> layers;
    
    layers.push_back("VK_LAYER_KHRONOS_validation");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Aero3D";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = layers.size();
    createInfo.ppEnabledLayerNames = layers.data();

    A3D_CHECK_VKRESULT(vkCreateInstance(&createInfo, nullptr, &instance));
    volkLoadInstance(instance);
}

void VulkanGraphicsDevice::CreateSurface()
{
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) 
    {
        LogErr(ERROR_INFO, "Failed to create Vulkan surface: %s", SDL_GetError());
    }
}

void VulkanGraphicsDevice::CreatePhysDevice()
{
    uint32_t physDeviceCount;
    A3D_CHECK_VKRESULT(vkEnumeratePhysicalDevices(instance, &physDeviceCount, nullptr));

    std::vector<VkPhysicalDevice> vkDevices(physDeviceCount);

    A3D_CHECK_VKRESULT(vkEnumeratePhysicalDevices(instance, &physDeviceCount, vkDevices.data()));

    for (auto& vkDevice : vkDevices) 
    {
        uint32_t graphicsIndex = UINT32_MAX;
        uint32_t presentIndex = UINT32_MAX;

        uint32_t queueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vkDevice, &queueCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueProps(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(vkDevice, &queueCount, queueProps.data());

        for (uint32_t i = 0; i < queueCount; ++i)
        {
            if (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphicsIndex = i;

            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(vkDevice, i, surface, &presentSupport);

            if (presentSupport)
                presentIndex = i;
        }

        if (graphicsIndex != UINT32_MAX && presentIndex != UINT32_MAX)
        {
            physDevice = vkDevice;
            graphicsQueueIndex = graphicsIndex;
            presentQueueIndex = presentIndex;

            vkGetPhysicalDeviceProperties(physDevice, &physDeviceProperties);
            vkGetPhysicalDeviceFeatures(physDevice, &physDeviceFeatures);
            vkGetPhysicalDeviceMemoryProperties(physDevice, &physDeviceMemoryProperties);

            break;
        }
    }
}

void VulkanGraphicsDevice::CreateDevice()
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = 
    {
        graphicsQueueIndex, presentQueueIndex
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

    VkPhysicalDeviceDynamicRenderingFeatures dynamicRendering = {};
    dynamicRendering.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    dynamicRendering.dynamicRendering = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.pNext = &dynamicRendering;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    std::vector<const char*> deviceExtensions;

    deviceExtensions.push_back("VK_KHR_swapchain");
    deviceExtensions.push_back("VK_KHR_dynamic_rendering");

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    createInfo.enabledLayerCount = 0;

    A3D_CHECK_VKRESULT(vkCreateDevice(physDevice, &createInfo, nullptr, &device));
    volkLoadDevice(device);
}

void VulkanGraphicsDevice::CreateQueues()
{
    vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentQueueIndex, 0, &presentQueue);
}

void VulkanGraphicsDevice::CreateCommandBuffers()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = graphicsQueueIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    A3D_CHECK_VKRESULT(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    A3D_CHECK_VKRESULT(vkAllocateCommandBuffers(device, &allocInfo, &transferCommandBuffer));
}

void VulkanGraphicsDevice::CreateLocks()
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    A3D_CHECK_VKRESULT(vkCreateFence(device, &fenceInfo, nullptr, &transferFinishedFence));
    A3D_CHECK_VKRESULT(vkResetFences(device, 1, &transferFinishedFence));

    A3D_CHECK_VKRESULT(vkCreateFence(device, &fenceInfo, nullptr, &renderFinishedFence));
    A3D_CHECK_VKRESULT(vkResetFences(device, 1, &renderFinishedFence));
}

} // namespace aero3d
