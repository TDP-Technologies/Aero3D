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
    m_Device = g_VulkanCore->GetDevice()->GetHandle();

    CreateBuffer(m_Device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, &m_StagingBuffer);

    AllocateBufferMemory(m_Device, m_StagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        g_VulkanCore->GetDevice()->GetPhysicalDevice().Device, &m_StagingMemory);

    A3D_CHECK_VKRESULT(vkBindBufferMemory(m_Device, m_StagingBuffer, m_StagingMemory, 0));

    WriteBufferMemory(m_Device, m_StagingMemory, data, size);

    CreateBuffer(m_Device, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, size, &m_Buffer);

    AllocateBufferMemory(m_Device, m_Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        g_VulkanCore->GetDevice()->GetPhysicalDevice().Device, &m_Memory);

    A3D_CHECK_VKRESULT(vkBindBufferMemory(m_Device, m_Buffer, m_Memory, 0));

    g_VulkanCore->CopyBuffer(m_StagingBuffer, m_Buffer, size);

    vkDestroyBuffer(m_Device, m_StagingBuffer, nullptr);
    vkFreeMemory(m_Device, m_StagingMemory, nullptr);
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

void VulkanVertexBuffer::SetData(void* data, size_t size)
{
    WriteBufferMemory(m_Device, m_StagingMemory, data, size);
}

VulkanIndexBuffer::VulkanIndexBuffer(void* data, size_t size, size_t count)
{
    m_Count = count;
    m_Device = g_VulkanCore->GetDevice()->GetHandle();

    CreateBuffer(m_Device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, &m_StagingBuffer);

    AllocateBufferMemory(m_Device, m_StagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        g_VulkanCore->GetDevice()->GetPhysicalDevice().Device, &m_StagingMemory);

    A3D_CHECK_VKRESULT(vkBindBufferMemory(m_Device, m_StagingBuffer, m_StagingMemory, 0));

    WriteBufferMemory(m_Device, m_StagingMemory, data, size);

    CreateBuffer(m_Device, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, size, &m_Buffer);

    AllocateBufferMemory(m_Device, m_Buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        g_VulkanCore->GetDevice()->GetPhysicalDevice().Device, &m_Memory);

    A3D_CHECK_VKRESULT(vkBindBufferMemory(m_Device, m_Buffer, m_Memory, 0));

    g_VulkanCore->CopyBuffer(m_StagingBuffer, m_Buffer, size);

    vkDestroyBuffer(m_Device, m_StagingBuffer, nullptr);
    vkFreeMemory(m_Device, m_StagingMemory, nullptr);
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