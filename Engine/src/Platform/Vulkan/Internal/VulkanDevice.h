#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANDEVICE_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANDEVICE_H_

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

namespace aero3d {

struct DeviceQueueFamilyIndices
{
    std::optional<uint32_t> GraphicsFamily;
    std::optional<uint32_t> PresentFamily;

    bool IsComplete() {
        return GraphicsFamily.has_value() && PresentFamily.has_value();
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
    VulkanDevice();
    ~VulkanDevice();

    bool Init(VkInstance instance, VkSurfaceKHR surface);
    void Shutdown();

    const VulkanPhysicalDevice& GetPhysicalDevice() { return m_PhysDevices[m_CurrentPhysDevice]; }
    unsigned int GetCurrentPhysDeviceIndex() { return m_CurrentPhysDevice; }

    VkDevice GetDevice() { return m_Device; }
    VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }
    VkQueue GetPresentQueue() { return m_PresentQueue; }

    void SetPhysicalDevice(unsigned int index) { m_CurrentPhysDevice = index; }
    
private:
    void SearchForPhysDevices(VkSurfaceKHR surface);
    void CreateLogicalDevice();

private:
    VkInstance m_Instance;

    unsigned int m_CurrentPhysDevice;
    std::vector<VulkanPhysicalDevice> m_PhysDevices;

    VkDevice m_Device;
    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANDEVICE_H_