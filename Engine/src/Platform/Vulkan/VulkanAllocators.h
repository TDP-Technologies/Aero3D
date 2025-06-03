#ifndef AERO3D_PLATFORM_VULKAN_VULKANALLOCATORS_H_
#define AERO3D_PLATFORM_VULKAN_VULKANALLOCATORS_H_

#include <vector>
#include <list>
#include <mutex>
#include <stdexcept>
#include <algorithm>

#include <vulkan/vulkan.h>

#include "Graphics/Allocators.h"

namespace aero3d {

class VulkanConstantBufferAllocator : public ConstantBufferAllocator
{
public:
    VulkanConstantBufferAllocator(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize initialSize = 1024 * 1024);
    ~VulkanConstantBufferAllocator();

    virtual ConstantBuffer Allocate(size_t size) override;
    virtual void Free(ConstantBuffer buffer) override;

    virtual void NextFrame(size_t index) override;

private:
    struct FreeRequest 
    {
        size_t Offset = 0;
        size_t Size = 0;
    };

    struct AllocationRequest 
    {
        size_t Size = 0;
        size_t Alignment = 0;
        ConstantBuffer* Out = nullptr;
    };

    struct FrameBuffer
    {
        VkBuffer Buffer = VK_NULL_HANDLE;
        VkDeviceMemory Memory = VK_NULL_HANDLE;
        void* Mapped = nullptr;
        VkDeviceSize Capacity = 0;
        VkDeviceSize Offset = 0;

        std::list<FreeRequest> FreeList {};
        std::list<FreeRequest> PendingFrees {};

        std::vector<AllocationRequest> PendingAllocations {};
    };

    VkDevice m_Device = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDeviceSize m_DefaultSize = 0;
    uint32_t m_CurrentFrameIndex = 0;
    std::vector<FrameBuffer> m_Buffers {};

    void createBuffer(FrameBuffer& fb, VkDeviceSize size);
    void destroyBuffer(FrameBuffer& fb);
    void resizeCurrentBuffer(VkDeviceSize newSize);
    uint32_t findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags props);
    static VkDeviceSize alignUp(VkDeviceSize value, VkDeviceSize alignment);

    ConstantBuffer allocate(FrameBuffer& fb, size_t size, size_t alignment);

};

} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_VULKANALLOCATORS_H_