#include "Platform/Vulkan/Internal/VulkanDevice.h"

#include <set>

#include "Platform/Vulkan/Internal/VulkanUtils.h"
#include "Platform/Vulkan/Internal/VulkanCore.h"

namespace aero3d {

VulkanDevice::VulkanDevice()
    : m_Device(VK_NULL_HANDLE), m_GraphicsQueue(VK_NULL_HANDLE), m_PresentQueue(VK_NULL_HANDLE)
{
}

VulkanDevice::~VulkanDevice()
{
}

bool VulkanDevice::Init()
{
    VulkanPhysicalDevice physDevice = g_VulkanCore->GetPhysicalDevice();

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
        return false;
    }

    vkGetDeviceQueue(m_Device, physDevice.QueueFamilyIndices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, physDevice.QueueFamilyIndices.PresentFamily.value(), 0, &m_PresentQueue);

    return true;
}

void VulkanDevice::Shutdown()
{
    vkDestroyDevice(m_Device, nullptr);
}

} // namespace aero3d