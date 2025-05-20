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
    VkDevice m_Device;
    
    VkBuffer m_Buffer;
    VkDeviceMemory m_Memory;

};

class VulkanIndexBuffer : public IndexBuffer
{
public:
    VulkanIndexBuffer(void* data, size_t size, size_t count);
    ~VulkanIndexBuffer();

    virtual void Bind() override;

private:
    VkDevice m_Device;

    VkBuffer m_Buffer;
    VkDeviceMemory m_Memory;

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_VULKANBUFFER_H_