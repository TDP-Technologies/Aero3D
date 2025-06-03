#include "Platform/Vulkan/VulkanAllocators.h"

namespace aero3d {

VkDeviceSize VulkanConstantBufferAllocator::alignUp(VkDeviceSize value, VkDeviceSize alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

uint32_t VulkanConstantBufferAllocator::findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags props)
{
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProps);

    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((typeBits & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & props) == props) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type");
}

void VulkanConstantBufferAllocator::createBuffer(FrameBuffer& fb, VkDeviceSize size)
{
    fb.Capacity = size;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_Device, &bufferInfo, nullptr, &fb.Buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_Device, fb.Buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &fb.Memory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory");
    }

    vkBindBufferMemory(m_Device, fb.Buffer, fb.Memory, 0);

    if (vkMapMemory(m_Device, fb.Memory, 0, size, 0, &fb.Mapped) != VK_SUCCESS) {
        throw std::runtime_error("Failed to map buffer memory");
    }

    fb.Offset = 0;
    fb.FreeList.clear();
    fb.PendingFrees.clear();
}

void VulkanConstantBufferAllocator::destroyBuffer(FrameBuffer& fb)
{
    if (fb.Mapped) {
        vkUnmapMemory(m_Device, fb.Memory);
        fb.Mapped = nullptr;
    }
    if (fb.Buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_Device, fb.Buffer, nullptr);
        fb.Buffer = VK_NULL_HANDLE;
    }
    if (fb.Memory != VK_NULL_HANDLE) {
        vkFreeMemory(m_Device, fb.Memory, nullptr);
        fb.Memory = VK_NULL_HANDLE;
    }
    fb.Capacity = 0;
    fb.Offset = 0;
    fb.FreeList.clear();
    fb.PendingFrees.clear();
}

VulkanConstantBufferAllocator::VulkanConstantBufferAllocator(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize initialSize)
    : m_Device(device), m_PhysicalDevice(physicalDevice), m_DefaultSize(initialSize), m_CurrentFrameIndex(0)
{
    m_Buffers.resize(3);
    for (auto& fb : m_Buffers) {
        createBuffer(fb, m_DefaultSize);
    }
}

VulkanConstantBufferAllocator::~VulkanConstantBufferAllocator()
{
    for (auto& fb : m_Buffers) {
        destroyBuffer(fb);
    }
}

void VulkanConstantBufferAllocator::resizeCurrentBuffer(VkDeviceSize newSize)
{
    FrameBuffer& fb = m_Buffers[m_CurrentFrameIndex];
    if (newSize <= fb.Capacity)
        return;

    destroyBuffer(fb);
    createBuffer(fb, newSize);
    fb.Offset = 0;
    fb.FreeList.clear();
    fb.PendingFrees.clear();
}

ConstantBuffer VulkanConstantBufferAllocator::allocate(FrameBuffer& fb, size_t size, size_t alignment)
{
    size = static_cast<size_t>(alignUp(size, alignment));

    for (auto it = fb.FreeList.begin(); it != fb.FreeList.end(); ++it) {
        if (it->Size >= size) {
            size_t offset = it->Offset;
            fb.FreeList.erase(it);
            return { size, offset };
        }
    }

    if (fb.Offset + size > fb.Capacity) {
        if (&fb == &m_Buffers[m_CurrentFrameIndex]) {
            resizeCurrentBuffer(std::max(fb.Capacity * 2, fb.Offset + size));
        } else {
            throw std::runtime_error("Buffer overflow on non-current frame buffer");
        }
        return allocate(fb, size, alignment);
    }

    size_t offset = static_cast<size_t>(alignUp(fb.Offset, alignment));
    fb.Offset = offset + size;

    return { size, offset };
}

ConstantBuffer VulkanConstantBufferAllocator::Allocate(size_t size)
{
    FrameBuffer& fb = m_Buffers[m_CurrentFrameIndex];
    ConstantBuffer result{};
    {
        fb.PendingAllocations.push_back({ size, 256, &result });
    }
    return result;
}

void VulkanConstantBufferAllocator::Free(ConstantBuffer buffer)
{
    FrameBuffer& fb = m_Buffers[m_CurrentFrameIndex];
    fb.PendingFrees.push_back({ buffer.Offset, buffer.Size });
}

void VulkanConstantBufferAllocator::NextFrame(size_t index)
{
    m_CurrentFrameIndex = static_cast<uint32_t>(index % m_Buffers.size());
    FrameBuffer& fb = m_Buffers[m_CurrentFrameIndex];

    for (const auto& req : fb.PendingFrees) {
        fb.FreeList.push_back(req);
    }
    fb.PendingFrees.clear();

    {
        for (auto& req : fb.PendingAllocations) {
            *(req.Out) = allocate(fb, req.Size, req.Alignment);
        }
        fb.PendingAllocations.clear();
    }

    fb.Offset = 0;
}

} // namespace aero3d