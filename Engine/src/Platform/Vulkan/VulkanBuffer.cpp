#include "Platform/Vulkan/VulkanBuffer.h"

#include "Platform/Vulkan/Internal/VulkanUtils.h"
#include "Platform/Vulkan/Internal/VulkanCore.h"

namespace aero3d {

static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    return 0;
}

VulkanVertexBuffer::VulkanVertexBuffer(void* data, size_t size)
    : m_Buffer(VK_NULL_HANDLE)
{
    m_Device = g_VulkanCore->GetDevice().GetDevice();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    A3D_CHECK_VKRESULT(vkCreateBuffer(m_Device, &bufferInfo, nullptr, &m_Buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_Device, m_Buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(
        memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        g_VulkanCore->GetDevice().GetPhysicalDevice().Device
    );

    A3D_CHECK_VKRESULT(vkAllocateMemory(m_Device, &allocInfo, nullptr, &m_Memory));

    A3D_CHECK_VKRESULT(vkBindBufferMemory(m_Device, m_Buffer, m_Memory, 0));

    void* dst;
    A3D_CHECK_VKRESULT(vkMapMemory(m_Device, m_Memory, 0, bufferInfo.size, 0, &dst));
    memcpy(dst, data, (size_t)bufferInfo.size);
    vkUnmapMemory(m_Device, m_Memory);
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

void VulkanVertexBuffer::SetData(const void* data, size_t size)
{
    
}

} // namespace aero3d