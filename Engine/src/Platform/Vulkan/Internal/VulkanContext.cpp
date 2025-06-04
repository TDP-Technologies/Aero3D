#include "Platform/Vulkan/Internal/VulkanContext.h"

#include <set>

#include <SDL3/SDL_vulkan.h>

#include "Utils/Log.h"
#include "Platform/Vulkan/Internal/VulkanUtils.h"

namespace aero3d {

void VulkanContext::Init(SDL_Window* window)
{
    Window = window;

    if (!SDL_Vulkan_LoadLibrary(nullptr)) 
    {
        LogErr(ERROR_INFO, "Failed to load Vulkan library: %s", SDL_GetError());
    }

    CreateInstance();
    CreateSurface();
    CreatePhysDevice();
    CreateDevice();
}

void VulkanContext::Shutdown()
{
    vkDeviceWaitIdle(Device);

    vkDestroyDevice(Device, nullptr);
    vkDestroySurfaceKHR(Instance, Surface, nullptr);
    vkDestroyInstance(Instance, nullptr);
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

    A3D_CHECK_VKRESULT(vkCreateInstance(&createInfo, nullptr, &Instance));
}

void VulkanContext::CreateSurface()
{
    if (!SDL_Vulkan_CreateSurface(Window, Instance, nullptr, &Surface)) 
    {
        LogErr(ERROR_INFO, "Failed to create Vulkan surface: %s", SDL_GetError());
    }
}

void VulkanContext::CreatePhysDevice()
{
    uint32_t physDeviceCount;
    A3D_CHECK_VKRESULT(vkEnumeratePhysicalDevices(Instance, &physDeviceCount, nullptr));

    std::vector<VkPhysicalDevice> vkDevices(physDeviceCount);

    A3D_CHECK_VKRESULT(vkEnumeratePhysicalDevices(Instance, &physDeviceCount, vkDevices.data()));

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
            vkGetPhysicalDeviceSurfaceSupportKHR(vkDevice, i, Surface, &presentSupport);

            if (presentSupport)
            {
                physDevInfo.QueueFamilyIndices.PresentFamily = i;
            }
        }

        if (physDevInfo.QueueFamilyIndices.IsComplete())
        {
            PhysDeviceInfo = physDevInfo;
            PhysDevice = vkDevice;
            break;
        }
    }
}

void VulkanContext::CreateDevice()
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = 
    {
        PhysDeviceInfo.QueueFamilyIndices.TransferFamily.value(),
        PhysDeviceInfo.QueueFamilyIndices.GraphicsFamily.value(), 
        PhysDeviceInfo.QueueFamilyIndices.PresentFamily.value()
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

    A3D_CHECK_VKRESULT(vkCreateDevice(PhysDevice, &createInfo, nullptr, &Device));
}

VulkanContext g_VulkanContext = {};

} // namespace aero3d