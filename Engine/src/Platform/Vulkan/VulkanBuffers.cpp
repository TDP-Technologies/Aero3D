#include "Platform/Vulkan/VulkanBuffers.h"

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
    vkCmdBindVertexBuffers(g_VulkanCore->GetCommandBuffer(), 0, 1, &buffer, offsets);
}

VulkanIndexBuffer::VulkanIndexBuffer(void* data, size_t size, size_t count)
{
    m_Count = count;
    m_Device = g_VulkanCore->GetDeviceHandle();

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
    vkCmdBindIndexBuffer(g_VulkanCore->GetCommandBuffer(), m_Buffer.GetBuffer(), 0, GetVkIndexType(m_Type));
}

VulkanConstantBuffer::VulkanConstantBuffer(size_t size)
{
    m_Device = g_VulkanCore->GetDeviceHandle();
    
    size_t numImages = g_VulkanCore->GetNumFrames();

    m_Buffer.Init(size, numImages, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

VulkanConstantBuffer::~VulkanConstantBuffer()
{
    vkDeviceWaitIdle(m_Device);

    m_Buffer.Shutdown();
}

void VulkanConstantBuffer::Bind(size_t slot)
{
    std::vector<VkDescriptorSet> descriptorSets = g_VulkanCore->GetDescriptorSets();
    VulkanDescriptorWriter* writter = g_VulkanCore->GetDescriptorWritter();
    for (int i = 0; i < descriptorSets.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_Buffer.GetBuffer();
        bufferInfo.offset = g_VulkanCore->GetCurrentFrame() * m_Buffer.GetAlignmentSize();
        bufferInfo.range  = m_Buffer.GetInstanceSize();

        writter->WriteBuffer(slot, &bufferInfo)
                .Overwrite(descriptorSets[i]);
    }
}

void VulkanConstantBuffer::WriteData(void* data)
{
    m_Buffer.Map();
    m_Buffer.WriteToIndex(data, g_VulkanCore->GetCurrentFrame());
    m_Buffer.Unmap();
}


} // namespace aero3d