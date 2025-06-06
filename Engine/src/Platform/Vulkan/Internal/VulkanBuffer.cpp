#include "Platform/Vulkan/Internal/VulkanBuffer.h"

#include "Platform/Vulkan/Internal/VulkanUtils.h"
#include "Platform/Vulkan/Internal/VulkanContext.h"

namespace aero3d {

static VkDeviceSize GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) 
{
    if (minOffsetAlignment > 0) 
    {
        return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }
    return instanceSize;
}

void VulkanBuffer::Init(VkDeviceSize instanceSize, uint32_t instanceCount,
    VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,VkDeviceSize minOffsetAlignment)
{
    m_Device = VulkanContext::Device;
    m_InstanceSize = instanceSize;
    m_InstanceCount = instanceCount;
    m_UsageFlags = usageFlags;
    m_MemoryPropertyFlags = memoryPropertyFlags;

    m_AlignmentSize = GetAlignment(instanceSize, minOffsetAlignment);
    m_BufferSize = m_AlignmentSize * instanceCount;
    CreateBuffer(m_Device, usageFlags, m_BufferSize, m_Buffer, memoryPropertyFlags,
    VulkanContext::PhysDevice, m_Memory);
}

void VulkanBuffer::Shutdown()
{
    Unmap();
    vkDestroyBuffer(m_Device, m_Buffer, nullptr);
    vkFreeMemory(m_Device, m_Memory, nullptr);
}

VkResult VulkanBuffer::Map(VkDeviceSize size, VkDeviceSize offset) 
{
    return vkMapMemory(m_Device, m_Memory, offset, size, 0, &m_Mapped);
}

void VulkanBuffer::Unmap()
{
    if (m_Mapped) 
    {
        vkUnmapMemory(m_Device, m_Memory);
        m_Mapped = nullptr;
    }
}

void VulkanBuffer::WriteToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset) 
{ 
    if (size == VK_WHOLE_SIZE) 
    {
        memcpy(m_Mapped, data, m_BufferSize);
    } 
    else 
    {
        char *memOffset = (char *)m_Mapped;
        memOffset += offset;
        memcpy(memOffset, data, size);
    }
}

VkResult VulkanBuffer::Flush(VkDeviceSize size, VkDeviceSize offset) 
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = m_Memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(m_Device, 1, &mappedRange);
}

VkResult VulkanBuffer::Invalidate(VkDeviceSize size, VkDeviceSize offset) 
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = m_Memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkInvalidateMappedMemoryRanges(m_Device, 1, &mappedRange);
}

VkDescriptorBufferInfo VulkanBuffer::DescriptorInfo(VkDeviceSize size, VkDeviceSize offset) 
{
    return VkDescriptorBufferInfo { m_Buffer, offset, size };
}

void VulkanBuffer::WriteToIndex(void *data, int index) 
{
    WriteToBuffer(data, m_InstanceSize, index * m_AlignmentSize);
}

VkResult VulkanBuffer::FlushIndex(int index) 
{
    return Flush(m_AlignmentSize, index * m_AlignmentSize); 
}

VkDescriptorBufferInfo VulkanBuffer::DescriptorInfoForIndex(int index) 
{
    return DescriptorInfo(m_AlignmentSize, index * m_AlignmentSize);
}
 
VkResult VulkanBuffer::InvalidateIndex(int index) 
{
    return Invalidate(m_AlignmentSize, index * m_AlignmentSize);
}

} // namespace aero3d