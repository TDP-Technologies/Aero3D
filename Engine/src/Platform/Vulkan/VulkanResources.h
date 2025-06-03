#ifndef AERO3D_PLATFORM_VULKAN_VULKANRESOURCES_H_
#define AERO3D_PLATFORM_VULKAN_VULKANRESOURCES_H_

#include <vulkan/vulkan.h>

#include "Graphics/Resources.h"

#include "Platform/Vulkan/Internal/VulkanBuffer.h"

namespace aero3d {

class VulkanVertexBuffer : public VertexBuffer
{
public:
    VulkanVertexBuffer(void* data, size_t size);
    ~VulkanVertexBuffer();

    virtual void Bind() override;

private:
    VkDevice m_Device = VK_NULL_HANDLE;

    VulkanBuffer m_Buffer {};

};

class VulkanIndexBuffer : public IndexBuffer
{
public:
    VulkanIndexBuffer(void* data, size_t size, size_t count);
    ~VulkanIndexBuffer();

    virtual void Bind() override;

private:
    VkDevice m_Device = VK_NULL_HANDLE;

    VulkanBuffer m_Buffer {};

};

class VulkanConstantBuffer : public ConstantBuffer
{
public:
    VulkanConstantBuffer(size_t size);
    ~VulkanConstantBuffer();

    virtual void Bind(size_t slot) override;

    virtual void WriteData(void* data) override;

private:
    VkDevice m_Device = VK_NULL_HANDLE;

    VulkanBuffer m_Buffer {};

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_VULKANRESOURCES_H_