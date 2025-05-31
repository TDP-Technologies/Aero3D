#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANBUFFER_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANBUFFER_H_

#include <vulkan/vulkan.h>

namespace aero3d {

class VulkanBuffer 
{
public:
    VulkanBuffer() = default;
    ~VulkanBuffer() = default;

    VulkanBuffer(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;

    void Init(VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment = 1);
    void Shutdown();

    VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void Unmap();

    void WriteToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkDescriptorBufferInfo DescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    void WriteToIndex(void* data, int index);
    VkResult FlushIndex(int index);
    VkDescriptorBufferInfo DescriptorInfoForIndex(int index);
    VkResult InvalidateIndex(int index);

    VkBuffer GetBuffer() const { return m_Buffer; }
    void* GetMappedMemory() const { return m_Mapped; }
    uint32_t GetInstanceCount() const { return m_InstanceCount; }
    VkDeviceSize GetInstanceSize() const { return m_InstanceSize; }
    VkDeviceSize GetAlignmentSize() const { return m_InstanceSize; }
    VkBufferUsageFlags GetUsageFlags() const { return m_UsageFlags; }
    VkMemoryPropertyFlags GetMemoryPropertyFlags() const { return m_MemoryPropertyFlags; }
    VkDeviceSize GetBufferSize() const { return m_BufferSize; }

private:
    VkDevice m_Device = VK_NULL_HANDLE;
    void* m_Mapped = nullptr;
    VkBuffer m_Buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_Memory = VK_NULL_HANDLE;

    VkDeviceSize m_BufferSize;
    uint32_t m_InstanceCount;
    VkDeviceSize m_InstanceSize;
    VkDeviceSize m_AlignmentSize;
    VkBufferUsageFlags m_UsageFlags;
    VkMemoryPropertyFlags m_MemoryPropertyFlags;

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANBUFFER_H_