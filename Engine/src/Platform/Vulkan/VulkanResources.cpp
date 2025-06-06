#include "Platform/Vulkan/VulkanResources.h"

#include "Platform/Vulkan/Internal/VulkanUtils.h"
#include "Platform/Vulkan/Internal/VulkanContext.h"

namespace aero3d {

static VkIndexType GetVkIndexType(IndexBufferType type)
{
    switch (type)
    {
    case IndexBufferType::UNSIGNED_SHORT: return VK_INDEX_TYPE_UINT16;
    case IndexBufferType::UNSIGNED_INT: return VK_INDEX_TYPE_UINT32;
    default: return VK_INDEX_TYPE_NONE_NV;
    }
}

VulkanVertexBuffer::VulkanVertexBuffer(void* data, size_t size)
{
    m_Device = VulkanContext::Device;

    m_Buffer.Init(size, 1, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    m_Buffer.Map();
    m_Buffer.WriteToBuffer(data);
    m_Buffer.Unmap();
}

VulkanVertexBuffer::~VulkanVertexBuffer()
{
    vkDeviceWaitIdle(m_Device);

    m_Buffer.Shutdown();
}

void VulkanVertexBuffer::Bind()
{
    VkBuffer buffer = m_Buffer.GetBuffer();
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(nullptr, 0, 1, &buffer, offsets);
}

VulkanIndexBuffer::VulkanIndexBuffer(void* data, size_t size, size_t count)
{
    m_Count = count;
    m_Device = VulkanContext::Device;

    m_Buffer.Init(size, 1, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    m_Buffer.Map();
    m_Buffer.WriteToBuffer(data);
    m_Buffer.Unmap();
}

VulkanIndexBuffer::~VulkanIndexBuffer()
{
    vkDeviceWaitIdle(m_Device);

    m_Buffer.Shutdown();
}

void VulkanIndexBuffer::Bind()
{
    vkCmdBindIndexBuffer(nullptr, m_Buffer.GetBuffer(), 0, GetVkIndexType(m_Type));
}

} // namespace aero3d