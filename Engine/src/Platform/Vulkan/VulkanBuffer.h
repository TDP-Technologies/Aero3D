#ifndef AERO3D_PLATFORM_VULKAN_VULKANBUFFER_H_
#define AERO3D_PLATFORM_VULKAN_VULKANBUFFER_H_

#include <vulkan/vulkan.h>

#include "Graphics/Buffer.h"

namespace aero3d {

class VulkanVertexBuffer : public VertexBuffer
{
public:
    VulkanVertexBuffer(void* data, size_t size);
    ~VulkanVertexBuffer();

    virtual void Bind() override;

    virtual void SetData(const void* data, size_t size) override;

private:
    VkDevice m_Device = VK_NULL_HANDLE;
    
    VkBuffer m_Buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_Memory = VK_NULL_HANDLE;

};

class VulkanIndexBuffer : public IndexBuffer
{
public:
    VulkanIndexBuffer(void* data, size_t size, size_t count);
    ~VulkanIndexBuffer();

    virtual void Bind() override;

private:
    VkDevice m_Device = VK_NULL_HANDLE;

    VkBuffer m_Buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_Memory = VK_NULL_HANDLE;

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_VULKANBUFFER_H_