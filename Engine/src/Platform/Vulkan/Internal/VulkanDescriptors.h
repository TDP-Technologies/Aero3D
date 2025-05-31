#ifndef AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANDESCRIPTORS_H_
#define AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANDESCRIPTORS_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.h>

namespace aero3d {

class VulkanDescriptorSetLayout 
{
public:
    VulkanDescriptorSetLayout() = default;
    ~VulkanDescriptorSetLayout() = default;

    void AddBinding(uint32_t binding, VkDescriptorType type,
        VkShaderStageFlags stageFlags, uint32_t count = 1);

    void Init();
    void Shutdown();

    VkDescriptorSetLayout Get() const { return m_DescriptorSetLayout; }

private:
    VkDevice m_Device = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_Bindings {};

friend class VulkanDescriptorWriter;
};

class VulkanDescriptorPool 
{
public:
    VulkanDescriptorPool() = default;
    ~VulkanDescriptorPool() = default;

    void AddPoolSize(VkDescriptorType type, uint32_t count);
    void SetMaxSets(uint32_t count);
    void SetFlags(VkDescriptorPoolCreateFlags flags);

    void Init();
    void Shutdown();

    void Allocate(VkDescriptorSetLayout layout, VkDescriptorSet &set);
    void Free(std::vector<VkDescriptorSet> &sets);
    void Reset();

private:
    VkDevice m_Device = VK_NULL_HANDLE;
    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorPoolSize> m_PoolSizes {};
    uint32_t m_MaxSets = 1000;
    VkDescriptorPoolCreateFlags m_Flags = 0;

friend class VulkanDescriptorWriter;
};

class VulkanDescriptorWriter 
{
public:
    VulkanDescriptorWriter(VulkanDescriptorSetLayout &layout, VulkanDescriptorPool &pool);

    VulkanDescriptorWriter &WriteBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
    VulkanDescriptorWriter &WriteImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

    void Build(VkDescriptorSet &set);
    void Overwrite(VkDescriptorSet &set);

private:
    VulkanDescriptorSetLayout &m_SetLayout;
    VulkanDescriptorPool &m_Pool;
    std::vector<VkWriteDescriptorSet> m_Writes {};

};
 
} // namespace aero3d

#endif // AERO3D_PLATFORM_VULKAN_INTERNAL_VULKANDESCRIPTORS_H_