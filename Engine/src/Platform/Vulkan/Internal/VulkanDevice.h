#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANDEVICE_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANDEVICE_H_

#include <optional>
#include <vector>
#include <memory>

#include <vulkan/vulkan.h>

#include "Utils/Common.h"
#include "Platform/Vulkan/Internal/VulkanQueue.h"

namespace aero3d {

struct DeviceQueueFamilyIndices
{
    std::optional<uint32_t> GraphicsFamily;
    std::optional<uint32_t> PresentFamily;
    std::optional<uint32_t> TransferFamily;

    bool IsComplete() {
        return GraphicsFamily.has_value() && PresentFamily.has_value() && TransferFamily.has_value();
    }
};

struct VulkanPhysicalDevice
{
    VkPhysicalDevice Device;
    VkPhysicalDeviceProperties Properties;
    VkPhysicalDeviceFeatures Features;
    VkPhysicalDeviceMemoryProperties MemoryProperties;
    std::vector<VkQueueFamilyProperties> QueueFamilyProperties;
    DeviceQueueFamilyIndices QueueFamilyIndices;
};

class VulkanDevice
{
public:
    VulkanDevice() = default;
    ~VulkanDevice() = default;

    void Init(VkInstance instance, VkSurfaceKHR surface);
    void Shutdown();

    const VulkanPhysicalDevice& GetPhysicalDevice() const { return m_PhysDevices[m_CurrentPhysDevice]; }
    unsigned int GetCurrentPhysDeviceIndex() { return m_CurrentPhysDevice; }

    VkDevice GetHandle() const { return m_Device; }
    
    void SetPhysicalDevice(unsigned int index) { m_CurrentPhysDevice = index; }
    
private:
    void SearchForPhysDevices(VkSurfaceKHR surface);
    void CreateLogicalDevice();

private:
    VkInstance m_Instance =VK_NULL_HANDLE;

    unsigned int m_CurrentPhysDevice = 0;
    std::vector<VulkanPhysicalDevice> m_PhysDevices {};

    VkDevice m_Device = VK_NULL_HANDLE;

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANDEVICE_H_