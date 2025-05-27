#include "Platform/Vulkan/VulkanBuffer.h"

#include "Platform/Vulkan/Internal/VulkanUtils.h"
#include "Platform/Vulkan/Internal/VulkanCore.h"

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
    m_Device = g_VulkanCore->GetDeviceHandle();

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

    PrepareStagingBuffer(m_Device, &stagingBuffer, g_VulkanCore->GetPhysDeviceHandle(), 
        &stagingMemory, data, size);

    CreateBuffer(m_Device, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, size, &m_Buffer,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, g_VulkanCore->GetPhysDeviceHandle(), &m_Memory);
    
    g_VulkanCore->CopyBuffer(stagingBuffer, m_Buffer, size);

    vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
    vkFreeMemory(m_Device, stagingMemory, nullptr);
}

VulkanVertexBuffer::~VulkanVertexBuffer()
{
    vkDeviceWaitIdle(m_Device);

    vkDestroyBuffer(m_Device, m_Buffer, nullptr);
    vkFreeMemory(m_Device, m_Memory, nullptr);
}

void VulkanVertexBuffer::Bind()
{
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(g_VulkanCore->GetCommandBuffer(), 0, 1, &m_Buffer, offsets);
}

VulkanIndexBuffer::VulkanIndexBuffer(void* data, size_t size, size_t count)
{
    m_Count = count;
    m_Device = g_VulkanCore->GetDeviceHandle();

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

    PrepareStagingBuffer(m_Device, &stagingBuffer, g_VulkanCore->GetPhysDeviceHandle(), 
        &stagingMemory, data, size);

    CreateBuffer(m_Device, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, size, &m_Buffer,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, g_VulkanCore->GetPhysDeviceHandle(), &m_Memory);

    g_VulkanCore->CopyBuffer(stagingBuffer, m_Buffer, size);

    vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
    vkFreeMemory(m_Device, stagingMemory, nullptr);
}

VulkanIndexBuffer::~VulkanIndexBuffer()
{
    vkDeviceWaitIdle(m_Device);

    vkDestroyBuffer(m_Device, m_Buffer, nullptr);
    vkFreeMemory(m_Device, m_Memory, nullptr);
}

void VulkanIndexBuffer::Bind()
{
    vkCmdBindIndexBuffer(g_VulkanCore->GetCommandBuffer(), m_Buffer, 0, GetVkIndexType(m_Type));
}

} // namespace aero3d