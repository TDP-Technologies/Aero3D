#include "Platform/Vulkan/Internal/VulkanDevice.h"

#include <set>

#include "Platform/Vulkan/Internal/VulkanUtils.h"
#include "Utils/Assert.h"
#include "Utils/Log.h"

namespace aero3d {

VulkanDevice::VulkanDevice()
{
}

VulkanDevice::~VulkanDevice()
{
}

bool VulkanDevice::Init(VkInstance instance, VkSurfaceKHR surface)
{
    m_Instance = instance;

    SearchForPhysDevices(surface);
    CreateLogicalDevice();

    return true;
}

void VulkanDevice::Shutdown()
{
    vkDestroyDevice(m_Device, nullptr);
}

void VulkanDevice::SearchForPhysDevices(VkSurfaceKHR surface)
{
    uint32_t physDeviceCount;
    A3D_CHECK_VKRESULT(vkEnumeratePhysicalDevices(m_Instance, &physDeviceCount, nullptr));

    if (!physDeviceCount)
    {
        LogErr(ERROR_INFO, "Failed to Enum PhysDevices.");
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
    std::set<uint32_t> uniqueQueueFamilies = {
        physDevice.QueueFamilyIndices.GraphicsFamily.value(), physDevice.QueueFamilyIndices.PresentFamily.value()
    };

    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
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

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    createInfo.enabledLayerCount = 0;

    A3D_CHECK_VKRESULT(vkCreateDevice(physDevice.Device, &createInfo, nullptr, &m_Device));
    if (m_Device == nullptr)
    {
        LogErr(ERROR_INFO, "Failed to create logical device.");
    }

    VkQueue queue = nullptr;
    uint32_t graphicsIndices = physDevice.QueueFamilyIndices.GraphicsFamily.value();
    uint32_t presentIndices = physDevice.QueueFamilyIndices.PresentFamily.value();

    vkGetDeviceQueue(m_Device, graphicsIndices, 0, &queue);
    m_GraphicsQueue = std::make_shared<VulkanQueue>(queue, graphicsIndices);

    vkGetDeviceQueue(m_Device, presentIndices, 0, &queue);
    m_PresentQueue = std::make_shared<VulkanQueue>(queue, presentIndices);
}

} // namespace aero3d