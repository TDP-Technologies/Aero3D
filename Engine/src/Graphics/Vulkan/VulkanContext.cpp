#include "Graphics/Vulkan/VulkanContext.h"

#include <vector>
#include <set>

#include <SDL3/SDL_vulkan.h>

#include "Utils/Log.h"

namespace aero3d {

VulkanContext::VulkanContext(SDL_Window* window)
    : m_Window(window)
{
    if (!SDL_Vulkan_LoadLibrary(nullptr)) 
    {
        LogErr(ERROR_INFO, "Failed to load Vulkan library: %s", SDL_GetError());
    }

    CreateInstance();
    CreateSurface();
    CreatePhysDevice();
    CreateDevice();
    CreateCommandPool();
    CreateSemaphores();
}

VulkanContext::~VulkanContext()
{
    vkDeviceWaitIdle(m_Device);

    vkDestroySemaphore(m_Device, m_ImageAvailableSemaphore, nullptr);
    vkDestroySemaphore(m_Device, m_RenderFinishedSemaphore, nullptr);
    vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
    vkDestroyDevice(m_Device, nullptr);
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}

void VulkanContext::CreateInstance()
{
    uint32_t extensionCount;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    if (!extensions) 
    {
        LogErr(ERROR_INFO, "Could not get Vulkan instance extensions: %s", SDL_GetError());
    }

    const char* validationLayers[] = 
    {
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

    vkCreateInstance(&createInfo, nullptr, &m_Instance);
}

void VulkanContext::CreateSurface()
{
    if (!SDL_Vulkan_CreateSurface(m_Window, m_Instance, nullptr, &m_Surface)) 
    {
        LogErr(ERROR_INFO, "Failed to create Vulkan surface: %s", SDL_GetError());
    }
}

void VulkanContext::CreatePhysDevice()
{
    uint32_t physDeviceCount;
    vkEnumeratePhysicalDevices(m_Instance, &physDeviceCount, nullptr);

    std::vector<VkPhysicalDevice> vkDevices(physDeviceCount);

    vkEnumeratePhysicalDevices(m_Instance, &physDeviceCount, vkDevices.data());

    for (auto& vkDevice : vkDevices) 
    {
        VulkanPhysicalDeviceInfo physDevInfo{};
        physDevInfo.Device = vkDevice;

        vkGetPhysicalDeviceProperties(vkDevice, &physDevInfo.Properties);
        vkGetPhysicalDeviceFeatures(vkDevice, &physDevInfo.Features);
        vkGetPhysicalDeviceMemoryProperties(vkDevice, &physDevInfo.MemoryProperties);

        uint32_t queueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vkDevice, &queueCount, nullptr);

        physDevInfo.QueueFamilyProperties.resize(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(vkDevice, &queueCount, physDevInfo.QueueFamilyProperties.data());

        for (uint32_t i = 0; i < physDevInfo.QueueFamilyProperties.size(); i++)
        {
            const auto& queueFamily = physDevInfo.QueueFamilyProperties[i];

            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                physDevInfo.QueueFamilyIndices.GraphicsFamily = i;
            }

            if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                physDevInfo.QueueFamilyIndices.TransferFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(vkDevice, i, m_Surface, &presentSupport);

            if (presentSupport)
            {
                physDevInfo.QueueFamilyIndices.PresentFamily = i;
            }
        }

        if (physDevInfo.QueueFamilyIndices.IsComplete())
        {
            m_PhysDeviceInfo = physDevInfo;
            m_PhysDevice = vkDevice;
            break;
        }
    }
}

void VulkanContext::CreateDevice()
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = 
    {
        m_PhysDeviceInfo.QueueFamilyIndices.TransferFamily.value(),
        m_PhysDeviceInfo.QueueFamilyIndices.GraphicsFamily.value(), 
        m_PhysDeviceInfo.QueueFamilyIndices.PresentFamily.value()
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

    vkCreateDevice(m_PhysDevice, &createInfo, nullptr, &m_Device);
}

void VulkanContext::CreateCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_PhysDeviceInfo.QueueFamilyIndices.GraphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool);
}

void VulkanContext::CreateSemaphores()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore);
    vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore);
}

} // namespace aero3d