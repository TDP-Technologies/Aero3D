#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANDEVICE_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANDEVICE_H_

#include <vulkan/vulkan.h>

namespace aero3d {

class VulkanDevice
{
public:
    VulkanDevice();
    ~VulkanDevice();

    bool Init();
    void Shutdown();
    
private:
    VkDevice m_Device;
    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANDEVICE_H_